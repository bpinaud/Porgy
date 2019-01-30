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

#ifdef _OPENMP
#include <omp.h>
#endif

#include <tulip/BooleanProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/StringProperty.h>

#include <portgraph/Bridge.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraph.h>
#include <portgraph/PortNode.h>

using namespace std;
using namespace tlp;

PortGraph::PortGraph(Graph *g)
    : _graph(g), container(new NodeStaticProperty<PortNode *>(g)),
      container_port(new NodeStaticProperty<Port *>(g)) {

  // get central nodes
  const vector<node> &nodes = g->nodes();
  unsigned nbNodes = nodes.size();
  container_port->setAll(nullptr);
  container->setAll(nullptr);
  NodeStaticProperty<int> type(g);
  type.copyFromProperty(PortNodeBase::getTypeProperty(g));
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (unsigned i = 0; i < nbNodes; ++i) {
    node n = nodes[i];
    if (type[n] == PorgyConstants::CENTER) {
      PortNode *pn = new PortNode(*this, n);
      (*container)[n] = pn;
    }
  }
}

PortGraph::~PortGraph() {
  for (PortNode *p : *container)
    delete p;
  for (Port *p : *container_port)
    delete p;

  delete container;
  delete container_port;
}

PorgyConstants::PortNodeType PortGraphDecorator::getType(const tlp::node &n) {
  assert(graph_component->isElement(n));
  return static_cast<PorgyConstants::PortNodeType>(getTypeProperty()->getNodeValue(n));
}

PorgyConstants::PortNodeType PortGraphDecorator::getType(const tlp::edge &e) {
  assert(graph_component->isElement(e));
  return static_cast<PorgyConstants::PortNodeType>(getTypeProperty()->getEdgeValue(e));
}

tlp::IntegerProperty *PortGraphDecorator::getTypeProperty() {
  return graph_component->getProperty<IntegerProperty>(PorgyConstants::PORTNODE_TYPE);
}

const PortNode *PortGraph::operator[](const tlp::node n) const {
  if (!Bridge::isBridge(n, _graph)) {
    PortNode *p = (*container)[n];
    if (p != nullptr)
      return p;
    else { // maybe we want a portnode from a port
      Port *pp = (*container_port)[n];
      if (pp != nullptr)
        return pp->getParentPortNode();
    }
  }
  return nullptr;
}

// PortNode*& PortGraph::operator[](const tlp::node n) {
//    unsigned pos = _graph->nodePos(n);
//    assert((*container)[pos]!=nullptr);
//    assert(!Bridge::isBridge(n, _graph));
//    return (*container)[pos];
//}

PortNode *PortGraph::getOnePortNode() const {
  for (PortNode *p : *container) {
    if (p != nullptr)
      return p;
  }
  return nullptr;
}

vector<PortNode *> PortGraph::getPortNodes(BooleanProperty *ban, bool random_order) const {
  vector<PortNode *> p;

  for (PortNode *n : *container) {
    if (n != nullptr) {
      if ((ban == nullptr) || (!ban->getNodeValue(n->getCenter()))) {
        p.push_back(n);
      }
    }
  }

  if (random_order) {
    shuffle(p.begin(), p.end(), PorgyTlpGraphStructure::gen);
  }
  return p;
}

bool PortGraphDecorator::isCenter(Graph *g, const node n) {
  assert(g->isElement(n));
  return (g->getProperty<IntegerProperty>(PorgyConstants::PORTNODE_TYPE)->getNodeValue(n) ==
          PorgyConstants::CENTER);
}

// static method
bool PortGraph::isPortGraphEdge(const edge e, Graph *g) {
  const pair<node, node> &ends = g->ends(e);
  return (PortBase::isPort(ends.first, g) && PortBase::isPort(ends.second, g) &&
          !Bridge::isBridge(ends.second, g) && !Bridge::isBridge(ends.first, g));
}

Port *PortGraph::getPortContainer(const node &n) const {
  assert((*container_port)[n] != nullptr);
  return (*container_port)[n];
}

pair<const PortNode *, const PortNode *> PortGraph::ends(const tlp::edge e) const {
  const pair<node, node> &ends = _graph->ends(e);
  Port *p_src = (*container_port)[ends.first];
  Port *p_tgt = (*container_port)[ends.second];
  assert(p_src != nullptr);
  assert(p_tgt != nullptr);
  return make_pair(p_src->getParentPortNode(), p_tgt->getParentPortNode());
}

void PortGraph::drawSameSizes(SizeProperty *NodeSize, LayoutProperty *layout) const {
  map<string, pair<unsigned, unsigned>> optimalSizes;
  pair<unsigned, unsigned> pair1;
  pair<unsigned, unsigned> pair2;
  optimalSizes[""] = make_pair(1, 1);
  for (PortNode *pn : getPortNodes()) {
    if (Bridge::isBridge(pn->getCenter(), _graph))
      continue;
    if (!pn->getName().empty()) {
      pair1 = pn->getOptimalSize(layout);
      pair2 = optimalSizes[pn->getName()];

      pair2.first = max(pair1.first, pair2.first);
      pair2.second = max(pair1.second, pair2.second);

      optimalSizes[pn->getName()] = pair2;
    }
  }
  for (PortNode *pn : getPortNodes()) {
    if (Bridge::isBridge(pn->getCenter(), _graph)) {
      continue;
    }
    if (optimalSizes.end() != optimalSizes.find(pn->getName())) { // found
      pn->setPositions(optimalSizes[pn->getName()].first, optimalSizes[pn->getName()].second,
                       layout);
      pn->resizeCenter(optimalSizes[pn->getName()].first, optimalSizes[pn->getName()].second,
                       NodeSize);
    } else {
      pn->setPositions(layout);
      pn->resizeCenter(NodeSize, layout);
    }
  }
}

PortGraphDecorator::PortGraphDecorator(Graph *g) : GraphDecorator(g) {}

vector<const PortNode *> PortGraph::getInOutPortNodes(PortNode *pn,
                                                      tlp::BooleanProperty *ban) const {
  vector<const PortNode *> p;
  for (Port *pt : pn->getPorts()) {
    for (Port *pt2 : pt->getConnectedPorts()) {
      const PortNode *pn2 = (*this)[pt2->getNode()];
      if ((ban == nullptr) || (!ban->getNodeValue(pn2->getCenter())))
        p.push_back(pn2);
    }
  }
  return p;
}

/**
 * @brief PortGraphModel::numberOfNodes
*/
unsigned PortGraphDecorator::numberOfNodes() const {
  unsigned nb = 0, nb_tulip = 0;
  if (graph_component->existAttribute("number_of_nodes")) {
    graph_component->getAttribute<unsigned>("number_of_nodes", nb);
    graph_component->getAttribute<unsigned>("number_of_nodes_tulip", nb_tulip);
  }

  if (nb_tulip != graph_component->numberOfNodes()) {
    nb = 0;
    auto nodes(graph_component->nodes());
    for (node n : nodes) {
      if (PortGraphDecorator::isCenter(graph_component, n))
        nb++;
    }
    graph_component->setAttribute<unsigned>("number_of_nodes", nb);
    graph_component->setAttribute<unsigned>("number_of_nodes_tulip",
                                            graph_component->numberOfNodes());
  }
  return nb;
}
