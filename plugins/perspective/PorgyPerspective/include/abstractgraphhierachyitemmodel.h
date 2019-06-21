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
#ifndef GRAPHTREEMODEL_H
#define GRAPHTREEMODEL_H

#include <QAbstractItemModel>

#include <tulip/Graph.h>
#include <tulip/Observable.h>

#include <unordered_set>

class AbstractGraphHierachyItemModel : public QAbstractItemModel, public tlp::Observable {

  Q_OBJECT

  std::vector<tlp::Graph *> _graphs;
  mutable std::map<tlp::Graph *, std::vector<tlp::Graph *>> _cache;
  std::unordered_set<tlp::Graph *> _subgraphsToAdd;

public:
  explicit AbstractGraphHierachyItemModel(QObject *parent = nullptr);
  ~AbstractGraphHierachyItemModel() override {}

  QModelIndex index(int row, int col, const QModelIndex &parent) const override;
  QModelIndex index(tlp::Graph *) const;

  QModelIndex parent(const QModelIndex &) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  tlp::Graph *indexGraph(const QModelIndex &index) const {
    return index.isValid() ? static_cast<tlp::Graph *>(index.internalPointer()) : nullptr;
  }

  std::vector<tlp::Graph *> indexesGraphs(const QModelIndexList &indexes) const;

  QStringList mimeTypes() const override;
  QMimeData *mimeData(const QModelIndexList &) const override;

protected:
  void treatEvent(const tlp::Event &evt) override;
  void treatEvents(const std::vector<tlp::Event> &events) override;
  void addRootGraph(tlp::Graph *graph);
  void removeRootGraph(tlp::Graph *graph);
  void removeGraph(tlp::Graph *graph);
  void addGraph(tlp::Graph *graph);
  void clearRootGraphs();
  void addSubgraphs(tlp::Graph *graph);

  template <typename ATTRIBUTE_TYPE>
  QVariant getAttribute(tlp::Graph *graph, const std::string &name) const {
    return QVariant::fromValue<ATTRIBUTE_TYPE>(graph->getAttribute<ATTRIBUTE_TYPE>(name));
  }

  template <typename ATTRIBUTE_TYPE>
  void setAttribute(tlp::Graph *graph, const std::string &name, const QVariant &data) const {
    graph->setAttribute<ATTRIBUTE_TYPE>(name, data.value<ATTRIBUTE_TYPE>());
  }

  virtual void treatGraphEvent(const tlp::GraphEvent &graphEvent);

private:
  unsigned int indexInVector(tlp::Graph *toSearch, const std::vector<tlp::Graph *> &graphs) const;
  tlp::Graph *graphForIndex(unsigned subGraphIndex, tlp::Graph *) const;
  unsigned int indexForGraph(tlp::Graph *subGraph, tlp::Graph *parent) const;
  unsigned int graphChildNumber(tlp::Graph *) const;

  void clearCacheForGraph(tlp::Graph *graph);
  void checkCachedDataForGraph(tlp::Graph *graph) const;
  void buildCachedDataForGraph(tlp::Graph *graph) const;

  /**
   * @brief removeGraphFromHierarchy remove the graph from the hierarchy and
   * clear it's cache.
   */
  void removeGraphFromHierarchy(tlp::Graph *);
};

template <>
QVariant AbstractGraphHierachyItemModel::getAttribute<std::string>(tlp::Graph *graph,
                                                                   const std::string &name) const;
template <>
void AbstractGraphHierachyItemModel::setAttribute<std::string>(tlp::Graph *graph,
                                                               const std::string &name,
                                                               const QVariant &data) const;

#endif // GRAPHTREEMODEL_H
