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
#include "InteractorPortGraphDelElementComponent.h"

#include <tulip/Graph.h>
#include <tulip/MouseInteractors.h>
#include <tulip/NodeLinkDiagramComponentInteractor.h>
#include <tulip/View.h>

#include <portgraph/PorgyTlpGraphStructure.h>

using namespace tlp;
using namespace std;

class PortGraphDelElement : public tlp::NodeLinkDiagramComponentInteractor {

public:
  PLUGININFORMATION("PortGraphDelElement", "Tulip Team", "01/04/2009",
                    "Delete Element in a PortGraph", "1.0", PorgyConstants::CATEGORY_NAME)

  PortGraphDelElement(const PluginContext *)
      : NodeLinkDiagramComponentInteractor(":/tulip/gui/icons/i_del.png",
                                           "Delete portnodes or edges between portnodes",StandardInteractorPriority::DeleteElement) {
      setConfigurationWidgetText(QString("<h3>Delete nodes or edges</h3>") +
                                 "<b>Mouse left</b> click on an element to delete it.<br/>No "
                                 "deletion confirmation will be asked.<br/><br/>" +
                     "<u>Navigation in the graph</u><br/><br/>" +
                     "Translation: <ul><li><b>Arrow</b> keys</li></ul>" +
  #if !defined(__APPLE__)
                     "Zoom/Unzoom: <ul><li><b>Mouse wheel</b> up/down</li><li> or <b>Pg up/Pg "
                     "down</b> keys</li></ul>"
  #else
                     "Zoom/Unzoom: <ul><li><b>Mouse wheel</b> down/up</li><li> or <b>Pg up/Pg "
                     "down</b> keys</li></ul>"
  #endif
                     );
  }

  bool isCompatible(const std::string &viewName) const override {
    return ((viewName == PorgyConstants::GRAPH_VIEW_NAME) ||
            (viewName == PorgyConstants::RULE_VIEW_NAME));
  }

  void construct() override {
    push_back(new MouseNKeysNavigator(false));
    push_back(new PortGraphDelElementComponent);
  }
};

PLUGIN(PortGraphDelElement)
