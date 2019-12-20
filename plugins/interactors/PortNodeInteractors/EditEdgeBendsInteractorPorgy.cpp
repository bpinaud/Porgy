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
#include "InteractorEditEdgeBendsComponent.h"

#include <porgy/SelectPortNodeInteractorComponent.h>

#include <portgraph/PorgyConstants.h>

#include <tulip/MouseInteractors.h>
#include <tulip/NodeLinkDiagramComponentInteractor.h>

using namespace std;
using namespace tlp;

class EditEdgeBendsPorgy : public tlp::NodeLinkDiagramComponentInteractor {

public:
  PLUGININFORMATION("EditEdgeBendsPorgy", "Tulip Team", "08/02/2010", "Edit Edge Bends Porgy",
                    "1.0", PorgyConstants::CATEGORY_NAME)

  EditEdgeBendsPorgy(const PluginContext *)
      : NodeLinkDiagramComponentInteractor(":/tulip/gui/icons/i_bends.png",
                                           "Porgy Edit edge bends",StandardInteractorPriority::EditEdgeBends) {
    setConfigurationWidgetText(
        QString("<h3>Edit edge bends interactor</h3>") + "Modify edge bends<br/><br/>" +
        "Select edge: <ul><li>use rectangle selection</li></ul>" +
        "Translate bend: <ul><li><b>Mouse left</b> down on a selected bend + "
        "moves</li></ul>" +
        "Change source node: <ul><li><b>Drag and drop</b> circle on source "
        "node</li></ul>" +
        "Change target node: <ul><li><b>Drag and drop</b> triangle on target "
        "node</li></ul>" +
        "Add bend: <ul><li><b>Double click with mouse left</b> click on the "
        "selected edge</li></ul>" +
#if !defined(__APPLE__)
        "Delete bend: <ul><li><b>Ctrl + Mouse left</b> click on a selected "
        "bend</li></ul>"
#else
        "Delete bend: <ul><li><b>Alt + Mouse left</b> click on a selected "
        "bend</li></ul>"
#endif
        );
  }

  /**
   * Construct chain of responsibility
   */

  void construct() override {
    push_back(new MouseNKeysNavigator(false));
    push_back(new SelectPortNodeInteractorComponent);
    push_back(new EditEdgeBendsPorgyComponent);
  }

  bool isCompatible(const std::string &viewName) const override {
    return ((viewName == PorgyConstants::GRAPH_VIEW_NAME) ||
            (viewName == PorgyConstants::RULE_VIEW_NAME));
  }
};

PLUGIN(EditEdgeBendsPorgy)
