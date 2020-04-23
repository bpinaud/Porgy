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
#include <portgraph/Trace.h>

#include <tulip/Algorithm.h>
#include <tulip/BooleanProperty.h>
#include <tulip/PythonInterpreter.h>
#include <tulip/PluginLister.h>

#include <chrono>

#include "RunStrategyElement.h"
#include "StrategyParser.h"

class StrategySpirit : public tlp::Algorithm {

    std::string strategy;
    StrategyElement::vector_strategyelts strategy_elements;
    bool _debug;

    // find a local property in the whole hierarchy
    bool findP(const std::string &property_name) const {
        tlp::Graph *models_root = PorgyTlpGraphStructure::getModelsRoot(graph);
        for (tlp::Graph *g : models_root->subGraphs()) {
            if (g->existLocalProperty(property_name))
                return true;
        }
        return false;
    }

public:
    PLUGININFORMATION(PorgyConstants::APPLY_STRATEGY_ALGORITHM, "Bruno Pinaud", "19/07/10",
                      "Comments", "1.0", PorgyConstants::CATEGORY_NAME)
    StrategySpirit(const tlp::PluginContext *context) : Algorithm(context), _debug(false) {
        addDependency(PorgyConstants::CHECK_APPLY_RULE, "1.0");

        addInParameter<std::string>(PorgyConstants::STRATEGY, "",
                               "ppick(rule_1;rule_1,0.5,rule_1||rule_1,0.5);rule_2");
        addInParameter<tlp::BooleanProperty>(PorgyConstants::POSITION, "", "", false);
        addInParameter<tlp::BooleanProperty>(PorgyConstants::BAN, "", "", false);
        addInParameter<bool>(PorgyConstants::DEBUG, "", "false");
    }

    bool check(std::string &errorMsg) override {
        // new seed for each call to the plugin
        PorgyTlpGraphStructure::gen.seed(
                std::chrono::high_resolution_clock::now().time_since_epoch().count());

        if (!PorgyTlpGraphStructure::isModelGraph(graph)) {
            errorMsg = "A strategy must be applied on a model";
            return false;
        }

        if (dataSet != nullptr) {
            dataSet->get(PorgyConstants::STRATEGY, strategy);
            dataSet->get(PorgyConstants::DEBUG, _debug);
        } else {
            errorMsg = "dataSet problem";
            return false;
        }
        if (strategy.empty()) {
            errorMsg = "Strategy is empty";
            return false;
        }

        // parse the strategy
        std::vector<StrategyElement::TulipProp> properties;
        std::vector<std::string> property_names;
        // built a vector with all properties and another one with only Boolean
        // properties
        for (const std::string &pname : graph->getProperties()) {
            StrategyElement::TulipProp p;
            p.name = pname;
            properties.push_back(p);
            property_names.push_back(pname);
        }
        // Special properties Arity, InArity, ...
        // not finished (see strategy language paper)
        // Add Arity as a valid property
        for (const auto &s : PorgyConstants::ArityProps) {
            StrategyElement::TulipProp p;
            p.name = s;
            properties.push_back(p);
            property_names.push_back(s);
        }
        // built a vector with all rulenames
        std::vector<std::string> rules;
        rules.reserve(PorgyTlpGraphStructure::getRulesRoot(graph)->numberOfSubGraphs());
        for(tlp::Graph* rule:PorgyTlpGraphStructure::getRulesRoot(graph)->subGraphs()) {
            rules.push_back(rule->getName());
        }

        // built a vector with all porgy plugins
        std::vector<std::string> porgy_plugins;
        std::vector<StrategyElement::PluginName> porgyplugins;
        std::list<std::string> plugins =
                tlp::PluginLister::availablePlugins<tlp::PropertyAlgorithm>();
        for (auto name : plugins) {
            Plugin *pl(tlp::PluginLister::getPluginObject(name, nullptr));
            if (pl->group() == PorgyConstants::CATEGORY_NAME) {
                porgy_plugins.push_back(name);
                StrategyElement::PluginName p;
                p.name = name;
                porgyplugins.push_back(p);
            }
            delete pl;
        }

        strategy_grammar strat_parser(errorMsg, rules, properties, property_names, graph, porgy_plugins,
                                      porgyplugins); // le parser du langage de stratégie
        std::string::const_iterator begin = strategy.begin();
        std::string::const_iterator end = strategy.end();
        comments comments_; // le parser pour les commentaires
        bool r = boost::spirit::qi::phrase_parse(begin, end, strat_parser, comments_, strategy_elements);

        // parsing ok ?
        if (r && begin == end)
            return true;
        else if (errorMsg.empty()) {
            errorMsg = " problem around " + std::string(begin, end);
        }
        return false;
    }

    bool run() override {
        pluginProgress->showPreview(false);
        std::string Pname;
        tlp::BooleanProperty *Pprop = nullptr, *Ban = nullptr;
        // position handling
        auto rest = graph->getProperty<tlp::BooleanProperty>("viewSelection");
        dataSet->get<tlp::BooleanProperty *>(PorgyConstants::POSITION, rest);
        dataSet->get<tlp::BooleanProperty *>(PorgyConstants::BAN, Ban);
        //Find an available P Property
        if (rest->getName() == "viewSelection") {
            int indice = 0;
            //looking for an available P property
            while (1) {
                std::string property_name(PorgyConstants::P_PREFIX + std::to_string(indice));
                if ((!graph->existLocalProperty(property_name)) && (!findP(property_name))) {
                    Pprop = graph->getLocalProperty<tlp::BooleanProperty>(property_name);
                    break;
                }
                ++indice;
            }
            assert(Pprop!=nullptr);
            if (!rest->hasNonDefaultValuatedNodes(graph)) { // nothing selected => select the whole graph
                for(auto n:graph->nodes())
                    Pprop->setNodeValue(n, true);
            } else // put into P property every selected elements
                Pprop->copy(rest);
            Pname = Pprop->getName();
        } else
            Pname = rest->getName();

        Trace traceobj(PorgyTlpGraphStructure::getMainTrace(graph));
        tlp::Observable::holdObservers();
        // runs strategy
        unsigned oldnumberofNodes(traceobj.numberOfNodes());
        StrategyElement::stackStrat s;

        std::string BanName;
        //Find an available Ban Property
        if (Ban == nullptr) {
            int indice = 0;
            while (1) {
                std::string property_name(PorgyConstants::BAN_PREFIX + std::to_string(indice));
                if ((!graph->existLocalProperty(property_name)) && (!findP(property_name))) {
                    BanName = property_name;
                    break;
                }
                ++indice;
            }
        }
        else
            BanName = Ban->getName();

        StrategyElement::stratElem elt(Pname, BanName, graph, strategy_elements);
        s.push(elt);
        StrategyElement::vectNewGraphs newGraphAll;
        runStrategyElement::run_strat(s, PorgyConstants::state::OTHER, pluginProgress, _debug,
                                      newGraphAll);
        // the strategy has at least one rewriting step. Strategy edge only added
        // when not using a all(R) operator
        if ((traceobj.numberOfNodes() > oldnumberofNodes) &&
                ((newGraphAll.size() == 1) ||
                        (newGraphAll.empty() &&
                         traceobj.existAttribute(PorgyConstants::FAILURE_NODE_ADDED)))) {
            tlp::node last;
            if (!traceobj.getAttribute(PorgyConstants::FAILURE_NODE_ADDED, last)) {
                tlp::Graph *new_graph = newGraphAll[0];
                last = traceobj.findNode(new_graph->getName());
            } else {
                traceobj.removeAttribute(PorgyConstants::FAILURE_NODE_ADDED);
            }
            tlp::node first = traceobj.findNode(graph->getName());
            traceobj.addStrategyTransformationEdge(first, last, Pname, BanName, strategy);
        }

        tlp::Observable::unholdObservers();
        return true;
    }
};

PLUGIN(StrategySpirit)
