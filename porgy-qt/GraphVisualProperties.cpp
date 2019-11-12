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
#include <porgy/GraphVisualProperties.h>

#include <tulip/ColorProperty.h>
#include <tulip/GlGraphInputData.h>
#include <tulip/GlGraphRenderingParameters.h>

using namespace tlp;
using namespace std;

//GraphVisualProperties::GraphVisualProperties(const GraphVisualProperties &toCopy)
//    : _properties(toCopy._properties) {}
GraphVisualProperties::GraphVisualProperties(GlGraphInputData *inputData,
                                             VisualPropertiesFlags propertiesToGet) {
  initFromInputData(inputData, propertiesToGet);
}

GraphVisualProperties::GraphVisualProperties(Graph *graph, VisualPropertiesFlags propertiesToGet) {
  initFromGraph(graph, propertiesToGet);
}

void GraphVisualProperties::deleteProperties() {
  for(auto i:_properties)
      delete i.second;
  _properties.clear();
}

void GraphVisualProperties::initFromInputData(tlp::GlGraphInputData *inputData,
                                              VisualPropertiesFlags propertiesToGet) {
  if (propertiesToGet.testFlag(ElementColor)) {
    setElementColor(inputData->getElementColor());
  }
  if (propertiesToGet.testFlag(ElementBorderColor)) {
    setElementBorderColor(inputData->getElementBorderColor());
  }
  if (propertiesToGet.testFlag(ElementLabelColor)) {
    setElementLabelColor(inputData->getElementLabelColor());
  }
  if (propertiesToGet.testFlag(ElementSize)) {
    setElementSize(inputData->getElementSize());
  }
  if (propertiesToGet.testFlag(ElementLayout)) {
    setElementLayout(inputData->getElementLayout());
  }
  if (propertiesToGet.testFlag(ElementBorderWidth)) {
    setElementBorderWidth(inputData->getElementBorderWidth());
  }
  if (propertiesToGet.testFlag(ElementLabelBorderColor)) {
    setElementLabelBorderColor(inputData->getElementLabelBorderColor());
  }
}

void GraphVisualProperties::initFromGraph(Graph *graph, VisualPropertiesFlags propertiesToGet) {
  GlGraphRenderingParameters parameters;
  GlGraphInputData data(graph, &parameters);
  initFromInputData(&data, propertiesToGet);
}

void GraphVisualProperties::setElementLayout(tlp::LayoutProperty *property) {
  setProperty(ElementLayout, property);
}

// Size property
SizeProperty *GraphVisualProperties::getElementSize() const {
  return static_cast<SizeProperty *>(property(ElementSize));
}
void GraphVisualProperties::setElementSize(SizeProperty *property) {
  setProperty(ElementSize, property);
}

DoubleProperty *GraphVisualProperties::getElementBorderWidth() const {
  return static_cast<DoubleProperty *>(property(ElementBorderWidth));
}
void GraphVisualProperties::setElementBorderWidth(DoubleProperty *property) {
  setProperty(ElementBorderWidth, property);
}

void GraphVisualProperties::setElementColor(ColorProperty *property) {
  setProperty(ElementColor, property);
}

void GraphVisualProperties::setElementBorderColor(ColorProperty *property) {
  setProperty(ElementBorderColor, property);
}
ColorProperty *GraphVisualProperties::getElementLabelColor() const {
  return static_cast<ColorProperty *>(property(ElementLabelColor));
}
void GraphVisualProperties::setElementLabelColor(ColorProperty *property) {
  setProperty(ElementLabelColor, property);
}

ColorProperty *GraphVisualProperties::getElementLabelBorderColor() const {
  return static_cast<ColorProperty *>(property(ElementLabelBorderColor));
}
void GraphVisualProperties::setElementLabelBorderColor(ColorProperty *property) {
  setProperty(ElementLabelBorderColor, property);
}

ColorProperty *GraphVisualProperties::getElementColor() const {
  return static_cast<ColorProperty *>(property(ElementColor));
}

ColorProperty *GraphVisualProperties::getElementBorderColor() const {
  return static_cast<ColorProperty *>(property(ElementBorderColor));
}

LayoutProperty *GraphVisualProperties::getElementLayout() const {
  return static_cast<LayoutProperty *>(property(ElementLayout));
}

void GraphVisualProperties::copyToInputData(GlGraphInputData *inputData) const {

  for (auto k: _properties) {
    switch (k.first) {
    case ElementColor:
      inputData->setElementColor(static_cast<ColorProperty *>(k.second));
      break;

    case ElementBorderColor:
      inputData->setElementBorderColor(static_cast<ColorProperty *>(k.second));
      break;

    case ElementLabelColor:
      inputData->setElementLabelColor(static_cast<ColorProperty *>(k.second));
      break;

    case ElementLayout:
      inputData->setElementLayout(static_cast<LayoutProperty *>(k.second));
      break;

    case ElementSize:
      inputData->setElementSize(static_cast<SizeProperty *>(k.second));
      break;

    case ElementBorderWidth:
      inputData->setElementBorderWidth(static_cast<DoubleProperty *>(k.second));
      break;

    case ElementLabelBorderColor:
      inputData->setElementLabelBorderColor(static_cast<ColorProperty *>(k.second));
      break;

    case NoVisualProperties:
    case AllVisualProperties:
      break;
    case AllColorProperties:
      inputData->setElementLabelBorderColor(static_cast<ColorProperty *>(k.second));
      inputData->setElementLabelColor(static_cast<ColorProperty *>(k.second));
      inputData->setElementBorderColor(static_cast<ColorProperty *>(k.second));
      inputData->setElementColor(static_cast<ColorProperty *>(k.second));
      break;
    }
  }
}
