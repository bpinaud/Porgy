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
#include <tulip/BooleanProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/StringCollection.h>

#include <portgraph/PorgyConstants.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraph.h>
#include <portgraph/PortNode.h>

using namespace std;
using namespace tlp;

Port::Port(const tlp::node n, const PortNode *pp) : PortBase(n, pp) {}

Port *Port::getOneConnectedPort() const {
  vector<Port *> connectedInPorts = getConnectedInPorts();
  if (!connectedInPorts.empty())
    return connectedInPorts[0];
  vector<Port *> connectedOutPorts = getConnectedOutPorts();
  if (!connectedOutPorts.empty())
    return connectedOutPorts[0];
  return nullptr;
}

std::vector<Port *> Port::getConnectedInPorts() const {
  vector<Port *> p;
  const PortGraph &pg = getParentPortNode()->getParentPortGraph();
  Graph *g = pg.getGraph();
  for (node n : g->getInNodes(port)) {
    if (PortBase::isPort(n, g))
      p.push_back(pg.getPortContainer(n));
  }
  return p;
}

std::vector<Port *> Port::getConnectedOutPorts() const {
  vector<Port *> p;
  const PortGraph &pg = getParentPortNode()->getParentPortGraph();
  Graph *g = pg.getGraph();
  for (node n : g->getOutNodes(port)) {
    assert(PortBase::isPort(n, g));
    p.push_back(pg.getPortContainer(n));
  }
  return p;
}

vector<Port *> Port::getConnectedPorts() const {
  vector<Port *> connectedPorts(getConnectedInPorts());
  vector<Port *> outport = getConnectedOutPorts();
  connectedPorts.insert(connectedPorts.end(), outport.begin(), outport.end());
  return connectedPorts;
}

bool Port::check(const Port *rule_port, const Port *modele_port, bool exact,
                 const PorgyTlpGraphStructure::matchpropvector &v) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
  cerr << __PRETTY_FUNCTION__ << ": checking model port " << modele_port->getNode()
       << " and rule port " << rule_port->getNode() << endl;
#endif

  if ((exact && (rule_port->deg() == modele_port->deg())) ||
      (!exact && (rule_port->getArity() <= modele_port->deg()))) {
    // checking the rest of the matching properties
    Graph *graph_modele_port = modele_port->getParentPortNode()->getParentPortGraph().getGraph();
    Graph *rule_port_graph = rule_port->getParentPortNode()->getParentPortGraph().getGraph();
    node rule_tlp_node = rule_port->getNode();
    node model_tlp_node = modele_port->getNode();
    bool ret = PorgyTlpGraphStructure::checkNode(graph_modele_port, rule_port_graph, rule_tlp_node,
                                                 model_tlp_node, v);
#ifdef PORGY_RULE_DEBUG_MESSAGES
    if (!ret)
      cerr << __PRETTY_FUNCTION__ << ": model port: " << model_tlp_node << " and rule port "
           << rule_tlp_node << " not compatible" << endl;
#endif
    return ret;
  }
#ifdef PORGY_RULE_DEBUG_MESSAGES
  else
    cerr << __PRETTY_FUNCTION__ << ": Problem with port degree (model): " << modele_port->deg()
         << " and (rule)" << rule_port->deg() << endl;
#endif

  return false;
}

unsigned int Port::getArity() const {
#ifdef PORGY_RULE_DEBUG_MESSAGES
  if (!PorgyTlpGraphStructure::isRuleGraph(getParentPortNode()->getParentPortGraph().getGraph()))
    std::cerr << __PRETTY_FUNCTION__ << " WARNING: graph is not a rule" << std::endl;
#endif
  tlp::IntegerProperty *arity =
      getParentPortNode()->getParentPortGraph().getGraph()->getProperty<tlp::IntegerProperty>(
          PorgyConstants::ARITY);
  return static_cast<unsigned int>(arity->getNodeValue(getNode()));
}

unsigned int Port::deg() const {
  return indeg() + outdeg();
}

unsigned int Port::indeg() const {
  assert(getParentPortNode()->getParentPortGraph().getGraph()->isElement(getNode()));
  //-1 to remove the edge between the port tulip node and the center tulip node
  return getParentPortNode()->getParentPortGraph().getGraph()->indeg(getNode()) - 1;
}

unsigned int Port::outdeg() const {
  assert(getParentPortNode()->getParentPortGraph().getGraph()->isElement(getNode()));
  return getParentPortNode()->getParentPortGraph().getGraph()->outdeg(getNode());
}
