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
#ifndef SUBGRAPHSHIERACHYTREEMODEL_H
#define SUBGRAPHSHIERACHYTREEMODEL_H

#include "abstractgraphhierachyitemmodel.h"
#include <porgy/graphsnapshotmanager.h>

#include <QSet>
#include <QVector>

using namespace std;

class SubgraphsHierachyTreeModel : public AbstractGraphHierachyItemModel {
  Q_OBJECT
public:
#ifdef NDEBUG
  enum Column{NameColumn = 0x1, NodesNumberColumn = 0x2, EdgesNumberColumn = 0x4, AllColumns = 0x7};
#else
  enum Column {
    IdColumn = 0x1,
    NameColumn = 0x2,
    NodesNumberColumn = 0x4,
    EdgesNumberColumn = 0x8,
    AllColumns = 0xF
  };
#endif
  Q_DECLARE_FLAGS(Columns, Column)

  SubgraphsHierachyTreeModel(QObject *parent = nullptr);

  inline Columns columns() const {
    return _columnsFlags;
  }
  void setColumns(Columns columns);

  inline int iconColumn() const {
    return _iconColumn;
  }
  void setIconColumn(int column);

  inline tlp::Graph *rootGraph() const {
    return _rootGraph;
  }
  void setRootGraph(tlp::Graph *root);

  inline GraphSnapshotManager &snapshotManager() {
    return _snapshotManager;
  }

  // QAbstractItemModel reimplemented functions.
  int columnCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  // Event handling
  void treatEvents(const std::vector<tlp::Event> &events) override;

protected:
  void treatGraphEvent(const tlp::GraphEvent &graphEvent) override;

  virtual QVariant getGraphName(tlp::Graph *graph) const;
  virtual bool setGraphName(tlp::Graph *graph, const QVariant &data) const;
  virtual QVariant getNodesNumber(tlp::Graph *graph) const;
  virtual QVariant getEdgesNumber(tlp::Graph *graph) const;

  Q_PROPERTY(Columns columns READ columns WRITE setColumns)
  Q_PROPERTY(tlp::Graph *rootGraph READ rootGraph WRITE setRootGraph)
  Q_PROPERTY(int iconColumn READ iconColumn WRITE setIconColumn)

  tlp::Graph *_rootGraph;
  unordered_set<tlp::Graph *> _toUpdate, _rootToAdd;
  Columns _columnsFlags;
  std::vector<Column> _visibleColumns;
  mutable GraphSnapshotManager _snapshotManager;
  int _iconColumn;

private slots:
  void graphSnapshotChanged(tlp::Graph *);
};

#endif // SUBGRAPHSHIERACHYTREEMODEL_H
