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
#include <portgraph/PorgyConstants.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraph.h>
#include <portgraph/Trace.h>

#include <tulip/BooleanProperty.h>
#include <tulip/PropertyAlgorithm.h>

using namespace tlp;
using namespace std;

class CheckApply : public tlp::Algorithm {
  string rulename;

public:
  PLUGININFORMATION(PorgyConstants::CHECK_APPLY_RULE, "Bruno Pinaud", "5/11/09", "Comments", "1.0",
                    PorgyConstants::CATEGORY_NAME_INTERNAL)
  CheckApply(const PluginContext *context) : Algorithm(context), rulename("") {
    addDependency(PorgyConstants::APPLY_RULE, "1.0.1");
    addDependency(PorgyConstants::CHECK_RULE, "1.0.1");

    addInParameter<string>(PorgyConstants::RuleName, "", "rule_6");
    addInParameter<bool>(PorgyConstants::checkRule, "", "true");
    addInParameter<bool>(PorgyConstants::applyRule, "", "true");
    addInParameter<int>(PorgyConstants::maximum_number_of_instances, "", "-1");
    addInParameter<unsigned int>(PorgyConstants::minimum_number_of_instances, "", "1");
    addInParameter<BooleanProperty>(PorgyConstants::POSITION, "", "", false);
    addInParameter<BooleanProperty>(PorgyConstants::BAN, "", "", false);
    addInParameter<bool>(PorgyConstants::applyConnectedComponentPacking, "", "true");
    addInParameter<bool>(PorgyConstants::isCondition, "", "false");
    addInParameter<bool>(PorgyConstants::DEBUG, "", "false");
    addInParameter<bool>(PorgyConstants::layoutNewModel, "", "false");

    addOutParameter<vector<Graph *>>(PorgyConstants::newModelAll, "", "", false);
  }

  bool check(string &errMsg) override {
    //    if(!PorgyTlpGraphStructure::isModelGraph(graph)) {
    //        errMsg = graph->getName() + " is not a valid graph to rewrite!";
    //        pluginProgress->setError(errMsg);
    //        return false;
    //    }

    if (dataSet != nullptr) {
      dataSet->get<string>(PorgyConstants::RuleName, rulename);
    }

    // Does the rule exist?
    if (PorgyTlpGraphStructure::findRule(graph, rulename) == nullptr) {
      errMsg = rulename + ": no such rule!";
      pluginProgress->setError(errMsg);
      return false;
    }
    return true;
  }

  bool run() override {
    // pas de preview
    pluginProgress->showPreview(false);

    int maximum_number = 0; //-1 si on veut toutes les possibilités
    bool check = true, apply = true, debug = false, isCondition = false;
    BooleanProperty *position = nullptr, *ban = nullptr;

    if (dataSet != nullptr) {
      dataSet->get<int>(PorgyConstants::maximum_number_of_instances, maximum_number);
      dataSet->get<bool>(PorgyConstants::checkRule, check);
      dataSet->get<bool>(PorgyConstants::applyRule, apply);
      dataSet->get<BooleanProperty *>(PorgyConstants::POSITION, position);
      dataSet->get<BooleanProperty *>(PorgyConstants::BAN, ban);
      dataSet->get<bool>(PorgyConstants::DEBUG, debug);
      dataSet->get<bool>(PorgyConstants::isCondition, isCondition);
    }
    if (debug && isCondition)
      tlp::debug() << "Trying to apply " << rulename << " on " << graph->getName()
                   << " inside a condition" << endl;

#ifdef PORGY_RULE_DEBUG_MESSAGES
    if (position != nullptr) {
      cerr << "position graph: " << position->getGraph()->getName()
           << ", graph: " << graph->getName() << endl;
      assert(position->getGraph()->getId() == graph->getId());
    }
    if (ban != nullptr)
      assert(ban->getGraph()->getId() == graph->getId());
#endif
    pluginProgress->setComment("Applying rule " + rulename);
    ObserverHolder hold;
    (void)hold; // to avoid unused variable warning
    // faut-il vérifier si la règle est applicable ?
    pluginProgress->progress(0, maximum_number + 1);
    string errMsg;
    Trace t(PorgyTlpGraphStructure::getMainTrace(graph));
    if (check) {
      if (!graph->applyAlgorithm(PorgyConstants::CHECK_RULE, errMsg, dataSet, pluginProgress)) {
        //            if(pluginProgress->getError()==PorgyConstants::all_instances_found)
        //                return true;
        t.addFailureNode(graph->getName(),
                         PorgyTlpGraphStructure::findRule(graph, rulename)->getId(), position?position->getName():"", ban?ban->getName():"");
        if (debug)
          tlp::warning() << "Error when applying rule " << rulename << " on model "
                         << graph->getName() << " : " << errMsg << endl;

        return false;
      }
    }
    if (apply) {
      pluginProgress->progress(1, maximum_number + 1);
      vector<Graph *> vector_sub;
      dataSet->get("Generated graph", vector_sub);
      if (maximum_number != 0) {
        if ((maximum_number == -1) || (unsigned(maximum_number) > vector_sub.size()))
          maximum_number = vector_sub.size();
        // apply rules in a random order
        vector<Graph *> newGraphs;
        DataSet old_ds = *dataSet;
        for (int i = 0; i < maximum_number; ++i) {
          auto it = vector_sub.begin();
          uniform_int_distribution<> num(0, std::distance(it, vector_sub.end()) - 1);
          std::advance(it, num(PorgyTlpGraphStructure::gen));
          Graph *s = *it;
          vector_sub.erase(it);
          if (!s->applyAlgorithm(PorgyConstants::APPLY_RULE, errMsg, &old_ds, pluginProgress)) {
            cerr << __PRETTY_FUNCTION__ << "(" << __LINE__ << "): " << errMsg;
            return false;
          }
          Graph *newgraph=nullptr;
          old_ds.get<Graph *>(PorgyConstants::newData, newgraph);
          newGraphs.push_back(newgraph);
          pluginProgress->progress(i + 1, maximum_number + 1);
        }
        dataSet->set<vector<Graph *>>(PorgyConstants::newModelAll, newGraphs);
      }
    }
    return true;
  }
};

PLUGIN(CheckApply)
