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
#include <tulip/LayoutProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/StableIterator.h>

#include <portgraph/Bridge.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>

using namespace std;
using namespace tlp;

PortGraphRule::PortGraphRule(Graph *g) : PortGraph(g), bridge(nullptr) {
  assert(PorgyTlpGraphStructure::isRuleGraph(g));
  IntegerProperty *type = PortNodeBase::getTypeProperty(g);
  for (node n : type->getNodesEqualTo(PorgyConstants::CENTER, g)) {
    if (Bridge::isBridge(n, g)) {
      bridge = new Bridge(*this, n);
      break;
    }
  }
  // Be sure to find the bridge
  assert(bridge != nullptr);
}

bool PortGraphRuleDecorator::isEmpty() const {
    return numberOfNodes() == 1;
}

bool PortGraphRuleDecorator::redraw(const std::string &plugin, tlp::DataSet &parameters,
                                    std::string &errMsg, tlp::PluginProgress *progress) {
  bool res = false;
  std::list<std::string> plugins = layoutPluginsList();
  if (std::find(plugins.begin(), plugins.end(), plugin) == plugins.end()) {
    errMsg = "The choosen plugin is not in the list";
  } else {
    if (!isEmpty()) { // la règle a plus qu'un bridge
      Observable::holdObservers();
      res = graph_component->applyPropertyAlgorithm(
          plugin, graph_component->getProperty<LayoutProperty>("viewLayout"), errMsg, &parameters, progress);
      Observable::unholdObservers();
    } else {
      errMsg = "The rule is empty";
    }
  }
  return res;
}

std::tuple<tlp::Graph *, tlp::Graph *, Graph *>
PortGraphRule::extractLHSRHS(Graph *g, PluginProgress *pp, string &errMsg) {
  PortGraphRuleDecorator dec(g);
  IntegerProperty *side = dec.getSideProperty();
  DataSet parameters;
  parameters.set("Property", side);
  StringCollection elementTypes("nodes;edges;");
  elementTypes.setCurrent("nodes");
  parameters.set("Type", elementTypes);
  parameters.set("Connected", false);
  if (!g->applyAlgorithm("Equal Value", errMsg, &parameters, pp))
    return make_tuple<Graph *, Graph *, Graph *>(nullptr, nullptr, nullptr);
  Graph *rhs = nullptr, *lhs = nullptr;
  for (Graph *sg : g->subGraphs()) {
    stringstream sstr; // use the code from EqualValueClustering.cpp to build graph name
    sstr << PorgyConstants::RULE_SIDE << ": ";
    sstr.width(8);
    sstr << PorgyConstants::SIDE_RIGHT;
    if (sg->getName() == sstr.str())
      rhs = sg;
    sstr.str(string());
    sstr.clear();
    sstr << PorgyConstants::RULE_SIDE << ": ";
    sstr.width(8);
    sstr << PorgyConstants::SIDE_LEFT;
    if (sg->getName() == sstr.str())
      lhs = sg;
  }
  if (lhs == nullptr) // useful when using the stop/cancel buttons
    return make_tuple<Graph *, Graph *, Graph *>(nullptr, nullptr, nullptr);
  // Do it again on lhs to check for anti-edge
  IntegerProperty *type = dec.getTypeProperty();
  parameters.set("Property", type);
  elementTypes.setCurrent("edges");
  parameters.set("Type", elementTypes);
  Graph *lhs_anti = nullptr;
  if (!lhs->applyAlgorithm("Equal Value", errMsg, &parameters, pp))
    return make_tuple<Graph *, Graph *, Graph *>(nullptr, nullptr, nullptr);
  for (Graph *sg : lhs->subGraphs()) {
    stringstream sstr; // use the code from EqualValueClustering.cpp to build graph name
    sstr << PorgyConstants::PORTNODE_TYPE << ": ";
    sstr.width(8);
    sstr << PorgyConstants::NO_TYPE;
    if (sg->getName() == sstr.str())
      lhs = sg;
    sstr.str(string());
    sstr.clear();
    sstr << PorgyConstants::PORTNODE_TYPE << ": ";
    sstr.width(8);
    sstr << PorgyConstants::ANTI;
    if (sg->getName() == sstr.str())
      lhs_anti = sg;
  }
  string rulename(g->getName());
  PorgyTlpGraphStructure::setRuleNameForSubGraph(lhs, rulename);
  if (rhs != nullptr)
    PorgyTlpGraphStructure::setRuleNameForSubGraph(rhs, rulename);
  if (lhs_anti != nullptr)
    PorgyTlpGraphStructure::setRuleNameForSubGraph(lhs_anti, rulename);
  return make_tuple(lhs, rhs, lhs_anti);
}

void PortGraphRule::cleanLHSRHS(Graph *r) {
  auto v = r->subGraphs();
    for (Graph *g : v) {
    r->delAllSubGraphs(g);
  }
}

PortGraphRule::~PortGraphRule() {
  delete bridge;
}

tlp::BooleanProperty *PortGraphRuleDecorator::getSelectionProperty() {
  return PorgyTlpGraphStructure::getRulesRoot(graph_component)
      ->getLocalProperty<BooleanProperty>("viewSelection");
}

const forward_list<node> PortGraphRule::findRightEquiv(const node &n) const {
  return bridge->getRight(getPortContainer(n));
}

vector<PortNode *> PortGraphRule::getMember(const PorgyConstants::RuleSide side) {
  PortGraphRuleDecorator dec(_graph);
  vector<PortNode *> v;
  for (PortNode *pn : PortGraph::getPortNodes()) {
    if (dec.getSide(pn->getCenter()) == side)
      v.push_back(pn);
  }
  return v;
}

std::vector<PortNode *> PortGraphRule::getPortNodes(BooleanProperty *, bool) const {
  vector<PortNode *> p;
  PortGraphRuleDecorator dec(_graph);

  for (unsigned i = 0; i < container->size(); ++i) {
    PortNode *n = (*container)[i];
    if (n != nullptr) {
      if ((dec.getSide(n->getCenter()) == PorgyConstants::SIDE_RIGHT) ||
          (dec.getSide(n->getCenter()) == PorgyConstants::SIDE_LEFT))
        p.push_back(n);
    }
  }

  return p;
}

vector<edge> PortGraphRule::getEdges(tlp::BooleanProperty *) {
  PortGraphRuleDecorator dec(_graph);
  vector<edge> v;
  for (edge e : getGraph()->edges()) {
    if (((dec.getSide(e) == PorgyConstants::SIDE_RIGHT) ||
         (dec.getSide(e) == PorgyConstants::SIDE_LEFT)) &&
        (PortGraph::isPortGraphEdge(e, getGraph()))) {
      v.push_back(e);
    }
  }
  return v;
}

vector<edge> PortGraphRule::getMemberEdges(const PorgyConstants::RuleSide side) {
  PortGraphRuleDecorator dec(_graph);
  vector<edge> v;
  for (edge e : getGraph()->edges()) {
    if (side == PorgyConstants::SIDE_BRIDGE) {
      if ((dec.getSide(e) == side)) { // useless?
        v.push_back(e);
      }
    }
    if ((dec.getSide(e) == side) && (PortGraph::isPortGraphEdge(e, getGraph()))) {
      v.push_back(e);
    }
  }
  return v;
}

StringCollection PortGraphRule::getLayoutAlgorithm() {
  string layout_algo;
  getGraph()->getAttribute(PorgyConstants::LAYOUTALGORITHM, layout_algo);
  StringCollection tmp(LAYOUT_ALGO);
  tmp.setCurrent(layout_algo);
  return tmp;
}

PorgyConstants::RuleSide PortGraphRuleDecorator::getSide(const tlp::node &n) {
  assert(PorgyTlpGraphStructure::isRuleGraph(graph_component));
  assert(graph_component->isElement(n));
  return static_cast<PorgyConstants::RuleSide>(getSideProperty()->getNodeValue(n));
}

PorgyConstants::RuleSide PortGraphRuleDecorator::getSide(const tlp::edge &e) {
  assert(PorgyTlpGraphStructure::isRuleGraph(graph_component));
  assert(graph_component->isElement(e));
  return static_cast<PorgyConstants::RuleSide>(getSideProperty()->getEdgeValue(e));
}

std::list<std::string> PortGraphRuleDecorator::layoutPluginsList() const {
  std::list<std::string> list;
  list.push_back(PorgyConstants::REDRAW_RULE);
  return list;
}

PortGraphRuleDecorator::PortGraphRuleDecorator(Graph *g) : PortGraphDecorator(g) {}

IntegerProperty *PortGraphRuleDecorator::getSideProperty() {
  return graph_component->getProperty<tlp::IntegerProperty>(PorgyConstants::RULE_SIDE);
}

IntegerProperty *PortGraphRuleDecorator::getArityProperty() {
  return graph_component->getProperty<IntegerProperty>(PorgyConstants::ARITY);
}
