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
#ifndef MODELSWIDGET_H
#define MODELSWIDGET_H

#include <QWidget>

#include <vector>

namespace Ui {
class GraphWidget;
}

namespace tlp {
class Graph;
}

class ModelGraphTreeModel;
class SortFilterProxyModel;
class QModelIndex;
class QItemSelectionModel;

class GraphWidget : public QWidget {
  Q_OBJECT

public:
  explicit GraphWidget(QWidget *parent = nullptr);
  ~GraphWidget() override;
  void setGraph(tlp::Graph *graph);
  std::vector<tlp::Graph *> selectedModels() const;
  bool eventFilter(QObject *, QEvent *) override;
  tlp::Graph *selectedGraph() const;
signals:
  void showGraph(tlp::Graph *);
  void copyGraph(tlp::Graph *);

private slots:
  void indexDoubleClicked(const QModelIndex &index);
  void copy_one_graph();
  void filterModels(const QString &pattern);
  void ShowTreeContextMenu(const QPoint &point);
  void updateIconSize();
  void showSelectedGraph();

private:
  Ui::GraphWidget *_ui;
  ModelGraphTreeModel *_model;
  SortFilterProxyModel *_proxyModel;
  tlp::Graph *_graph;
  QItemSelectionModel *_selectionModel;
};

#endif // MODELSWIDGET_H
