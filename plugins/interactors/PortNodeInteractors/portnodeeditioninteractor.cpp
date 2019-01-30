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
#include "../../StandardInteractorPriority.h"

#include <portgraph/PorgyConstants.h>

#include <porgy/elementinformationinteractorcomponent.h>

#include <tulip/MouseInteractors.h>
#include <tulip/NodeLinkDiagramComponentInteractor.h>

using namespace tlp;

class PortNodeEditionInteractor : public tlp::NodeLinkDiagramComponentInteractor {
public:
  PortNodeEditionInteractor(const tlp::PluginContext *)
      : NodeLinkDiagramComponentInteractor(":/tulip/gui/icons/i_select.png",
                                           "Get information on nodes/edges",StandardInteractorPriority::GetInformation) {
    setConfigurationWidgetText(QString("<h3>Get information interactor</h3>") +
                               "<b>Mouse left</b> click on an element to display its properties");
  }

  bool isCompatible(const std::string &viewName) const override {
    return viewName == PorgyConstants::GRAPH_VIEW_NAME;
  }

  void construct() override {
    push_front(new ElementInformationInteractorComponent());
    push_front(new MouseNKeysNavigator());
  }

  PLUGININFORMATION("PortNodeEditionInteractor", "Jonathan Dubois", "02/10/2012",
                    "PortGraph information interactor", "1.0", PorgyConstants::CATEGORY_NAME)
};

PLUGIN(PortNodeEditionInteractor)

class PortNodeRuleEditionInteractor : public tlp::NodeLinkDiagramComponentInteractor {
public:
  PortNodeRuleEditionInteractor(const tlp::PluginContext *)
      : NodeLinkDiagramComponentInteractor(":/tulip/gui/icons/i_select.png",
                                           "Get information on nodes/edges",StandardInteractorPriority::GetInformation) {
    setConfigurationWidgetText(QString("<h3>Get information interactor</h3>") +
                               "<b>Mouse left</b> click on an element to display its properties");
  }

  bool isCompatible(const std::string &viewName) const override {
    return viewName == PorgyConstants::RULE_VIEW_NAME;
  }

  void construct() override {
    push_front(new ElementInformationRuleInteractorComponent());
    push_front(new MouseNKeysNavigator());
  }

  PLUGININFORMATION("PortNodeRuleEditionInteractor", "Jonathan Dubois", "02/10/2012",
                    "PortGraph Rule information interactor", "1.0", PorgyConstants::CATEGORY_NAME)
};

PLUGIN(PortNodeRuleEditionInteractor)
