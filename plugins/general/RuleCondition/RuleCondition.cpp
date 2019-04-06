/**
 *
 * This file is part of Porgy (https://gforge.inria.fr/projects/porgy/)
 *
 * from LaBRI, University of Bordeaux 1, Inria Bordeaux - Sud Ouest and King's College London
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

#include <tulip/PropertyAlgorithm.h>
#include <tulip/BooleanProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/ForEach.h>

#include "RuleConditionParser.h"

#include <string>

using namespace std;
using namespace tlp;
using namespace boost;

namespace ConditionParser {
    bool parse(const string &cond,
                     string &errMsg,
                     std::vector<std::string> propertynames,
                     std::vector<std::string> lhsNodesId,
                     std::vector<std::string> lhsEdgesId,
                     ConditionParser::mini_syntax &ast) 
    {
        typedef std::string::const_iterator iterator_type;
        typedef ConditionParser::mini_syntax_grammar<iterator_type> mini_syntax_grammar;
        
        mini_syntax_grammar g(errMsg, propertynames, lhsNodesId, lhsEdgesId); // Conditions grammar
        iterator_type iter = cond.begin();
        iterator_type end = cond.end();
        comments<iterator_type> comments_;    //comment parsing
        bool r = phrase_parse(iter, end, g, comments_, ast); 
        if (!r || iter != end)
        {
            std::string rest(iter, end);
            tlp::debug() << "Parsing failed. Stopped at: " << rest << endl;
        }
        return (r && iter == end);
    }
}

class RuleCondition : public tlp::Algorithm {
    
public:
    PLUGININFORMATION(PorgyConstants::APPLY_RULE_CONDITION, "Janos Varga", "11/09/17", "Comments", "1.0", PorgyConstants::CATEGORY_NAME_INTERNAL)
    RuleCondition(const PluginContext *context):Algorithm(context) {
        // The working graph must contain the entire original graph
        addInParameter<string>("Conditions", "Set of instructions to evaluate", ";", true);
        addInParameter<string>("Mode", "The operation performed: parse, evaluate or execute", "parse", true);
        addInParameter<tlp::Graph*>("Model", "The entire graph the matching runs on.", "");
        addInParameter<std::string>("LhsMappingProperty", "The mapping between the graph being rewritten and the rule.", "");
        addOutParameter<string>("Error Message", "", "none");
    }

    bool check(string &errorMsg) {
        std::string cond_text("");
        std::string _mode("");
        std::string lhsMappingProperty("");
        tlp::Graph* pg_model;
        pluginProgress->showPreview(false);

        if(dataSet!=nullptr) {
            dataSet->get("Conditions", cond_text);
            dataSet->get("Mode", _mode);
            dataSet->get("Model", pg_model);
            dataSet->get("LhsMappingProperty", lhsMappingProperty);
        } else {
            errorMsg = "dataSet is null.";
            return false;
        }        
        if (cond_text.empty()) {
            return true;
        }
        if (lhsMappingProperty.empty()) {
            errorMsg = "No lhs mapping property.";
            return false;
        }
        if (_mode != "parse" && pg_model == nullptr) {
            errorMsg = "Empty model port graph.";
            return false;
        }

        std::vector<std::string> propertynames, lhsNodesId, lhsEdgesId;
        std::map<std::string,std::string> lhsNodeMap, lhsEdgeMap;
        std::string property;

        forEach(property, graph->getProperties()) {
            propertynames.push_back(property);
        }
        
        tlp::IntegerProperty *mappingProperty = graph->getProperty<tlp::IntegerProperty>(lhsMappingProperty);
        
        for(const node& n: graph->nodes()) {
            lhsNodeMap[mappingProperty->getNodeStringValue(n)] = to_string(n.id);
            lhsNodesId.push_back(mappingProperty->getNodeStringValue(n));
        }
        for(const edge& e: graph->edges()) {
            lhsEdgeMap[mappingProperty->getEdgeStringValue(e)] = to_string(e.id);
            lhsEdgesId.push_back(mappingProperty->getEdgeStringValue(e));
        }

        ConditionParser::mini_syntax ast;

        if(!ConditionParser::parse(cond_text, errorMsg, propertynames, lhsNodesId, lhsEdgesId, ast)) {
            errorMsg = "error parsing: " + errorMsg;
            return false;
        }
        else {
            if (_mode == "parse") {
                // Simple parsing operation
                return true;
            }
            else if (_mode == "evaluate" || _mode == "execute") {
                // Parsing and evaluation operation
                ConditionParser::mini_syntax_printer printer(graph, lhsNodeMap, lhsEdgeMap, pg_model);
                return printer(ast);
            }
        }
        return false;
    }

    bool run() {        
        return true;
    }

};

PLUGIN(RuleCondition)
