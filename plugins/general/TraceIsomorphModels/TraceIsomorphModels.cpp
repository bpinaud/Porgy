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
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Trace.h>

#include <tulip/Algorithm.h>
#include <tulip/BooleanProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>

#include "metavaluecalculator.h"

using namespace std;
using namespace tlp;

// corresponding static instances
static IntegerPropertyCalculator vIntegerPropertyCalc;
static viewColorPropertyCalculator vViewColorPropertyCalc;
static StringPropertyCalculator vStringPropertyCalculator;

class TraceIsomorphModels : public Algorithm {

public:
  PLUGININFORMATION("Rewrite derivation tree following isomorph models", "Bruno Pinaud", "09/04/10",
                    "Comments", "1.0", PorgyConstants::CATEGORY_NAME)

  TraceIsomorphModels(const PluginContext *context) : Algorithm(context) {
    addDependency(PorgyConstants::FIND_ISOMORPHIC_GRAPHS, "1.0");
    addDependency("Circular", "1.1");
    addDependency("Auto Sizing", "1.0");
  }

  bool check(string &errorMsg) override {
    if (!PorgyTlpGraphStructure::isTraceGraph(graph)) {
      errorMsg = "Graph is not a derivation tree";
      return false;
    }

    return true;
  }

  ///////////////
  bool run() override {
    // pas de preview
    pluginProgress->showPreview(false);
    Graph *traceRoot = PorgyTlpGraphStructure::getTraceRoot(graph);
    Graph *root = graph->getRoot();
    Graph *g = traceRoot->addSubGraph("Isomorphic graphs");
    g->addNodes(graph->nodes());
    g->addEdges(graph->edges());

    Trace t(g);
    int cpt = 0;

    // install predefined meta value calculators for g
    t.getTypeProperty()->setMetaValueCalculator(&vIntegerPropertyCalc);
    t.getTransformationStrategyProperty()->setMetaValueCalculator(&vStringPropertyCalculator);
    t.getTransformationPPropertyNameProperty()->setMetaValueCalculator(&vStringPropertyCalculator);
    t.getTransformationInstanceProperty()->setMetaValueCalculator(&vIntegerPropertyCalc);
    t.getTransformationRuleProperty()->setMetaValueCalculator(&vIntegerPropertyCalc);
    g->getProperty<ColorProperty>("viewColor")->setMetaValueCalculator(&vViewColorPropertyCalc);
    string errMsg;
    pluginProgress->progress(cpt, graph->numberOfNodes());
    for (auto n : t.getModelsMetaNodes()) {
      cpt++;
      pluginProgress->progress(cpt, graph->numberOfNodes());
      if (pluginProgress->state() == TLP_STOP) {
        t.redraw(errMsg, pluginProgress);
        return true;
      }
      if (pluginProgress->state() == TLP_CANCEL) {
        root->pop(false);
        return false;
      }
      if (!n.isValid())
        continue;
      BooleanProperty isomorph(root);
      BooleanProperty sel(g);
      sel.setNodeValue(n, true);
      DataSet ds;
      ds.set<BooleanProperty *>("Initial selection", &sel);
      string errorMsg;
      g->applyPropertyAlgorithm(PorgyConstants::FIND_ISOMORPHIC_GRAPHS, &isomorph, errorMsg,&ds,
                                pluginProgress);

      if (isomorph.numberOfNonDefaultValuatedNodes()>1) { // il y a au moins deux mtasommets isomorphes
        Graph *grp = traceRoot->addSubGraph(&isomorph);
        grp->setAttribute("PORGY, do not display", true); // UGLY HACK!!!
        grp->applyPropertyAlgorithm("Circular", grp->getProperty<LayoutProperty>("viewLayout"),
                                    errorMsg, &ds,pluginProgress);
        grp->applyPropertyAlgorithm("Auto Sizing", grp->getProperty<SizeProperty>("viewSize"),
                                    errorMsg, &ds,pluginProgress);
        IntegerProperty *shape_grp = grp->getProperty<IntegerProperty>("viewShape");
        IntegerProperty *label_position = grp->getProperty<IntegerProperty>("viewLabelPosition");
        stringstream sstr;
        for (auto n : grp->nodes()) {
          shape_grp->setNodeValue(n, NodeShape::Square);
          label_position->setNodeValue(n, LabelPosition::Right);
          sstr << grp->getNodeMetaInfo(n)->getName() << " ";
        }

        grp->setName(sstr.str());
        g->createMetaNode(grp, true, false);
      }
    }
    IntegerProperty *shapeArrow = g->getProperty<IntegerProperty>("viewTgtAnchorShape");
    SizeProperty *sizeArrow = g->getProperty<SizeProperty>("viewTgtAnchorSize");

    for (auto e : g->edges()) {
      shapeArrow->setEdgeValue(e, EdgeExtremityShape::Arrow);
      sizeArrow->setEdgeValue(e, Size(0.4f, 0.4f, 0.25));
    }

    t.redraw(errMsg, pluginProgress);
    return true;
  }
};

PLUGIN(TraceIsomorphModels)
