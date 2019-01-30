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
#include "graphstatemanager.h"

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Trace.h>

#include <porgy/graphsnapshotmanager.h>

#include <tulip/ColorProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/GlGraphRenderingParameters.h>
#include <tulip/StringProperty.h>

using namespace tlp;
using namespace std;

GraphState::GraphState() : graph(nullptr), rule(nullptr) {}

GraphState::GraphState(Graph *graph, const GraphVisualProperties &properties,
                       const string &stateName, Graph *rule)
    : graph(graph), properties(properties), name(stateName), rule(rule) {}

GraphStateManager::GraphStateManager(Graph *traceRoot, const vector<node> &models, QObject *parent)
    : QAbstractListModel(parent), traceRoot(nullptr) {

  setGraphData(traceRoot, models);
}

GraphStateManager::~GraphStateManager() {
  clearStateList();
}

void GraphStateManager::updateStateList(PorgyIntermediaryStateFlags flags) {
  beginResetModel();
  // Erase previous data.
  clearStateList();
  if (traceRoot != nullptr) {
    // Add state for each graph
    stateList.reserve(models.size());
    for (vector<node>::const_iterator from = models.begin(); from != models.end(); ++from) {
      // Create new state
      GraphState fromState = buildSimpleGraphState(*from);
      Trace trace(traceRoot);
      // Get the source of the current state.
      if (from != models.begin()) {
        edge transformation = traceRoot->existEdge(*from, *(from - 1), false);
        // If the last transformation was a simple transformation
        if (trace.getTransformationType(transformation) == Trace::Simple_Transformation) {
          fromState.rule = trace.getRuleForTransformation(transformation);
        }
      }
      stateList.push_back(fromState);

      // If there is transition
      vector<node>::const_iterator to = from + 1;
      if (to != models.end()) {
        // If we have a simple transformation show P, LHS, RHS
        edge transformation = traceRoot->existEdge(*from, *to, false);
        if (trace.getTransformationType(transformation) == Trace::Simple_Transformation) {
          Graph *rule = trace.getRuleForTransformation(transformation);
          // Highlight P
          if (flags.testFlag(ViewLHSP)) {
            GraphState s = buildStateForP(*from, *to, *from);
            s.rule = rule;
            stateList.push_back(s);
          }
          // Highligh left members
          if (flags.testFlag(ViewLHS)) {
            GraphState s = buildStateForL(*from, *to);
            s.rule = rule;
            stateList.push_back(s);
          }
          // Highligh right members
          if (flags.testFlag(ViewRHS)) {
            GraphState s = buildStateForR(*from, *to);
            s.rule = rule;
            stateList.push_back(s);
          }
        } else {
          // If the transformation is a P modification show PLHS,PRHS
          if (flags.testFlag(ViewLHSP)) {
            stateList.push_back(buildStateForP(*from, *to, *from));
          }
        }
      }
    }
  }
  endResetModel();
}

void GraphStateManager::setGraphData(Graph *graph, const vector<node> &models) {
  beginResetModel();
  // Supress old data.
  clearStateList();
  vector<node> elements = models;
  if (elements.empty() && graph != nullptr) {
    // Take all the nodes in the graph
    // Search the root node.
    node root = graph->getSource();
    if (root.isValid()) {
      // Fill elements
      elements.push_back(root);
      node current = root;
      while (graph->outdeg(current) != 0) {
        // error we can only generate states for one path
        if (graph->outdeg(current) > 1) {
          elements.clear();
          break;
        }
        node out = graph->getOutNode(current, 1);
        elements.push_back(out);
        current = out;
      }
    }
  }
  // Setting new data
  traceRoot = graph;
  this->models = elements;
  endResetModel();
}

GraphState &GraphStateManager::getState(unsigned int stateNumber) {
  assert(stateList.size() > stateNumber);
  return stateList[stateNumber];
}

static void hideUnselectedElements(Graph *graph, tlp::BooleanProperty *elements, const float alpha,
                                   tlp::ColorProperty *propertyToUpdate, bool treatNodes = true,
                                   bool treatEdges = true) {
  if (treatNodes) {
    for (node n : elements->getNodesEqualTo(false, graph)) {
      tlp::Color c = propertyToUpdate->getNodeValue(n);
      c.setA(alpha);
      propertyToUpdate->setNodeValue(n, c);
    }
  }
  if (treatEdges) {
    for (edge e : elements->getEdgesEqualTo(false, graph)) {
      tlp::Color c = propertyToUpdate->getEdgeValue(e);
      c.setA(alpha);
      propertyToUpdate->setEdgeValue(e, c);
    }
  }
}

GraphState GraphStateManager::buildSimpleGraphState(node model) const {
  // Build simple graph state.
  Graph *modelGraph = traceRoot->getNodeMetaInfo(model);
  // Default graph state don't override any properties.
  return GraphState(
      modelGraph,
      GraphVisualProperties(modelGraph, VisualPropertiesFlags(AllColorProperties | ElementLayout |
                                                              ElementBorderWidth)),
      modelGraph->getName());
}
GraphState GraphStateManager::buildStateForP(node fromModel, node toModel, node modelToSearchP) {
  assert(PorgyTlpGraphStructure::isTraceGraph(traceRoot));
  Trace trace(traceRoot);
  // Get the edge between theese two models.
  edge e = traceRoot->existEdge(fromModel, toModel);
  // Add from with P highlighted
  Graph *modelGraph = traceRoot->getNodeMetaInfo(modelToSearchP);
  GraphState state = buildSimpleGraphState(modelToSearchP);
  assert(state.properties.getElementSize() == nullptr);
  string modelName = modelGraph->getName();
  string PPropertyName = trace.getTransformationPPropertyNameProperty()->getEdgeValue(e);
  modelName += string(" - ");
  modelName += PPropertyName;
  state.name = modelName;

  // Get P value
  BooleanProperty PProperty(modelGraph);
  if ((modelToSearchP == fromModel ? trace.getSourcePPropertyForTransformation(e, &PProperty)
                                   : trace.getTargetPPropertyForTransformation(e, &PProperty))) {
    // build default visual state
    // Create a new color property.
    ColorProperty *colorProperty = new ColorProperty(modelGraph);
    DoubleProperty *borderWidth = new DoubleProperty(modelGraph);
    createdProperties.push_back(colorProperty);
    createdProperties.push_back(borderWidth);
    // Copy default color property
    colorProperty->copy(modelGraph->getProperty<ColorProperty>("viewBorderColor"));
    borderWidth->copy(modelGraph->getProperty<DoubleProperty>("viewBorderWidth"));
    // Highlight p values
    setValueForElements<ColorProperty, Color, Color>(
        modelGraph, &PProperty, GlDefaultSelectionColorManager::getDefaultSelectionColor(), Color(),
        colorProperty, true, true);
    setValueForElements<DoubleProperty, double, double>(modelGraph, &PProperty, 5, 1, borderWidth,
                                                        true, true);
    // Overload default colorProperty
    state.properties.setElementBorderColor(colorProperty);
    state.properties.setElementBorderWidth(borderWidth);
  }
  return state;
}

GraphState GraphStateManager::buildStateForL(node fromModel, node toModel) {
  assert(PorgyTlpGraphStructure::isTraceGraph(traceRoot));
  // Default visual values values
  GraphState state = buildSimpleGraphState(fromModel);
  assert(state.properties.getElementSize() == nullptr);
  Trace trace(traceRoot);
  // Get the edge between theese two models.
  edge e = traceRoot->existEdge(fromModel, toModel);
  // Getting left members in a boolean property.
  Graph *fromModelGraph = traceRoot->getNodeMetaInfo(fromModel);
  // Rule name of the transformation.
  string ruleName = trace.getRuleNameForTransformation(e);
  // Set the name of the state.
  string modelName = fromModelGraph->getName();
  modelName += string(" - ");
  modelName += ruleName;
  modelName += "(LHS)";
  state.name = modelName;

  BooleanProperty lelements(fromModelGraph);
  if (trace.getLeftMembersForTransformation(e, &lelements)) {
    // build default visual state
    // Create a new color property.
    ColorProperty *colorProperty = new ColorProperty(fromModelGraph);
    ColorProperty *bordercolorProperty = new ColorProperty(fromModelGraph);
    ColorProperty *labelcolor = new ColorProperty(fromModelGraph);
    ColorProperty *borderlabelcolor = new ColorProperty(fromModelGraph);
    createdProperties.push_back(colorProperty);
    createdProperties.push_back(bordercolorProperty);
    createdProperties.push_back(labelcolor);
    createdProperties.push_back(borderlabelcolor);
    // Copy default color property
    colorProperty->copy(fromModelGraph->getProperty<ColorProperty>("viewColor"));
    bordercolorProperty->copy(fromModelGraph->getProperty<ColorProperty>("viewBorderColor"));
    labelcolor->copy(fromModelGraph->getProperty<ColorProperty>("viewLabelColor"));
    borderlabelcolor->copy(fromModelGraph->getProperty<ColorProperty>("viewLabelBorderColor"));
    // Highlight p values
    hideUnselectedElements(fromModelGraph, &lelements, PorgyConstants::hidden_alpha, colorProperty);
    hideUnselectedElements(fromModelGraph, &lelements, PorgyConstants::hidden_alpha,
                           bordercolorProperty);
    hideUnselectedElements(fromModelGraph, &lelements, PorgyConstants::hidden_alpha, labelcolor);
    hideUnselectedElements(fromModelGraph, &lelements, PorgyConstants::hidden_alpha,
                           borderlabelcolor);
    // Overload default colorProperty
    state.properties.setElementColor(colorProperty);
    state.properties.setElementBorderColor(colorProperty);
    state.properties.setElementLabelColor(labelcolor);
    state.properties.setElementLabelBorderColor(borderlabelcolor);
  }
  return state;
}

GraphState GraphStateManager::buildStateForR(node fromModel, node toModel) {
  assert(PorgyTlpGraphStructure::isTraceGraph(traceRoot));
  Trace trace(traceRoot);
  // Get the edge between theese two models.
  edge e = traceRoot->existEdge(fromModel, toModel);
  // Default visual values values
  GraphState state = buildSimpleGraphState(toModel);
  assert(state.properties.getElementSize() == nullptr);
  // Getting left members in a boolean property.
  Graph *toModelGraph = traceRoot->getNodeMetaInfo(toModel);
  string modelName = toModelGraph->getName();
  // Rule name of the transformation.
  string ruleName = trace.getRuleNameForTransformation(e);
  modelName += string(" - ");
  modelName += ruleName;
  modelName += "(RHS)";
  state.name = modelName;
  BooleanProperty elements(toModelGraph);

  if (trace.getRightMembersForTransformation(e, &elements)) {
    ColorProperty *colorProperty = new ColorProperty(toModelGraph);
    ColorProperty *bordercolorProperty = new ColorProperty(toModelGraph);
    ColorProperty *labelcolor = new ColorProperty(toModelGraph);
    ColorProperty *labelbordercolor = new ColorProperty(toModelGraph);
    createdProperties.push_back(colorProperty);
    createdProperties.push_back(bordercolorProperty);
    createdProperties.push_back(labelcolor);
    createdProperties.push_back(labelbordercolor);
    // Copy default color property
    colorProperty->copy(toModelGraph->getProperty<ColorProperty>("viewColor"));
    bordercolorProperty->copy(toModelGraph->getProperty<ColorProperty>("viewBorderColor"));
    labelcolor->copy(toModelGraph->getProperty<ColorProperty>("viewLabelColor"));
    labelbordercolor->copy(toModelGraph->getProperty<ColorProperty>("viewLabelBorderColor"));
    // Highlight p values
    hideUnselectedElements(toModelGraph, &elements, PorgyConstants::hidden_alpha, colorProperty);
    hideUnselectedElements(toModelGraph, &elements, PorgyConstants::hidden_alpha,
                           bordercolorProperty);
    hideUnselectedElements(toModelGraph, &elements, PorgyConstants::hidden_alpha, labelcolor);
    hideUnselectedElements(toModelGraph, &elements, PorgyConstants::hidden_alpha, labelbordercolor);
    // Overload default colorProperty
    state.properties.setElementColor(colorProperty);
    state.properties.setElementBorderColor(colorProperty);
    state.properties.setElementLabelColor(labelcolor);
    state.properties.setElementLabelBorderColor(labelbordercolor);
  }
  return state;
}

void GraphStateManager::clearStateList() {
  stateList.clear();
  clearCreatedProperties();
}

void GraphStateManager::clearCreatedProperties() {
  for (vector<PropertyInterface *>::iterator it = createdProperties.begin();
       it != createdProperties.end(); ++it) {
    delete *it;
  }
  createdProperties.clear();
}

int GraphStateManager::rowCount(const QModelIndex &) const {
  return stateList.size();
}
QVariant GraphStateManager::data(const QModelIndex &index, int role) const {
  if (hasIndex(index.row(), index.column())) {
    if (role == Qt::DisplayRole) {
      return QString::fromUtf8(stateList[index.row()].name.c_str());
    } else if (role == Qt::DecorationRole) {
      return GraphSnapshotManager::takeSingleSnapshot(stateList[index.row()].graph,
                                                      QSize(256, 256));
    }
  }

  return QVariant();
}
