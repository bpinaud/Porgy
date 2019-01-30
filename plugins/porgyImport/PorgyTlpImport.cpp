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
    // filename
    "The file to import. The valid file extensions are .porgy.tlp or "
    ".porgy.tlp.gz.",

    // port
    "Level the number of ports per node.",

    // differenciate in/out port
    "Used to specify whether or not in and out edges will use same ports (if "
    "false, specific ports will be created to differenciate edge directions).",

    // apply drawing algorithm
    "Apply the default drawing algorithm (FM^3)."};
}

static bool endsWith(std::string const &value, std::string const &ending) {
  if (ending.size() > value.size())
    return false;
  return std::equal(ending.crbegin(), ending.crend(), value.crbegin());
}

class PorgyTlpImport : public ImportModule {
public:
  PLUGININFORMATION("Porgy Tlp Import", "Jason Vallet", "2014/06/31", "Description", "1.1",
                    PorgyConstants::CATEGORY_NAME)
  PorgyTlpImport(tlp::PluginContext *context) : ImportModule(context) {
    addInParameter<std::string>("file::filename", paramHelp[0], "");
    addInParameter<bool>("Additionnal ports", paramHelp[1], "true");
    addInParameter<bool>("Same In/Out ports", paramHelp[2], "true");
    addInParameter<bool>("Redraw the graph", paramHelp[3], "false");

    addDependency("TLP Import", "1.0");
    addDependency(PorgyConstants::REDRAW_GRAPH, "1.0");
  }

  std::list<std::string> fileExtensions() const override {
    list<string> l;
    l.push_back(".porgy.tlp");
    l.push_back(".porgy.tlp.gz");
    return l;
  }

  bool importGraph() override {

    string filename;
    dataSet->get("file::filename", filename);
    list<string> ext(fileExtensions());
    bool found = false;
    for (auto &st : ext) {
      if (endsWith(filename, st)) {
        found = true;
        break;
      }
    }
    if (!found) {
      pluginProgress->setError("file format not supported. You have to use the "
                               "old tlp format with a valid PORGY extension "
                               "(.porgy.tlp, .porgy.tlp.gz)");
      return false;
    }
    Graph *g = tlp::importGraph("TLP Import", *dataSet, pluginProgress, graph);

    if (g != nullptr) {
      Observable::holdObservers();

      bool multiPort = true, mergeInOut = true, redraw = false;
      dataSet->get<bool>("Additionnal ports", multiPort);
      dataSet->get<bool>("Same In/Out ports", mergeInOut);
      dataSet->get<bool>("Redraw the graph", redraw);
      PorgyTlpGraphStructure::Graph2PortGraph(graph, multiPort, mergeInOut);

      // layout new graph
      if (redraw) {
        DataSet dSet;
        string errMsg;
        StringCollection algo(LAYOUT_ALGO);
        algo.setCurrent("FM^3 (OGDF)");
        dSet.set("Layout", algo);
        LayoutProperty *layout = graph->getProperty<LayoutProperty>("viewLayout");
        if (!graph->applyPropertyAlgorithm(PorgyConstants::REDRAW_GRAPH, layout, errMsg,
                                           &dSet,pluginProgress)) {
          cerr << "Problem when applying layout algorithm: " << errMsg << endl;
          Observable::unholdObservers();
          return false;
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

PLUGIN(PorgyTlpImport)
