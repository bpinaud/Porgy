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
#include <portgraph/Port.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>

#include <tulip/Algorithm.h>
#include <tulip/BooleanProperty.h>
#include <tulip/ConnectedTest.h>
#include <tulip/DoubleProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/StaticProperty.h>

#include <unordered_set>

using namespace tlp;
using namespace std;

//////////////
PortGraphModel::PortGraphModel(tlp::Graph *g) : PortGraph(g) {}

//////////////
tlp::BooleanProperty *PortGraphModelDecorator::getSelectionProperty() {
  return PorgyTlpGraphStructure::getModelsRoot(graph_component)
      ->getLocalProperty<BooleanProperty>("viewSelection");
}

bool PortGraphModelDecorator::redraw(const std::string &plugin, tlp::DataSet &parameters,
                                     std::string &errMsg, tlp::PluginProgress *progress) {
  bool res = false;
  std::list<std::string> plugins = layoutPluginsList();
  if (std::find(plugins.begin(), plugins.end(), plugin) == plugins.end()) {
    errMsg = "The choosen plugin is not in the list";
  } else {
    if (numberOfNodes() > 1) {
      Observable::holdObservers();
      res = graph_component->applyPropertyAlgorithm(
          plugin, graph_component->getProperty<LayoutProperty>("viewLayout"), errMsg, &parameters, progress);
      Observable::unholdObservers();
    }
  }
  return res;
}

//////////////

/**
 * @brief The PartialMap class
 * Used to find instances of the LHS of a rule
 * @param current : currently mapped vertices. At the beginning, no vertex is
 * mapped. current is extended progressively
 * @param M : Matrix given in adgency list.
 * Gives the possible mappings for each node of the LHS of the rule
 */

class PartialMap {
  typedef unordered_set<node> possiblemap;
  typedef NodeStaticProperty<node> nodemap;
  typedef EdgeStaticProperty<edge> edgemap;
  typedef NodeStaticProperty<possiblemap> partialmap;

  const PortGraph *left;
  Graph *g_left;
  const PortGraph *model;
  Graph *g_model;

  // matrix containing the possibilities for the mapping
  partialmap *M;

  // number of nodes already mapped
  unsigned nb_mapped;

  // current mapping of the chosen nodes
  nodemap *current;
  edgemap *current_edge;

  // PluginProgress for feedback on the avancement
  PluginProgress *pp;
  int progress;
  int progress_step;
  int max_progress;
  bool _edgeOrientation, _debug, _exact;
  PorgyTlpGraphStructure::matchpropvector matchingPropertiesList;

public:
  PartialMap(PortGraph *l, PortGraph *model, PluginProgress *pp, const bool edgeOrientation,
             bool debug, bool exact)
      : left(l), g_left(l->getGraph()), model(model), g_model(model->getGraph()),
        M(new partialmap(g_left)), nb_mapped(0), current(new nodemap(g_left)),
        current_edge(new edgemap(g_left)), pp(pp), progress(0), progress_step(INT_MAX),
        max_progress(INT_MAX), _edgeOrientation(edgeOrientation), _debug(debug), _exact(exact) {}

  ~PartialMap() {
    delete M;
    delete current;
    delete current_edge;
  }

  explicit PartialMap(const PartialMap &partmap)
      : left(partmap.left), g_left(partmap.g_left), model(partmap.model), g_model(partmap.g_model),
        M(new partialmap(g_left)), nb_mapped(partmap.nb_mapped), current(new nodemap(g_left)),
        current_edge(new edgemap(g_left)), pp(partmap.pp), progress(partmap.progress),
        progress_step(partmap.progress_step), max_progress(partmap.max_progress),
        _edgeOrientation(partmap._edgeOrientation), _debug(partmap._debug), _exact(partmap._exact),
        matchingPropertiesList(partmap.matchingPropertiesList) {
    *M = *partmap.M;
    *current = *partmap.current;
    *current_edge = *partmap.current_edge;
  }

  bool initialisation(BooleanProperty *banp);
  void display();
  bool refine();
  void backtrack(int numtofind, int &nbr_found, BooleanProperty *pos, BooleanProperty *banp,
                 Graph *left_anti, vector<Graph *> &graph_names);

private:
  unsigned chooseNode() const;
  bool verifyPos(BooleanProperty *pos) const;
  bool verifyIso() const;
  bool verifyAntiEdge(Graph *left_anti) const;
  Graph *addSub(int num_solutions, BooleanProperty *position, BooleanProperty *banp) const;
};

bool PartialMap::initialisation(BooleanProperty *banp) {
  // Properties to match on
  vector<string> m;
  g_left->getRoot()->getAttribute<std::vector<std::string>>(PorgyConstants::MATCHING_PROPERTIES, m);
  for (const string &pname : m) {
    BooleanProperty *testCurrentProperty =
        g_left->getProperty<BooleanProperty>(pname + PorgyConstants::MATCHING_PROPERTIES_SUFFIX);
    IntegerProperty *currentPropertyComparisonOp = g_left->getProperty<IntegerProperty>(
        pname + PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX);
    PropertyInterface *pmodel = g_model->getProperty(pname);
    PropertyInterface *prule = g_left->getProperty(pname);
    matchingPropertiesList.push_back(
        make_tuple(pname, testCurrentProperty, currentPropertyComparisonOp, prule, pmodel));
  }

  // Begin with adding the portNodes possible images
  auto v_portnode = left->getPortNodes();
  for (PortNode *rule_pn : v_portnode) {
    unsigned rule_pos = left->getGraph()->nodePos(rule_pn->getCenter());
#ifdef PORGY_RULE_DEBUG_MESSAGES
    cerr << __PRETTY_FUNCTION__ << " : looking for candidates for PortNode :" << rule_pn << " "
         << rule_pn->getName() << endl;
#endif
    for (PortNode *model_pn : model->getPortNodes(banp, true)) {
      // only add compatible PortNodes (ie : same name, same state, compatible
      // degrees)
      if (PortNode::check(rule_pn, model_pn, _exact, matchingPropertiesList)) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
        cerr << __PRETTY_FUNCTION__ << " Found candidate:" << model_pn->getCenter() << " "
             << model_pn->getName() << endl;
#endif
        possiblemap &m = (*M)[rule_pos];
        m.insert(model_pn->getCenter());
        // PortNode centers ok. Check ports
        for (Port *rule_port : rule_pn->getPorts()) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
          if (!_exact)
            cerr << __PRETTY_FUNCTION__ << "Port considered: " << rule_port->getNode() << " "
                 << rule_port->getName() << " of arity " << rule_port->getArity() << endl;
          else
            cerr << __PRETTY_FUNCTION__ << "Port considered: " << rule_port->getNode() << " "
                 << rule_port->getName() << endl;
#endif
          for (auto n : (*M)[rule_pos]) {
            for(auto p2:g_model->getOutNodes(n)) {
              const Port *model_port = model->getPortContainer(p2);
              if (Port::check(rule_port, model_port, _exact, matchingPropertiesList)) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
                cerr << __PRETTY_FUNCTION__ << "Candidate mapping: " << p2 << endl;
#endif
                possiblemap &m = (*M)[rule_port->getNode()];
                m.insert(p2);
              }
              //       #ifdef PORGY_RULE_DEBUG_MESSAGES
              //                           else {
              //                               cerr << __PRETTY_FUNCTION__ <<
              //                               "Candidate refused: " << p2 << "
              //                               " << model_port->getName() << "
              //                               of degree :" << model_port->deg()
              //                               << endl;
              //                           }
              //       #endif
            }
          }
        }
      }
      //#ifdef PORGY_RULE_DEBUG_MESSAGES
      //            else {
      //                cerr <<__PRETTY_FUNCTION__ << " : Refused candidate :"
      //                << model_pn->getCenter() << " " << model_pn->getName()
      //                << " of state " << model_pn->getState() << endl ;
      //            }
      //#endif
    }
  }

  bool b = refine();
#ifdef PORGY_RULE_DEBUG_MESSAGES
  cerr << __PRETTY_FUNCTION__ << ": Initialisation end" << endl;
#endif
  // End with a call to refine
  return b;
}

// Debug function to display the content of current and of M
void PartialMap::display() {
  tlp::debug() << "Display of the current partialmap" << endl;
  tlp::debug() << "Number of nodes mapped for now: " << nb_mapped << endl;
  const vector<node> &nodes = g_left->nodes();
  unsigned nb_nodes(nodes.size());
  if (nb_mapped != 0) {
    tlp::debug() << "Current mapping: " << endl;
    for (unsigned i = 0; i < nb_nodes; ++i) {
      tlp::debug() << "node " << nodes[i] << " to node " << (*current)[i] << endl;
    }
  }
  tlp::debug() << "Possibilities for other nodes:" << endl;
  for (unsigned i = 0; i < nb_nodes; ++i) {
    possiblemap &m = (*M)[i];
    tlp::debug() << "Rule candidates for " << nodes[i] << " are: ";
    for (auto &nd : m) {
      tlp::debug() << nd << " ; ";
    }
    tlp::debug() << endl;
  }
}

unsigned PartialMap::chooseNode() const {
  unsigned int k = UINT_MAX;
  unsigned n = UINT_MAX;
  const vector<node> &nodes = g_left->nodes();
  unsigned nb_node(nodes.size());
  for (unsigned pos = 0; pos < nb_node; ++pos) {
    possiblemap &ma = (*M)[pos];
    assert(!ma.empty());
    if (!(*current)[pos].isValid() && ma.size() < k) {
      k = ma.size();
      n = pos;
    }
  }
#ifdef PORGY_RULE_DEBUG_MESSAGES
  if (!nodes[n].isValid()) {
    cerr << __PRETTY_FUNCTION__ << " ERROR: All vertices already mapped" << endl;
    assert(false);
  }
#endif

  return n;
}

/**
 * @brief PartialMap::verifyPos Check position set in the graph
 * @param pos
 * @return
 */
bool PartialMap::verifyPos(BooleanProperty *pos) const {
  // no position. That's fine
  if (pos == nullptr)
    return true;
  // W set define on the rule. We have to check if the image of the nodes/edges
  // from LHS with W=true are in Position
  BooleanProperty *W = g_left->getProperty<BooleanProperty>(PorgyConstants::W);
  for(auto n:W->getNodesEqualTo(true, g_left)) {
    assert(g_left->isElement(n));
    int posn = g_left->nodePos(n);
    if (!pos->getNodeValue((*current)[posn]))
      return false;
  }

  // Position defined without W. We checked if the intersection between the LHS
  // morphism and pos is not empty
  unsigned cursize(current->size());
  for (unsigned i = 0; i < cursize; ++i) {
    if (pos->getNodeValue((*current)[i])) {
      return true;
    }
  }
  return false;
}

bool PartialMap::verifyAntiEdge(Graph *left_anti) const {
  // anti-edges present. Must check that there is no edges between the image of
  // the extremities of the edge (port)
  if (left_anti != nullptr) {
    if (_debug)
      tlp::debug() << ": Checking anti-edges..." << endl;
    for (const edge e : left_anti->edges()) {
      const pair<node, node> &ends = left_anti->ends(e);
      vector<edge> v;
      v.push_back(e);
      node img_src((*current)[g_left->nodePos(ends.first)]);
      node img_tgt((*current)[g_left->nodePos(ends.second)]);
      std::vector<tlp::edge> model_edges = g_model->getEdges(img_src, img_tgt, _edgeOrientation);
      if (PorgyTlpGraphStructure::checkEdgeState(g_model, g_left, v, model_edges, false,
                                                 matchingPropertiesList))
        return false;
    }
  }
  return true;
}

bool PartialMap::verifyIso() const {
  // We verify that all edges in the rule have an image in the model
  //    for(edge e:*current_edge) {
  //        if(!e.isValid())

  for (edge e : g_left->edges()) {
    auto ends = g_left->ends(e);
    vector<edge> e_rule = g_left->getEdges(ends.first, ends.second, _edgeOrientation);
    vector<edge> e_model =
        g_model->getEdges((*current)[g_left->nodePos(ends.first)],
                          (*current)[g_left->nodePos(ends.second)], _edgeOrientation);
    if (!PorgyTlpGraphStructure::checkEdgeState(g_model, g_left, e_rule, e_model, _exact,
                                                matchingPropertiesList, current_edge)) {
            if(_debug)
                tlp::debug() << "Problem when matching edges (pass 2)" << endl;
#ifdef PORGY_RULE_DEBUG_MESSAGES
      cerr << __PRETTY_FUNCTION__ << " : The map given isn't an isomorphism" << endl;
#endif
      return false;
    }
  }

#ifdef PORGY_RULE_DEBUG_MESSAGES
  cerr << __PRETTY_FUNCTION__ << " : Isomorphism verified : ok" << endl;
#endif
  return true;
}

Graph *PartialMap::addSub(int num_solutions, BooleanProperty *position,
                          BooleanProperty *banp) const {
  string rulename = PorgyTlpGraphStructure::getRuleNameForSubGraph(g_left);
  Graph *sub =
      g_model->addSubGraph("sg_" + rulename + "_" + to_string(num_solutions) + "-" +
                           ((position != nullptr) ? position->getName() : PorgyConstants::NO_P));
  PorgyTlpGraphStructure::setPPropertyNameAttribute(
      sub, ((position != nullptr) ? position->getName() : PorgyConstants::NO_P));
  PorgyTlpGraphStructure::setBanPropertyNameAttribute(
      sub, ((banp != nullptr) ? banp->getName() : PorgyConstants::NO_BAN));
  PorgyTlpGraphStructure::setRuleNameForSubGraph(sub, rulename);
  IntegerProperty *tag = sub->getLocalProperty<IntegerProperty>(PorgyConstants::TAG);
  tag->setAllNodeValue(-1);
  tag->setAllEdgeValue(-1);

#ifdef PORGY_RULE_DEBUG_MESSAGES
  cerr << __PRETTY_FUNCTION__ << ": New subgraph found: " << sub->getName() << endl;
#endif
  const vector<node> &nodes = g_left->nodes();
  unsigned nbNodes = nodes.size();
  for (unsigned i = 0; i < nbNodes; ++i) {
    node img((*current)[i]);
    assert(!sub->isElement(img));
    sub->addNode(img);
    assert(tag->getNodeValue(img) == tag->getNodeDefaultValue());
    tag->setNodeValue(img, nodes[i]);
  }
  const vector<edge> &edges = g_left->edges();
  unsigned nbEdges = edges.size();
  for (unsigned i = 0; i < nbEdges; ++i) {
    edge img_e((*current_edge)[i]);
    assert(!sub->isElement(img_e));
    sub->addEdge(img_e);
    assert(tag->getEdgeValue(img_e) == tag->getEdgeDefaultValue());
    tag->setEdgeValue(img_e, edges[i]);
  }
  return sub;
}

bool PartialMap::refine() {

  if (_debug)
    display();
#ifdef PORGY_RULE_DEBUG_MESSAGES
  cerr << __PRETTY_FUNCTION__ << ": Begin refine" << endl;
#endif

  bool modified = false;
  const vector<node> &left_nodes = g_left->nodes();
  unsigned nb_node(M->size());
  for (unsigned nb = 0; nb < nb_node; ++nb) {
    possiblemap &pmap = (*M)[nb];
    node left_node = left_nodes[nb];
    vector<node> to_delete;
    for (const node model_node : pmap) {
      bool found = false;
      for(auto rule_vois:g_left->getInOutNodes(left_node)) {
        bool voisin_ok = false;
        for (auto model_vois : (*M)[g_left->nodePos(rule_vois)]) {
          vector<edge> edges_rule = g_left->getEdges(left_node, rule_vois, _edgeOrientation);
          vector<edge> edges_model = g_model->getEdges(model_node, model_vois, _edgeOrientation);
          if (PorgyTlpGraphStructure::checkEdgeState(g_model, g_left, edges_rule, edges_model,
                                                     _exact, matchingPropertiesList)) {
            // If there is an edge, with the same state as in the rule
            voisin_ok = true;
            break;
          }
          else {
              if(_debug)
                  tlp::debug() << "Problem when matching edges (pass 1)" << endl;
          }
        }
        if (!voisin_ok) {
          // There is a neighbour of the node in the rule which can't be mapped
          // to a neighbour of the image in the model
          found = true;
          break;
        }
      }
      if (found) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
        cerr << __PRETTY_FUNCTION__ << " : Found that node " << left_node
             << " could not be mapped to node " << model_node << endl;
#endif
        modified = true;
        // We suppress the node from the possibilities
        to_delete.push_back(model_node);
      }
    }
    for (node i : to_delete) {
      pmap.erase(i);
    }
  }

  // We verify that all nodes of the rule have at least one possible mapping
  for (unsigned nb = 0; nb < nb_node; ++nb) {
    possiblemap &p = (*M)[nb];
    if (p.empty()) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
      cerr << __PRETTY_FUNCTION__ << " : Found no mapping from here" << endl;
#endif
      return false; // One node has no possible image, we stop
    }
  }

  if (_exact) {
    for (node n : g_model->nodes()) {
      bool ok = false;
      for (unsigned nb = 0; nb < nb_node; ++nb) {
        possiblemap &p = (*M)[nb];
        if (p.find(n) != p.end()) {
          ok = true;
          break;
        }
      }
      if (!ok)
        return false;
    }
  }

  // The operation is repeated until a fixpoint is reached
  return modified ? refine() : true;
}

void PartialMap::backtrack(int numtofind, int &nbr_found, BooleanProperty *pos,
                           BooleanProperty *banp, Graph *left_anti, vector<Graph *> &graph_names) {

  unsigned v = chooseNode();
  const possiblemap &set_v = (*M)[v];
  if (set_v.empty())
    return;

  int nb_possibilities = set_v.size();
  const vector<node> &nodes = g_left->nodes();

  if (_debug)
    display();
#ifdef PORGY_RULE_DEBUG_MESSAGES

  cerr << __PRETTY_FUNCTION__ << " : Looking for images of node " << nodes[v] << endl;
  cerr << __PRETTY_FUNCTION__ << " : Number of possibilities : " << nb_possibilities << endl;
#endif

  progress_step /= nb_possibilities;

  // When only one node remains
  if (nb_mapped == g_left->numberOfNodes() - 1) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
    cerr << __PRETTY_FUNCTION__ << " : Last node to be mapped" << endl;
#endif

    nb_mapped++;
    for (const node w : set_v) {
      progress += progress_step;
      pp->progress(progress, max_progress);
      (*current)[v] = w;
#ifdef PORGY_RULE_DEBUG_MESSAGES
      cerr << __PRETTY_FUNCTION__ << " : Trying to send node " << v << " to node " << w << endl;
      cerr << __PRETTY_FUNCTION__ << " : All vertices mapped, checking the isomorphism" << endl;
#endif
      // All nodes have been mapped. Checking if we truly have found an
      // isomorphism
      if (verifyPos(pos)) {
          if(verifyIso()) {
              if(verifyAntiEdge(left_anti)) { // only uses the content of current (M is false)
                  Graph *added_sub = addSub(nbr_found, pos, banp);
                  ++nbr_found;
                  graph_names.push_back(added_sub);
                  if (nbr_found == numtofind) {
                      return;
                  }
              }
              else {
                  if(_debug)
                      tlp::debug() << "Error when checking anti-edges" << endl;
              }
          }
          else {
              if(_debug)
                  tlp::debug() << "Error when checking isomorphisms for edges" << endl;
          }
      }
      else {
          if(_debug)
              tlp::debug() << "Error when checking position set or ban set" << endl;
      }
    }

    progress_step *= nb_possibilities;
    current->erase(current->begin() + v);
    --nb_mapped;
    return;
  }

  // General case
  for (const node w : set_v) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
    cerr << __PRETTY_FUNCTION__ << " : Trying to send node " << v << " to node " << w << endl;
#endif

    PartialMap partmap(*this);
    (*partmap.current)[v] = w;
    partmap.nb_mapped++;

    (*partmap.M)[v].clear();
    (*partmap.M)[v].insert(w);

    // We suppress to other node the possibility to map to w
    unsigned nb_node(partmap.M->size());
    for (unsigned nb = 0; nb < nb_node; ++nb) {
      if (nb != v) {
        possiblemap &m = (*partmap.M)[nb];
        const auto &iter = m.find(w);
        if (iter != m.cend()) {
          m.erase(iter);
        }
      }
    }

    // Case lots of vertices few edges
    for(auto n_vois:g_left->getInOutNodes(nodes[v])) {
      unsigned n_vois_pos = g_left->nodePos(n_vois);
      if (!(*partmap.current)[n_vois].isValid() &&
          (*partmap.M)[n_vois_pos].size() > g_model->deg(w)) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
        cerr << __PRETTY_FUNCTION__ << " : Restraining images of " << n_vois
             << " to neighbors of node " << w << endl;
#endif
        possiblemap node_set((*partmap.M)[n_vois_pos]);
        (*partmap.M)[n_vois_pos].clear();
        for(auto node_model:g_model->getInOutNodes(w)) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
          cerr << __PRETTY_FUNCTION__ << " : Node considered : " << node_model << endl;
#endif
          if (node_set.find(node_model) != node_set.end()) {
            (*partmap.M)[n_vois_pos].insert(node_model);
#ifdef PORGY_RULE_DEBUG_MESSAGES
            cerr << __PRETTY_FUNCTION__ << " : Image Found : " << node_model << endl;
#endif
          }
        }
      }
    }

    if (partmap.refine()) {
      //#ifdef PORGY_RULE_DEBUG_MESSAGES
      //            cerr << __PRETTY_FUNCTION__ << ": Choice of mapping node "
      //            << v << " to node" << w << " may lead to solutions" << endl;
      //#endif
      partmap.backtrack(numtofind, nbr_found, pos, banp, left_anti, graph_names);
      if (nbr_found >= numtofind && numtofind != -1)
        return;
    }
    //#ifdef PORGY_RULE_DEBUG_MESSAGES
    //        else {
    //            cerr << __PRETTY_FUNCTION__ << ": Choice of mapping node " <<
    //            v << " to node " << w << " leads to no solutions" << endl;
    //        }
    //#endif
    progress += progress_step;
    pp->progress(progress, max_progress);
  }

  //#ifdef PORGY_RULE_DEBUG_MESSAGES
  //    cerr << __PRETTY_FUNCTION__ << " : All possibilites tested in this
  //    branch " << endl;
  //#endif
  progress_step *= nb_possibilities;
}
//////////////////////////////////////////////////////////////////////////
bool PortGraphModel::find(tlp::Graph *left, tlp::Graph *left_anti, tlp::DataSet *dataset,
                          bool exact, PluginProgress *pp, bool debug) {

  //#ifdef PORGY_RULE_DEBUG_MESSAGES
  //    cerr << __PRETTY_FUNCTION__ << ": Looking for subgraph of the rule " <<
  //    left->getSuperGraph()->getName() <<" in the graph :" <<
  //    getGraph()->getName() << endl;
  //#endif

  int numtofind = -1;
  BooleanProperty *banp = nullptr;
  BooleanProperty *pos = nullptr;
  int nb_found = 0;
  bool edgeOrientation = false;
  left->getAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED, edgeOrientation);

  if (dataset != nullptr) {
    dataset->get(PorgyConstants::maximum_number_of_instances,
                 numtofind); // generate only the required LHS instances
    dataset->get(PorgyConstants::BAN, banp);
    dataset->get(PorgyConstants::POSITION, pos);
  }

  pp->setComment("Trying to instantiate the LHS of " + left->getSuperGraph()->getName() + " on " +
                 _graph->getName());

  // on initialise notre mapping
  PortGraph *pg_left = new PortGraphModel(left);
  PartialMap partmap(pg_left, this, pp, edgeOrientation, debug, exact);
  if (!partmap.initialisation(banp)) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
    cerr << __PRETTY_FUNCTION__ << ": No solutions after initialisation" << endl;
#endif
    return false;
  }
  vector<Graph *> graph_names;
  partmap.backtrack(numtofind, nb_found, pos, banp, left_anti, graph_names);
  delete pg_left;
  if (dataset != nullptr) {
    dataset->set("number of instances", nb_found);
    dataset->set("Generated graph", graph_names);
  }
#ifdef PORGY_RULE_DEBUG_MESSAGES
  cerr << __PRETTY_FUNCTION__ << ": Search over,  " << nb_found << " solutions found" << endl;
#endif

  return (nb_found != 0);
}

//////////////
static void build_edges_vector(Graph *g, vector<edge> &v, BooleanProperty *ban = nullptr) {
  for (edge e : g->edges()) {
    if (PortGraph::isPortGraphEdge(e, g)) {
      // check if source and target are both in the object
      // cannot do that in an assert because in some cases the portgraph is not
      // built upon the whole graph returned by _graph
      const pair<node, node> &ends = g->ends(e);
      if (g->isElement(ends.first) && g->isElement(ends.second))
        if ((ban == nullptr) || (!ban->getEdgeValue(e)))
          v.push_back(e);
    }
  }
}

//////////////
vector<edge> PortGraphModel::getEdges(BooleanProperty *ban) {
  vector<edge> v;
  build_edges_vector(_graph, v, ban);
  return v;
}

PortGraphModelDecorator::PortGraphModelDecorator(Graph *g) : PortGraphDecorator(g) {}

/**
 * @brief PortGraphModel::numberOfEdges
*/
unsigned PortGraphModelDecorator::numberOfEdges() const {
  unsigned nb = 0, nb_tulip = 0;
  if (graph_component->existAttribute("number_of_edges")) {
    graph_component->getAttribute<unsigned>("number_of_edges", nb);
    graph_component->getAttribute<unsigned>("number_of_edges_tulip", nb_tulip);
  }

  if (nb_tulip != graph_component->numberOfEdges()) {
    vector<edge> v;
    build_edges_vector(graph_component, v);
    nb = v.size();
    graph_component->setAttribute<unsigned>("number_of_edges", nb);
    graph_component->setAttribute<unsigned>("number_of_edges_tulip",
                                            graph_component->numberOfEdges());
  }
  return nb;
}

///////////////
list<std::string> PortGraphModelDecorator::layoutPluginsList() const {
  std::list<std::string> list;
  list.push_back(PorgyConstants::REDRAW_GRAPH);
  return list;
}
