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
#include <tulip/Graph.h>
#include <tulip/ImportModule.h>
#include <tulip/LayoutProperty.h>
#include <tulip/StringCollection.h>
#include <tulip/StringProperty.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>

using namespace tlp;
using namespace std;

namespace {

const char *paramHelp[] = {
    // minsize
    "Minimal number of nodes in the tree.",

    // maxsize
    "Maximal number of nodes in the tree.",

    // maxdegree
    "Maximal degree of the nodes.",

    // Port labels
    "If true, the source port of each edge will be called \"Parent\", the "
    "target port will be called \"Child\".",

    // redraw algorithm
    "Layout algorithm to use."

};
}

class random_tree : public ImportModule {
public:
  PLUGININFORMATION("Random Tree", "Pinaud", "21/08/2013",
                    "Imports a new randomly generated tree (portgraph)", "1.1",
                    PorgyConstants::CATEGORY_NAME)
  random_tree(tlp::PluginContext *context) : ImportModule(context) {
    addInParameter<unsigned>("Minimum size", paramHelp[0], "10");
    addInParameter<unsigned>("Maximum size", paramHelp[1], "100");
    addInParameter<unsigned>("Maximal node's degree", paramHelp[2], "5");
    addInParameter<bool>("Add port labels", paramHelp[3], "true");
    addOutParameter<string>("Redraw algorithm", paramHelp[4], "Tree Leaf");
    addDependency("Random General Tree", "1.1");
  }

  bool importGraph() override {
    Graph *g = tlp::importGraph("Random General Tree", *dataSet, pluginProgress, graph);
    if (g != nullptr) {
      Observable::holdObservers();

      PorgyTlpGraphStructure::Graph2PortGraph(graph);

      bool addLabels = true;
      dataSet->get("Add port labels", addLabels);
      StringProperty *viewLabel = graph->getProperty<StringProperty>("viewLabel");
      if (addLabels) {
        PortGraphModel pg(graph);
        for (edge e : pg.getEdges()) {
          const pair<node, node> &ends = graph->ends(e);
          viewLabel->setNodeValue(ends.first, "Child");
          viewLabel->setNodeValue(ends.second, "Parent");
        }
      }

      Observable::unholdObservers();
      return true;
    } else {
      cerr << "problem when importing a new graph" << endl;
      return false;
    }
  }
};

PLUGIN(random_tree)
