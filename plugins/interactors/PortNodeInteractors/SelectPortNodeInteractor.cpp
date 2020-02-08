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
#include <tulip/NodeLinkDiagramComponentInteractor.h>

#include "../../StandardInteractorPriority.h"

#include <porgy/SelectPortNodeInteractorComponent.h>

#include <portgraph/PorgyConstants.h>

using namespace std;
using namespace tlp;

class SelectPortNodeInteractor : public tlp::NodeLinkDiagramComponentInteractor {

public:
  PLUGININFORMATION("SelectPortNodeInteractor", "Tulip Team", "08/02/2010",
                    "Select PortNode Interactor", "1.0", PorgyConstants::CATEGORY_NAME)

  SelectPortNodeInteractor(const PluginContext *)
      : NodeLinkDiagramComponentInteractor(":/tulip/gui/icons/i_selection.png", "Select PortNode",StandardInteractorPriority::RectangleSelection) {
    setConfigurationWidgetText(
        QString("<h3>Selection interactor</h3>") +
        "Select on rectangle.<br/><b>Mouse left</b> down indicates the first "
        "corner, <b>Mouse left</b> up indicates the opposite "
        "corner.<br/><br/>" +
#if !defined(__APPLE__)
        "Add to selection: <ul><li><b>Ctrl + Mouse left</b> click on an "
        "element</li></ul>" +
#else
        "Add/Remove from selection: <ul><li><b>Alt + Mouse left</b> "
        "click</li></ul>" +
#endif
        "Remove from selection: <ul><li><b>Shift + Mouse</b> click</li></ul>");
  }

  void construct() override {
    push_back(new MouseNKeysNavigator(false));
    push_back(new SelectPortNodeInteractorComponent);
  }

  virtual QCursor cursor() const override {
    return Qt::CrossCursor;
  }

  bool isCompatible(const std::string &viewName) const override {
    return ((viewName == PorgyConstants::GRAPH_VIEW_NAME) ||
            (viewName == PorgyConstants::RULE_VIEW_NAME));
  }
};

PLUGIN(SelectPortNodeInteractor)
