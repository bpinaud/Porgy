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
#include <tulip/Algorithm.h>
#include <tulip/BooleanProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/ForEach.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>
#include <portgraph/Trace.h>

using namespace std;
using namespace tlp;

const static std::string SAVEID = "saveId";

static node findNode(const int n, IntegerProperty *prop, Graph *g) {
  node nd;
  Iterator<node> *itn = prop->getNodesEqualTo(n, g);
  if (itn->hasNext())
    nd = itn->next();
  delete itn;
  return nd;
}

class ApplyRule : public tlp::Algorithm {
  bool _debug;

public:
  PLUGININFORMATION(PorgyConstants::APPLY_RULE, "Bruno Pinaud", "09/04/10", "Comments", "1.0.1",
                    PorgyConstants::CATEGORY_NAME_INTERNAL)
  ApplyRule(const PluginContext *context) : Algorithm(context), _debug(false) {
    addDependency(PorgyConstants::REDRAW_GRAPH, "1.0");
    addDependency(PorgyConstants::APPLY_RULE_ALGORITHM, "1.0");

    addInParameter<bool>(PorgyConstants::applyConnectedComponentPacking, "", "true");
    addInParameter<BooleanProperty>(PorgyConstants::POSITION, "", "", false);
    addInParameter<BooleanProperty>(PorgyConstants::BAN, "", "", false);
    addInParameter<PorgyConstants::state>(PorgyConstants::isCondition, "", "false");
    addInParameter<bool>(PorgyConstants::layoutNewModel, "", "false");
    addInParameter<bool>(PorgyConstants::DEBUG, "", "false");

    addOutParameter<Graph *>(PorgyConstants::newData, "", "", false);
  }

  ///////////////
  // graph => LHS instance to use for rewriting
  bool run() override {
    pluginProgress->showPreview(false);
    pluginProgress->setComment("rewriting graph " + graph->getSuperGraph()->getName());

    Graph *old_graph = graph->getSuperGraph();
    BooleanProperty *Pos = nullptr;
    BooleanProperty *Ban = nullptr;
    PorgyConstants::state st = PorgyConstants::state::OTHER;
    if (dataSet != nullptr) {
      dataSet->get<BooleanProperty *>(PorgyConstants::POSITION, Pos);
      dataSet->get<BooleanProperty *>(PorgyConstants::BAN, Ban);
      dataSet->get(PorgyConstants::isCondition, st);
      dataSet->get(PorgyConstants::DEBUG, _debug);
    }
    Graph *_rule = PorgyTlpGraphStructure::findRule(
        graph, PorgyTlpGraphStructure::getRuleNameForSubGraph(graph));
#ifdef PORGY_RULE_DEBUG_MESSAGES
    cerr << __PRETTY_FUNCTION__ << "Applying rewriting on " << graph->getName()
         << " (parent: " << graph->getSuperGraph()->getName() << ") for rule " << _rule->getName()
         << endl;
#else
    if (_debug)
      tlp::debug() << __PRETTY_FUNCTION__ << "Applying rewriting on " << graph->getName()
                   << " (parent: " << graph->getSuperGraph()->getName() << ") for rule "
                   << _rule->getName() << endl;
#endif

    Graph *new_graph = PorgyTlpGraphStructure::clone(
        old_graph, (Pos != nullptr) ? Pos->getName() : "", (Ban != nullptr) ? Ban->getName() : "");
    assert(_rule != nullptr);
    // the saveId property is used to save the id of the RHS nodes to have a
    // link between RHS and the graph being rewritten
    Observable::holdObservers();
    IntegerProperty *saveId = _rule->getLocalProperty<IntegerProperty>(SAVEID);
    // copying rule RHS to new_graph
    BooleanProperty newelt(_rule);

    newelt.setAllNodeValue(false);
    newelt.setAllEdgeValue(false);
    // select all elements of RHS
    PortGraphRuleDecorator dec(_rule);
    for (auto n : _rule->nodes()) {
      saveId->setNodeValue(n, n.id);
      if (dec.getSide(n) == PorgyConstants::SIDE_RIGHT)
        newelt.setNodeValue(n, true);
    }
    for (auto e : _rule->edges()) {
      saveId->setEdgeValue(e, e.id);
      if (dec.getSide(e) == PorgyConstants::SIDE_RIGHT) {
        newelt.setEdgeValue(e, true);
      }
    }

    BooleanProperty *new_strategy_application =
        new_graph->getLocalProperty<BooleanProperty>(PorgyConstants::NEW_STRATEGY_APPLICATION);
    BooleanProperty *newmodel_newelt =
        new_graph->getLocalProperty<BooleanProperty>(PorgyConstants::NEW);
    // copy RHS to the new graph
    tlp::copyToGraph(new_graph, _rule, &newelt, newmodel_newelt);
    // restore interface
    PortGraphRule PGMrule(_rule);
    IntegerProperty *oldId = new_graph->getLocalProperty<IntegerProperty>(SAVEID);
    IntegerProperty *tag_prop = graph->getProperty<IntegerProperty>(PorgyConstants::TAG);
    PortGraphModel lhs_instance_portgraph(graph);
    unordered_set<edge> to_restore; // use a set to avoid duplication of edges
                                    // if both source and target are in the LHS
                                    // instance
    for (auto pn : lhs_instance_portgraph.getPortNodes()) {
      for (auto p2 : pn->getPorts()) {
        node port_rule_left(tag_prop->getNodeValue(p2->getNode()));
        // check if the tag is valid (it should be a node in the rule)
        assert(_rule->isElement(port_rule_left));

        // copying properties of ports where the *_isused properties are set to
        // false in RHS
        // RHS has been copied to the new graph. So all RHS properties are
        // present in new_graph

        vector<string> matchingPropertiesList;
        if (graph->getRoot()->getAttribute<vector<string>>(PorgyConstants::MATCHING_PROPERTIES,
                                                           matchingPropertiesList)) {
          const forward_list<node> flist(PGMrule.findRightEquiv(port_rule_left));
          for (auto right_equiv_rule : flist) {
            if (right_equiv_rule.isValid()) {
              node right_equiv_model = findNode(right_equiv_rule, oldId, new_graph);
              for (auto propertyName : matchingPropertiesList) {
                BooleanProperty *prop_isused = new_graph->getProperty<BooleanProperty>(
                    propertyName + PorgyConstants::MATCHING_PROPERTIES_SUFFIX);
                if (prop_isused->getNodeValue(right_equiv_model) == false) {
                  PropertyInterface *prop = graph->getProperty(propertyName);
                  PropertyInterface *prop_new = new_graph->getProperty(propertyName);
                  prop_new->copy(right_equiv_model, p2->getNode(), prop);
                }
              }
            }
          }
        }

        // looking for edges
        // if the difference between the degree of the LHS instance nodes and
        // the model nodes is greater than 0, there are edges to save
        if (old_graph->deg(p2->getNode()) - graph->deg(p2->getNode()) > 0) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
          cerr << "There are edges connected to port " << p2->getNode()
               << " in the LHS instance interface to preserve" << endl;
#endif
          for (auto e : old_graph->allEdges(p2->getNode())) {
            // we do not want edges between center and port and edges present in
            // the LHS instance
            if (old_graph->isElement(e)) {
              const pair<node, node> &ends = old_graph->ends(e);
              if (!(PortNode::isCenter(ends.first, old_graph) ||
                    PortNode::isCenter(ends.second, old_graph) || graph->isElement(e))) {
                to_restore.insert(e);
              }
            }
          }
        }
      }
    }

    // restore edges
    for (auto e_to_restore : to_restore) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
      cerr << "restoring LHS interface for edge " << e_to_restore << endl;
#endif
      const pair<node, node> &ends = old_graph->ends(e_to_restore);
      node srcn(ends.first), dstn(ends.second);
      bool green_edge = false;
      vector<pair<node, node>> new_edges;
      if (graph->isElement(srcn)) {
        if (graph->isElement(dstn)) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
          cerr << "edge (both source and target) is connected to the LHS "
                  "instance but it was not matched as a part of it"
               << endl;
#endif
          const forward_list<node> flist(
              PGMrule.findRightEquiv(node(tag_prop->getNodeValue(srcn))));
          // getting src and dst for each new edges
          forward_list<node> dst, src;
          for (auto n : flist) {
            src.push_front(findNode(n, oldId, new_graph));
          }
          const forward_list<node> flist2 =
              PGMrule.findRightEquiv(node(tag_prop->getNodeValue(dstn)));
          for (auto n : flist2) {
            dst.push_front(findNode(n, oldId, new_graph));
          }
          // cartesian product to get each new edge
          for (auto s : src) {
            for (auto d : dst) {
              new_edges.push_back(make_pair(s, d));
            }
          }
        } else {
#ifdef PORGY_RULE_DEBUG_MESSAGES
          cerr << "Only edge source is in the LHS instance" << endl;
#endif
          node tmp_node(tag_prop->getNodeValue(srcn));
          const forward_list<node> n1(PGMrule.findRightEquiv(tmp_node));
          forward_list<node> src;
          if (!n1.empty()) {
            for (auto s : n1) {
              if (dec.getSide(s) == PorgyConstants::SIDE_LEFT) {
                cerr << "green edge detected" << endl;
                green_edge = true;
                node sr = findNode(s, tag_prop, graph);
                node nn;
                forEach (nn, old_graph->getInOutNodes(sr)) {
                  if (PortBase::isPort(nn, old_graph)) {
                    src.push_front(nn);
                    break;
                  }
                }
                if (new_graph->hasEdge(nn, dstn, false)) {
                  src.pop_front();
                  cerr << "edge already restored, skipping..." << endl;
                  continue;
                }
              } else
                src.push_front(findNode(s, oldId, new_graph));
            }
          } else {
            tlp::error() << "Cannot restore edge" << e_to_restore << endl;
            continue;
          }
          for (auto s : src)
            new_edges.push_back(make_pair(s, dstn));
        }
      } else {
#ifdef PORGY_RULE_DEBUG_MESSAGES
        cerr << "Only edge target is in the LHS instance " << endl;
#endif
        node tmp_node(tag_prop->getNodeValue(dstn));
        const forward_list<node> n1(PGMrule.findRightEquiv(tmp_node));
        forward_list<node> dst;
        if (!n1.empty()) {
          for (auto d : n1) {
            if (dec.getSide(d) == PorgyConstants::SIDE_LEFT) {
              cerr << "green edge detected" << endl;
              green_edge = true;
              node ds = findNode(d, tag_prop, graph);
              node nn;
              forEach (nn, old_graph->getInOutNodes(ds)) {
                if (PortBase::isPort(nn, old_graph)) {
                  dst.push_front(nn);
                  break;
                }
              }
              if (new_graph->hasEdge(srcn, nn, false)) {
                dst.pop_front();
                tlp::error() << "edge already restored, skipping..." << endl;
                continue;
              }
            } else
              dst.push_front(findNode(d, oldId, new_graph));
          }
        } else {
          tlp::error() << "Cannot restore edge" << e_to_restore << endl;
          continue;
        }
        for (const node ds : dst)
          new_edges.push_back(make_pair(srcn, ds));
      }
#ifdef PORGY_RULE_DEBUG_MESSAGES
      for (auto ed : new_edges) {
        cerr << "Adding edge between " << ed.first << " and " << ed.second << endl;
      }
#endif
      vector<edge> new_e;
      new_graph->addEdges(new_edges, new_e);
      // restoring properties
      for (auto en : new_e) {
        if (green_edge) {
          newmodel_newelt->setEdgeValue(en, true);
          green_edge = false;
        } else {
            //TODO: not good! We should only copy properties checked (*_isused) in RHS (like for nodes)
          for (auto prop:new_graph->getObjectProperties()) {
              prop->copy(en, e_to_restore, prop);
          }
        }
      }
    }

    // handle M and N
    // if the M property is true, put elements inside the current P property
    // if the N property is true, put elements inside the current Ban property
    PortGraphModel pgm(new_graph);
    BooleanProperty *newP = nullptr, *newM = nullptr, *newN = nullptr, *newBan = nullptr;
    if (Pos != nullptr) {
      newP = new_graph->getProperty<BooleanProperty>(Pos->getName());
      newM = new_graph->getProperty<BooleanProperty>(PorgyConstants::M);
    }
    if (Ban != nullptr) {
      newN = new_graph->getProperty<BooleanProperty>(PorgyConstants::N);
      newBan = new_graph->getProperty<BooleanProperty>(Ban->getName());
    }

    for (auto e : pgm.getEdges()) {
      if (Pos != nullptr)
        newP->copy(e, e, newM);
      if (Ban != nullptr)
        newBan->copy(e, e, newN);
      if (newmodel_newelt->getEdgeValue(e) == true)
        new_strategy_application->setEdgeValue(e, true);
    }
    for (auto pn : pgm.getPortNodes()) {
      if (Pos != nullptr && newM->getNodeValue(pn->getCenter())) {
        pn->select(true, newP);
      }
      if (Ban != nullptr && newN->getNodeValue(pn->getCenter())) {
        pn->select(true, newBan);
      }
      if (newmodel_newelt->getNodeValue(pn->getCenter()))
        pn->select(true, new_strategy_application);
    }

    set<node> setnewnode;
    node n;
    forEach (n, newmodel_newelt->getNodesEqualTo(true)) {
      if (PortNode::isCenter(n, new_graph))
        setnewnode.insert(n);
    }

    LayoutProperty *layout_old_graph = graph->getProperty<LayoutProperty>("viewLayout");
    SizeProperty *size_old_graph = graph->getProperty<SizeProperty>("viewSize");
    StringProperty *label_old_graph = graph->getProperty<StringProperty>("viewLabel");

    StringProperty *label_new_graph = new_graph->getProperty<StringProperty>("viewLabel");
    SizeProperty *size_new_graph = new_graph->getProperty<SizeProperty>("viewSize");
    LayoutProperty *layout_new_graph = new_graph->getProperty<LayoutProperty>("viewLayout");

    // instead of leaving new nodes at (0,0,0), put them where the old ones are
    // if a LHS node linked to a RHS node
    BooleanProperty used(graph);
    used.setAllNodeValue(false);
    for (auto p : lhs_instance_portgraph.getPortNodes()) {
      for (auto po : p->getPorts()) {
        // use a random RHS equivalent node found (should test all nodes and
        // take the best one instead)
        const forward_list<node> equiv(
            PGMrule.findRightEquiv(node(tag_prop->getNodeValue(po->getNode()))));
        if (!equiv.empty()) {
          node p_rhs(equiv.front());
          const PortNode *pp = PGMrule[p_rhs];
          node new_node(findNode(pp->getCenter(), oldId, new_graph)); // replacing node from RHS
          layout_new_graph->setNodeValue(new_node, layout_old_graph->getNodeValue(p->getCenter()));
          size_new_graph->setNodeValue(new_node, size_old_graph->getNodeValue(p->getCenter()));
          used.setNodeValue(p->getCenter(), true);
          setnewnode.erase(new_node);
          break;
        }
      }
    }

    // LHS and RHS are not linked. Try to put each new node at the same position
    // of an old node with the same label
    vector<set<node>::const_iterator> toremove;
    for (auto it = setnewnode.cbegin(); it != setnewnode.cend(); ++it) {
      node no(*it);
      // looking for an old node with the same label
      stableForEach(n, used.getNodesEqualTo(false)) {
        if (PortGraphDecorator::isCenter(graph, n)) {
          if (label_old_graph->getNodeValue(n) == label_new_graph->getNodeValue(no)) {
            layout_new_graph->setNodeValue(no, layout_old_graph->getNodeValue(n));
            size_new_graph->setNodeValue(no, size_old_graph->getNodeValue(n));
            used.setNodeValue(n, true);
            toremove.push_back(it);
            break;
          }
        }
      }
    }
    // to avoid invalidating iterators, setnewnode must be updated after
    // iterating on it
    for (auto it : toremove)
      setnewnode.erase(it);
    toremove.clear();

    // for the remaining nodes, put them at the same position of an old node
    for (auto it = setnewnode.cbegin(); it != setnewnode.cend(); ++it) {
      node no(*it);
      stableForEach(n, used.getNodesEqualTo(false)) {
        layout_new_graph->setNodeValue(no, layout_old_graph->getNodeValue(n));
        size_new_graph->setNodeValue(no, size_old_graph->getNodeValue(n));
        used.setNodeValue(n, true);
        toremove.push_back(it);
        break;
      }
    }
    for (auto it : toremove)
      setnewnode.erase(it);
    if (!setnewnode.empty())
      cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": TODO!! " << endl;

    // rule algorithm
    std::string text("");
    if (_rule->getAttribute(PorgyConstants::RULE_ALGORITHM,
                            text)) { // no need to apply rule_algorithm plugin
                                     // if there is no algorithm
#ifdef PORGY_RULE_DEBUG_MESSAGES
      cerr << "Applying rule algorithm" << endl;
#endif
      BooleanProperty *lhs_rule_selection =
          graph->getLocalProperty<BooleanProperty>(PorgyConstants::NEW);
      lhs_rule_selection->setAllNodeValue(true);
      lhs_rule_selection->setAllEdgeValue(true);
      tlp::DataSet algoDataSet;
      algoDataSet.set<string>("Instructions", text);
      algoDataSet.set<string>("Mode", "execute");
      algoDataSet.set<tlp::Graph *>("Lhs", graph);
      algoDataSet.set<string>("RhsMappingProperty", SAVEID);
      algoDataSet.set<string>("LhsMappingProperty", PorgyConstants::TAG);
      algoDataSet.set<string>("RhsSelectionProperty", PorgyConstants::NEW);
      algoDataSet.set<string>("LhsSelectionProperty", PorgyConstants::NEW);
      std::string err("");
      if (!new_graph->applyAlgorithm(PorgyConstants::APPLY_RULE_ALGORITHM, err, &algoDataSet))
        tlp::warning() << "Unable to parse the algorithm: " << err << " - step ignored"
                       << std::endl;
    }

    // removing remaining nodes from the LHS instance (edges will be removed
    // automatically)
    new_graph->delNodes(graph->nodes());

    // nettoyage
    // new_graph->delLocalProperty(SAVEID);
    _rule->delLocalProperty(SAVEID);
    if (new_graph->existLocalProperty(PorgyConstants::ARITY))
      new_graph->delLocalProperty(PorgyConstants::ARITY);
    if (new_graph->existLocalProperty(PorgyConstants::M))
      new_graph->delLocalProperty(PorgyConstants::M);
    if (new_graph->existLocalProperty(PorgyConstants::N))
      new_graph->delLocalProperty(PorgyConstants::N);
    if (new_graph->existLocalProperty(PorgyConstants::W))
      new_graph->delLocalProperty(PorgyConstants::W);
    if (new_graph->existLocalProperty(PorgyConstants::RULE_SIDE))
      new_graph->delLocalProperty(PorgyConstants::RULE_SIDE);

    vector<string> matchingPropertiesList;
    if (graph->getRoot()->getAttribute<vector<string>>(PorgyConstants::MATCHING_PROPERTIES,
                                                       matchingPropertiesList)) {
      for (auto propertyName : matchingPropertiesList) {
        string name(propertyName + PorgyConstants::MATCHING_PROPERTIES_SUFFIX);
        if (new_graph->existLocalProperty(name))
          new_graph->delLocalProperty(name);
      }
    }

    if (st != PorgyConstants::state::IF) {
      Trace traceobj(PorgyTlpGraphStructure::getMainTrace(graph));
      node tgt = traceobj.createMetaNode(new_graph);
      string pos="", ban="";
      if(Pos!=nullptr)
          pos = Pos->getName();
      if(Ban!=nullptr)
          ban = Ban->getName();
      traceobj.addSimpleTransformationEdge(traceobj.findNode(old_graph->getName()), tgt, pos, ban, graph, _rule->getId());

      bool draw_model = true;
      if (dataSet != nullptr)
        dataSet->get<bool>(PorgyConstants::layoutNewModel, draw_model);
      if (draw_model) {
        string errMsg;
        DataSet ds;
        ds.set("Layout", PGMrule.getLayoutAlgorithm());
        if (!new_graph->applyPropertyAlgorithm(PorgyConstants::REDRAW_GRAPH,
                                               new_graph->getProperty<LayoutProperty>("viewLayout"),
                                               errMsg, &ds,pluginProgress))
          cerr << "cannot apply layout algorithm: " << errMsg << endl;
      }
    }
#ifdef PORGY_RULE_DEBUG_MESSAGES
    else
      cerr << __PRETTY_FUNCTION__ << ": if or while test condition detected => "
                                     "no update to the trace graph"
           << endl;
#endif
    Observable::unholdObservers();
    dataSet->set(PorgyConstants::newData, new_graph);
    if (_debug)
      tlp::debug() << "Rule " << _rule->getName() << " applied" << endl;
    return true;
  }
};

PLUGIN(ApplyRule)
