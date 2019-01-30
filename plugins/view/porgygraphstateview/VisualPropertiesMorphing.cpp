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
#include "VisualPropertiesMorphing.h"

#include <tulip/ColorPropertyAnimation.h>
#include <tulip/DoublePropertyAnimation.h>
#include <tulip/LayoutProperty.h>
#include <tulip/LayoutPropertyAnimation.h>
#include <tulip/SizePropertyAnimation.h>

using namespace tlp;
using namespace std;

VisualPropertiesMorphing::VisualPropertiesMorphing(
    Graph *from, const GraphVisualProperties &fromVisualProperties,
    const GraphVisualProperties &toVisualProperties, GraphVisualProperties &outVisualProperties,
    int frameNumber, VisualPropertiesFlags propertiesToAnimate, QObject *parent)
    : Animation(frameNumber, parent) {
  if (propertiesToAnimate.testFlag(ElementColor)) {
    if (fromVisualProperties.getElementColor() != toVisualProperties.getElementColor()) {
      propertiesAnimations.push_back(new ColorPropertyAnimation(
          from, fromVisualProperties.getElementColor(), toVisualProperties.getElementColor(),
          outVisualProperties.getElementColor(), nullptr, frameCount(), true, true, this));
    }
  }
  if (propertiesToAnimate.testFlag(ElementBorderColor)) {
    if (fromVisualProperties.getElementBorderColor() !=
        toVisualProperties.getElementBorderColor()) {
      propertiesAnimations.push_back(new ColorPropertyAnimation(
          from, fromVisualProperties.getElementBorderColor(),
          toVisualProperties.getElementBorderColor(), outVisualProperties.getElementBorderColor(),
          nullptr, frameCount(), true, true, this));
    }
  }
  if (propertiesToAnimate.testFlag(ElementLabelColor)) {
    if (fromVisualProperties.getElementLabelColor() != toVisualProperties.getElementLabelColor()) {
      propertiesAnimations.push_back(new ColorPropertyAnimation(
          from, fromVisualProperties.getElementLabelColor(),
          toVisualProperties.getElementLabelColor(), outVisualProperties.getElementLabelColor(),
          nullptr, frameCount(), true, true, this));
    }
  }
  if (propertiesToAnimate.testFlag(ElementSize)) {
    if (fromVisualProperties.getElementSize() != toVisualProperties.getElementSize()) {
      propertiesAnimations.push_back(new SizePropertyAnimation(
          from, fromVisualProperties.getElementSize(), toVisualProperties.getElementSize(),
          outVisualProperties.getElementSize(), nullptr, frameCount(), true, true, this));
    }
  }
  if (propertiesToAnimate.testFlag(ElementLayout)) {
    if (fromVisualProperties.getElementLayout() != toVisualProperties.getElementLayout()) {
      propertiesAnimations.push_back(new LayoutPropertyAnimation(
          from, fromVisualProperties.getElementLayout(), toVisualProperties.getElementLayout(),
          outVisualProperties.getElementLayout(), nullptr, frameCount(), true, true, this));
    }
  }
  if (propertiesToAnimate.testFlag(ElementBorderWidth)) {
    if (fromVisualProperties.getElementBorderWidth() !=
        toVisualProperties.getElementBorderWidth()) {
      propertiesAnimations.push_back(new DoublePropertyAnimation(
          from, fromVisualProperties.getElementBorderWidth(),
          toVisualProperties.getElementBorderWidth(), outVisualProperties.getElementBorderWidth(),
          nullptr, frameCount(), true, true, this));
    }
  }
}
VisualPropertiesMorphing::~VisualPropertiesMorphing() {
  for (vector<Animation *>::iterator it = propertiesAnimations.begin();
       it != propertiesAnimations.end(); ++it) {
    delete (*it);
  }
  propertiesAnimations.clear();
}

void VisualPropertiesMorphing::setFrameCount(int frameCount) {
  Animation::setFrameCount(frameCount);
  for (vector<Animation *>::iterator it = propertiesAnimations.begin();
       it != propertiesAnimations.end(); ++it) {
    (*it)->setFrameCount(frameCount);
  }
}

void VisualPropertiesMorphing::frameChanged(int frame) {
  Observable::holdObservers();
  for (vector<Animation *>::iterator it = propertiesAnimations.begin();
       it != propertiesAnimations.end(); ++it) {
    (*it)->frameChanged(frame);
  }
  Observable::unholdObservers();
}
