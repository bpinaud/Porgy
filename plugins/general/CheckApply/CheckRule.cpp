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

#include <tulip/BooleanProperty.h>
#include <tulip/PropertyAlgorithm.h>

using namespace std;
using namespace tlp;

class CheckRule : public tlp::Algorithm {
  tlp::Graph *rulegraph;
  string rulename;
  tuple<Graph *, Graph *, Graph *> lhs_rhs; //<LHS, RHS, LHS_anti_edges>
  bool _debug;

public:
  PLUGININFORMATION("Check Rule", "Bruno Pinaud", "20/10/2010", "Comments", "1.0.1",
                    PorgyConstants::CATEGORY_NAME_INTERNAL)
  CheckRule(const PluginContext *context) : Algorithm(context), rulegraph(nullptr), _debug(false) {

    addDependency("Equal Value", "1.1");

    addInParameter<string>(PorgyConstants::RuleName, "", "rule_1");
    addInParameter<int>(PorgyConstants::maximum_number_of_instances, "", "-1");
    addInParameter<int>(PorgyConstants::minimum_number_of_instances, "", "1");
    addInParameter<BooleanProperty>(PorgyConstants::POSITION, "", "", false);
    addInParameter<BooleanProperty>(PorgyConstants::BAN, "", "", false);
    addInParameter<bool>(PorgyConstants::DEBUG, "", "false");

    addOutParameter<int>("number of instances", "", "0", false);
    addOutParameter<vector<Graph *>>("Generated graph", "", "", false);
  }

  bool check(string &errorMsg) override {
    //        if(!PorgyTlpGraphStructure::isModelGraph(graph)) {
    //            errorMsg = graph->getName() + " is not a valid graph to
    //            rewrite!";
    //            pluginProgress->setError(errorMsg);
    //            return false;
    //        }

    if (dataSet != nullptr) {
      dataSet->get(PorgyConstants::RuleName, rulename);
      dataSet->get(PorgyConstants::DEBUG, _debug);
    } else
      return false;
    if (_debug) {
      tlp::debug() << "Trying to match " << rulename << " on " << graph->getName() << endl;
      BooleanProperty *position = nullptr, *ban = nullptr;
      dataSet->get(PorgyConstants::POSITION, position);
      dataSet->get(PorgyConstants::BAN, ban);
      if (position != nullptr)
        tlp::debug() << "Position set: " << position->getName() << endl;
      if (ban != nullptr)
        tlp::debug() << "Ban set: " << ban->getName() << endl;
    }
    rulegraph = PorgyTlpGraphStructure::findRule(graph, rulename);
    if (rulegraph == nullptr) {
      errorMsg = rulename + ": no such rule!";
      return false;
    }

    // Create a subgraph for each member of the rule (RHS/LHS/Bridge)
    lhs_rhs = PortGraphRule::extractLHSRHS(rulegraph, pluginProgress, errorMsg);
    if (get<0>(lhs_rhs) == nullptr) {
      return false;
    }

    // Save whether to check for edge orientation during matching
    bool edgeOrientation = false;
    if (rulegraph->getAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED, edgeOrientation))
      get<0>(lhs_rhs)->setAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED, edgeOrientation);
    return true;
  }

  bool run() override {
    pluginProgress->showPreview(false);

    int min_num = 1;
    //  int max_num=1;
    //    BooleanProperty *position=nullptr, *ban=nullptr;
    dataSet->get(PorgyConstants::minimum_number_of_instances, min_num);
    //    dataSet->get(PorgyConstants::maximum_number_of_instances, max_num);
    //    dataSet->get(PorgyConstants::POSITION, position);
    //    dataSet->get(PorgyConstants::BAN, ban);

    // Rule already applied with the same parameters?
    //        Graph *sub;
    //        int cpt=0;
    //        forEach(sub, graph->getSubGraphs()) {
    //            if(PorgyTlpGraphStructure::getRuleNameForSubGraph(sub)==rulename)
    //            {
    //                if(position==nullptr&&ban==nullptr)
    //                    ++cpt;
    //                else
    //                if((ban==nullptr)&&(PorgyTlpGraphStructure::getPPropertyNameAttribute(sub)==position->getName()))
    //                    ++cpt;
    //                else
    //                if((position==nullptr)&&(PorgyTlpGraphStructure::getBanPropertyNameAttribute(sub)==ban->getName()))
    //                    ++cpt;
    //                else
    //                if((ban!=nullptr)&&(PorgyTlpGraphStructure::getBanPropertyNameAttribute(sub)==ban->getName())&&(PorgyTlpGraphStructure::getBanPropertyNameAttribute(sub)==ban->getName()))
    //                    ++cpt;
    //            }
    //        }
    //        if((max_num!=-1)&&(max_num<=cpt)) {
    //            tlp::warning() << "Rule already applied with the same
    //            parameters." << endl;
    //            return true;
    //        }

    // parsing du graphe
    PortGraphModel p(graph);

    // maintenant la recherche du motif revient à trouver l'ensemble des
    // sous-graphes de left dans le modèle
    // cherche une instance de la règle dans le modèle
    if (!p.find(get<0>(lhs_rhs), get<2>(lhs_rhs), dataSet, false, pluginProgress,
                _debug)) { // la partie gauche de la règle n'a pas été trouvée
      PortGraphRule::cleanLHSRHS(rulegraph);
      pluginProgress->setError("No morphism found");
      if (_debug)
        tlp::debug() << "No morphism found" << endl;
      return false;
    }

    // All LHS instances already found before
    int number_of_instances = 0;
    dataSet->get("number of instances", number_of_instances);
    //        if(number_of_instances==cpt) {
    //            pluginProgress->setError(PorgyConstants::all_instances_found);
    //            PortGraphRule::cleanLHSRHS(rulegraph);
    //            if(_debug)
    //                tlp::debug() << PorgyConstants::all_instances_found <<
    //                endl;
    //            return false;
    //        }
    if (min_num > number_of_instances) {
      pluginProgress->setError("Minimum number of instances not reached");
      PortGraphRule::cleanLHSRHS(rulegraph);
      if (_debug)
        tlp::debug() << "Minimum number of instances not reached" << endl;
      return false;
    }

    // en cas d'application en //, les instances trouvées doivent être disjointe
    //=>le sous-graphe induit d'une instance dans le modèle ne doit pas
    // comporter
    // d'arêtes supplémentaires.
    //        if(rulegraph->getAttribute(PorgyConstants::IS_PARALLEL_RULE)) {
    //            Graph *inst;
    //            stableForEach(inst, graph->getSubGraphs()) {
    // je ne teste que les sous-graphes qui correspondent à la recherche
    // actuelle
    //                if((PorgyTlpGraphStructure::getRuleNameForSubGraph(inst)==rulename)&&(PorgyTlpGraphStructure::getPPropertyNameAttribute(inst)==((position==nullptr)?PorgyConstants::NO_P:position->getName())))
    //                {
    //                    unsigned num_edges_instance = inst->numberOfEdges();
    //                    BooleanProperty sel(graph);
    //                    sel.setAllNodeValue(false);
    //                    sel.setAllEdgeValue(false);
    //                    //sélection de tous les élements du sous-graphe
    //                    Iterator<node> *itn = inst->getNodes();
    //                    while(itn->hasNext()) {
    //                        node n=itn->next();
    //                        sel.setNodeValue(n, true);
    //                    }delete itn;
    //                    Iterator<edge> *ite = inst->getEdges();
    //                    while(ite->hasNext()) {
    //                        edge e=ite->next();
    //                        sel.setEdgeValue(e, true);
    //                    }delete ite;
    //                    DataSet ds;
    //                    ds.set<BooleanProperty *>("Nodes", &sel);
    //                    BooleanProperty temp(graph);
    //                    temp.setAllEdgeValue(false);
    //                    string errMsg;

    //                    if(!graph->applyPropertyAlgorithm("Induced Sub-Graph",
    //                    &temp, errMsg, 0, &ds)) {
    //                        pluginProgress->setError(errMsg);
    //                        clean();
    //                        return false;
    //                    }
    //                    //je compte maintenant combien d'arêtes sont
    //                    sélectionnées dans graph
    //                    ite = temp.getNonDefaultValuatedEdges();
    //                    unsigned num=0;
    //                    while(ite->hasNext()) {
    //                        ite->next();
    //                        ++num;
    //                    }delete ite;
    //				cerr << "Nombre d'arêtes dans l'instance : " << num_edges_instance
    //<< endl;
    //				cerr << "Nombre d'arêtes sélectionnées dans le modèle : " << num
    //<< endl;
    //                    //num>num_edges_instance : les instances de la règle
    //                    // ne sont pas disjointes => arrêt
    //                    if(num>num_edges_instance) {
    //                        graph->delAllSubGraphs(inst);
    //                        pluginProgress->setError("The instances of the
    //                        parallel rule in the model are not disconnected");
    //                        --number_of_instances;
    //                        dataSet->set("number of instances",
    //                        number_of_instances);
    //                    }
    //                }
    //            }
    //        }

    PortGraphRule::cleanLHSRHS(rulegraph);
    if (_debug)
      tlp::debug() << "Morphisms found for rule " << rulename << endl;
    return true;
  }
};

PLUGIN(CheckRule)
