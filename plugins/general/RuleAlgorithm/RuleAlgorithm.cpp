/**
 *
 * This file is part of Porgy (https://gforge.inria.fr/projects/porgy/)
 *
 * from LaBRI, University of Bordeaux 1, Inria Bordeaux - Sud Ouest and King's
 * College London
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
#include <portgraph/Trace.h>

#include "RuleAlgorithmElement.h"
#include "RuleAlgorithmParser.h"
#include <tulip/BooleanProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/PropertyAlgorithm.h>

#include <string>

using namespace std;
using namespace tlp;
using namespace boost;

namespace AlgoParser {
bool parse(const string &instr, string &errMsg, std::vector<std::string> propertynames,
           std::vector<std::string> lhsNodesId, std::vector<std::string> rhsNodesId,
           std::vector<std::string> lhsEdgesId, std::vector<std::string> rhsEdgesId,
           AlgoParser::mini_syntax &ast) {

  using boost::spirit::ascii::space;
  typedef std::string::const_iterator iterator_type;
  typedef AlgoParser::mini_syntax_grammar<iterator_type> mini_syntax_grammar;

  mini_syntax_grammar g(errMsg, propertynames, lhsNodesId, rhsNodesId, lhsEdgesId,
                        rhsEdgesId); // Our grammar
  iterator_type iter = instr.begin();
  iterator_type end = instr.end();
  comments<iterator_type> comments_; // le parser pour les commentaires
  bool r = phrase_parse(iter, end, g, comments_, ast);

  // AlgoParser::mini_syntax_printer printer;
  // printer(ast);

  // parsing ok ?
  return (r && iter == end);
}
}

class RuleAlgorithm : public tlp::Algorithm {

  std::string instr;
  std::string _mode;
  tlp::Graph *lhs_graph;

public:
  PLUGININFORMATION(PorgyConstants::APPLY_RULE_ALGORITHM, "Bruno Pinaud", "19/07/10", "Comments",
                    "1.0", PorgyConstants::CATEGORY_NAME_INTERNAL)
  RuleAlgorithm(const PluginContext *context) : Algorithm(context) {
    // The working graph must contain the entire original graph with the
    // additionnal elements from the rhs
    addInParameter<string>("Instructions", "Set of instructions to execute", ";", true);
    addInParameter<string>("Mode", "The operation performed: parse, evaluate or execute", "parse",
                           true);
    addInParameter<tlp::Graph>("Lhs", "The graph containing the LHS elements marked as such", "");
    addInParameter<std::string>("LhsMappingProperty",
                                "The integer property mapping the original lhs "
                                "element ids with the current ones",
                                "");
    addInParameter<std::string>("LhsSelectionProperty",
                                "The boolean property marking the lhs elements", "");
    addInParameter<std::string>("RhsMappingProperty",
                                "The integer property mapping the original rhs "
                                "element ids with the current ones",
                                "");
    addInParameter<std::string>("RhsSelectionProperty",
                                "The boolean property marking the rhs elements", "");

    addOutParameter<string>("Error Message", "", "none");
  }

  bool check(string &errorMsg) override {
    pluginProgress->showPreview(false);
    std::string lhsMappingProperty(""), rhsMappingProperty(""), lhsSelectionProperty(""),
        rhsSelectionProperty("");

    if (dataSet != nullptr) {
      dataSet->get("Instructions", instr);
      dataSet->get("Mode", _mode);
      dataSet->get("Lhs", lhs_graph);
      dataSet->get("LhsMappingProperty", lhsMappingProperty);
      dataSet->get("LhsSelectionProperty", lhsSelectionProperty);
      dataSet->get("RhsMappingProperty", rhsMappingProperty);
      dataSet->get("RhsSelectionProperty", rhsSelectionProperty);
    } else {
      errorMsg = "dataSet problem: no set of instructions";
      return false;
    }
    if (instr.empty()) {
      errorMsg = "Set of instructions is empty";
      return true;
    }
    /* if(lhs_graph->empty()) {
         errorMsg = "Lhs graph is empty";
         return true;
     }*/
    if (lhsMappingProperty.empty()) {
      errorMsg = "No lhs mapping property";
      return true;
    }
    if (rhsMappingProperty.empty()) {
      errorMsg = "No rhs mapping property";
      return true;
    }
    if (rhsSelectionProperty.empty()) {
      errorMsg = "No rhs selection property";
      return true;
    }

    std::vector<std::string> propertynames, lhsNodesId, rhsNodesId, lhsEdgesId, rhsEdgesId;
    std::map<std::string, std::string> lhsNodeMap, lhsEdgeMap, rhsNodeMap, rhsEdgeMap;
    for (auto property : graph->getProperties()) {
      propertynames.push_back(property);
    }
    tlp::IntegerProperty *mappingProperty =
        graph->getLocalProperty<tlp::IntegerProperty>(rhsMappingProperty);
    tlp::BooleanProperty *ruleSideSelection =
        graph->getLocalProperty<tlp::BooleanProperty>(rhsSelectionProperty);
    for (auto n : graph->nodes()) {
      if (ruleSideSelection->getNodeValue(n)) {
        rhsNodeMap[mappingProperty->getNodeStringValue(n)] = to_string(n);
        rhsNodesId.push_back(mappingProperty->getNodeStringValue(n));
      }
    }
    for (auto e : graph->edges()) {
      if (ruleSideSelection->getEdgeValue(e)) {
        rhsEdgeMap[mappingProperty->getEdgeStringValue(e)] = to_string(e.id);
        rhsEdgesId.push_back(mappingProperty->getEdgeStringValue(e));
      }
    }
    mappingProperty = lhs_graph->getLocalProperty<tlp::IntegerProperty>(lhsMappingProperty);
    ruleSideSelection = lhs_graph->getLocalProperty<tlp::BooleanProperty>(lhsSelectionProperty);
    for (auto n : lhs_graph->nodes()) {
      if (ruleSideSelection->getNodeValue(n)) {
        lhsNodeMap[mappingProperty->getNodeStringValue(n)] = to_string(n.id);
        lhsNodesId.push_back(mappingProperty->getNodeStringValue(n));
      }
    }
    for (auto e : lhs_graph->edges()) {
      if (ruleSideSelection->getEdgeValue(e)) {
        lhsEdgeMap[mappingProperty->getEdgeStringValue(e)] = to_string(e.id);
        lhsEdgesId.push_back(mappingProperty->getEdgeStringValue(e));
      }
    }

    AlgoParser::mini_syntax ast;

    // parse the strategy
    if (!AlgoParser::parse(instr, errorMsg, propertynames, lhsNodesId, rhsNodesId, lhsEdgesId,
                           rhsEdgesId, ast)) {
      errorMsg = "error parsing: " + errorMsg;
      return false;
    } else {
      if (_mode == "parse") {
        // Simple parsing operation
        return true;
      } else if (_mode == "evaluate") {
        // Parsing and evaluation operation

        // TODO: create a new printer to check the type of elements
        AlgoParser::mini_syntax_printer printer(graph, lhsNodeMap, lhsEdgeMap, rhsNodeMap,
                                                rhsEdgeMap);
        printer(ast); //
        return true;
      } else if (_mode == "execute") {
        // execute operations
        // TODO: add bool, coord and color types
        AlgoParser::mini_syntax_printer printer(graph, lhsNodeMap, lhsEdgeMap, rhsNodeMap,
                                                rhsEdgeMap);
        printer(ast);
        return true;
      }
    }
    return false;
  }

  bool run() override {
    return true;
  }
};

PLUGIN(RuleAlgorithm)
