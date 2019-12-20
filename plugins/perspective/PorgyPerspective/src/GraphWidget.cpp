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
#include "GraphWidget.h"
#include "modelgraphtreemodel.h"
#include "sortfilterproxymodel.h"
#include "ui_GraphWidget.h"

#include <tulip/Graph.h>
#include <tulip/LayoutProperty.h>
#include <tulip/TlpQtTools.h>

#include <QHelpEvent>
#include <QMenu>
#include <QToolTip>

using namespace tlp;
using namespace std;

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::GraphWidget), _model(nullptr) {
  _ui->setupUi(this);

  connect(_ui->modelsTreeView, SIGNAL(doubleClicked(QModelIndex)),
          SLOT(indexDoubleClicked(QModelIndex)));
  connect(_ui->modelsTreeView, SIGNAL(customContextMenuRequested(const QPoint &)), this,
          SLOT(ShowTreeContextMenu(const QPoint &)));
  _ui->modelsTreeView->installEventFilter(this);
  // Intialize proxy
  _proxyModel = new SortFilterProxyModel(this);
  _proxyModel->setDynamicSortFilter(true);
  _proxyModel->setFilterOrder(SortFilterProxyModel::BottomToTop);

  // Initialize model
  _model = new ModelGraphTreeModel(this);
  GlGraphRenderingParameters parameters;
  parameters.setEdgeColorInterpolate(false);
  parameters.setLabelsDensity(0);
  parameters.setLabelScaled(true);
  _model->snapshotManager().setRenderingParameters(parameters);
  _model->snapshotManager().setPreviewSize(QSize(32, 32));
  _model->setIconColumn(-1);

  connect(_ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateIconSize()));

  _proxyModel->setSourceModel(_model);
#ifndef NDEBUG
  // If in debug mode the name column is set to 1
  _proxyModel->setFilterKeyColumn(1);
#endif

  connect(_ui->filterModelLineEdit, SIGNAL(textEdited(QString)), this, SLOT(filterModels(QString)));
  _ui->modelsTreeView->setModel(_proxyModel);

  _selectionModel = new QItemSelectionModel(_proxyModel, this);
  _ui->modelsTreeView->setSelectionModel(_selectionModel);
}

GraphWidget::~GraphWidget() {
  delete _ui;
}

void GraphWidget::setGraph(Graph *graph) {
  _graph = graph;
  _model->setRootGraph(graph);
}

void GraphWidget::ShowTreeContextMenu(const QPoint &point) {
  QAbstractItemView *view = qobject_cast<QAbstractItemView *>(sender());
  assert(view);
  AbstractGraphHierachyItemModel *itemModel =
      qobject_cast<AbstractGraphHierachyItemModel *>(view->model());
  assert(_model);
  QMenu menu(this);
  Graph *selectedGraph = itemModel->indexGraph(_proxyModel->mapToSource(view->indexAt(point)));

  QAction *showGraph = menu.addAction(tr("&Show"), this, SLOT(showSelectedGraph()));
  showGraph->setEnabled(selectedGraph != nullptr);

  QAction *copygraph =
      menu.addAction(tr("&Create new derivation from this node"), this, SLOT(copy_one_graph()));
  copygraph->setEnabled(selectedGraph != nullptr);

  menu.exec(_ui->modelsTreeView->viewport()->mapToGlobal(point));
}

void GraphWidget::copy_one_graph() {
  emit copyGraph(selectedGraph());
}

void GraphWidget::showSelectedGraph() {
  emit showGraph(selectedGraph());
}

Graph *GraphWidget::selectedGraph() const {
  QModelIndexList selectedIndexes = _selectionModel->selectedRows();
  if (selectedIndexes.empty()) {
    return nullptr;
  } else {
    return _model->indexGraph(_proxyModel->mapToSource(selectedIndexes.front()));
  }
}

vector<Graph *> GraphWidget::selectedModels() const {
  QModelIndexList indexes = _ui->modelsTreeView->selectionModel()->selectedRows();
  QModelIndexList sourcesIndexes;
  sourcesIndexes.reserve(indexes.size());

  foreach (const QModelIndex &m, indexes) { sourcesIndexes.push_back(_proxyModel->mapToSource(m)); }
  return _model->indexesGraphs(sourcesIndexes);
}

void GraphWidget::indexDoubleClicked(const QModelIndex &index) {
  emit showGraph(_model->indexGraph(_proxyModel->mapToSource(index)));
}

bool GraphWidget::eventFilter(QObject *obj, QEvent *evt) {
  if (evt->type() == QEvent::ToolTip) {
      if (obj == _ui->modelsTreeView) {
      // Need to use viewport widget to compute correct coordinate
      // tranformation.
      // Return wrong QModelIndex if we use he->Pos();
      QHelpEvent *he = static_cast<QHelpEvent *>(evt);
      Graph *graph = _model->indexGraph(_proxyModel->mapToSource(_ui->modelsTreeView->indexAt(
          _ui->modelsTreeView->viewport()->mapFromGlobal(he->globalPos()))));
      if (graph) {
        GlGraphRenderingParameters parameters;
        parameters.setEdgeColorInterpolate(false);
        parameters.setLabelsDensity(0);
        QString name("Graph: "+tlp::tlpStringToQString(graph->getName()));

        QToolTip::showText(he->globalPos(),  name+"<br/>"+GraphSnapshotManager::snapshot2base64html(graph, parameters), this, QRect(), 5000);

        return true;
      }
    }
  }

  return false;
}

void GraphWidget::filterModels(const QString &pattern) {
  _proxyModel->setFilterWildcard(pattern);
}

void GraphWidget::updateIconSize() {
  int iconsSize = _ui->comboBox->currentIndex();
  if (iconsSize > 0) {
    QSize iconSize;
    if (iconsSize == 1) {
      iconSize = QSize(32, 32);
    } else if (iconsSize == 2) {
      iconSize = QSize(64, 64);
    } else {
      iconSize = QSize(128, 128);
    }
    _model->snapshotManager().setPreviewSize(iconSize);
    _model->setIconColumn(0);
  } else {
    _model->setIconColumn(-1);
  }
}
