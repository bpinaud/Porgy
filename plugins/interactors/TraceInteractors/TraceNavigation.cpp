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

#include <porgy/MetaNodeZoomInteractorComponent.h>

#include <portgraph/PorgyConstants.h>

using namespace std;
using namespace tlp;

class TraceNavigation : public tlp::NodeLinkDiagramComponentInteractor {

public:
  PLUGININFORMATION("TraceNavigation", "Tulip Team", "08/02/2010",
                    "Navigate in the derivation tree", "1.0", PorgyConstants::CATEGORY_NAME)

  TraceNavigation(const PluginContext *)
      : NodeLinkDiagramComponentInteractor(":/tulip/gui/icons/i_navigation.png", "Navigation",StandardInteractorPriority::Navigation) {
    setConfigurationWidgetText(
        QString("<h3>Navigation interactor</h3>") + "3D Navigation in the graph<br><br>" +
        "Translation: <ul><li><b>Mouse left</b> down + moves</li><li>or "
        "<b>Arrow</b> keys down</li></ul>" +
        "X or Y rotation: <ul><li><b>Shift + Mouse left</b> down + up/down or "
        "left/right moves</li></ul>" +
#if !defined(__APPLE__)
        "Z rotation: <ul><li><b>Ctrl + Mouse left</b> down + left/right "
        "moves</li><li> or <b>Insert</b> key</li></ul>" +
        "Zoom/Unzoom: <ul><li><b>Ctrl + Mouse left</b> down + up/down "
        "moves</li><li> or <b>Pg up/Pg down</b> keys</li></ul>"
#else
        "Z rotation: <ul><li><b>Alt + Mouse left</b> down + left/right "
        "moves</li><li> or <b>Insert</b> key</li></ul>" +
        "Zoom/Unzoom: <ul><li><b>Alt + Mouse left</b> down + up/down "
        "moves</li><li> or <b>Pg up/Pg down</b> keys</li></ul>" +
#endif
        "Zoom on the selected metanode: <ul><li><b>Ctrl + Mouse</b> left click "
        "on the metanode</li></ul>");
  }

  void construct() override {
      push_back(new MouseNKeysNavigator);
      push_back(new MetaNodeZoomInteractorComponent);

  }

  bool isCompatible(const std::string &viewName) const override {
    return (viewName == PorgyConstants::TRACE_VIEW_NAME);
  }

  QCursor cursor() const override {
    return QCursor(Qt::OpenHandCursor);
  }
};

PLUGIN(TraceNavigation)
