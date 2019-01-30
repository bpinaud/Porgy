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
#ifndef TRACESWIDGET_H
#define TRACESWIDGET_H

#include "tracesitemmodel.h"
#include <QWidget>

class StatusBarPluginProgress;

namespace Ui {
class TracesWidget;
}

namespace tlp {
class Graph;
}

class QModelIndex;
class GraphHierarchyTreeWidget;
class SortFilterProxyModel;

class TracesWidget : public QWidget, tlp::Observable {
  Q_OBJECT

  Ui::TracesWidget *ui;
  tlp::Graph *graph;
  tlp::Graph *selectedGraph;
  StatusBarPluginProgress *_pp;

public:
  explicit TracesWidget(QWidget *parent = nullptr);
  ~TracesWidget() override;
  void setGraph(tlp::Graph *graph);
  inline tlp::Graph *getGraph() const {
    return graph;
  }
  std::vector<tlp::Graph *> selectedTraces() const;
  bool eventFilter(QObject *, QEvent *) override;
  void addListener();
  void removeListener();

private:
  TracesItemModel *_model;
  SortFilterProxyModel *_proxyModel;
  void treatEvent(const tlp::Event &message) override;

private slots:
  void addNewTrace();
  void computeOnTrace();
  void deleteTrace();
  void showTrace();
  void ShowTreeContextMenu(const QPoint &point);
  void indexDoubleClicked(const QModelIndex &index);
  void filterTraces(const QString &pattern);
  void exportCSV();

signals:
  void deleteTrace(tlp::Graph *);
  void showTrace(tlp::Graph *, bool openWithDefaultView);
};

#endif // TRACESWIDGET_H
