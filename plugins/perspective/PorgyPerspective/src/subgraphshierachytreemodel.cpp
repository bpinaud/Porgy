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
#include "subgraphshierachytreemodel.h"

#include <portgraph/PorgyTlpGraphStructure.h>

#include <tulip/TlpQtTools.h>

using namespace tlp;
using namespace std;

SubgraphsHierachyTreeModel::SubgraphsHierachyTreeModel(QObject *parent)
    : AbstractGraphHierachyItemModel(parent), _rootGraph(nullptr),
      _columnsFlags(Columns(AllColumns)), _iconColumn(-1) {
  setColumns(AllColumns);
  connect(&_snapshotManager, SIGNAL(snapshotChanged(tlp::Graph *)), this,
          SLOT(graphSnapshotChanged(tlp::Graph *)));
}

void SubgraphsHierachyTreeModel::setRootGraph(Graph *root) {
  if (_rootGraph != nullptr) {
    _rootGraph->removeListener(this);
  }
  clearRootGraphs();
  _rootGraph = root;
  if (_rootGraph != nullptr) {
    _rootGraph->addListener(this);
    _rootGraph->addObserver(this);
    addSubgraphs(_rootGraph);
  }
}

void SubgraphsHierachyTreeModel::setColumns(Columns columns) {
  beginResetModel();
  _columnsFlags = columns;
  _visibleColumns.clear();
  for (int i = 0; i <= 3; ++i) {
    Column flag = static_cast<Column>(1 << i);
    if (columns.testFlag(flag)) {
      _visibleColumns.push_back(static_cast<Column>(flag));
    }
  }
  endResetModel();
}

int SubgraphsHierachyTreeModel::columnCount(const QModelIndex &) const {
  return _visibleColumns.size();
}

QVariant SubgraphsHierachyTreeModel::data(const QModelIndex &index, int role) const {
  if (index.isValid()) {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
      Graph *graph = indexGraph(index);
      switch (_visibleColumns[index.column()]) {
#ifndef NDEBUG
      case IdColumn:
        return QVariant(graph->getId());
        break;
#endif
      case NameColumn:
        return getGraphName(graph);

        break;
      case NodesNumberColumn:
        return getNodesNumber(graph);
        break;
      case EdgesNumberColumn:
        return getEdgesNumber(graph);
        break;
      default:
        break;
      }
    } break;
    case Qt::DecorationRole:
      if (index.column() == _iconColumn) {
        return _snapshotManager.takeSnapshot(indexGraph(index));
      }
        break;
    default:
      return QVariant();
      break;
    }
  }
  return QVariant();
}

QVariant SubgraphsHierachyTreeModel::getGraphName(Graph *graph) const {
  return QVariant(tlpStringToQString(graph->getName()));
}

bool SubgraphsHierachyTreeModel::setGraphName(Graph *graph, const QVariant &data) const {
  graph->setName(tlp::QStringToTlpString(data.toString()));
  return true;
}

QVariant SubgraphsHierachyTreeModel::getNodesNumber(Graph *graph) const {
  return QVariant(graph->numberOfNodes());
}
QVariant SubgraphsHierachyTreeModel::getEdgesNumber(Graph *graph) const {
  return QVariant(graph->numberOfEdges());
}

bool SubgraphsHierachyTreeModel::setData(const QModelIndex &index, const QVariant &value,
                                         int role) {
  if (index.isValid() && role == Qt::EditRole) {
    Graph *graph = indexGraph(index);
    switch (_visibleColumns[index.column()]) {
    case NameColumn:
      if (setGraphName(graph, value)) {
        emit dataChanged(index, index);
        return true;
      }
      break;
    default:
      return false;
    }
  }
  return false;
}

Qt::ItemFlags SubgraphsHierachyTreeModel::flags(const QModelIndex &index) const {
  Qt::ItemFlags defaultFlags = AbstractGraphHierachyItemModel::flags(index);
  if (index.isValid()) {
    defaultFlags |= Qt::ItemIsDragEnabled;
    switch (_visibleColumns[index.column()]) {
    case NameColumn:
      defaultFlags |= Qt::ItemIsEditable;
      break;
    default:
      defaultFlags &= (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      break;
    }
  }
  return defaultFlags;
}
QVariant SubgraphsHierachyTreeModel::headerData(int section, Qt::Orientation orientation,
                                                int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (_visibleColumns[section]) {
#ifndef NDEBUG
    case IdColumn:
      return QVariant("Id");
      break;
#endif
    case NameColumn:
      return QVariant("Name");
      break;
    case NodesNumberColumn:
      return QVariant("#Nodes");
      break;
    case EdgesNumberColumn:
      return QVariant("#Edges");
      break;
    default:
      break;
    }
  }
  return QVariant();
}

void SubgraphsHierachyTreeModel::treatGraphEvent(const GraphEvent &graphEvent) {
  Graph *graph = graphEvent.getGraph();
  if (graph == _rootGraph) { // The root graph is updated.
    switch (graphEvent.getType()) {
    case GraphEvent::TLP_AFTER_ADD_SUBGRAPH:
      _rootToAdd.insert(
          const_cast<Graph *>(graphEvent.getSubGraph())); // Mark the graph for addition treat the
                                                          // added graph in the treatEvents function
      break;
    case GraphEvent::TLP_BEFORE_DEL_SUBGRAPH: {
      tlp::Graph *graph = const_cast<Graph *>(graphEvent.getSubGraph());
      if (!_rootToAdd.erase(graph)) { // The graph was not already added but
                                      // marked for addition
        // We need to supress the graph immediatly as it is valid
        removeRootGraph(graph); // Remove the graph from the hierarchy
        _toUpdate.erase(graph); // Supress all the possible modifications stored
      }
    } break;
    default:
      break;
    }

  } else {
    // A subgraph is updated
    switch (graphEvent.getType()) {
    case GraphEvent::TLP_AFTER_SET_ATTRIBUTE:
      // If the attribute updated is not the same
      if (_columnsFlags.testFlag(NameColumn) && graphEvent.getAttributeName() != string("name")) {
        return;
      }
    case GraphEvent::TLP_ADD_NODE:
    case GraphEvent::TLP_ADD_NODES:
      if (_columnsFlags.testFlag(NodesNumberColumn)) {
        return;
      }
    case GraphEvent::TLP_ADD_EDGE:
    case GraphEvent::TLP_ADD_EDGES:
      if (_columnsFlags.testFlag(EdgesNumberColumn)) {
        return;
      }
      _toUpdate.insert(graph);
      break;
    case GraphEvent::TLP_BEFORE_DEL_SUBGRAPH:
      _toUpdate.erase(const_cast<Graph *>(graphEvent.getSubGraph()));
      AbstractGraphHierachyItemModel::treatGraphEvent(graphEvent);
      break;
    default:
      AbstractGraphHierachyItemModel::treatGraphEvent(graphEvent);
      break;
    }
  }
}

void SubgraphsHierachyTreeModel::treatEvents(const vector<Event> &events) {
  AbstractGraphHierachyItemModel::treatEvents(events);
  for (Graph *g : _toUpdate) { // Treat modification list
    QModelIndex graphIndex = index(g);
    if (graphIndex.isValid()) {
      QModelIndex parentIndex = graphIndex.parent();
      emit dataChanged(index(graphIndex.row(), 0, parentIndex),
                       index(graphIndex.row(), columnCount(graphIndex), parentIndex));
    }
  }
  _toUpdate.clear();            // Clear list
  for (Graph *g : _rootToAdd) { // Treat addition list
      addRootGraph(g);
  }
  _rootToAdd.clear(); // Clear list
}

void SubgraphsHierachyTreeModel::graphSnapshotChanged(Graph *graph) {
  QModelIndex graphIndex = index(graph);
  emit dataChanged(graphIndex, graphIndex);
}

void SubgraphsHierachyTreeModel::setIconColumn(int column) {
  beginResetModel();
  _iconColumn = column;
  endResetModel();
}
