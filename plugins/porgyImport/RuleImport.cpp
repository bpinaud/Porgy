#include <tulip/ConnectedTest.h>
#include <tulip/Graph.h>
#include <tulip/ImportModule.h>
#include <tulip/LayoutProperty.h>
#include <tulip/StringCollection.h>

#include <portgraph/PorgyConstants.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>

#include <set>

using namespace tlp;
using namespace std;

namespace {

const char *paramHelp[] = {
    // filename
    "The file to import with an extension .porgy.rule (instead of .tlp). The "
    "file must be composed of one graph which will be imported as a rule."};
}

static bool endsWith(std::string const &value, std::string const &ending) {
  if (ending.size() > value.size())
    return false;
  return std::equal(ending.crbegin(), ending.crend(), value.crbegin());
}

class PorgyRuleImport : public ImportModule {
private:
  TLP_HASH_MAP<node, node> nodeMap;
  TLP_HASH_MAP<edge, edge> edgeMap;

  void copyNodes(Graph *sourceGraph, Graph *targetGraph) {
    targetGraph->reserveNodes(sourceGraph->numberOfNodes());
    for (node currentNode : sourceGraph->nodes()) {
      node newNode = targetGraph->addNode();
      nodeMap.insert(make_pair(currentNode, newNode));
    }
  }

  void copyEdges(Graph *sourceGraph, Graph *targetGraph) {
    targetGraph->reserveEdges(sourceGraph->numberOfEdges());
    for (edge currentEdge : sourceGraph->edges()) {
      const pair<node, node> &ends = sourceGraph->ends(currentEdge);
      node sourceNode = nodeMap[ends.first];
      assert(sourceNode.isValid());
      node targetNode = nodeMap[ends.second];
      assert(targetNode.isValid());
      edge newEdge = targetGraph->addEdge(sourceNode, targetNode);
      edgeMap.insert(make_pair(currentEdge, newEdge));
    }
  }

  void copyProperties(Graph *sourceGraph, Graph *targetGraph) {
    // all properties are inherited
    for (const string &currentPropertyName : sourceGraph->getProperties()) {
      tlp::PropertyInterface *currentProperty = sourceGraph->getProperty(currentPropertyName);
      tlp::PropertyInterface *newProperty;
      if (currentPropertyName == PorgyConstants::ARITY ||
          currentPropertyName == PorgyConstants::N || currentPropertyName == PorgyConstants::M ||
          currentPropertyName == PorgyConstants::W)
        newProperty = targetGraph->getProperty(currentPropertyName, currentProperty->getTypename());
      else if (currentPropertyName == PorgyConstants::RULE_SIDE)
        newProperty = PorgyTlpGraphStructure::getRulesRoot(targetGraph)
                          ->getProperty(currentPropertyName, currentProperty->getTypename());
      else if (sourceGraph->existLocalProperty(currentPropertyName.substr(
                   0,
                   currentPropertyName.length() -
                       PorgyConstants::MATCHING_PROPERTIES_SUFFIX.length()))) {
        if (PorgyConstants::MATCHING_PROPERTIES_SUFFIX.length() < currentPropertyName.length()) {
          newProperty = PorgyTlpGraphStructure::getRulesRoot(targetGraph)
                            ->getProperty(currentPropertyName, currentProperty->getTypename());
          // std::cout << currentPropertyName << " match substr: " <<
          // currentPropertyName.substr(PorgyConstants::MATCHING_PROPERTIES_PREFIX.length(),
          // currentPropertyName.length()-PorgyConstants::MATCHING_PROPERTIES_SUFFIX.length())
          // << " with start: " <<
          // PorgyConstants::MATCHING_PROPERTIES_PREFIX.length() << " end: " <<
          // currentPropertyName.length()-PorgyConstants::MATCHING_PROPERTIES_SUFFIX.length()
          // << endl;
        } else
          newProperty = targetGraph->getRoot()->getProperty(currentPropertyName,
                                                            currentProperty->getTypename());
      } else if (sourceGraph->existLocalProperty(currentPropertyName.substr(
                     0,
                     currentPropertyName.length() -
                         PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX.length()))) {
        if (PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX.length() <
            currentPropertyName.length()) {
          newProperty = PorgyTlpGraphStructure::getRulesRoot(targetGraph)
                            ->getProperty(currentPropertyName, currentProperty->getTypename());
          // std::cout << currentPropertyName << " comp substr: " <<
          // currentPropertyName.substr(PorgyConstants::MATCHING_PROPERTIES_PREFIX.length(),
          // currentPropertyName.length()-PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX.length())
          // << " with start: " << 0 << " end: " <<
          // currentPropertyName.length()-PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX.length()
          // << endl;
        } else
          newProperty = targetGraph->getRoot()->getProperty(currentPropertyName,
                                                            currentProperty->getTypename());
      } else
        newProperty = targetGraph->getRoot()->getProperty(currentPropertyName,
                                                          currentProperty->getTypename());
      // possible conflict with NodeType (substr to N) and viewLabelColor
      // (substr to viewLabel)
      for (node n : sourceGraph->nodes()) {
        node targetNode = nodeMap[n];
        assert(targetNode.isValid());
        newProperty->setNodeStringValue(targetNode, currentProperty->getNodeStringValue(n));
      }
      for (edge e : sourceGraph->edges()) {
        edge targetEdge = edgeMap[e];
        assert(targetEdge.isValid());
        newProperty->setEdgeStringValue(targetEdge, currentProperty->getEdgeStringValue(e));
      }
    }
  }

  void copyRuleAttributes(Graph *sourceGraph, Graph *targetGraph) {
    TLP_HASH_SET<std::string> allowed_property(PorgyConstants::RULE_IMPORT_ALLOWED,
                                               PorgyConstants::RULE_IMPORT_ALLOWED +
                                                   PorgyConstants::RULE_IMPORT_ALLOWED_SIZE);
    for (const std::pair<std::string, tlp::DataType *> &currentData :
         sourceGraph->getAttributes().getValues()) {
      if (currentData.first == "name")
        continue;
      if (allowed_property.find(currentData.first) != allowed_property.end()) {
        std::string temp;
        sourceGraph->getAttribute<std::string>(currentData.first, temp);
        targetGraph->setAttribute<std::string>(currentData.first, temp);
      }
    }
  }

  void transformAlgorithm(Graph *targetGraph) {
    std::string algorithm;
    targetGraph->getAttribute<std::string>(PorgyConstants::RULE_ALGORITHM, algorithm);
    std::string ancient, recent, strIdentifier = "#?*";
    std::size_t index;
    for (const auto &it : nodeMap) {
      ancient = "node(" + to_string(it.first.id) + ")";
      recent = "node(" + strIdentifier + to_string(it.second.id) + ")";
      index = algorithm.find(ancient);
      while (index != std::string::npos) {
        algorithm.replace(index, ancient.length(), recent);
        index = algorithm.find(ancient);
      }
      ancient = "n(" + to_string(it.first.id) + ")";
      recent = "n(" + strIdentifier + to_string(it.second.id) + ")";
      index = algorithm.find(ancient);
      while (index != std::string::npos) {
        algorithm.replace(index, ancient.length(), recent);
        index = algorithm.find(ancient);
      }
    }
    for (const auto &it : edgeMap) {
      ancient = "edge(" + to_string(it.first.id) + ")";
      recent = "edge(" + strIdentifier + to_string(it.second.id) + ")";
      index = algorithm.find(ancient);
      while (index != std::string::npos) {
        algorithm.replace(index, ancient.length(), recent);
        index = algorithm.find(ancient);
      }
      ancient = "e(" + to_string(it.first.id) + ")";
      recent = "e(" + strIdentifier + to_string(it.second.id) + ")";
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
    targetGraph->setAttribute<std::string>(PorgyConstants::RULE_ALGORITHM, algorithm);
  }

public:
  PLUGININFORMATION("Rule Import", "Jason Vallet", "2014/06/31", "Description", "1.1",
                    PorgyConstants::CATEGORY_NAME)
  PorgyRuleImport(tlp::PluginContext *context) : ImportModule(context) {
    addInParameter<std::string>("file::filename", paramHelp[0], "");

    addDependency("TLP Import", "1.0");
    addDependency(PorgyConstants::REDRAW_GRAPH, "1.0");
  }

  std::list<std::string> fileExtensions() const override {
    list<string> l;
    l.push_back("porgy.rule");
    return l;
  }

  bool importGraph() override {
    string filename;
    dataSet->get("file::filename", filename);
    list<string> ext(fileExtensions());
    bool found = false;
    for (const auto &st : ext) {
      if (endsWith(filename, st)) {
        found = true;
        break;
      }
    }
    if (!found) {
      pluginProgress->setError("File format not supported. You have to use the "
                               "PORGY rule format (.porgy.rule).");
      return false;
    }

    Graph *rule = tlp::importGraph("TLP Import", *dataSet, pluginProgress);

    if (rule != nullptr) {
      Observable::holdObservers();
      string rulename = "";
      string baserulename = rule->getName();
      unsigned i = 0;
      while (rulename.empty()) {
        ++i;
        if (graph->getSubGraph(rule->getName()) != nullptr) {
          // already have a rule with this name
          rule->setName(baserulename + "_" + to_string(i));
        } else
          rulename = rule->getName();
      }

      Graph *newRule = graph->addSubGraph(rulename);

      copyNodes(rule, newRule);
      copyEdges(rule, newRule);
      copyProperties(rule, newRule);
      copyRuleAttributes(rule, newRule);
      transformAlgorithm(newRule);

      Observable::unholdObservers();
      delete rule;
      return true;
    } else {
      std::cerr << "problem when importing a new rule" << endl;
      return false;
    }
  }
};

PLUGIN(PorgyRuleImport)
