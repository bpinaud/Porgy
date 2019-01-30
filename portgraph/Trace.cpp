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
#include <tulip/ColorProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/GraphProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/TulipViewSettings.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortNode.h>
#include <portgraph/Trace.h>

using namespace tlp;
using namespace std;

const std::unordered_map<Trace::TransformationType, const char *, PorgyConstants::EnumClassHash>
    Trace::TransformationTypeToString = {
        {Trace::TransformationType::Simple_Transformation, "Simple Transformation"},
        {Trace::TransformationType::Strategy_Transformation, "Strategy"},
        {Trace::TransformationType::Focus_Transformation, "Focus"},
        {Trace::TransformationType::Ban_Transformation, "Ban"}};

const std::unordered_map<Trace::TransformationParallelInformation, const char *,
                         PorgyConstants::EnumClassHash>
    Trace::TransformationParallelInformationToString = {
        {Trace::TransformationParallelInformation::No_parallel, "No parallel"},
        {Trace::TransformationParallelInformation::Parallel, "Real parallel"}};

const std::unordered_map<Trace::NodeType, const char *, PorgyConstants::EnumClassHash>
    Trace::NodeTypeToString = {{Trace::NodeType::Failure_Node, "Failure"},
                               {Trace::NodeType::Model_Node, "Graph"}};

// viewBorderWidth
class ViewborderWidthCalculator : public DoubleMinMaxProperty::MetaValueCalculator {
public:
  void computeMetaValue(AbstractProperty<DoubleType, DoubleType, NumericProperty> *width, node mN,
                        Graph *, Graph *) override {
    // meta node border width is 1
    width->setNodeValue(mN, 1);
  }
};

// Color properties
class ViewColCalculator : public AbstractColorProperty::MetaValueCalculator {
public:
  void computeMetaValue(AbstractColorProperty *color, node mN, Graph *, Graph *) override {
    // meta node color is half opaque white
    color->setNodeValue(mN, Color(255, 255, 255, 0));
  }

  void computeMetaValue(AbstractColorProperty *color, edge mE, Iterator<edge> *itE,
                        Graph *) override {
    // meta edge color is the color of the first underlying edge
    color->setEdgeValue(mE, color->getEdgeValue(itE->next()));
  }
};

// Integer Properties
class ViewIntCalculator : public IntegerMinMaxProperty::MetaValueCalculator {
public:
  void computeMetaValue(AbstractProperty<IntegerType, IntegerType, NumericProperty> *s, node mN,
                        Graph *, Graph *) override {
    if (s->getName() == "viewShape")
      s->setNodeValue(mN, NodeShape::Square);
    else if (s->getName() == PorgyConstants::TYPE_PROPERTY)
      s->setNodeValue(mN, static_cast<unsigned>(Trace::NodeType::Model_Node));
  }
};

// String properties
class ViewlabelCalculator : public AbstractStringProperty::MetaValueCalculator {
public:
  void computeMetaValue(AbstractStringProperty *label, node mN, Graph *metagraph,
                        Graph *) override {
    // use the metagraph name
    label->setNodeValue(mN, metagraph->getName());
  }
};

static ViewborderWidthCalculator vbordercalc;
static ViewColCalculator vcolorcalc;
static ViewIntCalculator vintcalc;
static ViewlabelCalculator vlabelCalc;

Trace::Trace(Graph *g) : GraphDecorator(g) {
  // assert(PorgyTlpGraphStructure::isTraceGraph(g));
  graph_component->getProperty<DoubleProperty>("viewBorderWidth")
      ->setMetaValueCalculator(&vbordercalc);
  graph_component->getProperty<ColorProperty>("viewColor")->setMetaValueCalculator(&vcolorcalc);
  graph_component->getProperty<IntegerProperty>("viewShape")->setMetaValueCalculator(&vintcalc);
  getTypeProperty()->setMetaValueCalculator(&vintcalc);
  graph_component->getProperty<StringProperty>("viewLabel")->setMetaValueCalculator(&vlabelCalc);
}

IntegerProperty *Trace::getTransformationRuleProperty() const {
  Graph *traceRoot = PorgyTlpGraphStructure::getTraceRoot(graph_component);
  bool initProperty = !traceRoot->existLocalProperty(PorgyConstants::TRANSFORMATION_RULE_PROPERTY);
  IntegerProperty *property =
      traceRoot->getLocalProperty<IntegerProperty>(PorgyConstants::TRANSFORMATION_RULE_PROPERTY);
  if (initProperty) {
    property->setAllNodeValue(UINT_MAX);
    property->setAllEdgeValue(UINT_MAX);
  }
  return property;
}
IntegerProperty *Trace::getTransformationInstanceProperty() const {
  Graph *traceRoot = PorgyTlpGraphStructure::getTraceRoot(graph_component);
  bool initProperty =
      !traceRoot->existLocalProperty(PorgyConstants::TRANSFORMATION_INSTANCE_PROPERTY);
  IntegerProperty *property = traceRoot->getLocalProperty<IntegerProperty>(
      PorgyConstants::TRANSFORMATION_INSTANCE_PROPERTY);
  if (initProperty) {
    property->setAllNodeValue(UINT_MAX);
    property->setAllEdgeValue(UINT_MAX);
  }
  return property;
}

IntegerProperty *Trace::getTypeProperty() const {
  return PorgyTlpGraphStructure::getTraceRoot(graph_component)
      ->getLocalProperty<IntegerProperty>(PorgyConstants::TYPE_PROPERTY);
}

BooleanProperty *Trace::getSelectionProperty() {
  return PorgyTlpGraphStructure::getTraceRoot(graph_component)
      ->getLocalProperty<BooleanProperty>("viewSelection");
}

IntegerProperty *Trace::getTransformationParallelInformationProperty() const {
  return PorgyTlpGraphStructure::getTraceRoot(graph_component)
      ->getLocalProperty<IntegerProperty>(
          PorgyConstants::TRANSFORMATION_PARALLEL_INFORMATION_PROPERTY);
}

StringProperty *Trace::getTransformationStrategyProperty() const {
  return PorgyTlpGraphStructure::getTraceRoot(graph_component)
      ->getLocalProperty<StringProperty>(PorgyConstants::TRANSFORMATION_STRATEGY_PROPERTY);
}

StringProperty *Trace::getTransformationPPropertyNameProperty() const {
  return PorgyTlpGraphStructure::getTraceRoot(graph_component)
      ->getLocalProperty<StringProperty>(PorgyConstants::TRANSFORMATION_PPROPERTYNAME_PROPERTY);
}

StringProperty *Trace::getTransformationBanPropertyNameProperty() const {
  return PorgyTlpGraphStructure::getTraceRoot(graph_component)
      ->getLocalProperty<StringProperty>(PorgyConstants::TRANSFORMATION_BANPROPERTYNAME_PROPERTY);
}

bool Trace::redraw(string &errMsg, PluginProgress *pp) {
  Observable::holdObservers();
  LayoutProperty *layout = graph_component->getLocalProperty<LayoutProperty>("viewLayout");
  bool res =
      graph_component->applyPropertyAlgorithm(PorgyConstants::REDRAW_TRACE, layout, errMsg, nullptr, pp);
  Observable::unholdObservers();
  return res;
}

edge Trace::addStrategyTransformationEdge(const node source, const node target, const string& p,
                                          const string &ban, const string &strategyCode) const {
#ifndef NDEBUG
  cerr << __PRETTY_FUNCTION__ << ": Add a strategy edge to the derivation tree between nodes "
       << source << " and " << target << endl;
#endif
  edge meta_e =
      addTransformationEdge(source, target, p, ban, TransformationType::Strategy_Transformation);
  // Set strategy specific var
  getTransformationStrategyProperty()->setEdgeValue(meta_e, strategyCode);
  graph_component->getProperty<ColorProperty>("viewColor")
      ->setEdgeValue(meta_e, PorgyConstants::DERIVATION_STRATEGY_EDGE_COLOR);
  graph_component->getProperty<ColorProperty>("viewBorderColor")
      ->setEdgeValue(meta_e, PorgyConstants::DERIVATION_STRATEGY_EDGE_COLOR);
  return meta_e;
}

edge Trace::addSimpleTransformationEdge(const node source, const node target, const string& p,
                                        const string &ban, Graph *instanceGraph,
                                        const int rule_graph_id) const {
  edge meta_e =
      addTransformationEdge(source, target, p, ban, TransformationType::Simple_Transformation);
  // Set simple transformation specific var
  // Instance graph can be nullptr in case of an edge to failure node
  getTransformationInstanceProperty()->setEdgeValue(
      meta_e, instanceGraph == nullptr ? UINT_MAX : instanceGraph->getId());
  getTransformationRuleProperty()->setEdgeValue(meta_e, rule_graph_id);
  getTransformationParallelInformationProperty()->setEdgeValue(
      meta_e, TransformationParallelInformation::No_parallel);

  return meta_e;
}

edge Trace::addFocusTransformationEdge(const node source, const node target, const string& p,
                                       Graph *instanceGraph,
                                       const PorgyConstants::strat_focus_op op) const {
  edge meta_e;
  if (op == PorgyConstants::strat_focus_op::SETBAN)
    meta_e =
        addTransformationEdge(source, target, "", p, TransformationType::Ban_Transformation);
  else
    meta_e =
        addTransformationEdge(source, target, p, "", TransformationType::Focus_Transformation);
  // Set simple transformation specific var
  // Instance graph can be nullptr in case of an edge to a failure node
  getTransformationInstanceProperty()->setEdgeValue(
      meta_e, instanceGraph == nullptr ? UINT_MAX : instanceGraph->getId());
  getTransformationParallelInformationProperty()->setEdgeValue(
      meta_e, TransformationParallelInformation::No_parallel);
  // getTransformationRuleProperty()->setEdgeValue(meta_e,op);
  return meta_e;
}

edge Trace::addOtherEdge(const node src, const node tgt,
                         TransformationType transformationType) const {
  return addTransformationEdge(src, tgt, "", "", transformationType);
}

edge Trace::addTransformationEdge(const node src, const node tgt, const string& pname,
                                  const string& ban_name,TransformationType transformationType) const {
    edge meta_e = graph_component->addEdge(src, tgt);
    // Generic information
    getTypeProperty()->setEdgeValue(meta_e, static_cast<unsigned>(transformationType));
    graph_component->getProperty<IntegerProperty>("viewTgtAnchorShape")->setEdgeValue(meta_e, EdgeExtremityShape::Arrow);

    getTransformationPPropertyNameProperty()->setEdgeValue(meta_e, pname);
    getTransformationBanPropertyNameProperty()->setEdgeValue(meta_e, ban_name);
    return meta_e;
}

node Trace::createMetaNode(Graph *content, bool multiEdges, bool delAllEdge) {
  graph_component->addNodes(content->nodes());
  graph_component->addEdges(content->edges());
  node meta = graph_component->createMetaNode(content, multiEdges, delAllEdge);
  return meta;
}

node Trace::addFailureNode(const string &name_src, const int rule_graph_id,
                           const string &PProperty, const string & BanProperty) const {
  ColorProperty *col = graph_component->getProperty<ColorProperty>("viewColor");

  node src = findNode(name_src);
  node n = graph_component->addNode();
  col->setNodeValue(n, Color(255, 0, 0, 255));
  getTransformationRuleProperty()->setNodeValue(n, rule_graph_id);
  addSimpleTransformationEdge(src, n, PProperty, BanProperty, nullptr, rule_graph_id);
  getTypeProperty()->setNodeValue(n, static_cast<unsigned>(NodeType::Failure_Node));
  graph_component->setAttribute<node>(PorgyConstants::FAILURE_NODE_ADDED, n);
  return n;
}

bool Trace::isFailureNode(const node &n) const {
  return (getTypeProperty()->getNodeValue(n) == Failure_Node);
}

bool Trace::isModelNode(const node &n) const {
  return (getTypeProperty()->getNodeValue(n) == Model_Node);
}

bool Trace::isStrategyEdge(const edge &e) const {
  return (getTypeProperty()->getEdgeValue(e) == Strategy_Transformation);
}

bool Trace::isSimpleTransformationEdge(const edge &e) const {
  return (getTypeProperty()->getEdgeValue(e) == Simple_Transformation);
}

// string Trace::existEdge(const string &node_src, const string &edge_label)
// const {
//    edge e;
//    StringProperty
//    *label=graph_component->getProperty<StringProperty>("viewLabel");
//    forEach(e, graph_component->getOutEdges(findNode(node_src))) {
//        if(label->getEdgeValue(e).compare(edge_label)==0) {
//            return label->getNodeValue(graph_component->target(e));
//        }
//    }
//    return string("");
//}

// bool Trace::existEdge(const string &node_src, const node node_dst, const
// string &edge_label) const {
//    StringProperty
//    *label=graph_component->getProperty<StringProperty>("viewLabel");
//    //peut pas utiliser existEdge car il peut y avoir plusieurs arêtes entre
//    deux sommets
//    edge e;
//    forEach(e, graph_component->getOutEdges(findNode(node_src))) {
//        if(graph_component->target(e)==node_dst)
//            if(label->getEdgeValue(e).compare(edge_label)==0)
//                return true;
//    }

//    return false;
//}

node Trace::findNode(const string &str) const {
  StringProperty *label = graph_component->getProperty<StringProperty>("viewLabel");
  auto *itn = label->getNodesEqualTo(str);
  node n;
  if (itn->hasNext())
    n = itn->next();
  delete itn;
  return n;
}

Graph *Trace::getInstanceForTransformation(edge e) const {
  // Get the source model graph.
  assert(graph_component->isElement(e));
  if (graph_component->isMetaNode(graph_component->source(e)))
    return graph_component->getRoot()->getDescendantGraph(
        getTransformationInstanceProperty()->getEdgeValue(e));
  else
    return nullptr;
}

Graph *Trace::getRuleForTransformation(tlp::edge e) const {
  return graph_component->getRoot()->getDescendantGraph(
      getTransformationRuleProperty()->getEdgeValue(e));
}
Graph *Trace::getRuleForTransformation(tlp::node n) const {
  return graph_component->getRoot()->getDescendantGraph(
      getTransformationRuleProperty()->getNodeValue(n));
}
string Trace::getRuleNameForTransformation(edge e) const {
  assert(graph_component->isElement(e));
  Graph *rule = getRuleForTransformation(e);
  return rule != nullptr ? rule->getName() : string();
}

bool Trace::getBanFocusPropertyForTransformation(edge e, BooleanProperty *resultProperty,
                                                 EdgeExtremitiy origin, const bool isBan) const {
  assert(graph_component->isElement(e));
  assert(resultProperty != nullptr);

  string propertyName;
  if (isBan)
    propertyName = getTransformationBanPropertyNameProperty()->getEdgeValue(e);
  else
    propertyName = getTransformationPPropertyNameProperty()->getEdgeValue(e);
  bool found = false;
  node model = origin == SourceNode ? graph_component->source(e) : graph_component->target(e);
  assert(graph_component->isMetaNode(model));
  Graph *modelGraph = graph_component->getNodeMetaInfo(model);
  if (!propertyName.empty() && modelGraph->existProperty(propertyName)) {
    // Copy property in result.
    resultProperty->copy(modelGraph->getProperty<BooleanProperty>(propertyName));
    found = true;
  }
  return found;
}

bool Trace::getLeftMembersForTransformation(edge e, tlp::BooleanProperty *result) const {
  assert(graph_component->isElement(e));
  // Getting instance graph
  Graph *instanceGraph = getInstanceForTransformation(e);
  bool returnResult = false;
  result->setAllNodeValue(false);
  result->setAllEdgeValue(false);
  if (instanceGraph != nullptr) {
    // Copying all instance graph elements into boolean property
    for (node n : instanceGraph->nodes()) {
      result->setNodeValue(n, true);
    }
    for (edge e : instanceGraph->edges()) {
      result->setEdgeValue(e, true);
    }
    returnResult = true;
  }
  return returnResult;
}
bool Trace::getRightMembersForTransformation(edge e, BooleanProperty *result) const {
  assert(isElement(e));
  // Getting target model graph
  node target = graph_component->target(e);
  assert(isMetaNode(target));
  Graph *targetGraph = getNodeMetaInfo(target);
  // Search for the NEW property containing all the elements created during the
  // rewriting.
  bool returnResult = false;
  if (targetGraph->existProperty(PorgyConstants::NEW)) {
    // Copy properties values.
    result->copy(targetGraph->getProperty<BooleanProperty>(PorgyConstants::NEW));
    returnResult = true;
  }
  return returnResult;
}

vector<node> Trace::getModelsMetaNodes() const {
  vector<node> v;
  GraphProperty *prop = graph_component->getProperty<GraphProperty>("viewMetaGraph");
  for (node n : prop->getNonDefaultValuatedNodes()) {
    v.push_back(n);
  }
  return v;
}

node Trace::nodeForModel(Graph *modelGraph) const {
  GraphProperty *prop = graph_component->getProperty<GraphProperty>("viewMetaGraph");
  auto *it = prop->getNodesEqualTo(modelGraph);
  node n;
  if (it->hasNext())
    n = it->next();
  delete it;
  return n;
}

list<string> Trace::computeStrategy(node from, node to) const {
  list<edge> path;
  node current = to;
  while (current.isValid()) {
    // While there is more nodes to treat
    if (current != from) {
      vector<edge> inEdges;
      for (edge in : graph_component->getInEdges(current)) {
        inEdges.push_back(in);
      }
      switch (inEdges.size()) {
      case 1: {
        // Only one inedge case of a rule application
        current = graph_component->source(inEdges[0]);
        path.push_front(inEdges[0]);
        break;
      }
      case 2: {
        // Two inedges case of a strategy application
        edge strategy = getTypeProperty()->getEdgeValue(inEdges[0]) == Strategy_Transformation
                            ? inEdges[0]
                            : inEdges[1];
        current = graph_component->source(strategy);
        path.push_front(strategy);
        break;
      }
      default:
        // No edge there is a gap in the tree return an empty strategy.
        // A model cannot have more than 2 in edges.
        return list<string>();
      }
    } else {
      current = node();
    }
  }
  return computeStrategy(path);
}

list<string> Trace::computeStrategy(const list<edge> &path) const {
  if (path.empty())
    return list<string>();

  node lastNode;
  list<string> strategy;
  for (auto &e : path) {
    const pair<node, node> &ends = graph_component->ends(e);
    if (e.isValid() && !graph_component->isElement(e))
      return list<string>();

    string command;
    int operationType = getTypeProperty()->getEdgeValue(e);
    switch (operationType) {
    case Simple_Transformation:
      command = getRuleNameForTransformation(e);
      break;
    case Focus_Transformation:
      command = string("setPos");
      command.append(")");
      break;
    case Ban_Transformation:
      command = string("setBan");
      command.append(")");
      break;
    case Strategy_Transformation:
      command = getTransformationStrategyProperty()->getEdgeValue(e);
      break;
    default:
      break;
    }
    if (!command.empty()) {
      strategy.push_back(command);
    }
    lastNode = ends.second;
  }
  return strategy;
}

string Trace::computeStrategyText(tlp::BooleanProperty *selection, node &sourceNode,
                                  node &destinationNode) const {
  Graph *subgraph = graph_component->addSubGraph(selection);
  list<string> strategylist;
  string strategy = "";
  // Test if the induced subgraph of selected elements is a simple path.
  if (PorgyTlpGraphStructure::graphIsASimplePath(subgraph)) {
    sourceNode = subgraph->getSource();
    destinationNode = subgraph->getSink();

    list<edge> edge_list;
    for (edge e : selection->getEdgesEqualTo(true, subgraph)) {
      edge_list.push_back(e);
    }
    strategylist = computeStrategy(edge_list);

    // build strategy
    for (const string &elt : strategylist) {
      if (!strategy.empty())
        strategy.append(";");
      strategy.append("one(" + elt + ")");
    }
  }
  graph_component->delSubGraph(subgraph);
  return strategy;
}

Trace::TransformationType Trace::getTransformationType(const edge &transformation) const {
  assert(graph_component->isElement(transformation));
  return static_cast<Trace::TransformationType>(getTypeProperty()->getEdgeValue(transformation));
}

Trace::TraceType Trace::type() const {
  if (graph_component->existAttribute(TRACETYPEATTRIBUTE)) {
    unsigned tt = 0;
    graph_component->getAttribute<unsigned int>(TRACETYPEATTRIBUTE, tt);
    return static_cast<TraceType>(tt);
  } else {
    return TraceType::DerivationTree;
  }
}

vector<string> Trace::followedPortnodes() const {
  vector<string> v;
  if (graph_component->existAttribute(PORTNODENAMEATTRIBUTE))
    graph_component->getAttribute<vector<string>>(PORTNODENAMEATTRIBUTE, v);

  return v;
}
vector<IntegerProperty *> Trace::followedPortnodesProperties() {
  vector<string> portnodes;
  graph_component->getAttribute<vector<string>>(PORTNODENAMEATTRIBUTE, portnodes);
  vector<IntegerProperty *> properties;
  properties.reserve(portnodes.size());
  for (auto &str : portnodes) {
    properties.push_back(graph_component->getLocalProperty<IntegerProperty>(str));
  }
  return properties;
}

std::list<IntegerProperty *> Trace::followPortNode(const list<string> &toFollow) {
  list<IntegerProperty *> fol;
  for (const auto &portnodeName : toFollow) {
    // Create the new property
    IntegerProperty *prop = graph_component->getLocalProperty<IntegerProperty>(portnodeName);
    // add nodes
    for (node n : graph_component->nodes()) {
      unsigned count = 0;
      PortGraphModel pg(graph_component->getNodeMetaInfo(n));
      for (PortNode *pn : pg.getPortNodes()) {
        if (pn->getName() == portnodeName)
          count++;
      }
      prop->setNodeValue(n, count);
    }
    vector<string> portnodes = followedPortnodes();
    portnodes.push_back(portnodeName);
    graph_component->setAttribute<vector<string>>(PORTNODENAMEATTRIBUTE, portnodes);
    graph_component->setAttribute<unsigned int>(TRACETYPEATTRIBUTE,
                                                static_cast<unsigned>(TraceType::Histogram));
    fol.push_back(prop);
  }
  return fol;
}

unsigned int followProperty_testValue(const string &comparisonOperator, const string &left,
                                      const string &right) {
  // unsigned int count = 0;
  char test = comparisonOperator[0];
  if (test == '!') {
    if (left != right)
      return 1;
    return 0;
  }
  if (test == '>') {
    if (left > right) {
      return 1;
    }
    test = comparisonOperator[1];
  }
  if (test == '<') {
    if (left < right)
      return 1;
    test = comparisonOperator[1];
  }
  if (test == '=') {
    if (left == right)
      return 1;
  }
  return 0;
}

IntegerProperty *Trace::followPropertyValue(const string &newPropertyName,
                                            const string &typeElement,
                                            const string &analysedPropertyName,
                                            const string &comparisonOperator,
                                            const string &propertyValue) {
  // Used in NewTrace to build a scatter plot, this variant allows to follow the
  // number of elements "typeElement"
  // for which the value of the property "analysedPropertyName" is
  // "comparisonOperator" than "propertyValue"

  // Create the new property
  IntegerProperty *prop = graph_component->getLocalProperty<IntegerProperty>(
      newPropertyName + "_" + analysedPropertyName + comparisonOperator + propertyValue);
  bool checkAllElements = (typeElement == "All elements"), checkNodes = (typeElement == "Nodes");
  // for each metanode
  for (node n : graph_component->nodes()) {
    unsigned count = 0;
    PortGraphModel pg(graph_component->getNodeMetaInfo(n));
    PropertyInterface *currentProperty = pg.getGraph()->getProperty(analysedPropertyName);
    // test the property value for the edges
    if (typeElement == "Edges" || checkAllElements) {
      for (edge e : pg.getEdges()) {
        count += followProperty_testValue(comparisonOperator,
                                          currentProperty->getEdgeStringValue(e), propertyValue);
      }
    }
    // test the property value for the port-nodes
    if (typeElement == "Port nodes" || checkAllElements || checkNodes) {
      for (PortNode *pn : pg.getPortNodes()) {
        count += followProperty_testValue(comparisonOperator,
                                          currentProperty->getNodeStringValue(pn->getCenter()),
                                          propertyValue);
      }
    }
    prop->setNodeValue(n, count);
  }
  graph_component->setAttribute<unsigned int>(TRACETYPEATTRIBUTE,
                                              static_cast<unsigned>(TraceType::Histogram));
  return prop;
}

IntegerProperty *Trace::depthProperty() {
  if (!graph_component->existLocalProperty(DEPTHPROPERTY)) {
    Observable::holdObservers();
    string err;
    DoubleProperty depth(graph_component);
    graph_component->applyPropertyAlgorithm("Dag Level", &depth, err);
    IntegerProperty *depthInteger =
        graph_component->getLocalProperty<IntegerProperty>(DEPTHPROPERTY);
    for (node n : graph_component->nodes()) {
      depthInteger->setNodeValue(n, static_cast<int>(depth.getNodeValue(n)));
    }
    Observable::unholdObservers();
    return depthInteger;
  } else {
    return graph_component->getLocalProperty<IntegerProperty>(DEPTHPROPERTY);
  }
}

DoubleProperty *Trace::computePropertyRatioValue(const string &antecedentPropertyName,
                                                 const string &currentPropertyName) {
  // Used in NewTrace

  // Create the new property
  DoubleProperty *prop = graph_component->getLocalProperty<DoubleProperty>(
      "ratio_" + currentPropertyName + "_" + antecedentPropertyName);
  PropertyInterface *antecedentProperty = graph_component->getProperty(antecedentPropertyName);
  PropertyInterface *currentProperty = graph_component->getProperty(currentPropertyName);
  tlp::node n = graph_component->getSource();

  ratioComputingDFS(n, 0, antecedentProperty, currentProperty, prop);

  return prop;
}

void Trace::ratioComputingDFS(tlp::node n, double previousValue,
                              PropertyInterface *antecedentProperty,
                              PropertyInterface *currentProperty, DoubleProperty *prop) {
  // Set an epsilon value to avoid NaN problems
  if (previousValue == 0) {
    previousValue = 0.000001;
  }
  double antecedent = static_cast<NumericProperty *>(antecedentProperty)->getNodeDoubleValue(n);
  double current = static_cast<NumericProperty *>(currentProperty)->getNodeDoubleValue(n);

  prop->setNodeValue(n, current / previousValue);
  for (node son : graph_component->getOutNodes(n)) {
    ratioComputingDFS(son, antecedent, antecedentProperty, currentProperty, prop);
  }
}
