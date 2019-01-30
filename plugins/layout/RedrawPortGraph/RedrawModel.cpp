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
#include "LayoutUtils.h"

#include <portgraph/PorgyConstants.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortNode.h>

#include <tulip/ColorProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>
#include <tulip/StringCollection.h>

using namespace std;
using namespace tlp;

namespace {
const char *paramHelp[] = {
    // LayoutAlgorithm
    "Choose the layout algorithm to use",

    // node size
    "This parameter defines the property used for node's sizes.",

    // node color
    "This parameter defines the property used for node/edge's color.",

    // border color
    "This parameter defines the property used for node/edge border color.",

    // use current layout
    "This parameter defines if the current graph layout has to be used as a "
    "starting point.",

    //
    "This parameter defines the layout property to use when the current layout "
    "needs to be used as a starting point."};
}

class RedrawModel : public LayoutAlgorithm {

  string layout_algo;

  Graph *createAbstractGraph(PortGraphModel &p) {
    Graph *sub = graph->addSubGraph();

    //    BooleanProperty *new_node =
    //    sub->getProperty<BooleanProperty>(PorgyConstants::NEW);
    //    BooleanProperty *temp =
    //    sub->getLocalProperty<BooleanProperty>("temp");
    //    temp->setAllNodeValue(false);
    //    LayoutProperty *layout =
    //    sub->getProperty<LayoutProperty>("viewLayout");

    // ajout des centres des portnodes
    for (PortNode *pn : p.getPortNodes()) {
      sub->addNode(pn->getCenter());
      // si le sommet est nouveau, je mets un sommet identique considr comme non
      // nouveau est je le relie   l'autre.
      // ce nouveau sommet permettra d'attirer l'autre dans GEM
      //        if((new_node->getNodeValue(n))&&(layout_algo==PorgyConstants::PORGY_GEM))
      //        {
      //            node n2 = sub->addNode();
      //            new_node->setNodeValue(n2, false);
      //            layout->setNodeValue(n2, layout->getNodeValue(n));
      //            temp->setNodeValue(n2, true);
      //            sub->addEdge(n,n2);
      //        }
    }
    // ajout d'une arte si les centres sont connects par l'intermdiaire des
    // ports
    for (edge e : p.getEdges()) {
      const pair<const PortNode *, const PortNode *> ends = p.ends(e);
      sub->addEdge(ends.first->getCenter(), ends.second->getCenter());
    }

    return sub;
  }

public:
  PLUGININFORMATION(PorgyConstants::REDRAW_GRAPH, "Bruno pinaud", "28/09/09", "Comments", "1.0",
                    PorgyConstants::CATEGORY_NAME)
  RedrawModel(const tlp::PluginContext *context) : LayoutAlgorithm(context) {

    addDependency(PorgyConstants::GEM, PorgyConstants::GEM_VERSION);
    addDependency(PorgyConstants::SUGIYAMA, "1.5");
    addDependency("Connected Component Packing", "1.0");
    addDependency("FM^3 (OGDF)", "1.2");

    addInParameter<StringCollection>("Layout", paramHelp[0], LAYOUT_ALGO);
    addInParameter<bool>("Use a given layout", paramHelp[4], "true");
    addInParameter<LayoutProperty>("Initial layout", paramHelp[5], "viewLayout", false);
    addInOutParameter<SizeProperty>("node size", paramHelp[1], "viewSize");
    addInOutParameter<ColorProperty>("Color", paramHelp[2], "viewColor");
    addInOutParameter<ColorProperty>("Bordercolor", paramHelp[3], "viewBorderColor");
  }

  bool check(string &errMsg) override {
    if (!PorgyTlpGraphStructure::isModelGraph(graph)) {
      errMsg = "Graph is not a model";
      return false;
    }

    return true;
  }

#define ELT_RANKINGLIST "LongestPathRanking;OptimalRanking"
  bool run() override {
    StringCollection tmp;
    dataSet->get("Layout", tmp);
    layout_algo = tmp.getCurrentString();

    // parse graph into proper structure and classes
    PortGraphModel p(graph);
    PortGraphModelDecorator dec(graph);
    string errMsg;
    Graph *sub = createAbstractGraph(p);
    if (dec.numberOfEdges() > 0) {
      if (layout_algo == PorgyConstants::SUGIYAMA) {
        StringCollection sc(ELT_RANKINGLIST);
        sc.setCurrent(1);
        dataSet->set("Ranking", sc);
      }
      if (layout_algo != PorgyConstants::NONE) {
#ifdef PORGY_RULE_DEBUG_MESSAGES
        cerr << "Applying " << layout_algo << endl;
#endif
        if (!sub->applyPropertyAlgorithm(layout_algo, result, errMsg, dataSet,pluginProgress)) {
          cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << " problem with " << layout_algo << ": "
               << errMsg << endl;
          return false;
        }
        //            }
        //            else {
        //                LayoutProperty *l =
        //                sub->getProperty<LayoutProperty>("viewLayout");
        //                result->copy(l);
        //            }
        //    PorgyTlpGraphStructure::removeAbstractGraph(sub);
        // fast overlap removal
        dataSet->set("layout", result);
        // dataSet->set<double>("x border", 1.0f);
        // dataSet->set<double>("y border", 1.0f);
        if (!sub->applyPropertyAlgorithm("Connected Component Packing", result, errMsg,
                                         dataSet,pluginProgress)) {
          cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << "Fast Overlap Removal: " << errMsg
               << endl;
        }
      }
    }

    SizeProperty *nodeSize = graph->getProperty<SizeProperty>("viewSize");
    ColorProperty *viewColor = graph->getProperty<ColorProperty>("viewColor");
    ColorProperty *bordercolor = graph->getProperty<ColorProperty>("viewBorderColor");
    dataSet->get("node size", nodeSize);
    dataSet->get("Color", viewColor);
    dataSet->get("Bordercolor", bordercolor);

    removeAbstractGraph(sub);
    for (PortNode *pn : p.getPortNodes()) {
      pn->draw(nodeSize, viewColor, result, bordercolor);
    }
    p.drawSameSizes(nodeSize, result);
    return true;
  }
};
PLUGIN(RedrawModel)
