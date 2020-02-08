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
#ifndef PORGYGRAPHSTRUCTURE_H
#define PORGYGRAPHSTRUCTURE_H

#include <random>
#include <string>

#include <portgraph/PorgyConstants.h>
#include <portgraph/porgyconf.h>

#include <tulip/StaticProperty.h>
#include <tulip/StringCollection.h>

namespace tlp {
class BooleanProperty;
class IntegerProperty;
}

struct PORTGRAPH_SCOPE PorgyTlpGraphStructure {

  static tlp::Graph *setPorgyStructure(tlp::Graph *root = nullptr);

  static std::mt19937 gen;

  /**
  * @brief Transform a classic graph into a portgraph.
  *
  * @param graph A graph of a Porgy graph structure.
  * @param pluginProgress The plugin progress manager.
  * @param multiPort A boolean indicating if the generated portgraph will get
  *multiple ports (one per link) [default - true].
  * @param mergeInOut A boolean indicating if the in- and outcoming edges will
  *use the same port [default - true].
  **/
  static void Graph2PortGraph(tlp::Graph *graph, bool multiPort = true, bool mergeInOut = true);

  // this method must be called only when the perspective is started
  static bool isPorgyGraph(const tlp::Graph *graph, std::string &);
  /**
       * @brief Returns the associate root Graph (i.e. TraceRoot, ModelsRoot or
   * RulesRoot)
       * @param graph: a graph of a Porgy graph structure
       */
  static tlp::Graph *getRootGraph(const tlp::Graph *graph);

  static bool isTraceGraph(const tlp::Graph *);
  static bool fixTraceGraph(tlp::Graph *g);
  static bool isRuleUsed(tlp::Graph *g);
  static bool isGraphUsed(tlp::Graph *g);

  static inline tlp::Graph *getModelsRoot(const tlp::Graph *g) {
    tlp::Graph *r = g->getRoot();
    return (r == nullptr) ? nullptr : r->getDescendantGraph(PorgyConstants::GRAPHS_GRAPH_NAME);
  }

  static inline bool isModelGraph(const tlp::Graph *g) {
    tlp::Graph *modelsRoot = getModelsRoot(g);
    return modelsRoot->isDescendantGraph(g);
  }

  static inline tlp::Graph *findModel(const tlp::Graph *g, const std::string &name) {
    return getModelsRoot(g)->getDescendantGraph(name);
  }

  static inline tlp::Graph *getRulesRoot(const tlp::Graph *g) {
    tlp::Graph *r = g->getRoot();
    return (r == nullptr) ? nullptr : r->getDescendantGraph(PorgyConstants::RULES_GRAPH_NAME);
  }

  static inline tlp::Graph *findRule(const tlp::Graph *g, const std::string &name) {
    tlp::Graph *r = getRulesRoot(g);
    return (r == nullptr) ? nullptr : r->getDescendantGraph(name);
  }

  static inline bool isRuleGraph(const tlp::Graph *g) {
    tlp::Graph *r = getRulesRoot(g);
    return r != nullptr && r->isDescendantGraph(g);
  }

  static inline tlp::Graph *getTraceRoot(const tlp::Graph *g) {
    tlp::Graph *r = g->getRoot();
    return (r == nullptr) ? nullptr
                          : r->getRoot()->getDescendantGraph(PorgyConstants::TRACE_GRAPH_NAME);
  }

  typedef std::tuple<const std::string, tlp::BooleanProperty *, tlp::IntegerProperty *,
                     tlp::PropertyInterface *, tlp::PropertyInterface *>
      matchproptuple;
  typedef std::vector<matchproptuple> matchpropvector;
  static bool checkEdgeState(const bool debug, tlp::Graph *modele, tlp::Graph *rule, const tlp::edge rule_e,
                             const tlp::edge graph_e,
                             const matchpropvector &matchingPropertiesList);
  static bool checkEdgeState(const bool debug, tlp::Graph *modele, tlp::Graph *rule,
                             const std::vector<tlp::edge> &rule_e,
                             const std::vector<tlp::edge> &graph_e, bool exact,
                             const matchpropvector &matchingPropertiesList,
                             tlp::EdgeStaticProperty<tlp::edge> *edge_map = nullptr);
  static bool checkNode(tlp::Graph *g1, tlp::Graph *g2, const tlp::node n1, const tlp::node n2,
                        const matchpropvector &matchingPropertiesList);

  // P property set and get
  static void setPPropertyNameAttribute(tlp::Graph *g, const std::string &name);
  static std::string getPPropertyNameAttribute(tlp::Graph *g);

  static void setBanPropertyNameAttribute(tlp::Graph *g, const std::string &name);
  static std::string getBanPropertyNameAttribute(tlp::Graph *g);

  // rulename set and get
  static void setRuleNameForSubGraph(tlp::Graph *g, const std::string &);
  static std::string getRuleNameForSubGraph(tlp::Graph *g);

  /**
       * @brief Find the main trace graph in the given graph hierachy and return
*it.
*
* @param graph A graph of a Porgy graph structure.
       * @return A tlp::Graph * if the traces graph is found.
**/
  static tlp::Graph *getMainTrace(const tlp::Graph *graph);

  static tlp::Graph *clone(tlp::Graph *old_graph, std::string pName, std::string BanName);

  static bool graphIsASimplePath(tlp::Graph *graph);
  /**
   * @brief hasLeaf Checks if the given graph with the given property as a leaf
   * (at least one node with outdegree==0
   * @param graph
   * @return true if the graph has at least one node with a outdegree==0
   */
  static bool hasLeaf(tlp::BooleanProperty *sel, tlp::Graph *traceMain);

  static void getMatchingPropertyNames(tlp::Graph *g,
                                       std::unordered_set<std::string> &propertiesNames,
                                       bool comp = false);

  static void setMatchingPropertyNames(tlp::Graph *g,
                                       const std::unordered_set<std::string> &propertiesNames);

  static void getPropertiesToDisplay(tlp::Graph *g,
                                     std::unordered_set<std::string> &propertiesNames);

  static void setPropertiesToDisplay(tlp::Graph *g,
                                     const std::unordered_set<std::string> &propertiesNames);

  static bool hasProperty(tlp::Graph *g);

  template <typename T>
  static bool compareSamePropertyValue(const T &currentModelPropertyValue,
                                       const T &currentRulePropertyValue,
                                       const PorgyConstants::ComparisonOp &op_comp) {
    switch (op_comp) {
    case PorgyConstants::ComparisonOp::EQ:
      return currentModelPropertyValue == currentRulePropertyValue;

    case PorgyConstants::ComparisonOp::NEQ:
      return currentModelPropertyValue != currentRulePropertyValue;

    case PorgyConstants::ComparisonOp::GT:
      return currentModelPropertyValue > currentRulePropertyValue;

    case PorgyConstants::ComparisonOp::GEQ:
      return (currentModelPropertyValue > currentRulePropertyValue) ||
             (currentModelPropertyValue == currentRulePropertyValue);

    case PorgyConstants::ComparisonOp::LT:
      return currentModelPropertyValue < currentRulePropertyValue;

    case PorgyConstants::ComparisonOp::LEQ:
      return (currentModelPropertyValue < currentRulePropertyValue) ||
             (currentModelPropertyValue == currentRulePropertyValue);
    }
    return false;
  }
};
#endif // PORGYGRAPHSTRUCTURE_H
