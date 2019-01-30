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
#ifndef RULESWIDGET_H
#define RULESWIDGET_H

#include <QWidget>

#include <tulip/Observable.h>

namespace Ui {
class RulesWidget;
}

namespace tlp {
class Graph;
class View;
}

class QModelIndex;
class GraphHierarchyTreeWidget;
class RulesGraphItemModel;
class QItemSelectionModel;
class SortFilterProxyModel;

class RulesWidget : public QWidget, public tlp::Observable {
  Q_OBJECT

  Ui::RulesWidget *_ui;
  tlp::Graph *_graph;
  RulesGraphItemModel *_model;
  SortFilterProxyModel *_proxyModel;
  QItemSelectionModel *_selectionModel;

public:
  explicit RulesWidget(QWidget *parent = nullptr);
  ~RulesWidget() override;
  void setGraph(tlp::Graph *_graph);
  inline tlp::Graph *getGraph() const {
    return _graph;
  }
  tlp::Graph *selectedRule() const;

  bool eventFilter(QObject *, QEvent *) override;
  void treatEvent(const tlp::Event &event) override;

private slots:
  void ShowTreeContextMenu(const QPoint &);
  void copy_one_rule();
  void rename_rule();
  //    void import_one_rule();
  void export_one_rule();
  void deleteSelectedRule();
  void highlight();
  void updateIconSize();
  void showRule(const QModelIndex &index);
  void showSelectedRule();

  void filterRules(const QString &pattern);

signals:

  void addNewRule();
  void copyRule(tlp::Graph *);
  void renameRule(tlp::Graph *);
  void showRule(tlp::Graph *);
  void deleteRule(tlp::Graph *);
  void importRule();
  void exportRule(tlp::Graph *);
  void highlightrule(tlp::Graph *);
};

#endif // RULESWIDGET_H
