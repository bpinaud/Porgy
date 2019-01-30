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
#include <tulip/PropertyAlgorithm.h>

#include <portgraph/PorgyConstants.h>

using namespace tlp;
using namespace std;

class SimpleCheckApply : public tlp::Algorithm {

public:
  PLUGININFORMATION(PorgyConstants::SIMPLECHECKAPPLY, "Bruno Pinaud", "5/11/09", "Comments", "1.0",
                    PorgyConstants::CATEGORY_NAME)
  SimpleCheckApply(const tlp::PluginContext *context) : Algorithm(context) {
    addDependency(PorgyConstants::CHECK_APPLY_RULE, "1.0");

    addInParameter<string>(PorgyConstants::RuleName, "", "rule_6");
    addInParameter<int>(PorgyConstants::maximum_number_of_instances, "", "-1");
    addInParameter<BooleanProperty>(PorgyConstants::POSITION, "", "", false);
    addInParameter<BooleanProperty>(PorgyConstants::BAN, "", "", false);
    addInParameter<bool>(PorgyConstants::DEBUG, "", "false");
  }

  bool run() override {
    dataSet->set<int>(PorgyConstants::minimum_number_of_instances, 1);
    dataSet->set<bool>(PorgyConstants::checkRule, true);
    dataSet->set<bool>(PorgyConstants::applyConnectedComponentPacking, true);
    string errorMsg;
    return graph->applyAlgorithm(PorgyConstants::CHECK_APPLY_RULE, errorMsg, dataSet,
                                 pluginProgress);
  }
};

PLUGIN(SimpleCheckApply)
