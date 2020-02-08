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
#include "ruleswidget.h"
#include "ui_ruleswidget.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QHelpEvent>
#include <QItemSelectionModel>
#include <QMenu>
#include <QPushButton>
#include <QToolTip>

#include <tulip/BooleanProperty.h>
#include <tulip/Graph.h>
#include <tulip/TlpQtTools.h>

#include "rulesgraphitemmodel.h"
#include "sortfilterproxymodel.h"

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortNode.h>

using namespace tlp;
using namespace std;

RulesWidget::RulesWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::RulesWidget), _graph(nullptr), _model(nullptr) {
  _ui->setupUi(this);
  connect(_ui->ruleTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showRule(QModelIndex)));
  connect(_ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showRule(QModelIndex)));
  connect(_ui->ruleTreeView, SIGNAL(customContextMenuRequested(const QPoint &)), this,
          SLOT(ShowTreeContextMenu(const QPoint &)));
  connect(_ui->listView, SIGNAL(customContextMenuRequested(const QPoint &)), this,
          SLOT(ShowTreeContextMenu(const QPoint &)));

  connect(_ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateIconSize()));

  _ui->listView->installEventFilter(this);
  _ui->ruleTreeView->installEventFilter(this);

  _proxyModel = new SortFilterProxyModel(this);
  _proxyModel->setDynamicSortFilter(true);
  _ui->listView->setModel(_proxyModel);
  _ui->ruleTreeView->setModel(_proxyModel);

  connect(_ui->lineEdit, SIGNAL(textEdited(QString)), SLOT(filterRules(QString)));

  _model = new RulesGraphItemModel(this);
  GlGraphRenderingParameters parameters;
  parameters.setEdgeColorInterpolate(false);
  parameters.setLabelsDensity(0);
  parameters.setLabelScaled(true);
  _model->snapshotManager().setRenderingParameters(parameters);
  _model->snapshotManager().setPreviewSize(QSize(32, 32));
  _proxyModel->setSourceModel(_model);

  _selectionModel = new QItemSelectionModel(_proxyModel, this);
  _ui->listView->setSelectionModel(_selectionModel);
  _ui->ruleTreeView->setSelectionModel(_selectionModel);

#ifndef NDEBUG
  // If in debug mode the name column is set to 1
  _proxyModel->setFilterKeyColumn(1);
#endif
}

RulesWidget::~RulesWidget() {
  delete _ui;
}

// Ensure W, M and N sets are always graph. If the value for an edge is set to
// true, also adjust its extremities.
void RulesWidget::treatEvent(const Event &event) {
  const PropertyEvent *gEvt = dynamic_cast<const PropertyEvent *>(&event);
  if (gEvt) {
    if (gEvt->getType() == tlp::PropertyEvent::TLP_AFTER_SET_EDGE_VALUE) {
      BooleanProperty *prop = static_cast<BooleanProperty *>(gEvt->getProperty());
      edge e = gEvt->getEdge();
      if (prop->getEdgeValue(e)) { // value changed on an edge to true. Adjust
                                   // its portnodes extremities
        PortGraphModel pgm(gEvt->getProperty()->getGraph()); // very ugly hack. Should be
                                                             // PortGraphRule but the property
                                                             // W is defined on RulesRoot and
                                                             // not on each rule graph
        prop->removeListener(this);
        if (PortGraph::isPortGraphEdge(e, pgm.getGraph())) {
          const pair<const PortNode *, const PortNode *> ends = pgm.ends(e);
          ends.first->select(true, prop);
          ends.second->select(true, prop);
          prop->addListener(this);
        }
      }
    }
  }
}

void RulesWidget::setGraph(Graph *graph) {
  _graph = graph;
  _model->setRootGraph(graph);
  updateIconSize();
  if (graph != nullptr) {
    for(Graph* g:graph->subGraphs()) {
      g->getProperty(PorgyConstants::W)->addListener(this);
      g->getProperty(PorgyConstants::M)->addListener(this);
      g->getProperty(PorgyConstants::N)->addListener(this);
    }
  }
}

void RulesWidget::ShowTreeContextMenu(const QPoint &point) {
  QAbstractItemView *view = qobject_cast<QAbstractItemView *>(sender());
  assert(view);
  AbstractGraphHierachyItemModel *itemModel =
      qobject_cast<AbstractGraphHierachyItemModel *>(view->model());
  assert(_model);
  QMenu menu(this);
  Graph *selectedGraph = itemModel->indexGraph(_proxyModel->mapToSource(view->indexAt(point)));

  QAction *visualizeRule = menu.addAction(tr("&Show"), this, SLOT(showSelectedRule()));
  visualizeRule->setEnabled(selectedGraph != nullptr);

  menu.addAction(tr("&Create new"), this, SIGNAL(addNewRule()));

  QAction *renamerule = menu.addAction(tr("&Rename"), this, SLOT(rename_rule()));
  renamerule->setEnabled((selectedGraph != nullptr) &&
                         !PorgyTlpGraphStructure::isRuleUsed(selectedGraph));

  QAction *copyrule = menu.addAction(tr("&Clone"), this, SLOT(copy_one_rule()));
  copyrule->setEnabled(selectedGraph != nullptr);

  QAction *delrule = menu.addAction(tr("&Delete"), this, SLOT(deleteSelectedRule()));
  delrule->setEnabled(
      ((selectedGraph != nullptr) && !PorgyTlpGraphStructure::isRuleUsed(selectedGraph)));

  menu.addSeparator();

  QAction *importrule = menu.addAction(tr("&Import"), this, SIGNAL(importRule()));
  importrule->setEnabled(selectedGraph == nullptr);

  QAction *exportrule = menu.addAction(tr("&Export"), this, SLOT(export_one_rule()));
  exportrule->setEnabled(selectedGraph != nullptr);

  QAction *showinderivationtree =
      menu.addAction(tr("&Highlight in derivation tree"), this, SLOT(highlight()));
  showinderivationtree->setEnabled(selectedGraph != nullptr);

  menu.exec(_ui->ruleTreeView->viewport()->mapToGlobal(point));
}

void RulesWidget::highlight() {
  emit highlightrule(selectedRule());
}

void RulesWidget::rename_rule() {
  emit renameRule(selectedRule());
}

void RulesWidget::copy_one_rule() {
  emit copyRule(selectedRule());
}

void RulesWidget::deleteSelectedRule() {
  emit deleteRule(selectedRule());
}

void RulesWidget::showRule(const QModelIndex &index) {
  if (index.isValid()) {
    emit showRule(_model->indexGraph(_proxyModel->mapToSource(index)));
  }
}

void RulesWidget::showSelectedRule() {
  emit showRule(selectedRule());
}

// void RulesWidget::import_one_rule() {
//    emit importRule();
//}

void RulesWidget::export_one_rule() {
  emit exportRule(selectedRule());
}

Graph *RulesWidget::selectedRule() const {
  QModelIndexList selectedIndexes = _selectionModel->selectedRows();
  if (selectedIndexes.empty()) {
    return nullptr;
  } else {
    return _model->indexGraph(_proxyModel->mapToSource(selectedIndexes.front()));
  }
}

void RulesWidget::updateIconSize() {
  //    int slideValue = _ui->horizontalSlider->value();
  int slideValue = _ui->comboBox->currentIndex();
  if (slideValue > 0) {
    QSize iconSize;
    if (slideValue == 1) {
      iconSize = QSize(32, 32);
    } else if (slideValue == 2) {
      iconSize = QSize(64, 64);
    } else {
      iconSize = QSize(128, 128);
    }
    _model->snapshotManager().setPreviewSize(iconSize);
    _ui->stackedWidget->setCurrentIndex(1);
#ifndef NDEBUG
    // Only display Name column
    _model->setIconColumn(1);
    _ui->listView->setModelColumn(1);
#else
    _model->setIconColumn(0);
    _ui->listView->setModelColumn(0);
#endif
  } else {
    _ui->stackedWidget->setCurrentIndex(0);
    _model->setIconColumn(-1);
  }
}

bool RulesWidget::eventFilter(QObject *obj, QEvent *evt) {

  if (evt->type() == QEvent::ToolTip) {
    QHelpEvent *he = static_cast<QHelpEvent *>(evt);
    Graph *graph = nullptr;
    if (obj == _ui->ruleTreeView) {
      // Need to use viewport widget to compute correct coordinate
      // tranformation.
      // Return wrong QModelIndex if we use he->Pos();
      graph = _model->indexGraph(_proxyModel->mapToSource(_ui->ruleTreeView->indexAt(
          _ui->ruleTreeView->viewport()->mapFromGlobal(he->globalPos()))));
    } else if (obj == _ui->listView) {
      graph = _model->indexGraph(_proxyModel->mapToSource(_ui->listView->indexAt(he->pos())));
    }
    if (graph) {
      GlGraphRenderingParameters parameters;
      parameters.setEdgeColorInterpolate(false);
      parameters.setLabelsDensity(0);
      parameters.setAntialiasing(true);
      parameters.setLabelScaled(true);
      bool orientation = false;
      graph->getAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED, orientation);
      parameters.setViewArrow(orientation);
      QString name("Rule: "+tlp::tlpStringToQString(graph->getName()));

      QToolTip::showText(he->globalPos(),  name+"<br/>"+GraphSnapshotManager::snapshot2base64html(graph, parameters), this, QRect(), 5000);
      return true;
    }
  }
  return false;
}

void RulesWidget::filterRules(const QString &pattern) {
  _proxyModel->setFilterWildcard(pattern);
}
