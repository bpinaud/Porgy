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
#include <tulip/ConnectedTest.h>
#include <tulip/Graph.h>
#include <tulip/ImportModule.h>
#include <tulip/LayoutProperty.h>
#include <tulip/StringCollection.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>

using namespace tlp;
using namespace std;

namespace {

const char *paramHelp[] = {
    // nodes
    "Number of nodes in the final graph.",

    // edges
    "Number of edges in the final graph.",

    // connected
    "True if the graph has to be connected.",

    // port
    "Level the number of ports per node.",

    // differenciate in/out port
    "Used to specify whether or not in and out will use same ports (if false, "
    "specific ports will be created to differenciate edge directions).",

    // redraw algorithm
    "Layout algorithm to use."};
}

class random_simple_graph : public ImportModule {
public:
  PLUGININFORMATION("Random Simple PortGraph", "Pinaud", "19/09/2013",
                    "Imports a new randomly generated simple graph (portgraph)", "1.1",
                    PorgyConstants::CATEGORY_NAME)
  random_simple_graph(tlp::PluginContext *context) : ImportModule(context) {
    addInParameter<unsigned int>("nodes", paramHelp[0], "5");
    addInParameter<unsigned int>("edges", paramHelp[1], "9");
    addInParameter<bool>("Connected", paramHelp[2], "true");
    addInParameter<bool>("Additionnal ports", paramHelp[3], "true");
    addInParameter<bool>("Same In/Out ports", paramHelp[4], "true");
    addOutParameter<string>("Redraw algorithm", paramHelp[5], "FM^3 (OGDF)");

    addDependency("Random Simple Graph", "1.0");
  }

  bool importGraph() override {
    Graph *g = tlp::importGraph("Random Simple Graph", *dataSet, pluginProgress, graph);

    bool connected = false;
    dataSet->get("Connected", connected);

    if (connected) {
      std::vector<tlp::edge> edges;
      tlp::ConnectedTest::makeConnected(graph, edges);
    }

    if (g != nullptr) {
      Observable::holdObservers();
      bool multiPort = true, mergeInOut = true;
      dataSet->get<bool>("Additionnal ports", multiPort);
      dataSet->get<bool>("Same In/Out ports", mergeInOut);

      PorgyTlpGraphStructure::Graph2PortGraph(graph, multiPort, mergeInOut);

      Observable::unholdObservers();
      return true;
    } else {
      cerr << "problem when importing a new graph" << endl;
      return false;
    }
  }
};

PLUGIN(random_simple_graph)
