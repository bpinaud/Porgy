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
#include <tulip/ColorProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/PropertyAlgorithm.h>
#include <tulip/SizeProperty.h>
#include <tulip/TulipViewSettings.h>

#include <portgraph/PorgyConstants.h>

using namespace tlp;
using namespace std;

namespace {
const char *paramHelp[] = {
    // node size
    "This parameter defines the property used for node's sizes.",

    // node color
    "This parameter defines the property used for node/edge's color.",

    // node/edge shape
    "This parameter defines the property used for node/edge shape.",

    // Label position
    "This parameter defines the property used for node's label position."};
}
class RedrawTrace : public LayoutAlgorithm {
public:
  PLUGININFORMATION(PorgyConstants::REDRAW_TRACE, "Bruno Pinaud", "22/01/2010", "Alpha", "1.0",
                    PorgyConstants::CATEGORY_NAME)

  RedrawTrace(const PluginContext *context) : LayoutAlgorithm(context) {
    addDependency("Sugiyama (OGDF)", "1.7");
    addDependency("Auto Sizing", "1.0");

    addInOutParameter<SizeProperty>("Elements size", paramHelp[0], "viewSize");
    addInOutParameter<ColorProperty>("Color", paramHelp[1], "viewColor");
    addInOutParameter<IntegerProperty>("Label position", paramHelp[3], "viewLabelPosition");
    addInOutParameter<IntegerProperty>("Shape", paramHelp[2], "viewShape");
    addInOutParameter<IntegerProperty>("Anchor Shape", "", "viewTgtAnchorShape");
    addInOutParameter<SizeProperty>("Anchor Size", "", "viewTgtAnchorSize");
    addInOutParameter<DoubleProperty>("Border Width", "", "viewBorderWidth");
  }

  bool run() override {
    string errMsg;
    DataSet ds;
    ds.set<bool>("transpose vertically", true);
    ds.set<bool>("fixed layer distance", true);
    ds.set<int>("layer distance", 2);
    ds.set<int>("node distance", 2);
    if (!graph->applyPropertyAlgorithm("Sugiyama (OGDF)", result, errMsg, &ds,pluginProgress)) {
      cerr << __PRETTY_FUNCTION__ << ": Pb avec Sugiyama (OGDF)" << errMsg << endl;
      return false;
    }

    SizeProperty *EltSize = graph->getLocalProperty<SizeProperty>("viewSize");
    ColorProperty *color_prop = graph->getLocalProperty<ColorProperty>("viewColor");
    IntegerProperty *shape = graph->getLocalProperty<IntegerProperty>("viewShape");
    IntegerProperty *label_position = graph->getLocalProperty<IntegerProperty>("viewLabelPosition");
    IntegerProperty *anchorshape = graph->getLocalProperty<IntegerProperty>("viewTgtAnchorShape");
    SizeProperty *anchorsize = graph->getLocalProperty<SizeProperty>("viewTgtAnchorSize");
    DoubleProperty *border_width = graph->getLocalProperty<DoubleProperty>("viewBorderWidth");

    if (dataSet != nullptr) {
      dataSet->get("Color", color_prop);
      dataSet->get("Shape", shape);
      dataSet->get("Elements size", EltSize);
      dataSet->get("Anchor Size", anchorsize);
      dataSet->get("Anchor Shape", anchorshape);
      dataSet->get("Label position", label_position);
      dataSet->get("Border Width", border_width);
    }
    graph->applyPropertyAlgorithm("Auto Sizing", EltSize, errMsg, nullptr, pluginProgress);

    for (node n : graph->nodes()) {
      shape->setNodeValue(n, NodeShape::Square);
      label_position->setNodeValue(n, LabelPosition::Right);
      //  EltSize->setNodeValue(n, Size(5,5,1));
      if (graph->isMetaNode(n)) {
        Color c(color_prop->getNodeValue(n));
        c.setA(0);
        color_prop->setNodeValue(n, c);
        border_width->setNodeValue(n, 1);
      }
    }
    for (edge e : graph->edges()) {
      shape->setEdgeValue(e, EdgeShape::CatmullRomCurve);
      anchorshape->setEdgeValue(e, EdgeExtremityShape::Arrow);
      anchorsize->setEdgeValue(e, Size(0.55, 0.55, 0.55));
      EltSize->setEdgeValue(e, Size(0.35, 0.35, 0.5));
    }
    return true;
  }
};
PLUGIN(RedrawTrace)
