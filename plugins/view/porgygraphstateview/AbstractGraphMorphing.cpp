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
#include "AbstractGraphMorphing.h"
#include "VisualPropertiesMorphing.h"

#include <tulip/DoubleProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>

using namespace tlp;
using namespace std;

template <>
Color AbstractGraphMorphing::computeMiddleValue<Color>(const Color &from, const Color &to) const {
  return Color(from[0] + (to[0] - from[0]) / 2, from[1] + (to[1] - from[1]) / 2,
               from[2] + (to[2] - from[2]) / 2, from[3] + (to[3] - from[3]) / 2);
}

template <>
vector<Coord>
AbstractGraphMorphing::computeMiddleValue<vector<Coord>>(const vector<Coord> &from,
                                                         const vector<Coord> &) const {
  return from;
}

AbstractGraphMorphing::AbstractGraphMorphing(
    Graph *from, const GraphVisualProperties &fromVisualProperties,
    GraphVisualProperties &fromOutVisualProperties, Graph *to,
    const GraphVisualProperties &toVisualProperties, GraphVisualProperties &toOutVisualProperties,
    int frameNumber, VisualPropertiesFlags propertiesToAnimate, QObject *parent)
    : Animation(frameNumber, parent), fromGraph(from), toGraph(to),
      animationProperties(propertiesToAnimate), switchFrame(computeSwitchFrame()),
      firstAnimation(nullptr), firstAnimationOutVisualProperties(fromOutVisualProperties),
      secondAnimation(nullptr), secondAnimationOutVisualProperties(toOutVisualProperties) {
  initInternalAnimations(fromVisualProperties, toVisualProperties);
}
AbstractGraphMorphing::~AbstractGraphMorphing() {
  delete firstAnimation;
  delete secondAnimation;
}

void AbstractGraphMorphing::setFrameCount(int frameCount) {
  Animation::setFrameCount(frameCount);
}

int AbstractGraphMorphing::computeSwitchFrame() const {
  return (frameCount() / 2) - 1;
}

int AbstractGraphMorphing::computeFirstAnimationFrameNumber() const {
  return switchFrame + 1;
}
int AbstractGraphMorphing::computeSecondAnimationFrameNumber() const {
  return frameCount() - switchFrame;
}

void AbstractGraphMorphing::frameChanged(int frame) {
  Graph *toDisplay = nullptr;
  int relativeFrame = 0;
  VisualPropertiesMorphing *currentAnimation = nullptr;
  GraphVisualProperties &overloadedProperties = firstAnimationOutVisualProperties;
  if (frame <= switchFrame) {
    toDisplay = fromGraph;
    relativeFrame = frame;
    currentAnimation = firstAnimation;
    overloadedProperties = firstAnimationOutVisualProperties;
  } else {
    toDisplay = toGraph;
    relativeFrame = frame - switchFrame;
    currentAnimation = secondAnimation;
    overloadedProperties = secondAnimationOutVisualProperties;
  }
  // If the displayed graph is not the right one change the displayed graph.
  if (displayedGraph() != toDisplay) {
    switchDisplayedGraph(toDisplay, overloadedProperties);
  }
  currentAnimation->frameChanged(relativeFrame);

  // widget->draw();
}

void AbstractGraphMorphing::initInternalAnimations(const GraphVisualProperties &from,
                                                   const GraphVisualProperties &to) {
  delete firstAnimation;
  delete secondAnimation;

  GraphVisualProperties firstAnimationOverloadedVisualProperties;
  GraphVisualProperties secondAnimationOverloadedVisualProperties;

  // Generate new out properties.
  //firstAnimationOverloadedVisualProperties = GraphVisualProperties();
  //secondAnimationOverloadedVisualProperties = GraphVisualProperties();
  if (animationProperties.testFlag(ElementColor)) {
    // Create abstract properties to overload GlMainWidget default properties.
    ColorProperty *colorFromResult = new ColorProperty(fromGraph);
    *colorFromResult = *(from.getElementColor());
    ColorProperty *colorToResult = new ColorProperty(toGraph);
    *colorToResult = *(to.getElementColor());
    computeMiddleValueForProperty<ColorProperty, Color, Color>(fromGraph, from.getElementColor(),
                                                               toGraph, to.getElementColor(),
                                                               colorFromResult, colorToResult);
    // Oveloading properties with middle values.
    firstAnimationOverloadedVisualProperties.setElementColor(colorFromResult);
    secondAnimationOverloadedVisualProperties.setElementColor(colorToResult);
  }
  if (animationProperties.testFlag(ElementBorderColor)) {
    // Create abstract properties to overload GlMainWidget default properties.
    ColorProperty *colorFromResult = new ColorProperty(fromGraph);
    *colorFromResult = *(from.getElementBorderColor());
    ColorProperty *colorToResult = new ColorProperty(toGraph);
    *colorToResult = *(to.getElementBorderColor());
    computeMiddleValueForProperty<ColorProperty, Color, Color>(
        fromGraph, from.getElementBorderColor(), toGraph, to.getElementBorderColor(),
        colorFromResult, colorToResult);
    // Oveloading properties with middle values.
    firstAnimationOverloadedVisualProperties.setElementBorderColor(colorFromResult);
    secondAnimationOverloadedVisualProperties.setElementBorderColor(colorToResult);
  }
  if (animationProperties.testFlag(ElementLabelColor)) {
    // Create abstract properties to overload GlMainWidget default properties.
    ColorProperty *colorFromResult = new ColorProperty(fromGraph);
    *colorFromResult = *(from.getElementLabelColor());
    ColorProperty *colorToResult = new ColorProperty(toGraph);
    *colorToResult = *(to.getElementLabelColor());
    computeMiddleValueForProperty<ColorProperty, Color, Color>(
        fromGraph, from.getElementLabelColor(), toGraph, to.getElementLabelColor(), colorFromResult,
        colorToResult);
    // Oveloading properties with middle values.
    firstAnimationOverloadedVisualProperties.setElementLabelColor(colorFromResult);
    secondAnimationOverloadedVisualProperties.setElementLabelColor(colorToResult);
  }
  if (animationProperties.testFlag(ElementLabelBorderColor)) {
    // Create abstract properties to overload GlMainWidget default properties.
    ColorProperty *colorFromResult = new ColorProperty(fromGraph);
    *colorFromResult = *(from.getElementLabelBorderColor());
    ColorProperty *colorToResult = new ColorProperty(toGraph);
    *colorToResult = *(to.getElementLabelBorderColor());
    computeMiddleValueForProperty<ColorProperty, Color, Color>(
        fromGraph, from.getElementLabelBorderColor(), toGraph, to.getElementLabelBorderColor(),
        colorFromResult, colorToResult);
    // Oveloading properties with middle values.
    firstAnimationOverloadedVisualProperties.setElementLabelBorderColor(colorFromResult);
    secondAnimationOverloadedVisualProperties.setElementLabelBorderColor(colorToResult);
  }
  if (animationProperties.testFlag(ElementLayout)) {
    // Create abstract properties to overload GlMainWidget default properties.
    LayoutProperty *layoutFromResult = new LayoutProperty(fromGraph);
    *layoutFromResult = *(from.getElementLayout());
    LayoutProperty *layoutToResult = new LayoutProperty(toGraph);
    *layoutToResult = *(to.getElementLayout());
    computeMiddleValueForProperty<LayoutProperty, Coord, vector<Coord>>(
        fromGraph, from.getElementLayout(), toGraph, to.getElementLayout(), layoutFromResult,
        layoutToResult);
    // Oveloading properties with middle values.
    firstAnimationOverloadedVisualProperties.setElementLayout(layoutFromResult);
    secondAnimationOverloadedVisualProperties.setElementLayout(layoutToResult);
  }

  if (animationProperties.testFlag(ElementBorderWidth)) {
    // Create abstract properties to overload GlMainWidget default properties.
    DoubleProperty *borderWidthFromResult = new DoubleProperty(fromGraph);
    *borderWidthFromResult = *(from.getElementBorderWidth());
    DoubleProperty *borderWidthToResult = new DoubleProperty(toGraph);
    *borderWidthToResult = *(to.getElementBorderWidth());
    computeMiddleValueForProperty<DoubleProperty, double, double>(
        fromGraph, from.getElementBorderWidth(), toGraph, to.getElementBorderWidth(),
        borderWidthFromResult, borderWidthToResult);
    // Oveloading properties with middle values.
    firstAnimationOverloadedVisualProperties.setElementBorderWidth(borderWidthFromResult);
    secondAnimationOverloadedVisualProperties.setElementBorderWidth(borderWidthToResult);
  }
  if (animationProperties.testFlag(ElementSize)) {
    SizeProperty *elementSizeFromResult = new SizeProperty(fromGraph);
    *elementSizeFromResult = *(from.getElementSize());
    SizeProperty *elementSizeToResult = new SizeProperty(toGraph);
    *elementSizeToResult = *(to.getElementSize());
    computeMiddleValueForProperty<SizeProperty, Size, Size>(
        fromGraph, from.getElementSize(), toGraph, to.getElementSize(), elementSizeFromResult,
        elementSizeToResult);
    // Oveloading properties with middle values.
    firstAnimationOverloadedVisualProperties.setElementSize(elementSizeFromResult);
    secondAnimationOverloadedVisualProperties.setElementSize(elementSizeToResult);
  }

  // Make node/edge appear/Disappear
  computeDisparitionApparition(fromGraph, toGraph, firstAnimationOverloadedVisualProperties,
                               secondAnimationOverloadedVisualProperties);
  // generate animations with middle values.
  firstAnimation = new VisualPropertiesMorphing(
      fromGraph, from, firstAnimationOverloadedVisualProperties, firstAnimationOutVisualProperties,
      computeFirstAnimationFrameNumber(), animationProperties);
  secondAnimation = new VisualPropertiesMorphing(
      toGraph, secondAnimationOverloadedVisualProperties, to, secondAnimationOutVisualProperties,
      computeSecondAnimationFrameNumber(), animationProperties);

  firstAnimationOverloadedVisualProperties.deleteProperties();
  secondAnimationOverloadedVisualProperties.deleteProperties();
}

void AbstractGraphMorphing::computeDisparitionApparition(Graph *fromGraph, Graph *toGraph,
                                                         GraphVisualProperties &fromProperties,
                                                         GraphVisualProperties &toProperties) {
  // Makes elements existing only in first graph dispear
  hideElements(fromGraph, toGraph, fromProperties);
  // Makes elements existing only in second graph appear
  hideElements(toGraph, fromGraph, toProperties);
}
void AbstractGraphMorphing::hideElements(Graph *fromGraph, Graph *toGraph,
                                         GraphVisualProperties &visualProperties) {
  // Disapearing nodes
  for (node n : fromGraph->nodes()) {
    if (!toGraph->isElement(n)) {
      // Make node disapear.
      if (visualProperties.getElementColor()) {
        Color c = visualProperties.getElementColor()->getNodeValue(n);
        c.setA(0);
        visualProperties.getElementColor()->setNodeValue(n, c);
      }
      if (visualProperties.getElementLabelColor()) {
        Color c = visualProperties.getElementLabelColor()->getNodeValue(n);
        c.setA(0);
        visualProperties.getElementLabelColor()->setNodeValue(n, c);
      }
      if (visualProperties.getElementLabelBorderColor()) {
        Color c = visualProperties.getElementLabelBorderColor()->getNodeValue(n);
        c.setA(0);
        visualProperties.getElementLabelBorderColor()->setNodeValue(n, c);
      }
      if (visualProperties.getElementBorderColor()) {
        Color c = visualProperties.getElementBorderColor()->getNodeValue(n);
        c.setA(0);
        visualProperties.getElementBorderColor()->setNodeValue(n, c);
      }
      if (visualProperties.getElementSize()) {
        visualProperties.getElementSize()->setNodeValue(n, Size(0, 0, 0));
      }
    }
  }

  for (edge e : fromGraph->edges()) {
    if (!toGraph->isElement(e)) {
      // Make edge disapear.
      if (visualProperties.getElementColor()) {
        Color c = visualProperties.getElementColor()->getEdgeValue(e);
        c.setA(0);
        visualProperties.getElementColor()->setEdgeValue(e, c);
      }
      if (visualProperties.getElementLabelColor()) {
        Color c = visualProperties.getElementLabelColor()->getEdgeValue(e);
        c.setA(0);
        visualProperties.getElementLabelColor()->setEdgeValue(e, c);
      }
      if (visualProperties.getElementBorderColor()) {
        Color c = visualProperties.getElementBorderColor()->getEdgeValue(e);
        c.setA(0);
        visualProperties.getElementBorderColor()->setEdgeValue(e, c);
      }
      if (visualProperties.getElementLabelBorderColor()) {
        Color c = visualProperties.getElementLabelBorderColor()->getEdgeValue(e);
        c.setA(0);
        visualProperties.getElementLabelBorderColor()->setEdgeValue(e, c);
      }
    }
  }
}
