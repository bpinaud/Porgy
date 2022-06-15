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
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNodeBase.h>
#include <portgraph/Trace.h>

#include <tulip/BooleanProperty.h>
#include <tulip/ColorProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/GraphMeasure.h>
#include <tulip/IntegerProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/StableIterator.h>

#include <chrono>

using namespace std;
using namespace tlp;

mt19937
    PorgyTlpGraphStructure::gen(chrono::high_resolution_clock::now().time_since_epoch().count());

bool PorgyTlpGraphStructure::isRuleUsed(Graph *rule) {
  // check if rule has been used for rewriting
  Graph *models_root = PorgyTlpGraphStructure::getModelsRoot(rule);
  for (Graph *g : models_root->getDescendantGraphs()) {
    if (rule->getName() == getRuleNameForSubGraph(g))
      return true;
  }
  return false;
}

bool PorgyTlpGraphStructure::isGraphUsed(Graph *g) {

  // LHS morphisms must not be modified
  if (g->existLocalProperty(PorgyConstants::TAG))
    return true;
  // check if the graph has been used for rewriting (degree > 0 in the
  // derivation tree)
  Trace t(PorgyTlpGraphStructure::getMainTrace(g));
  for (node n : t.getModelsMetaNodes()) {
    if ((t.getNodeMetaInfo(n)->getName() == g->getName()) && (t.deg(n) > 0))
      return true;
  }
  return false;
}

bool PorgyTlpGraphStructure::fixTraceGraph(Graph *g) {
  bool fix = false;
  if (!g->existLocalProperty("viewSize") || !g->existLocalProperty("viewColor") ||
      !g->existLocalProperty("viewShape") || !g->existLocalProperty("viewLabelPosition") ||
      !g->existLocalProperty("viewTgtAnchorShape") || !g->existLocalProperty("viewTgtAnchorSize") ||
      !g->existLocalProperty("viewBorderWidth")) {
    fix = true;

    SizeProperty *EltSize = g->getProperty<SizeProperty>("viewSize");
    ColorProperty *color_prop = g->getProperty<ColorProperty>("viewColor");
    IntegerProperty *shape = g->getProperty<IntegerProperty>("viewShape");
    IntegerProperty *label_position = g->getProperty<IntegerProperty>("viewLabelPosition");
    IntegerProperty *anchorshape = g->getProperty<IntegerProperty>("viewTgtAnchorShape");
    SizeProperty *anchorsize = g->getProperty<SizeProperty>("viewTgtAnchorSize");
    DoubleProperty *border_width = g->getProperty<DoubleProperty>("viewBorderWidth");
    SizeProperty *EltSizel = g->getLocalProperty<SizeProperty>("viewSize");
    ColorProperty *color_propl = g->getLocalProperty<ColorProperty>("viewColor");
    IntegerProperty *shapel = g->getLocalProperty<IntegerProperty>("viewShape");
    IntegerProperty *label_positionl = g->getLocalProperty<IntegerProperty>("viewLabelPosition");
    IntegerProperty *anchorshapel = g->getLocalProperty<IntegerProperty>("viewTgtAnchorShape");
    SizeProperty *anchorsizel = g->getLocalProperty<SizeProperty>("viewTgtAnchorSize");
    DoubleProperty *border_widthl = g->getLocalProperty<DoubleProperty>("viewBorderWidth");
    EltSizel->copy(EltSize);
    color_propl->copy(color_prop);
    shapel->copy(shape);
    label_positionl->copy(label_position);
    anchorshapel->copy(anchorshape);
    anchorsizel->copy(anchorsize);
    border_widthl->copy(border_width);

    string errMsg;
    Trace Tr(g);
    Tr.redraw(errMsg);
  }
  return fix;
}

tlp::Graph *PorgyTlpGraphStructure::setPorgyStructure(tlp::Graph *root) {
  if (root == nullptr)
    root = tlp::newGraph();

  // Put the whole simple graph in G0 (no subgraph/no metadata)
  root->setName("root");
  root->getProperty<IntegerProperty>("viewTgtAnchorShape")
      ->setAllEdgeValue(EdgeExtremityShape::None);
  root->getProperty<IntegerProperty>("viewSrcAnchorShape")
      ->setAllEdgeValue(EdgeExtremityShape::None);
  root->getProperty<DoubleProperty>("viewBorderWidth")->setAllNodeValue(1.0);
  PortNodeBase::getTypeProperty(root);


  // graphs
  Graph *models_root = root->addCloneSubGraph(PorgyConstants::GRAPHS_GRAPH_NAME);
  BooleanProperty *selectModel = models_root->getLocalProperty<BooleanProperty>("viewSelection");
  Graph *G0 = models_root->addCloneSubGraph(PorgyConstants::INITIAL_GRAPH);

  models_root->getProperty<IntegerProperty>("viewShape")
      ->setAllNodeValue(PorgyConstants::CENTER_SHAPE);
  LayoutProperty *layout_root = root->getProperty<LayoutProperty>("viewLayout");
  LayoutProperty *layout =
      G0->getLocalProperty<LayoutProperty>("viewLayout"); // must be local to each graph
  layout->copy(layout_root);                              // restore value to the local property
  SizeProperty *size_root = root->getProperty<SizeProperty>("viewSize");
  SizeProperty *size =
      G0->getLocalProperty<SizeProperty>("viewSize"); // must be local to each graph
  size->copy(size_root);                              // restore value to the local property
  models_root->setAttribute<unsigned>(PorgyConstants::NUMBER, 0);

  // rules
  Graph *rules_root = root->addSubGraph(PorgyConstants::RULES_GRAPH_NAME);
  rules_root->getLocalProperty<BooleanProperty>("viewSelection");
  rules_root->getProperty<IntegerProperty>("viewShape")
      ->setAllNodeValue(PorgyConstants::CENTER_SHAPE);
  BooleanProperty *Mprop = rules_root->getLocalProperty<BooleanProperty>(PorgyConstants::M);
  Mprop->setAllNodeValue(true);
  Mprop->setAllEdgeValue(true);
  rules_root->getLocalProperty<BooleanProperty>(PorgyConstants::N);
  rules_root->getLocalProperty<BooleanProperty>(PorgyConstants::W);

  PortGraphRuleDecorator dec(rules_root);
  dec.getSideProperty();

  // derivation trees
  Graph *traceroot = root->addSubGraph(PorgyConstants::TRACE_GRAPH_NAME);
  traceroot->getLocalProperty<BooleanProperty>("viewSelection");
  Graph *tracemain = traceroot->addSubGraph(PorgyConstants::TRACEMAIN);
  Trace t(tracemain);
  t.createMetaNode(G0);
  fixTraceGraph(tracemain);

  selectModel->setAllNodeValue(false);
  selectModel->setAllEdgeValue(false);

  return root;
}

void PorgyTlpGraphStructure::Graph2PortGraph(Graph *graph, bool multiPort, bool mergeInOut) {
  // add ports
  IntegerProperty *type = graph->getProperty<IntegerProperty>(PorgyConstants::PORTNODE_TYPE);
  StringProperty *viewLabel = graph->getProperty<StringProperty>("viewLabel");
  type->setAllNodeValue(PorgyConstants::CENTER);

  // if we want the same number of port everywhere
  if (multiPort) {
    // find the node with the higher degree => max number of ports
    vector<node> v(graph->nodes());
    unsigned int nb_ports = 0;
    for (node p : v) {
      if (graph->deg(p) > nb_ports)
        nb_ports = graph->deg(p);
    }

    for (node n : v) {
      for (edge e : stableIterator(graph->getOutEdges(n))) {
        node p = graph->addNode();
        type->setNodeValue(p, PorgyConstants::PORT);
        graph->setSource(e, p);
        graph->addEdge(n, p);
      }
      for (edge e : stableIterator(graph->getInEdges(n))) {
        node p = graph->addNode();
        type->setNodeValue(p, PorgyConstants::PORT);
        graph->setTarget(e, p);
        graph->addEdge(n, p);
      }
      for (unsigned int i = graph->deg(n); i < nb_ports; ++i) {
        node p = graph->addNode();
        type->setNodeValue(p, PorgyConstants::PORT);
        graph->addEdge(n, p);
      }
    }
  } else {
    // if we use only one port to get all the connections
    // mergeInOut is used to specify whether or not specific ports should be
    // created to differenciate in- and outcoming edges
    vector<node> v(graph->nodes());
    for (node n : v) {
      node p = graph->addNode();
      type->setNodeValue(p, PorgyConstants::PORT);
      for (edge e : stableIterator(graph->getOutEdges(n))) {
        graph->setSource(e, p);
      }
      graph->addEdge(n, p);
      if (!mergeInOut) {
        viewLabel->setNodeValue(p, "Out");
        p = graph->addNode();
        type->setNodeValue(p, PorgyConstants::PORT);
        graph->addEdge(n, p);
        viewLabel->setNodeValue(p, "In");
      }
      for (edge e : stableIterator(graph->getInEdges(n))) {
        graph->setTarget(e, p);
      }
    }
  }
}

Graph *PorgyTlpGraphStructure::getRootGraph(const Graph *graph) {
  Graph *root = nullptr;
  if (PorgyTlpGraphStructure::isTraceGraph(graph))
    root = getTraceRoot(graph);
  else if (PorgyTlpGraphStructure::isModelGraph(graph))
    root = getModelsRoot(graph);
  else if (PorgyTlpGraphStructure::isRuleGraph(graph))
    root = getRulesRoot(graph);
  assert(root != nullptr);
  return root;
}

bool PorgyTlpGraphStructure::checkEdgeState( const bool debug,Graph *modele, Graph *rule, const edge rule_e,
                                            const edge graph_e,
                                            const matchpropvector &matchingPropertiesList) {
    if(debug)
        tlp::debug() << "Checking rule edge " << rule_e << " against model edge " << graph_e << endl;
  for (const matchproptuple &pName : matchingPropertiesList) {
    const string &propertyName = get<0>(pName);
    BooleanProperty *testCurrentProperty = get<1>(pName);
    if (testCurrentProperty->getEdgeValue(rule_e) == true) {
      IntegerProperty *currentPropertyComparisonOp = get<2>(pName);
      tlp::PropertyInterface *currentModelProperty = get<4>(pName);
      tlp::PropertyInterface *currentRuleProperty = get<3>(pName);
      PorgyConstants::ComparisonOp op_comp = static_cast<PorgyConstants::ComparisonOp>(
          currentPropertyComparisonOp->getEdgeValue(rule_e));
      // generalisation taking the comparison operator into account
      bool comparison = false;
      if (currentModelProperty->getTypename() ==
          BooleanProperty::propertyTypename) { // only for bool (necessary?)
        BooleanProperty *tmpBoolProp = modele->getProperty<BooleanProperty>(propertyName);
        bool model_val = tmpBoolProp->getEdgeValue(graph_e);
        tmpBoolProp = rule->getProperty<BooleanProperty>(propertyName);
        bool rule_val = tmpBoolProp->getEdgeValue(rule_e);
        comparison =
            PorgyTlpGraphStructure::compareSamePropertyValue<bool>(model_val, rule_val, op_comp);
      } else if ((currentModelProperty->getTypename() == tlp::DoubleProperty::propertyTypename) ||
                 ((currentModelProperty->getTypename() ==
                   tlp::IntegerProperty::propertyTypename))) { // specific
                                                               // treatment for
                                                               // double values
        NumericProperty *tmpNumProp =
            static_cast<NumericProperty *>(modele->getProperty(propertyName));
        double model_val = tmpNumProp->getEdgeDoubleValue(graph_e);
        tmpNumProp = static_cast<NumericProperty *>(rule->getProperty(propertyName));
        double rule_val = tmpNumProp->getEdgeDoubleValue(rule_e);
        comparison =
            PorgyTlpGraphStructure::compareSamePropertyValue<double>(model_val, rule_val, op_comp);
      } else if (currentModelProperty->getTypename() ==
                 tlp::ColorProperty::propertyTypename) { // specific treatment
                                                         // for double values
        ColorProperty *tmpNumProp = modele->getProperty<ColorProperty>(propertyName);
        Color model_val = tmpNumProp->getEdgeValue(graph_e);
        tmpNumProp = rule->getProperty<ColorProperty>(propertyName);
        Color rule_val = tmpNumProp->getEdgeValue(rule_e);
        comparison =
            PorgyTlpGraphStructure::compareSamePropertyValue<Color>(model_val, rule_val, op_comp);
      } else { // everything else (string, layout, size, graph,...), watch out
               // for unpredictable behaviour on non-atomic types
        comparison = PorgyTlpGraphStructure::compareSamePropertyValue<std::string>(
            currentModelProperty->getEdgeStringValue(graph_e),
            currentRuleProperty->getEdgeStringValue(rule_e), op_comp);
      }
      if (!comparison) {
// if(currentRuleProperty->getEdgeStringValue(rule_e) !=
// currentModelProperty->getEdgeStringValue(graph_e)) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
        cerr << __PRETTY_FUNCTION__ << ": edge matching problem: property " << propertyName
             << " not compatible" << endl;
#endif
        return false;
      }
    }
#ifdef PORGY_RULE_DEBUG_MESSAGES
    else {
      cerr << __PRETTY_FUNCTION__ << ": edge : test on property " << propertyName
           << " not performed" << endl;
    }
#endif
  }

  return true;
}

// Remarque : prend en compte les multi-aretes
bool PorgyTlpGraphStructure::checkEdgeState(const bool debug, Graph *modele, Graph *rule, const vector<edge> &rule_e,
                                            const std::vector<edge> &modele_e, bool exact,
                                            const matchpropvector &matchingPropertiesList,
                                            tlp::EdgeStaticProperty<edge> *edge_map) {
  if ((exact && rule_e.size() != modele_e.size()) || rule_e.size() > modele_e.size()) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
    cerr << __PRETTY_FUNCTION__ << ": rule and model sizes (resp. " << rule_e.size() << ":"
         << modele_e.size() << ") are incompatible (exact check=" << exact << ")." << endl;
#endif
    return false;
  }
  for (edge e_rule : rule_e) {
    bool match = false;
    for (edge e_modele : modele_e) {
      if (checkEdgeState(debug, modele, rule, e_rule, e_modele, matchingPropertiesList)) {
        if(debug)
            tlp::debug() << "matching verified between rule edge " << e_rule << " and model edge " << e_modele << endl;
          if (edge_map != nullptr)
          (*edge_map)[rule->edgePos(e_rule)] = e_modele;
        match = true;
      }
    }
    if (!match) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
      cerr << __PRETTY_FUNCTION__ << ": unable to match edge " << e_rule.id << endl;
#endif
      return false;
    }
  }
  return true;
}

void PorgyTlpGraphStructure::setPPropertyNameAttribute(Graph *g, const std::string &name) {
  g->setAttribute(PorgyConstants::P_NAME, name);
}

string PorgyTlpGraphStructure::getPPropertyNameAttribute(Graph *g) {
  string s;
  g->getAttribute(PorgyConstants::P_NAME, s);
  return s;
}

void PorgyTlpGraphStructure::setBanPropertyNameAttribute(Graph *g, const std::string &name) {
  g->setAttribute(PorgyConstants::BAN_NAME, name);
}

string PorgyTlpGraphStructure::getBanPropertyNameAttribute(Graph *g) {
  string s;
  g->getAttribute(PorgyConstants::BAN_NAME, s);
  return s;
}

// rulename set and get
void PorgyTlpGraphStructure::setRuleNameForSubGraph(tlp::Graph *g, const std::string &rulename) {
  g->setAttribute(PorgyConstants::RULENAME, rulename);
}

std::string PorgyTlpGraphStructure::getRuleNameForSubGraph(tlp::Graph *g) {
  string s;
  g->getAttribute(PorgyConstants::RULENAME, s);
  return s;
}

bool PorgyTlpGraphStructure::isPorgyGraph(const Graph *graph, string &errMsg) {
  if (graph->isEmpty())
    return true;
  // Check if the top-level graph structure is valid.
  Graph *rules_root = getRulesRoot(graph), *models_root = getModelsRoot(graph),
        *traces_root = getTraceRoot(graph), *traceMain = getMainTrace(graph);
  if (rules_root == nullptr) {
    errMsg =
        string("A required graph was not found (") + PorgyConstants::RULES_GRAPH_NAME + string(")");
    return false;
  } else if (models_root == nullptr) {
    errMsg = string("A required graph was not found (") + PorgyConstants::GRAPHS_GRAPH_NAME +
             string(")");
    return false;
  } else if (traces_root == nullptr) {
    errMsg =
        string("A required graph was not found (") + PorgyConstants::TRACE_GRAPH_NAME + string(")");
    return false;
  } else if (traceMain == nullptr) {
    errMsg = string("A required graph was not found (") + PorgyConstants::TRACEMAIN + string(")");
    return false;
  }

  // node types
  IntegerProperty *type = PortNodeBase::getTypeProperty(graph->getRoot());
  if (type == nullptr) {
    errMsg = "Cannot find the " + PorgyConstants::PORTNODE_TYPE + " Property";
    return false;
  }

  // verif que les propriétés sont correctement définies pour les sommets
  auto v_rules_root = rules_root->nodes();
  for (node n : v_rules_root) {
    if ((type->getNodeValue(n) != PorgyConstants::CENTER) &&
        (type->getNodeValue(n) != PorgyConstants::PORT)) {
      errMsg = "Property " + PorgyConstants::PORTNODE_TYPE +
               " not defined for every nodes (at least for node " + to_string(n.id) + ") in " +
               rules_root->getName();
#ifdef PORGY_RULE_DEBUG_MESSAGES
      errMsg += " (value: " + to_string(type->getNodeValue(n)) + ")";
#endif
      return false;
    }
  }

  // checking edge orientation between centre nodes and their ports for all graphs and rules (not derivation trees)
  // (center-->port)
  auto portnodesToCheck(PorgyTlpGraphStructure::getModelsRoot(graph)->nodes());
  auto rulesRootNodes(PorgyTlpGraphStructure::getRulesRoot(graph)->nodes());
  portnodesToCheck.insert(portnodesToCheck.end(),rulesRootNodes.begin(),rulesRootNodes.end());
  for (node n : portnodesToCheck) {
    if (type->getNodeValue(n) == PorgyConstants::CENTER) {
      for (edge e : graph->getRoot()->getInEdges(n)) {
        if (type->getNodeValue(graph->getRoot()->target(e)) != PorgyConstants::PORT) {
          errMsg = "Graph not valid.<br>Edge " + to_string(e) +
                   " is connected between two center nodes.";
          return false;
        }
        graph->getRoot()->reverse(e);
        // fix edge orientation of inner edge portnodes
        errMsg += "Edge " + to_string(e) + " going from node centre " + to_string(n) +
                  " is in the wrong direction<br>";
      }
    }
  }

  PortGraphRuleDecorator dec(rules_root);
  IntegerProperty *side = dec.getSideProperty();
  if (side == nullptr) {
    errMsg = "Cannot find the " + PorgyConstants::RULE_SIDE + " Property";
    return false;
  }

  // fix corrupted/outdated file
  // checking edge orientation between centre nodes and their ports
  // (center-->port)
  bool fix(false);
  // edges between center and port must go from center to port
  for (node n : portnodesToCheck) {
    if (type->getNodeValue(n) == PorgyConstants::CENTER) {
      for (edge e : graph->getRoot()->getInEdges(n)) {
        graph->getRoot()->reverse(e);
        fix = true;
      }
    }
  }

  for (edge e : rules_root->edges()) {
    if (side->getEdgeValue(e) == PorgyConstants::NO_SIDE) {
      errMsg = "Property " + PorgyConstants::RULE_SIDE +
               " not defined for every edges (at least for edge " + to_string(e.id) + ") in " +
               rules_root->getName();
      return false;
    }

  }
  for (node n : v_rules_root) {
    if (side->getNodeValue(n) == PorgyConstants::NO_SIDE) {
      errMsg = "Property " + PorgyConstants::RULE_SIDE +
               " not defined for every nodes (at least for node " + to_string(n.id) + ") in " +
               rules_root->getName();
      return false;
    }
  }

  // no attribute PorgyConstants::FAILURE_NODE_ADDED should exist here
  if (traceMain->existAttribute(PorgyConstants::FAILURE_NODE_ADDED)) {
    traceMain->removeAttribute(PorgyConstants::FAILURE_NODE_ADDED);
    fix = true;
  }

  //no subgraphs for rules at this point
  for (Graph *g : rules_root->subGraphs()) {
    fix = fix || (g->numberOfSubGraphs() > 0);
    auto v = g->subGraphs();
    for (Graph *gr : v) {
      g->delAllSubGraphs(gr);
    }   
  }

  // All nodes in rules_root must be at least in one rule
  for (node n : v_rules_root) {
    bool found(false);
    for (Graph *g : rules_root->subGraphs()) {
      if (g->isElement(n)) {
        found = true;
        break;
      }
    }
    if (!found) {
      rules_root->delNode(n, true);
      fix = true;
    }
    found = false;
  }

  // local properties on trace graphs
  fix = fix || fixTraceGraph(traceMain);
  for (Graph *g : traces_root->subGraphs()) {
    if (g != traceMain) {
      fix = fix || fixTraceGraph(g);
    }
  }

  if (fix) {
    errMsg = "Corrupted/outdated file fixed (hopefully)";
  }

  return true;
}

bool PorgyTlpGraphStructure::checkNode(tlp::Graph *g1, tlp::Graph *g2, tlp::node n1, tlp::node n2,
                                       const matchpropvector &matchingPropertiesList) {
  for (const matchproptuple &pName : matchingPropertiesList) {
    BooleanProperty *testCurrentProperty = get<1>(pName);
    if (testCurrentProperty->getNodeValue(n1)) {
      const string &propertyName = get<0>(pName);
      IntegerProperty *currentPropertyComparisonOp = get<2>(pName);
      PropertyInterface *currentModelProperty = get<4>(pName);
      PropertyInterface *currentRuleProperty = get<3>(pName);
      PorgyConstants::ComparisonOp op_comp =
          static_cast<PorgyConstants::ComparisonOp>(currentPropertyComparisonOp->getNodeValue(n1));
      // generalisation taking the comparison operator into account
      bool comparison = false;
      if (currentModelProperty->getTypename() ==
          tlp::BooleanProperty::propertyTypename) { // only for bool
                                                    // (necessary?)
        BooleanProperty *tmpBoolProp = g1->getProperty<BooleanProperty>(propertyName);
        bool model_val = tmpBoolProp->getNodeValue(n2);
        tmpBoolProp = g2->getProperty<BooleanProperty>(propertyName);
        bool rule_val = tmpBoolProp->getNodeValue(n1);
        comparison =
            PorgyTlpGraphStructure::compareSamePropertyValue<bool>(model_val, rule_val, op_comp);
      } else if ((currentModelProperty->getTypename() == tlp::DoubleProperty::propertyTypename) ||
                 ((currentModelProperty->getTypename() ==
                   tlp::IntegerProperty::propertyTypename))) { // specific
                                                               // treatment for
                                                               // double values
        NumericProperty *tmpNumProp = static_cast<NumericProperty *>(g1->getProperty(propertyName));
        double model_val = tmpNumProp->getNodeDoubleValue(n2);
        tmpNumProp = static_cast<NumericProperty *>(g2->getProperty(propertyName));
        double rule_val = tmpNumProp->getNodeDoubleValue(n1);
        comparison =
            PorgyTlpGraphStructure::compareSamePropertyValue<double>(model_val, rule_val, op_comp);
      } else if (currentModelProperty->getTypename() ==
                 tlp::ColorProperty::propertyTypename) { // specific treatment
                                                         // for double values
        ColorProperty *tmpNumProp = g1->getProperty<ColorProperty>(propertyName);
        Color model_val = tmpNumProp->getNodeValue(n2);
        tmpNumProp = g2->getProperty<ColorProperty>(propertyName);
        Color rule_val = tmpNumProp->getNodeValue(n1);
        comparison =
            PorgyTlpGraphStructure::compareSamePropertyValue<Color>(model_val, rule_val, op_comp);
      } else { // everything else (string, layout, size, graph,...), watch out
               // for unpredictable behaviour on non-atomic types
        comparison = PorgyTlpGraphStructure::compareSamePropertyValue<std::string>(
            currentModelProperty->getNodeStringValue(n2),
            currentRuleProperty->getNodeStringValue(n1), op_comp);
      }
      if (!comparison) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
        cerr << __PRETTY_FUNCTION__ << ": port problem: property " << propertyName << endl;
#endif
        return false;
      }
    }
  }
  return true;
}

bool PorgyTlpGraphStructure::isTraceGraph(const Graph *g) {
  assert(g != nullptr);
  Graph *traceRoot = getTraceRoot(g);
  return traceRoot == nullptr ? false : traceRoot->isDescendantGraph(g);
}

Graph *PorgyTlpGraphStructure::getMainTrace(const Graph *graph) {
  Graph *traceRoot = getTraceRoot(graph);
  return (traceRoot == nullptr) ? nullptr
                                : traceRoot->getDescendantGraph(PorgyConstants::TRACEMAIN);
}

// clone du graphe courant vers le nouveau graphe
Graph *PorgyTlpGraphStructure::clone(Graph *old_graph, string pName, string BanName) {
  Graph *root_models = PorgyTlpGraphStructure::getModelsRoot(old_graph);

  // calcul du nom du nouveau modèle
  unsigned num = 0;
  root_models->getAttribute<unsigned>(PorgyConstants::NUMBER,
                                      num); // recherche numéro du dernier modèle créé
  ++num;
  Graph *new_graph =
      old_graph->addCloneSubGraph(PorgyConstants::GRAPH_PREFIX + to_string(num), true, false);
  root_models->setAttribute<unsigned>(PorgyConstants::NUMBER, num);

  // copy viewLayout and viewSize
  LayoutProperty *layout_graph = old_graph->getProperty<LayoutProperty>("viewLayout");
  SizeProperty *size_graph = old_graph->getProperty<SizeProperty>("viewSize");
  LayoutProperty *layout_new_graph = new_graph->getLocalProperty<LayoutProperty>("viewLayout");
  SizeProperty *size_new_graph = new_graph->getLocalProperty<SizeProperty>("viewSize");
  layout_new_graph->copy(layout_graph);
  size_new_graph->copy(size_graph);

  // copy *Arity properties if necessary
  for (const string &arity : PorgyConstants::ArityProps) {
    if (old_graph->existLocalProperty(arity)) {
      IntegerProperty *pold = old_graph->getProperty<IntegerProperty>(arity);
      IntegerProperty *p = new_graph->getLocalProperty<IntegerProperty>(arity);
      p->copy(pold);
    }
  }

  BooleanProperty *new_element = new_graph->getLocalProperty<BooleanProperty>(PorgyConstants::NEW);
  new_element->setAllNodeValue(false);
  new_element->setAllEdgeValue(false);

  // copy P and Q properties if necessary
  if (!pName.empty()) {
    BooleanProperty *oldP = old_graph->getProperty<BooleanProperty>(pName);
    BooleanProperty *newP = new_graph->getLocalProperty<BooleanProperty>(pName);
    newP->copy(oldP);
  }
  if (!BanName.empty()) {
    BooleanProperty *oldBan = old_graph->getProperty<BooleanProperty>(BanName);
    BooleanProperty *newBan = new_graph->getLocalProperty<BooleanProperty>(BanName);
    newBan->copy(oldBan);
  }

  return new_graph;
}

bool PorgyTlpGraphStructure::hasLeaf(BooleanProperty *sel, Graph *traceMain) {
  bool hasLeaf(false), degok(true);
  for (node n : sel->getNodesEqualTo(true, traceMain)) {
    if (traceMain->outdeg(n) > 1)
      degok = false;
    if (traceMain->outdeg(n) == 0)
      hasLeaf = true;
  }
  return hasLeaf && degok;
}

bool PorgyTlpGraphStructure::graphIsASimplePath(Graph *graph) {
  // number of edges should be numberOfNodes-1
  unsigned nbEdges = graph->numberOfEdges();
  if (nbEdges != graph->numberOfNodes() - 1)
    return false;

  // node's degree should be 1 or 2
  unsigned int maxdeg = 0, mindeg = INT_MAX;
  const std::vector<node> &nodes = graph->nodes();
  for (node n : nodes) {
    maxdeg = std::max(maxdeg, graph->deg(n));
    mindeg = std::min(mindeg, graph->deg(n));
  }
  if ((maxdeg > 2) || (mindeg < 1))
    return false;

  // from one source (indeg=0), we must reach all nodes
  node n = graph->getSource();
  unordered_map<node, bool> reach;
  tlp::markReachableNodes(graph, n, reach, nbEdges, DIRECTED);
  if (reach.size() != graph->numberOfNodes() - 1)
    return false;

  return true;
}

bool PorgyTlpGraphStructure::hasProperty(Graph *g) {
  std::unordered_set<std::string> propertiesNames;
  getMatchingPropertyNames(g, propertiesNames);
  return !propertiesNames.empty();
}

void PorgyTlpGraphStructure::getMatchingPropertyNames(
    tlp::Graph *g, std::unordered_set<std::string> &propertiesNames, bool comp) {
  std::vector<std::string> matchingPropertiesList;
  if (g->getRoot()->existAttribute(PorgyConstants::MATCHING_PROPERTIES)) {
    g->getRoot()->getAttribute<std::vector<std::string>>(PorgyConstants::MATCHING_PROPERTIES,
                                                         matchingPropertiesList);
    if (!comp)
      propertiesNames.insert(matchingPropertiesList.begin(), matchingPropertiesList.end());
    else {
      for (const string &p : matchingPropertiesList) {
        propertiesNames.insert(p + PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX);
      }
    }
  }
}

void PorgyTlpGraphStructure::setMatchingPropertyNames(
    tlp::Graph *g, const std::unordered_set<std::string> &propertiesNames) {
  vector<string> vect(propertiesNames.begin(), propertiesNames.end());
  // Ensure specific matching properties are there
  Graph *rules_root(getRulesRoot(g));
  for (auto name : vect) {
     rules_root->getLocalProperty<BooleanProperty>(name + PorgyConstants::MATCHING_PROPERTIES_SUFFIX);
    rules_root->getLocalProperty<IntegerProperty>(name + PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX);
  }
  g->getRoot()->setAttribute<std::vector<std::string>>(PorgyConstants::MATCHING_PROPERTIES, vect);
}

void PorgyTlpGraphStructure::getPropertiesToDisplay(
    tlp::Graph *g, std::unordered_set<std::string> &propertiesNames) {
  std::vector<std::string> propertyList;
  if (g->getRoot()->existAttribute(PorgyConstants::PROPERTIES_TO_DISPLAY)) {
    g->getRoot()->getAttribute<std::vector<std::string>>(PorgyConstants::PROPERTIES_TO_DISPLAY,
                                                         propertyList);
  }

  for (auto st : propertyList) {
    propertiesNames.insert(st);
  }
}

void PorgyTlpGraphStructure::setPropertiesToDisplay(
    tlp::Graph *g, const std::unordered_set<std::string> &propertiesNames) {
  vector<string> vect(propertiesNames.begin(), propertiesNames.end());
  g->getRoot()->setAttribute<std::vector<std::string>>(PorgyConstants::PROPERTIES_TO_DISPLAY, vect);
}
