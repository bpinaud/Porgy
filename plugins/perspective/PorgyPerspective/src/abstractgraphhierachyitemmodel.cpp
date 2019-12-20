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
#include "abstractgraphhierachyitemmodel.h"

#include <portgraph/PorgyTlpGraphStructure.h>

#include <tulip/TlpQtTools.h>
#include <tulip/TulipMimes.h>

using namespace tlp;
using namespace std;
template <>
QVariant AbstractGraphHierachyItemModel::getAttribute<string>(Graph *graph,
                                                              const string &name) const {
  string s;
  graph->getAttribute<string>(name, s);
  return QVariant(tlpStringToQString(s));
}
template <>
void AbstractGraphHierachyItemModel::setAttribute<string>(Graph *graph, const string &name,
                                                          const QVariant &data) const {
  graph->setAttribute<string>(name, QStringToTlpString(data.toString()));
}

AbstractGraphHierachyItemModel::AbstractGraphHierachyItemModel(QObject *parent)
    : QAbstractItemModel(parent) {}

QModelIndex AbstractGraphHierachyItemModel::index(int row, int col,
                                                  const QModelIndex &parent) const {
  Graph *g = nullptr;
  // Root element.
  if (!parent.isValid()) {
    assert(row < static_cast<int>(_graphs.size()));
    g = _graphs[row];
  } else {
    Graph *parentGraph = indexGraph(parent);
    assert(parentGraph != nullptr);
    g = graphForIndex(row, parentGraph);
  }
  assert(g != nullptr);
  return createIndex(row, col, g);
}

QModelIndex AbstractGraphHierachyItemModel::index(Graph *g) const {
  // The graph is a root graph
  for (unsigned i = 0; i < _graphs.size(); ++i) {
    if (_graphs[i] == g)
      return createIndex(i, 0, g);
  }
  Graph *parent = g->getSuperGraph();
  // Count the position in the child list
  return createIndex(indexForGraph(g, parent), 0, g);
}

QModelIndex AbstractGraphHierachyItemModel::parent(const QModelIndex &parentIndex) const {
  Graph *g = indexGraph(parentIndex);
  // Search in the root graphs
  if (find(_graphs.begin(), _graphs.end(), g) != _graphs.end()) {
    return QModelIndex();
  } else {
    // Compute the index of the
    Graph *parentGraph = g->getSuperGraph();
    return index(parentGraph);
  }
}
int AbstractGraphHierachyItemModel::rowCount(const QModelIndex &parent) const {
  // Root elements
  if (!parent.isValid()) {
    // Root graph.
    return _graphs.size();
  } else {
    // Childs numbers.
    return graphChildNumber(indexGraph(parent));
  }
}

std::vector<Graph *>
AbstractGraphHierachyItemModel::indexesGraphs(const QModelIndexList &indexes) const {
  vector<Graph *> graphs;
  graphs.reserve(indexes.size());
  foreach (const QModelIndex &m, indexes) { graphs.push_back(indexGraph(m)); }
  return graphs;
}

QStringList AbstractGraphHierachyItemModel::mimeTypes() const {
  QStringList types(GRAPH_MIME_TYPE);
  return types;
}
QMimeData *AbstractGraphHierachyItemModel::mimeData(const QModelIndexList &indexes) const {
  set<Graph *> graphsSet;
  foreach (const QModelIndex &m, indexes) {
    Graph *g = indexGraph(m);
    graphsSet.insert(g);
  }
  GraphMimeType *mimeData = nullptr;
  if (!graphsSet.empty()) {
    mimeData = new GraphMimeType();
    mimeData->setGraph(*(graphsSet.begin()));
  }
  return mimeData;
}
Graph *AbstractGraphHierachyItemModel::graphForIndex(unsigned subGraphIndex, Graph *graph) const {
  checkCachedDataForGraph(graph);
  const auto &it = _cache.find(graph);
  assert(it->second.size() > subGraphIndex);
  return it->second[subGraphIndex];
}
unsigned int AbstractGraphHierachyItemModel::indexForGraph(Graph *subGraph, Graph *parent) const {
  checkCachedDataForGraph(parent);
  const auto &it = _cache.find(parent);
  return indexInVector(subGraph, it->second);
}

unsigned int AbstractGraphHierachyItemModel::indexInVector(Graph *toSearch,
                                                           const vector<Graph *> &graphs) const {
  for (unsigned i = 0; i < graphs.size(); ++i) {
    if (graphs[i] == toSearch) {
      return i;
    }
  }
  return UINT_MAX;
}
unsigned int AbstractGraphHierachyItemModel::graphChildNumber(Graph *parentGraph) const {
  checkCachedDataForGraph(parentGraph);
  return _cache.find(parentGraph)->second.size();
}

/**
 * @brief AbstractGraphHierachyItemModel::checkCachedDataForGraph checks if the
 * cache is updated and initialize it if not. This model use a cache to work.
 * When adding a graph the cache is not updated.
 * Later when asking for this graph if the cache is not initialized it will be
 * initialized.
 * The graph is only listened when the cache is initialized.
 * @param graph
 */
void AbstractGraphHierachyItemModel::checkCachedDataForGraph(Graph *graph) const {
  // Data are not in cache generate them
  if (_cache.find(graph) == _cache.end()) {
    buildCachedDataForGraph(graph);
  }
}

/**
 * @brief AbstractGraphHierachyItemModel::buildCachedDataForGraph build the
 * cache for the given graph and listen it.
 * @param parent
 */
void AbstractGraphHierachyItemModel::buildCachedDataForGraph(Graph *parent) const {
  auto &vec = _cache[parent];
  vec.reserve(vec.size()+parent->numberOfSubGraphs());
  vec.insert(end(vec),begin(parent->subGraphs()),end(parent->subGraphs()));

  // Need to const cast the object.
  // If we remove the const we cannot build cache on demand.
  parent->addListener(const_cast<AbstractGraphHierachyItemModel *>(this));
  parent->addObserver(const_cast<AbstractGraphHierachyItemModel *>(this));
}

void AbstractGraphHierachyItemModel::clearCacheForGraph(Graph *g) {
  auto it = _cache.find(g);

  if (it != _cache.end()) {
    vector<Graph *> subGraphs = it->second;
    _cache.erase(it);
    g->removeListener(this);
    g->removeObserver(this);
    // Stop listening subgraphs
    for (Graph *gr : subGraphs) {
      clearCacheForGraph(gr);
    }
  }
}

/**
 * @brief AbstractGraphHierachyItemModel::addGraph Add a top level graph.
 * @param graph
 */
void AbstractGraphHierachyItemModel::addRootGraph(Graph *graph) {
  // Check if the graph is already present in the hierarchy or if it's a
  // descendant of a graph in the hierarchy.
  for (Graph *gr : _graphs) {
    // Don't add the graph to the model.
    if (gr == graph || gr->isDescendantGraph(graph)) {
      return;
    }
  }

  // Append the graph at the end of the vector
  unsigned int addIndex = _graphs.size();
  beginInsertRows(QModelIndex(), addIndex, addIndex);
  _graphs.push_back(graph);
  endInsertRows();
  graph->addListener(this);
  graph->addObserver(this);
}

/**
 * @brief AbstractGraphHierachyItemModel::addSubgraphs add all the the sub
 * graphs of the given graph as the graph list. The graph in parameter is not
 * added to the list.
 * @param graph
 */
void AbstractGraphHierachyItemModel::addSubgraphs(Graph *graph) {
  assert(graph != nullptr);
  for(Graph* sg:graph->subGraphs()) {
      addRootGraph(sg);
  }
}

void AbstractGraphHierachyItemModel::removeRootGraph(Graph *graph) {
  unsigned int graphIndex = indexInVector(graph, _graphs);
  if (graphIndex != UINT_MAX) {
    beginRemoveRows(QModelIndex(), graphIndex, graphIndex);
    graph->removeListener(this);
    graph->removeObserver(this);
    _graphs.erase(_graphs.begin() + graphIndex);
    endRemoveRows();
    clearCacheForGraph(graph);
  }
}

void AbstractGraphHierachyItemModel::clearRootGraphs() {

  beginResetModel();
  for (const auto &it : _cache) {
    it.first->removeListener(this);
    it.first->removeObserver(this);
  }
  _cache.clear();

  for (Graph *g : _graphs) {
    g->removeListener(this);
    g->removeObserver(this);
  }
  _graphs.clear();
  endResetModel();
}

void AbstractGraphHierachyItemModel::treatEvent(const Event &evt) {
  switch (evt.type()) {
  case Event::TLP_DELETE: {
    // Destruction of a graph
    // Can only occur when a root graph is destructed
    Graph *graph = dynamic_cast<Graph *>(evt.sender());
    assert(graph != nullptr);
    removeRootGraph(graph);
  } break;
  case Event::TLP_MODIFICATION:
  case Event::TLP_INFORMATION: {
    const GraphEvent *gEvt = dynamic_cast<const GraphEvent *>(&evt);
    if (gEvt != nullptr) {
      treatGraphEvent(*gEvt);
    }
    break;
  }
  case Event::TLP_INVALID:
    assert(false);
    break;
  }
}

void AbstractGraphHierachyItemModel::treatGraphEvent(const tlp::GraphEvent &gEvt) {
  switch (gEvt.getType()) {
  case GraphEvent::TLP_AFTER_ADD_SUBGRAPH: {
    tlp::Graph *graph = const_cast<Graph *>(gEvt.getSubGraph());
    _subgraphsToAdd.insert(graph); // Mark the graph for addition
  } break;
  case GraphEvent::TLP_BEFORE_DEL_SUBGRAPH: {
    tlp::Graph *graph = const_cast<Graph *>(gEvt.getSubGraph());
    const auto &it = _subgraphsToAdd.find(graph);
    if (it != _subgraphsToAdd.end()) { // The graph was marked for addition
      _subgraphsToAdd.erase(it);       // Only supress them from the addition list.
    } else
      removeGraphFromHierarchy(graph); // Need to remove the graph as soon as we
                                       // know it will be destructed.
  } break;
  default:
    break;
  }
}

void AbstractGraphHierachyItemModel::addGraph(tlp::Graph *graph) {
  tlp::Graph *parentGraph = graph->getSuperGraph();
  const auto &it = _cache.find(parentGraph);
  // If the parent graph is displayed
  if (it != _cache.end()) {
    // Get the insertion position
    int position = it->second.size();
    // Get the parent graph
    QModelIndex parentIndex = index(parentGraph);
    // Notify the view
    beginInsertRows(parentIndex, position, position);
    // Update the cache
    it->second.push_back(graph);
    endInsertRows();
    // Observe new graph
    graph->addListener(this);
    graph->addObserver(this);
  }
}

void AbstractGraphHierachyItemModel::treatEvents(const std::vector<tlp::Event> &) {
  for (Graph *g : _subgraphsToAdd) {
    addGraph(g);
  }
  _subgraphsToAdd.clear();
}

void AbstractGraphHierachyItemModel::removeGraph(Graph *graph) {
  unsigned int graphIndex = indexInVector(graph, _graphs);
  if (graphIndex != UINT_MAX) {
    removeRootGraph(graph);
  } else {
    removeGraphFromHierarchy(graph);
  }
}

void AbstractGraphHierachyItemModel::removeGraphFromHierarchy(Graph *toRemove) {
  Graph *parent = toRemove->getSuperGraph();
  // Stop listening subgraph
  const auto &it = _cache.find(parent);
  if (it != _cache.end()) {
    toRemove->removeListener(this);
    // Get the insertion position
    unsigned int position = indexInVector(toRemove, it->second);
    if (position == UINT_MAX)
      return;
    // Get the parent graph
    QModelIndex parentIndex = index(parent);
    // Notify the view
    beginRemoveRows(parentIndex, position, position);
    // Update the cache
    it->second.erase(it->second.begin() + position);
    endRemoveRows();
    clearCacheForGraph(toRemove);
  }
}
