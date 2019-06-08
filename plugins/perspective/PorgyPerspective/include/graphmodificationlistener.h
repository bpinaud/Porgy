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

#ifndef GRAPHPROPERTIESRECORDER_H_
#define GRAPHPROPERTIESRECORDER_H_

#include <tulip/Graph.h>

#include <set>
#include <vector>

namespace tlp {
class Graph;
class PropertyInterface;
}
/**
* @brief Register all modification performed on a graph.
*
* Register created, deleted or updated properties name and the nodes that have
*changed for the updated properties.
**/
class GraphModficationListener : public tlp::Observable {
public:
  GraphModficationListener();
  ~GraphModficationListener() override;
  void startRecording(tlp::Graph *);
  void stopRecording(tlp::Graph *);

  const std::vector<std::string> &getAddedProperties() const {
    return addedProperties;
  }
  const std::set<std::string> &getRemovedProperties() const {
    return removedProperties;
  }
  const std::set<std::string> &getUpdatedProperties() const {
    return updatedProperties;
  }

  const std::set<tlp::node> &getUpdatedNodes() const {
    return updatedNodes;
  }
  const std::set<tlp::node> &getCreatedNodes() const {
    return createdNodes;
  }
  const std::set<tlp::node> &getDeletedNodes() const {
    return deletedNodes;
  }
  const std::set<tlp::edge> &getCreatedEdges() const {
    return createdEdges;
  }
  const std::set<tlp::edge> &getDeletedEdges() const {
    return deletedEdges;
  }

protected:
  void addLocalProperty(tlp::Graph *, const std::string &);
  void addNode(tlp::Graph *graph, const tlp::node n);
  void addNodes(tlp::Graph *, const std::vector<tlp::node> &nodes);
  void delNode(tlp::Graph *graph, const tlp::node n);
  void addEdge(tlp::Graph *graph, const tlp::edge e);
  void addEdges(tlp::Graph *, const std::vector<tlp::edge> &edges);
  void delEdge(tlp::Graph *graph, const tlp::edge e);
  void afterSetNodeValue(tlp::PropertyInterface *, const tlp::node);
  void afterSetEdgeValue(tlp::PropertyInterface *, const tlp::edge);
  void afterSetAllNodeValue(tlp::PropertyInterface *);
  void afterSetAllEdgeValue(tlp::PropertyInterface *);
  void destroy(tlp::PropertyInterface *);

  void treatEvent(const tlp::Event &) override;
  std::vector<std::string> addedProperties;
  std::set<std::string> removedProperties;
  std::set<std::string> updatedProperties;
  std::set<tlp::node> updatedNodes;

  std::set<tlp::node> createdNodes;
  std::set<tlp::node> deletedNodes;
  std::set<tlp::edge> createdEdges;
  std::set<tlp::edge> deletedEdges;
};

#endif /* GRAPHPROPERTIESRECORDER_H_ */
