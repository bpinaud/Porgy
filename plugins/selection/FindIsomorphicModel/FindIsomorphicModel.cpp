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
#include <tulip/BooleanProperty.h>
#include <tulip/ConnectedTest.h>
#include <tulip/StableIterator.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/Trace.h>

using namespace tlp;
using namespace std;

class FindIsomorphicModel : public tlp::BooleanAlgorithm {

public:
  PLUGININFORMATION(PorgyConstants::FIND_ISOMORPHIC_GRAPHS, "Bruno Pinaud", "18/01/11", "Comments",
                    "1.0", PorgyConstants::CATEGORY_NAME)

  bool check(std::string &errMsg) override {
    if (!(PorgyTlpGraphStructure::isTraceGraph(graph))) {
      errMsg = "Graph is not the trace graph";
      return false;
    }
    return true;
  }

  FindIsomorphicModel(const tlp::PluginContext *context) : BooleanAlgorithm(context) {
    addInParameter<BooleanProperty>("Initial selection", "", "viewSelection");
  }

  bool run() override {
    Observable::holdObservers();
    BooleanProperty *select = graph->getProperty<BooleanProperty>("viewSelection");
    if (dataSet != nullptr)
      dataSet->get("Initial selection", select);

    // find the selected node
    node start;
    for (node n : select->getNonDefaultValuatedNodes()) {
      if (graph->isMetaNode(n)) {
        result->setNodeValue(n, true);
        start = n;
        break;
      }
    }

#ifndef NDEBUG
    cerr << "Searching ismorphic models of metanodes " << start << endl;
#endif
    // a selected metanode was found
    if (start.isValid() && result->getNodeValue(start)) {
      Graph *g_start = graph->getNodeMetaInfo(start);

      Trace t(graph);
      // pour chaque metasommet du graph de trace
      PortGraphModel pgm_start(g_start);
      unsigned compt = 0;
      for (node n : t.getModelsMetaNodes()) {
        pluginProgress->progress(compt++, graph->numberOfNodes());
        if (n != start) {
          Graph *g = graph->getNodeMetaInfo(n);
#ifndef NDEBUG
          cerr << "Testing metanode " << n << ": ";
#endif
          // tests basiques : nombre de sommets et d'arêtes identiques
          if ((g->numberOfNodes() == g_start->numberOfNodes()) &&
              (g->numberOfEdges() == g_start->numberOfEdges())) {
            if (ConnectedTest::numberOfConnectedComponents(g) ==
                ConnectedTest::numberOfConnectedComponents(g_start)) {
// maintenant, je teste l'isomorphisme
#ifndef NDEBUG
              cerr << "Using " << g_start->getName() << " as the model" << endl;
              cerr << "Using " << g->getName() << " as the rule" << endl;
#endif
              if (pgm_start.find(g, nullptr, nullptr, true, pluginProgress, false)) {
                // temporary hack begin
                for (Graph *tmpg : g_start->subGraphs()) {
                  g_start->delSubGraph(tmpg);
                }
                // temporary hack end
                result->setNodeValue(n, true);
              }
#ifndef NDEBUG
              else
                cerr << "no isomorphism found" << endl;
#endif
            }
#ifndef NDEBUG
            else
              cerr << "not the same number of connected component" << endl;
#endif
          }
#ifndef NDEBUG
          else
            cerr << "not the same number of nodes (" << g->numberOfNodes() << " vs "
                 << g_start->numberOfNodes() << ") or edges (" << g->numberOfEdges() << " vs "
                 << g_start->numberOfEdges() << ")" << endl;
#endif
        }
      }
    } else {
      Observable::unholdObservers();
      return false;
    }

    Observable::unholdObservers();
    return true;
  }
};

PLUGIN(FindIsomorphicModel)
