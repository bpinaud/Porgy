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
#ifndef PORGYPROPERTYMANAGER_H
#define PORGYPROPERTYMANAGER_H

#include <QAbstractTableModel>
#include <QDialog>
#include <QVariant>

#include <unordered_set>
#include <vector>

namespace Ui {
class PorgyPropertyManager;
}

namespace tlp {
class Graph;
}

class PropertyModel : public QAbstractTableModel {
  tlp::Graph *graph;
  std::vector<std::string> _propertyTable;
  std::unordered_set<std::string> matchingprop, showprop;

public:
  PropertyModel(tlp::Graph *graph, QObject *parent);
  QVariant data(const QModelIndex &index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex &) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;
  int rowCount(const QModelIndex &parent) const override;
  int columnCount(const QModelIndex &parent) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  void sort(int column = 0, Qt::SortOrder order = Qt::AscendingOrder) override;
  void addProp(const std::string &name);
  bool delProp(QModelIndexList list_idx);
  void save() const;
};

class PorgyPropertyManager : public QDialog {
  Q_OBJECT

  tlp::Graph *graph;
  Ui::PorgyPropertyManager *ui;

public:
  PorgyPropertyManager(QWidget *parent, tlp::Graph *graph);
  ~PorgyPropertyManager() override;

protected slots:
  void create();
  void remove();
  void accept() override;
};

#endif
