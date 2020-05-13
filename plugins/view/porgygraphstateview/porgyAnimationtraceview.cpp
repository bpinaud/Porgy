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
#include "porgyAnimationtraceview.h"
#include "../../StandardInteractorPriority.h"
#include "AnimationViewQuickAccessBar.h"
#include "GlSceneMorphing.h"
#include "SimpleCameraMorphing.h"
#include "VisualPropertiesMorphing.h"
#include "porgyAnimationconfigurationwidget.h"

#include <tulip/DrawingTools.h>
#include <tulip/GlGraphComposite.h>
#include <tulip/Interactor.h>
#include <tulip/TlpQtTools.h>

#include <QGraphicsProxyWidget>
#include <QParallelAnimationGroup>

using namespace tlp;
using namespace std;

PorgyAnimationTraceView::PorgyAnimationTraceView(const PluginContext *)
    : AbstractTraceView(), _bar(nullptr), _configurationWidget(nullptr), _currentState(0),
      _currentAnimation(nullptr), _goNextStep(false) {}

PorgyAnimationTraceView::~PorgyAnimationTraceView() {
  clearRedrawTriggers();
  if (_currentAnimation != nullptr) {
    _currentAnimation->disconnect(this);
    _currentAnimation->stop();
    _currentAnimation->deleteLater();
    _visualProperties.deleteProperties();
  }
  delete _configurationWidget;
}

void PorgyAnimationTraceView::setAnimationData(tlp::Graph *graph,
                                               GraphVisualProperties &properties) {
  clearRedrawTriggers();
  AbstractTraceView::graphChanged(graph);
  GlScene *scene(getGlMainWidget()->getScene());
  scene->getGlGraphComposite()->setRenderingParameters(renderingParameters());
  properties.copyToInputData(scene->getGlGraphComposite()->getInputData());
  addRedrawTriggers(scene);
}

void PorgyAnimationTraceView::addRedrawTriggers(tlp::GlScene *scene) {
  GlGraphComposite *composite = scene->getGlGraphComposite();
  addRedrawTrigger(composite->getGraph());
  const auto &properties = composite->getInputData()->properties();
  for (auto it: properties) {
    addRedrawTrigger(it);
  }
}

QuickAccessBar *PorgyAnimationTraceView::getQuickAccessBarImpl() {
  _bar = new PorgyAnimationViewQuickAccessBar(_quickAccessBarItem);
  _bar->setStateName(tlpStringToQString(_states.getState(0).name));
  connect(_bar, SIGNAL(intermediateStatesChanged()), this, SLOT(updateIntermediateStates()));
  connect(_bar, SIGNAL(play()), this, SLOT(start()));
  connect(_bar, SIGNAL(pause()), this, SLOT(pause()));
  connect(_bar, SIGNAL(stop()), this, SLOT(stop()));
  connect(_bar, SIGNAL(seekBackward()), this, SLOT(seekBackward()));
  connect(_bar, SIGNAL(seekForward()), this, SLOT(seekForward()));
  connect(_bar, SIGNAL(skipBackward()), this, SLOT(seekFirst()));
  connect(_bar, SIGNAL(skipForward()), this, SLOT(seekLast()));

  return _bar;
}

void PorgyAnimationTraceView::setupWidget() {
  AbstractTraceView::setupWidget();
  // Build configuration widget.
  _configurationWidget = new PorgyAnimationConfigurationWidget();
  connect(_configurationWidget, SIGNAL(intermediateStatesChanged()), this,
          SLOT(updateIntermediateStates()));
  connect(_configurationWidget, SIGNAL(rulePreviewGeometryChanged()),
          SLOT(rulePreviewGeometryChanged()));
  connect(_configurationWidget, SIGNAL(scalelabels(bool)), this, SLOT(scalelabels(bool)));
  connect(_configurationWidget, SIGNAL(nodeLabelsVisibilityChanged(bool)), this,
          SLOT(updateNodeLabesVisibility(bool)));
}

tlp::DataSet PorgyAnimationTraceView::state() const {
  DataSet ds(AbstractTraceView::state());

  // general configuration
  PorgyIntermediaryStateFlags flags(_bar->intermediateStates());
  ds.set<bool>("ShowRHS", flags.testFlag(ViewRHS));
  ds.set<bool>("ShowLHS", flags.testFlag(ViewLHS));
  ds.set<bool>("ShowP", flags.testFlag(ViewLHSP));

  // animation
  ds.set<unsigned>("anim_duration", _configurationWidget->animationDuration());

  return ds;
}

void PorgyAnimationTraceView::setState(const tlp::DataSet &ds) {
  AbstractTraceView::setState(ds);
  PorgyIntermediaryStateFlags flags;
  if (ds.exists("ShowRHS")) {
    bool boolflags = false;
    ds.get("ShowRHS", boolflags);
    if (boolflags) {
      flags = flags | ViewRHS;
    }
    ds.get("ShowLHS", boolflags);
    if (boolflags) {
      flags = flags | ViewLHSP;
    }
    ds.get("ShowP", boolflags);
    if (boolflags) {
      flags = flags | ViewLHS;
    }
    _bar->setintermediateStates(flags);
    updateIntermediateStates();
  }

  // animation
  if (ds.exists("anim_duration")) {
    unsigned duration = 1000;
    ds.get<unsigned>("anim_duration", duration);
    _configurationWidget->setAnimationDuration(duration);
  }
}

QList<QWidget *> PorgyAnimationTraceView::configurationWidgets() const {
  return QList<QWidget *>() << _configurationWidget;
}

void PorgyAnimationTraceView::graphChanged(tlp::Graph *graph) {
  clearRedrawTriggers();
  _states.setGraphData(graph);
  if (_bar != nullptr)
    _states.updateStateList(_bar->intermediateStates());
  else
    _states.updateStateList();
  _visualProperties.deleteProperties(); // Clear old properties.
  GlScene *scene = getGlMainWidget()->getScene();
  if (_states.stateNumber() != 0) {
    // Create visual properties for the graph
    _visualProperties.setElementColor(new ColorProperty(graph->getRoot()));
    _visualProperties.setElementBorderColor(new ColorProperty(graph->getRoot()));
    _visualProperties.setElementLabelColor(new ColorProperty(graph->getRoot()));
    _visualProperties.setElementLabelBorderColor(new ColorProperty(graph->getRoot()));
    _visualProperties.setElementLayout(new LayoutProperty(graph->getRoot()));
    _visualProperties.setElementBorderWidth(new DoubleProperty(graph->getRoot()));

    // Update animation
    _currentState = 0;
    GraphState &state = _states.getState(0);
    // Overload visual properties.
    for (node n : state.graph->nodes()) {
      if (state.properties.getElementColor() != nullptr) {
        _visualProperties.getElementColor()->setNodeValue(
            n, state.properties.getElementColor()->getNodeValue(n));
      }
      if (state.properties.getElementBorderColor() != nullptr) {
        _visualProperties.getElementBorderColor()->setNodeValue(
            n, state.properties.getElementBorderColor()->getNodeValue(n));
      }
      if (state.properties.getElementLabelColor() != nullptr) {
        _visualProperties.getElementLabelColor()->setNodeValue(
            n, state.properties.getElementLabelColor()->getNodeValue(n));
      }
      if (state.properties.getElementLabelBorderColor() != nullptr) {
        _visualProperties.getElementLabelBorderColor()->setNodeValue(
            n, state.properties.getElementLabelBorderColor()->getNodeValue(n));
      }
      if (state.properties.getElementLayout() != nullptr) {
        _visualProperties.getElementLayout()->setNodeValue(
            n, state.properties.getElementLayout()->getNodeValue(n));
      }
      if (state.properties.getElementBorderWidth() != nullptr) {
        _visualProperties.getElementBorderWidth()->setNodeValue(
            n, state.properties.getElementBorderWidth()->getNodeValue(n));
      }
    }

    for (edge e : state.graph->edges()) {
      if (state.properties.getElementColor() != nullptr) {
        _visualProperties.getElementColor()->setEdgeValue(
            e, state.properties.getElementColor()->getEdgeValue(e));
      }
      if (state.properties.getElementBorderColor() != nullptr) {
        _visualProperties.getElementBorderColor()->setEdgeValue(
            e, state.properties.getElementBorderColor()->getEdgeValue(e));
      }
      if (state.properties.getElementLabelColor() != nullptr) {
        _visualProperties.getElementLabelColor()->setEdgeValue(
            e, state.properties.getElementLabelColor()->getEdgeValue(e));
      }
      if (state.properties.getElementLabelBorderColor() != nullptr) {
        _visualProperties.getElementLabelBorderColor()->setEdgeValue(
            e, state.properties.getElementLabelBorderColor()->getEdgeValue(e));
      }
      if (state.properties.getElementLayout() != nullptr) {
        _visualProperties.getElementLayout()->setEdgeValue(
            e, state.properties.getElementLayout()->getEdgeValue(e));
      }
      if (state.properties.getElementBorderWidth() != nullptr) {
        _visualProperties.getElementBorderWidth()->setEdgeValue(
            e, state.properties.getElementBorderWidth()->getEdgeValue(e));
      }
    }
    AbstractTraceView::graphChanged(_states.getState(0).graph);
    GlGraphComposite *composite = scene->getGlGraphComposite();
    _visualProperties.copyToInputData(composite->getInputData());
    composite->setRenderingParameters(renderingParameters());
  }
  addRedrawTriggers(scene); // Listen animation
}

tlp::GlGraphRenderingParameters PorgyAnimationTraceView::renderingParameters() const {
  GlGraphRenderingParameters parameters;
  parameters.setEdgeColorInterpolate(false);
  parameters.setLabelsDensity(0);
  parameters.setEdgeColorInterpolate(false);
  return parameters;
}

void PorgyAnimationTraceView::animate(unsigned int to) {
  // Avoid video update during animation
  _bar->setIntermediateStatesEnabled(false);

  // There is already an animation stop it.
  if (_currentAnimation) {
    // Avoid to receive the animation state changed signal
    _currentAnimation->disconnect(this);
    _currentAnimation->stop();
    _currentAnimation->deleteLater();
  }

  _currentState = to;
  Observable::holdObservers();
  _currentAnimation = buildMorphingBetweenStates(to);
  Observable::unholdObservers();
  connect(_currentAnimation,
          SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)),
          SLOT(currentAnimationStateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));
  _currentAnimation->start();
}

QAbstractAnimation *PorgyAnimationTraceView::buildMorphingBetweenStates(unsigned int to) {
  // Init state from current view
  GlScene *scene = getGlMainWidget()->getScene();
  GlGraphComposite *composite = scene->getGlGraphComposite();
  GraphState fromState(
      composite->getGraph(),
      GraphVisualProperties(composite->getInputData(),
                            VisualPropertiesFlags(AllColorProperties | ElementSize | ElementLayout |
                                                  ElementBorderWidth)));
  GraphState &toState = _states.getState(to);
  // Compute frame number
  unsigned int framesCount =
      (_configurationWidget->animationDuration() / 1000.0) * _configurationWidget->framePerSecond();
  // Avoid invalid frame count.
  if (framesCount == 0) {
    framesCount = 1;
  }
  QPropertyAnimation *animation = nullptr;
  // Use a simple graph morphing.
  if (fromState.graph == toState.graph) {
    assert(fromState.properties.getElementColor() != nullptr);
    assert(toState.properties.getElementColor() != nullptr);
    animation = new VisualPropertiesMorphing(
        fromState.graph, fromState.properties, toState.properties, _visualProperties, framesCount,
        VisualPropertiesFlags(AllColorProperties | ElementLayout | ElementBorderWidth), this);
  } else {
    animation = new GlSceneMorphing(
        this, fromState.graph, fromState.properties, _visualProperties, toState.graph,
        toState.properties, _visualProperties, framesCount, scene,
        VisualPropertiesFlags(AllColorProperties | ElementLayout | ElementBorderWidth), this);
  }
  animation->setDuration(_configurationWidget->animationDuration());

  // If the graph of the two different state is different update the rule previe
  // widget.
  Graph *rule = toState.rule;
  bool rulePreviewIsVisible = _bar->rulePreviewVisible() && rule != nullptr;
  if (rulePreviewIsVisible) {
    _bar->setRule(rule);
  }

  QParallelAnimationGroup *graphAndCameraAnimation = new QParallelAnimationGroup(this);
  _bar->setStateName(tlpStringToQString(toState.name));
  graphAndCameraAnimation->addAnimation(animation);

  // Compute destination bounding box
  BoundingBox bboxTo =
      computeBoundingBox(toState.graph,
                         toState.properties.getElementLayout() != nullptr
                             ? toState.properties.getElementLayout()
                             : toState.graph->getProperty<LayoutProperty>("viewLayout"),
                         toState.properties.getElementSize() != nullptr
                             ? toState.properties.getElementSize()
                             : toState.graph->getProperty<SizeProperty>("viewSize"),
                         toState.graph->getProperty<DoubleProperty>("viewRotation"), nullptr);
  // Compute destination bounding box
  BoundingBox from =
      computeBoundingBox(fromState.graph, fromState.properties.getElementLayout(),
                         fromState.properties.getElementSize(),
                         fromState.graph->getProperty<DoubleProperty>("viewRotation"), nullptr);
  if (!from.contains(bboxTo[0]) || !from.contains(bboxTo[1]) ||
      bboxTo.width() * 1.1 < from.width() || bboxTo.height() * 1.1 < from.height()) {
    // Add margin
    float widthMargin = bboxTo.width() * 0.1;
    float heightMargin = bboxTo.height() * 0.1;
    Vec3f newMin = bboxTo[0];
    newMin[0] -= widthMargin / 2;
    newMin[1] -= heightMargin / 2;
    Vec3f newMax = bboxTo[1];
    newMax[0] += widthMargin / 2;
    newMax[1] += heightMargin / 2;
    bboxTo.expand(newMin);
    bboxTo.expand(newMax);
    QPropertyAnimation *cameraAnimation = new SimpleCameraMorphing(
        getGlMainWidget()->getScene()->getGraphCamera(), bboxTo, framesCount, this);
    cameraAnimation->setDuration(_configurationWidget->animationDuration());
    graphAndCameraAnimation->addAnimation(cameraAnimation);
  }

  return graphAndCameraAnimation;
}

void PorgyAnimationTraceView::start() {
  if (_currentAnimation && _currentAnimation->state() == QAbstractAnimation::Paused) {
    _currentAnimation->resume();
  } else {
    _goNextStep = true;
    seekForward();
  }
}

void PorgyAnimationTraceView::pause() {
  if (_currentAnimation && _currentAnimation->state() == QAbstractAnimation::Running) {
    _currentAnimation->pause();
  }
}

void PorgyAnimationTraceView::stop() {
  _goNextStep = false;
  if (_currentAnimation) {
    _currentAnimation->stop();
  }

  // Animation is stopped allow user to edit configuration widget and
  // quickaccessbar.
  _bar->endAnim(_currentState == _states.stateNumber() - 1);
}

void PorgyAnimationTraceView::seekBackward() {
  if (_currentState != 0) {
    animate(_currentState - 1);
  }
}

void PorgyAnimationTraceView::seekForward() {
  if (_currentState != _states.stateNumber() - 1) {
    animate(_currentState + 1);
  } else {
    stop(); // Stop playing
  }
}

void PorgyAnimationTraceView::seekFirst() {
  if (_currentState != 0) {
    animate(0);
  }
}

void PorgyAnimationTraceView::seekLast() {
  if (_currentState != _states.stateNumber() - 1) {
    animate(_states.stateNumber() - 1);
  }
}

void PorgyAnimationTraceView::currentAnimationStateChanged(QAbstractAnimation::State newState,
                                                           QAbstractAnimation::State) {
  // Animation will be deleted
  if (newState == QAbstractAnimation::Stopped) {
    _currentAnimation->setCurrentTime(_currentAnimation->duration() -
                                      1); // Force to display the last step
    _currentAnimation->deleteLater();     // Delete ended animation
    _currentAnimation = nullptr;
    if (_goNextStep) { // If we are playing all the video go next.
      seekForward();
    } else {
      // Stop the animation.
      stop();
    }
  }
}

void PorgyAnimationTraceView::updateIntermediateStates() {
  _states.updateStateList(_bar->intermediateStates());
  // Go to the first step.
  animate(0);
}

PLUGIN(PorgyAnimationTraceView)

INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(AnimViewNavigation, "AnimViewNavigation",
                                          "InteractorNavigation",
                                          PorgyConstants::ANIMATION_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Navigate", "1.0",
                                          StandardInteractorPriority::Navigation)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(AnimViewMagnifyingGlass, "AnimViewMagnifyingGlass",
                                          "MouseMagnifyingGlassInteractor",
                                          PorgyConstants::ANIMATION_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Magnyfiying glass", "1.0",
                                          StandardInteractorPriority::MagnifyingGlass)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(AnimViewFishEye, "AnimViewFishEye", "FishEyeInteractor",
                                          PorgyConstants::ANIMATION_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Fisheye", "1.0",
                                          StandardInteractorPriority::FishEye)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(AnimViewRectangleZoom, "AnimViewRectangleZoom",
                                          "InteractorRectangleZoom",
                                          PorgyConstants::ANIMATION_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Rectangle zoom", "1.0",
                                          StandardInteractorPriority::ZoomOnRectangle)
