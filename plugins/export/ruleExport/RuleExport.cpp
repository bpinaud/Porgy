#include <tulip/ConnectedTest.h>
#include <tulip/ExportModule.h>
#include <tulip/Graph.h>
#include <tulip/LayoutProperty.h>
#include <tulip/StringCollection.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>

using namespace tlp;
using namespace std;

static const char *paramHelp[] = {
    // name
    "Name of the graph being exported.",

    // author
    "Authors",

    // comments
    "Description of the graph."};

class PorgyRuleExport : public ExportModule {
private:
  std::map<unsigned int, unsigned int> nodeMap;
  std::map<unsigned int, unsigned int> edgeMap;

  void buildNodeMap(Graph *sourceGraph) {
    unsigned int newNode_id = 0;
    for (auto currentNode : sourceGraph->nodes()) {
      nodeMap.insert(std::pair<unsigned int, unsigned int>(currentNode.id, newNode_id));
      newNode_id++;
    }
  }

  void buildEdgeMap(Graph *sourceGraph) {
    unsigned int newEdge_id = 0;
    for (auto currentEdge : sourceGraph->edges()) {
      edgeMap.insert(std::pair<unsigned int, unsigned int>(currentEdge.id, newEdge_id));
      newEdge_id++;
    }
  }

  void transformAlgorithm(Graph *sourceGraph) {
    std::string algorithm;
    sourceGraph->getAttribute<std::string>("Rule Algorithm", algorithm);
    std::string ancient, recent, strIdentifier = "#?*";
    std::size_t index;
    for (auto it : nodeMap) {
        auto nodest = to_string(it.first);
        auto edgest = to_string(it.second);

      ancient = "node(" + nodest + ")";
      recent = "node(" + strIdentifier + edgest + ")";
      index = algorithm.find(ancient);
      while (index != std::string::npos) {
        algorithm.replace(index, ancient.length(), recent);
        index = algorithm.find(ancient);
      }
      ancient = "n(" + nodest + ")";
      recent = "n(" + strIdentifier + edgest + ")";
      index = algorithm.find(ancient);
      while (index != std::string::npos) {
        algorithm.replace(index, ancient.length(), recent);
        index = algorithm.find(ancient);
      }
    }
    for (auto it : edgeMap) {
      stringstream ss1;
      ss1 << it.first;
      ancient = "edge(" + ss1.str() + ")";
      stringstream ss2;
      ss2 << it.second;
      recent = "edge(" + strIdentifier + ss2.str() + ")";
      index = algorithm.find(ancient);
      while (index != std::string::npos) {
        algorithm.replace(index, ancient.length(), recent);
        index = algorithm.find(ancient);
      }
      ancient = "e(" + ss1.str() + ")";
      recent = "e(" + strIdentifier + ss2.str() + ")";
      index = algorithm.find(ancient);
      while (index != std::string::npos) {
        algorithm.replace(index, ancient.length(), recent);
        index = algorithm.find(ancient);
      }
    }
    ancient = "(" + strIdentifier;
    recent = "(";
    index = algorithm.find(ancient);
    while (index != std::string::npos) {
      algorithm.replace(index, ancient.length(), recent);
      index = algorithm.find(ancient);
    }
    sourceGraph->setAttribute<std::string>("Rule Algorithm", algorithm);
  }

public:
  PLUGININFORMATION("Rule Export", "Name", "Date", "Description", "1.1",
                    PorgyConstants::CATEGORY_NAME)
  PorgyRuleExport(tlp::PluginContext *context) : ExportModule(context, {"porgy.rule"}) {
    addInParameter<string>("name", paramHelp[0], "");
    addInParameter<string>("author", paramHelp[1], "");
    addInParameter<string>("text::comments", paramHelp[2], "This file was generated by Porgy.");

    addDependency("TLP Export", "1.0");
  }

  bool exportGraph(ostream &os) override {
    if (graph != nullptr) {
      Observable::holdObservers();

      buildNodeMap(graph);
      buildEdgeMap(graph);

      std::string algorithm;
      graph->getAttribute<std::string>("Rule Algorithm", algorithm);
      transformAlgorithm(graph);

      bool result = tlp::exportGraph(graph, os, "TLP Export", *dataSet, pluginProgress);

      graph->setAttribute<std::string>("Rule Algorithm", algorithm);

      Observable::unholdObservers();
      return result;
    } else {
      std::cerr << "Graph is empty" << endl;
      return false;
    }
  }
};

PLUGIN(PorgyRuleExport)
