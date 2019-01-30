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
#include <tulip/MouseInteractors.h>
#include <tulip/MouseSelectionEditor.h>
#include <tulip/NodeLinkDiagramComponentInteractor.h>

#include <porgy/SelectPortNodeInteractorComponent.h>

#include <portgraph/PorgyConstants.h>

#include "../../StandardInteractorPriority.h"

using namespace tlp;

class PortGraphSelectionModifier : public tlp::NodeLinkDiagramComponentInteractor {

public:
  PLUGININFORMATION("PortGraphSelectionModifier", "Tulip Team", "01/04/2009",
                    "PortGraph Selection Modifier Interactor", "1.0", PorgyConstants::CATEGORY_NAME)

  PortGraphSelectionModifier(const tlp::PluginContext *)
      : NodeLinkDiagramComponentInteractor(":/tulip/gui/icons/i_move.png",
                                           "PortGraph Move/Reshape selection",StandardInteractorPriority::RectangleSelectionModifier) {
    setConfigurationWidgetText(
        QString("<h3>Selection modifier interactor for PortGraph</h3>") +
        "Modify selection<br/><br/>" + "Resize : <ul><li><b>Mouse left</b> "
                                       "down on triangle + moves</li></ul>" +
        "<ul><li><b>Mouse left</b> down on square + moves</li></ul>" +
        "Only change node size : <ul><li><b>Ctrl + Mouse left</b> down on "
        "triangle + moves</li></ul>" +
        "Only change selection size : <ul><li><b>Shift + Mouse left</b> down "
        "on triangle + moves</li></ul>" +
        "Rotate : <ul><li><b>Mouse left</b> down on circle + moves</li></ul>" +
        "Only rotate nodes : <ul><li><b>Ctrl + Mouse left</b> down on circle + "
        "moves</li></ul>" +
        "Only rotate selection : <ul><li><b>Shift + Mouse left</b> down on "
        "circle + moves</li></ul>" +
        "Translate : <ul><li><b>Mouse left</b> down inside rectangle + "
        "moves</li></ul>" +
        "Align vertically/horizontally : <ul><li><b>Mouse left</b> click on "
        "two arrows icon in top right zone</li></ul>" +
        "Align left/right/top/bottom : <ul><li><b>Mouse left</b> click on "
        "simple arrow icon in top right zone</li></ul>");
  }

  /**
   * Construct chain of responsibility
   */
  void construct() override {
    push_front(new MouseSelectionEditor);
    push_front(new SelectPortNodeInteractorComponent);
    push_front(new MousePanNZoomNavigator);
  }

  bool isCompatible(const std::string &viewName) const override {
    return ((viewName == PorgyConstants::GRAPH_VIEW_NAME) ||
            (viewName == PorgyConstants::RULE_VIEW_NAME));
  }

  //	QCursor cursor(){return QCursor(Qt::CrossCursor);}
};

PLUGIN(PortGraphSelectionModifier)
