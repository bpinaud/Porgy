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
      setConfigurationWidgetText(
          QString("<h3>Display node or edge properties</h3>") +
          "<b>Mouse left click</b> on an element (the mouse cursor must be as <img "
          "src=\":/tulip/gui/icons/i_select.png\">),<br/>"
          "to display a panel showing its properties.<br/>"
          "As the panel is displayed, <b>Mouse left click</b> in a property row to edit the "
          "corresponding value.<br/>"
          "<u>3D Navigation in the graph</u><br/><br/>" +
          "Translation: <ul><li><b>Mouse left</b> down + moves</li><li>or <b>Arrow</b> keys </li></ul>" +
          "X or Y rotation: <ul><li><b>Shift + Mouse left</b> down + up/down or left/right "
          "moves</li></ul>" +
  #if !defined(__APPLE__)
          "Z rotation: <ul><li><b>Ctrl + Mouse left</b> down + left/right moves</li><li> or "
          "<b>Insert</b> key</li></ul>" +
          "Zoom/Unzoom: <ul><li><b>Mouse wheel</b> up/down</li><li> or <b>Ctrl + Mouse left</b> down + up/down moves</li><li> or <b>Pg "
          "up/Pg down</b> keys</li></ul>"
  #else
          "Z rotation: <ul><li><b>Alt + Mouse left</b> down + left/right moves</li><li> or "
          "<b>Insert</b> key</li></ul>" +
          "Translation: <ul><li><b>Arrow</b> keys</li></ul>" +
          "Zoom/Unzoom: <ul><li><b>Mouse wheel</b> down/up</li><li> or <b>Alt + Mouse left</b> down + up/down moves</li><li> or <b>Pg up/Pg "
          "down</b> keys</li></ul>"
  #endif
);
  }

  bool isCompatible(const std::string &viewName) const override {
    return viewName == PorgyConstants::GRAPH_VIEW_NAME;
  }

  void construct() override {
      push_back(new MouseNKeysNavigator);
      push_back(new ElementInformationInteractorComponent());

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
    push_back(new MouseNKeysNavigator(false));
    push_back(new ElementInformationRuleInteractorComponent());

  }

  PLUGININFORMATION("PortNodeRuleEditionInteractor", "Jonathan Dubois", "02/10/2012",
                    "PortGraph Rule information interactor", "1.0", PorgyConstants::CATEGORY_NAME)
};

PLUGIN(PortNodeRuleEditionInteractor)
