/**
 *
 * This file is part of Porgy (http://tulip.labri.fr/TulipDrupal/?q=porgy)
 *
 * from LaBRI, University of Bordeaux, Inria and King's College London
 *
 * Porgy is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Porgy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this Porgy.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "traceswidget.h"
#include "NewTrace.h"
#include "qtimagetooltip.h"
#include "sortfilterproxymodel.h"
#include "statusbarpluginprogress.h"
#include "ui_traceswidget.h"

#include <QAbstractItemModel>
#include <QFileDialog>
#include <QHelpEvent>
#include <QMenu>
#include <QMessageBox>

#include <tulip/BooleanProperty.h>
#include <tulip/ConnectedTest.h>
#include <tulip/GlGraphRenderingParameters.h>
#include <tulip/StringProperty.h>
#include <tulip/TlpQtTools.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Trace.h>

#include <porgy/pluginparametersconfigurationdialog.h>

using namespace tlp;
using namespace std;

TracesWidget::TracesWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::TracesWidget), graph(nullptr), _model(nullptr) {
  ui->setupUi(this);
  connect(ui->traceTreeView, SIGNAL(customContextMenuRequested(const QPoint &)), this,
          SLOT(ShowTreeContextMenu(const QPoint &)));
  connect(ui->traceTreeView, SIGNAL(doubleClicked(QModelIndex)), this,
          SLOT(indexDoubleClicked(QModelIndex)));

  _proxyModel = new SortFilterProxyModel(this);
  _proxyModel->setDynamicSortFilter(true);

  _model = new TracesItemModel(this);
  GlGraphRenderingParameters parameters;
  parameters.setViewArrow(true);
  parameters.setEdgeColorInterpolate(false);
  parameters.setEdgeSizeInterpolate(true);
  parameters.setViewEdgeLabel(false);
  parameters.setViewNodeLabel(true);
  parameters.setViewMetaLabel(true);
  parameters.setLabelsDensity(0);
  parameters.setAntialiasing(true);
  parameters.setLabelScaled(true);
  parameters.setViewArrow(true);
  _model->snapshotManager().setRenderingParameters(parameters);
  _model->snapshotManager().setPreviewSize(QSize(32, 32));

  _proxyModel->setSourceModel(_model);
  ui->traceTreeView->setModel(_proxyModel);
  ui->traceTreeView->installEventFilter(this);
  connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(filterTraces(QString)));
}

TracesWidget::~TracesWidget() {
  delete ui;
  delete _model;
}

vector<Graph *> TracesWidget::selectedTraces() const {
  QModelIndexList indexes = ui->traceTreeView->selectionModel()->selectedRows();
  QModelIndexList sourcesIndexes;
  foreach (const QModelIndex &m, indexes) { sourcesIndexes.push_back(_proxyModel->mapToSource(m)); }
  return _model->indexesGraphs(sourcesIndexes);
}

void TracesWidget::addNewTrace() {
  Graph *newTrace =
      NewTrace::createNewTrace(graph->getProperty<BooleanProperty>("viewSelection"), graph, this);
  if (newTrace != nullptr) {
    update();
    emit showTrace(newTrace, false);
  }
}

void TracesWidget::computeOnTrace() {
  vector<Graph *> graphs = selectedTraces();
  if (!graphs.empty()) {
    Graph *newTrace = NewTrace::computeOnTrace(
        graphs.front()->getProperty<BooleanProperty>("viewSelection"), graphs.front(), this);
    if (newTrace != nullptr) {
      update();
      // emit showTrace(newTrace,true);
    }
  }
}

// triggered when deleting elements from a trace tree
void TracesWidget::treatEvent(const Event &message) {
  if (message.type() == tlp::Event::TLP_MODIFICATION) {
    const tlp::GraphEvent *gEvt = dynamic_cast<const tlp::GraphEvent *>(&message);
    if (gEvt) {
      if (gEvt->getType() == tlp::GraphEvent::TLP_DEL_EDGE) {
        edge e = gEvt->getEdge();
        Trace t(gEvt->getGraph());
        if (t.isSimpleTransformationEdge(e)) {
          Graph *g = t.getInstanceForTransformation(e);
          // delete LHS morphism
          if (g != nullptr) { // nullptr => deletion of a failure node
            Graph *s = g->getSuperGraph();
            s->delSubGraph(g);
          }
          // delete new nodes/edges in the target graph
          const pair<node, node> &ends = t.ends(e);
          if (t.isMetaNode(ends.second)) {
            Graph *g_target = t.getNodeMetaInfo(ends.second);
            BooleanProperty *newN = g_target->getProperty<BooleanProperty>(PorgyConstants::NEW);
            Iterator<edge> *ite = newN->getEdgesEqualTo(true);
            Iterator<node> *itn = newN->getNodesEqualTo(true);
            g_target->delEdges(ite, true);
            g_target->delNodes(itn, true);
            delete ite;
            delete itn;
          }

          // The associated P and Q properties should be removed if they are not
          // used in any other computation (could be the case with the usage of
          // all() or rule applied by hand)
          node n = t.source(e);
          g = t.getNodeMetaInfo(n);
          StringProperty *Pprop = t.getTransformationPPropertyNameProperty();
          StringProperty *Bprop = t.getTransformationBanPropertyNameProperty();
          string PpropName = Pprop->getEdgeValue(e);
          string BanPropName = Bprop->getEdgeValue(e);
          bool delban(true);
          bool delPprop(true);
          // Are properties used in another computation?
          if (PpropName.empty())
            delPprop = false;
          if (BanPropName.empty())
            delban = false;
          for(auto ed:t.getOutEdges(n)) {
            if (ed != e) {
              if (Pprop->getEdgeValue(ed) == PpropName)
                delPprop = false;
              if (Bprop->getEdgeValue(ed) == BanPropName)
                delban = false;
              if (delban == false && delPprop == false)
                break;
            }
          }

          if (delPprop) {
            g->delLocalProperty(PpropName);
          }
          if (delban) {
            g->delLocalProperty(BanPropName);
          }
        }
      }
    }
  }
}

void TracesWidget::setGraph(Graph *graph) {
  this->graph = graph;
  _model->setRootGraph(graph);
}

void TracesWidget::addListener() {
  Graph *g = PorgyTlpGraphStructure::getMainTrace(graph);
  g->addListener(this);
}

void TracesWidget::removeListener() {
  Graph *g = PorgyTlpGraphStructure::getMainTrace(graph);
  g->removeListener(this);
}

void TracesWidget::ShowTreeContextMenu(const QPoint &point) {
  QMenu menu(this);

  selectedGraph = _model->indexGraph(_proxyModel->mapToSource(ui->traceTreeView->indexAt(point)));
  BooleanProperty *selectionProperty = graph->getProperty<BooleanProperty>("viewSelection");
  Iterator<node> *nodes = selectionProperty->getNodesEqualTo(true, graph);
  bool hasNext = nodes->hasNext();
  delete nodes;
  bool selected(selectedGraph != nullptr);
  QAction *showTrace = menu.addAction(tr("&Show"), this, SLOT(showTrace()));
  showTrace->setEnabled(selected);

  QAction *createNewAction = menu.addAction(tr("&Create new"), this, SLOT(addNewTrace()));
  // If nodes are selected in the trace
  createNewAction->setEnabled(hasNext);

  QAction *deltrace = menu.addAction(tr("&Delete"), this, SLOT(deleteTrace()));
  // If there is a trace selected and if the selected trace is not the main
  // trace graph.
  deltrace->setEnabled(selected &&
                       PorgyTlpGraphStructure::getMainTrace(selectedGraph) != selectedGraph);

  QAction *updateMetricAction = menu.addAction(tr("&Update Metric"), this, SLOT(computeOnTrace()));
  // If nodes are selected in the trace
  updateMetricAction->setEnabled(hasNext);

  QAction *csv_export = menu.addAction("Export to a CSV file", this, SLOT(exportCSV()));
  csv_export->setEnabled(selected);

  menu.exec(ui->traceTreeView->viewport()->mapToGlobal(point));
}

void TracesWidget::exportCSV() {
  QString filesave(QFileDialog::getSaveFileName(this, "CSV filename to save", "", "csv (*.csv)"));
  if (!filesave.isEmpty()) {
    DataSet dataSet;
    ParameterDescriptionList params = PluginLister::getPluginParameters("CSV Export");
    params.buildDefaultDataSet(dataSet, selectedGraph);
    bool ok = true;
    dataSet = PluginParametersConfigurationDialog::getParameters(
        this, "CSV Export plugin parameters", params, ok, &dataSet, selectedGraph);
    if (ok) {
      ok = tlp::saveGraph(selectedGraph, QStringToTlpString(filesave), nullptr, &dataSet);
      if (ok)
        QMessageBox::information(this, "CSV export",
                                 "CSV Export of " + tlpStringToQString(selectedGraph->getName()) +
                                     " successfully done!");
      else
        QMessageBox::warning(this, "CSV export",
                             "Cannot use CSV Export on " +
                                 tlpStringToQString(selectedGraph->getName()) + ": " +
                                 tlpStringToQString(_pp->getError()));
    }
  }
}

void TracesWidget::deleteTrace() {
  vector<Graph *> graphs = selectedTraces();
  if (!graphs.empty()) {
    emit deleteTrace(*graphs.begin());
  }
}

void TracesWidget::showTrace() {
  vector<Graph *> graphs = selectedTraces();
  if (!graphs.empty()) {
    emit showTrace(*graphs.begin(), false);
  }
}

bool TracesWidget::eventFilter(QObject *obj, QEvent *evt) {
  switch (evt->type()) {
  case QEvent::ToolTip: {
    QHelpEvent *he = static_cast<QHelpEvent *>(evt);
    if (obj == ui->traceTreeView) {
      // Need to use viewport widget to compute correct coordinate
      // tranformation.
      // Return wrong QModelIndex if we use he->Pos();
      Graph *graph = nullptr;
      graph = _model->indexGraph(_proxyModel->mapToSource(ui->traceTreeView->indexAt(
          ui->traceTreeView->viewport()->mapFromGlobal(he->globalPos()))));
      if (graph) {
        // Check if the trace graph have been already drawn.
        // If not init it's layout.
        if (!graph->existLocalProperty("viewLayout")) {
          string errMsg;
          Trace trace(graph);
          trace.redraw(errMsg, nullptr);
        }
        QtImageToolTip::showGraph(he->globalPos(), graph, QSize(512, 512), Color(255, 255, 255),
                                  _model->snapshotManager().renderingParameters(), this, QRect());
        return true;
      }
    }
  } break;
  default:
    break;
  }

  return QWidget::eventFilter(obj, evt);
}

void TracesWidget::indexDoubleClicked(const QModelIndex &index) {
  Graph *graph = _model->indexGraph(_proxyModel->mapToSource(index));
  if (graph) {
    emit showTrace(graph, false);
  }
}

void TracesWidget::filterTraces(const QString &pattern) {
  _proxyModel->setFilterWildcard(pattern);
}
