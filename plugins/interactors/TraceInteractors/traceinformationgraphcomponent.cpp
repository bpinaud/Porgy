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
#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainView.h>
#include <tulip/GlMainWidget.h>
#include <tulip/TlpQtTools.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/Trace.h>

#include <QEvent>
#include <QMouseEvent>
#include <QToolTip>

#include "TraceInformationConfigWidget.h"
#include "traceinformationgraphcomponent.h"
#include "traceinformationinteractor.h"
#include "traceinformationmetanoderenderer.h"

using namespace tlp;
using namespace std;

TraceInformationGraphComponent::TraceInformationGraphComponent(TraceInformationInteractor *parent)
    : _glMainWidget(nullptr), _originalRenderer(nullptr), _interactorComponentRenderer(nullptr),
      parentInteractor(parent) {}

TraceInformationGraphComponent::TraceInformationGraphComponent(
    const TraceInformationGraphComponent &trace)
    : GLInteractorComponent(), Observable(), _glMainWidget(trace._glMainWidget),
      _originalRenderer(trace._originalRenderer),
      _interactorComponentRenderer(trace._interactorComponentRenderer),
      parentInteractor(trace.parentInteractor) {}

void TraceInformationGraphComponent::restoreOriginalVisualProperties() {
  // Restore original properties
  clearOverloadedProperties();
  if (_glMainWidget) {
    GlGraphInputData *inputdata = _glMainWidget->getScene()->getGlGraphComposite()->getInputData();
    if (_originalRenderer != nullptr) {
      inputdata->setMetaNodeRenderer(_originalRenderer, false);
      // If the view have it's graph changed we need to reinitialize the
      // metanode renderer with the right input data.
      _originalRenderer->setInputData(inputdata);
    }
    inputdata->getGraph()->removeObserver(this);
    // Ensure remove will be called only one time for each set view call.
    _glMainWidget = nullptr;
    _originalRenderer = nullptr;
    delete _interactorComponentRenderer;
    _interactorComponentRenderer = nullptr;
  }
}

void TraceInformationGraphComponent::setGlMainWidget(tlp::GlMainWidget *widget) {
  // ensure previously called set will be deleted.
  restoreOriginalVisualProperties();
  _glMainWidget = widget;
  GlGraphInputData *inputdata = _glMainWidget->getScene()->getGlGraphComposite()->getInputData();
  _originalRenderer = inputdata->getMetaNodeRenderer();
  _interactorComponentRenderer =
      new TraceInformationMetanodeRenderer(inputdata, VisualPropertiesFlags(AllColorProperties));
  inputdata->setMetaNodeRenderer(_interactorComponentRenderer, false);
  // Init observer
  inputdata->getGraph()->addObserver(this);
}

bool TraceInformationGraphComponent::eventFilter(QObject *, QEvent *event) {
  bool acceptEvent = false;
  edge e;
  if (event->type() == QEvent::ToolTip) {
    // Avoid displaying tool tips.
    event->ignore();
    acceptEvent = true;
  } else if (event->type() == QEvent::MouseMove) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    SelectedEntity selectedEntity;
    GlGraphInputData *inputdata = _glMainWidget->getScene()->getGlGraphComposite()->getInputData();
    Graph *g = inputdata->getGraph();
    Trace trace(g);
    if (_glMainWidget->pickNodesEdges(mouseEvent->pos().x(), mouseEvent->pos().y(), selectedEntity)) {
      if (selectedEntity.getEntityType() == SelectedEntity::EDGE_SELECTED) {
        QString text;
        e = selectedEntity.getEdge();
        if (trace.getTransformationType(e) == Trace::TransformationType::Simple_Transformation) {
          Graph *rule = trace.getRuleForTransformation(e);
          //                    if(trace.getTransformationType(e) ==
          //                    Trace::Simple_Transformation)
          //                        text.append(QString("<p
          //                        align=\"center\">Rule (All)</p>"));
          //                    else {
          text.append(QString("<p align=\"center\">Rule (One)</p>"));
          //                        text.append("<b>Probability:</b>");
          //                        text.append(QString::number(trace.getTransformationProbaProperty()->getEdgeValue(e)));
          //                        text.append("<br/>");
          //                    }

          text.append("<b>Name:</b> ");
          if (rule) {
            text.append(tlpStringToQString(rule->getName()));
          }
          text.append("<br/>");
          const pair<node, node> &ends = trace.ends(e);
          if (trace.isMetaNode(ends.first) &&
              trace.isMetaNode(ends.second)) { // si l'arête ne pointe pas vers
                                               // un noeud failure
            if (static_cast<TraceInformationConfigWidget *>(parentInteractor->configurationOptionsWidget())
                    ->ShowInstance()) { // est-ce que Instance est coché dans la
                                        // configuration
              Graph *instance = trace.getInstanceForTransformation(e);
              text.append("<b>Instance:</b> ");
              if (instance) {
                text.append(tlpStringToQString(instance->getName()));
              }
              text.append("<br/>");
            }
          }
          if (static_cast<TraceInformationConfigWidget *>(parentInteractor->configurationOptionsWidget())
                  ->ShowPProperty()) {
            string pPropertyName = trace.getTransformationPPropertyNameProperty()->getEdgeValue(e);
            text.append(QString("<b>Focus Property:</b> "));
            if (!pPropertyName.empty()) {
              text.append(tlpStringToQString(pPropertyName));
            }
            text.append("<br/>");
          }
          if (static_cast<TraceInformationConfigWidget *>(parentInteractor->configurationOptionsWidget())
                  ->ShowBanProperty()) {
            string BanPropertyName =
                trace.getTransformationBanPropertyNameProperty()->getEdgeValue(e);
            text.append(QString("<b>Ban Property:</b> "));
            if (!BanPropertyName.empty()) {
              text.append(tlpStringToQString(BanPropertyName));
            }
          }
        } else if (trace.getTransformationType(e) == Trace::Focus_Transformation) {
          text.append(QString("<p align=\"center\">Set focus property</p><br/>"));
          text.append("<b>Operation:</b> ");
          //                    switch(trace.getTransformationRuleProperty()->getEdgeValue(e))
          //                    {
          //                    case PorgyConstants::CRTGRAPH:
          //                        text.append("CrtGraph");
          //                        break;
          //                    case PorgyConstants::PROPERTY:
          //                        text.append("Property");
          //                        break;
          //                    default:
          //                        cerr << __PRETTY_FUNCTION__ << ":" <<
          //                        __LINE__ << ": TODO !!!" << endl;
          //                        break;
          //                    }
          text.append("<br/>");
          string pPropertyName = trace.getTransformationPPropertyNameProperty()->getEdgeValue(e);
          text.append(QString("<b>Focus Property:</b> "));
          if (!pPropertyName.empty())
            text.append(tlpStringToQString(pPropertyName));
        } else if (trace.getTransformationType(e) == Trace::Ban_Transformation) {
          text.append(QString("<p align=\"center\">Set Ban property</p><br/>"));
          text.append("<b>Operation:</b> ");
          //                    switch(trace.getTransformationRuleProperty()->getEdgeValue(e))
          //                    {
          //                    case PorgyConstants::CRTGRAPH:
          //                        text.append("CrtGraph");
          //                        break;
          //                    case PorgyConstants::PROPERTY:
          //                        text.append("Property");
          //                        break;
          //                    case PorgyConstants::CRTPOS:
          //                    case PorgyConstants::CRTBAN:
          //                    case PorgyConstants::EMPTYSET:
          //                        cerr << __PRETTY_FUNCTION__ << ":" <<
          //                        __LINE__ << ": TODO !!!" << endl;
          //                        break;
          //                    }
          text.append("<br/>");
          string BanPropertyName =
              trace.getTransformationBanPropertyNameProperty()->getEdgeValue(e);
          text.append(QString("<b>Ban Property:</b> "));
          if (!BanPropertyName.empty())
            text.append(tlpStringToQString(BanPropertyName));
        } else {
          text.append(QString("<p align=\"center\">Strategy</p><br/>"));
          if (static_cast<TraceInformationConfigWidget *>(parentInteractor->configurationOptionsWidget())
                  ->ShowPProperty()) {
            string pPropertyName = trace.getTransformationPPropertyNameProperty()->getEdgeValue(e);
            if (!pPropertyName.empty()) {
              text.append(QString("<b>Focus Property:</b> "));
              text.append(tlpStringToQString(pPropertyName));
              text.append("<br/>");
            }
          }
          if (static_cast<TraceInformationConfigWidget *>(parentInteractor->configurationOptionsWidget())
                  ->ShowBanProperty()) {
            string BanPropertyName =
                trace.getTransformationBanPropertyNameProperty()->getEdgeValue(e);
            if (!BanPropertyName.empty()) {
              text.append(QString("<b>Ban Property:</b> "));
              text.append(tlpStringToQString(BanPropertyName));
              text.append("<br/>");
            }
          }
          text.append("<b>Strategy code:</b> ");
          text.append(
              tlpStringToQString(trace.getTransformationStrategyProperty()->getEdgeValue(e)));
        }

        QToolTip::showText(mouseEvent->globalPos(), text, _glMainWidget);
        acceptEvent = true;
      } else if (selectedEntity.getEntityType() == SelectedEntity::NODE_SELECTED) {
        node n = selectedEntity.getNode();
        QString text;
        text.append(QString("<p align=\"center\">"));
        if (g->isMetaNode(n)) {
          text.append(tlpStringToQString(g->getNodeMetaInfo(n)->getName()));
        } else { // failure node
          text.append("Failure");
          if (trace.getRuleForTransformation(n) != nullptr)
            text.append(": " + tlpStringToQString(trace.getRuleForTransformation(n)->getName()));
        }
        text.append(QString("</p><br/>"));

        list<string> strategy(strategyCode(n));
        if (!strategy.empty()) {
          text.append(QString("<b>List of operations run from the root of the tree</b>: "));
          unsigned cpt(0);
          for (auto i = strategy.cbegin(); i != strategy.cend();) {
            text.append(QString("one(") + tlp::tlpStringToQString(*i) + ")");
            if (++i != strategy.end())
              text.append(QString(";"));
            if (++cpt % 20 == 0) {
              text.append("<br>");
            }
          }
        }
        QToolTip::showText(mouseEvent->globalPos(), text, _glMainWidget);
        acceptEvent = true;
      }
    } else {
      // Hide text
      QToolTip::showText(mouseEvent->pos(), "", _glMainWidget);
      event->ignore();
    }
    updateColors(e, _currentTransformation);
    _currentTransformation = e;
  }

  return acceptEvent;
}

namespace {
void hideUnselectedElements(Graph *graph, BooleanProperty *elements,
                            vector<ColorProperty *> colorProp) {
  for (ColorProperty *c : colorProp) {
    for(auto n:elements->getNodesEqualTo(false, graph)) {
      Color col = c->getNodeValue(n);
      col.setA(PorgyConstants::hidden_alpha);
      c->setNodeValue(n, col);
    }
    for(auto e:elements->getEdgesEqualTo(false, graph)) {
      Color col = c->getEdgeValue(e);
      col.setA(PorgyConstants::hidden_alpha);
      c->setEdgeValue(e, col);
    }
  }
}

vector<tlp::ColorProperty *> create_list(tlp::Graph *g, GraphVisualProperties &prop) {
  vector<ColorProperty *> colorvect;
  ColorProperty *color = new ColorProperty(g);
  ColorProperty *borderColor = new ColorProperty(g);
  ColorProperty *labelColor = new ColorProperty(g);
  ColorProperty *labelBorderColor = new ColorProperty(g);
  prop.setElementColor(color);
  prop.setElementBorderColor(borderColor);
  prop.setElementLabelColor(labelColor);
  prop.setElementLabelBorderColor(labelBorderColor);
  color->copy(g->getProperty<ColorProperty>("viewColor"));
  borderColor->copy(g->getProperty<ColorProperty>("viewBorderColor"));
  labelColor->copy(g->getProperty<ColorProperty>("viewLabelColor"));
  labelBorderColor->copy(g->getProperty<ColorProperty>("viewLabelBorderColor"));
  colorvect.push_back(color);
  colorvect.push_back(borderColor);
  colorvect.push_back(labelColor);
  colorvect.push_back(labelBorderColor);
  return colorvect;
}
}

void TraceInformationGraphComponent::updateColors(edge newEdge, edge oldEdge) {
  Trace trace(view()->graph());
  if (newEdge.isValid()) {
    // If the new edge is different of the old edge update the visual
    // parameters.
    const pair<node, node> &ends = trace.ends(newEdge);
    if (newEdge != oldEdge && trace.isMetaNode(ends.first) && trace.isMetaNode(ends.second)) {
      Graph *origModel = trace.getNodeMetaInfo(ends.first);
      Graph *destModel = trace.getNodeMetaInfo(ends.second);
      // Clear old data.
      _interactorComponentRenderer->clearStoredInputData();
      clearOverloadedProperties();
      // Create new visual properties for metanodes.
      vector<ColorProperty *> origProp(create_list(origModel, origModelVisualProperties));
      vector<ColorProperty *> destProp(create_list(destModel, destModelVisualProperties));
      // Change to new edge.
      if (trace.getTypeProperty()->getEdgeValue(newEdge) == Trace::Focus_Transformation) {
        // Focus transformation show old P and new P
        BooleanProperty sourceP(origModel);
        BooleanProperty destP(destModel);
        if (!trace.getSourcePPropertyForTransformation(newEdge, &sourceP)) {
          cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << " cannot find PProperty for edge "
               << newEdge.id << std::endl;
        }
        if (!trace.getTargetPPropertyForTransformation(newEdge, &destP)) {
          cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << " cannot find PProperty for edge "
               << newEdge.id << std::endl;
        }
        // Create new visual properties
        hideUnselectedElements(origModel, &sourceP, origProp);
        hideUnselectedElements(destModel, &destP, destProp);

      } else if (trace.getTypeProperty()->getEdgeValue(newEdge) == Trace::Ban_Transformation) {
        // Focus transformation show old Ban and new Ban Property
        BooleanProperty sourceBan(origModel);
        BooleanProperty destBan(destModel);
        if (!trace.getSourceBanPropertyForTransformation(newEdge, &sourceBan)) {
          cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << " cannot find BanProperty for edge "
               << newEdge.id << std::endl;
        }
        if (!trace.getTargetBanPropertyForTransformation(newEdge, &destBan)) {
          cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << " cannot find BanProperty for edge "
               << newEdge.id << std::endl;
        }

        // Create new visual properties
        hideUnselectedElements(origModel, &sourceBan, origProp);
        hideUnselectedElements(destModel, &destBan, destProp);

      } else {
        // Simple transformation show rhs and lhs
        BooleanProperty lhs(origModel);
        trace.getLeftMembersForTransformation(newEdge, &lhs);
        hideUnselectedElements(origModel, &lhs, origProp);
        BooleanProperty rhs(destModel);
        trace.getRightMembersForTransformation(newEdge, &rhs);
        hideUnselectedElements(destModel, &rhs, destProp);
      }
      // Overload properties in metanode renderer
      _interactorComponentRenderer->setOverloadedPropertiesForGraph(origModel,
                                                                    origModelVisualProperties);
      _interactorComponentRenderer->setOverloadedPropertiesForGraph(destModel,
                                                                    destModelVisualProperties);
      _glMainWidget->draw();
    }
  } else if (oldEdge.isValid()) {
    // If we leave an edge clear the overloaded properties
    _interactorComponentRenderer->clearStoredInputData();
    clearOverloadedProperties();
    _glMainWidget->draw();
  }
}

std::list<string> TraceInformationGraphComponent::strategyCode(const node to) const {
  Graph *graph = view()->graph();
  if (!PorgyTlpGraphStructure::isTraceGraph(graph)) {
    return list<string>();
  }
  // Get root node.
  node rootNode = graph->getSource();

  if (!rootNode.isValid()) {
    return list<string>();
  }
  Trace trace(graph);
  return trace.computeStrategy(rootNode, to);
}

void TraceInformationGraphComponent::clearOverloadedProperties() {
  origModelVisualProperties.deleteProperties();
  destModelVisualProperties.deleteProperties();
}
