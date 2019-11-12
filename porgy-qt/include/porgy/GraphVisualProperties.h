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
#ifndef GRAPHVISUALPROPERTIES_H
#define GRAPHVISUALPROPERTIES_H

#include <unordered_map>

#include <portgraph/porgyconf.h>

#include <QObject>

namespace tlp {
class Graph;
class PropertyInterface;
class GlGraphInputData;
class ColorProperty;
class LayoutProperty;
class SizeProperty;
class DoubleProperty;
}

enum VisualProperties {
  NoVisualProperties = 0,
  ElementColor = 0x1,
  ElementBorderColor = 0x2,
  ElementLabelColor = 0x4,
  AllColorProperties = 0x7,
  ElementSize = 0x8,
  ElementLayout = 0x10,
  ElementBorderWidth = 0x20,
  ElementLabelBorderColor = 0x21,
  AllVisualProperties = 0xffffffff
};

Q_DECLARE_FLAGS(VisualPropertiesFlags, VisualProperties)

class PORGY_SCOPE GraphVisualProperties {
public:
  /**
    * @brief Build an object with no visual properties.
    **/
  GraphVisualProperties() = default;
  /**
    * @brief Delete all the initialized properties.
    **/
  void deleteProperties();
  /**
    * @brief Copy contructor.
    **/
  GraphVisualProperties(const GraphVisualProperties &toCopy) = default;
  /**
    * @brief Build an object from a GlGraphInputData visual properties.
    **/
  GraphVisualProperties(tlp::GlGraphInputData *inputData, VisualPropertiesFlags propertiesToGet);
  /**
    * @brief Build an object from a graph.
    **/
  GraphVisualProperties(tlp::Graph *graph, VisualPropertiesFlags propertiesToGet);
  virtual ~GraphVisualProperties();

  void initFromInputData(tlp::GlGraphInputData *inputData, VisualPropertiesFlags propertiesToGet);
  void initFromGraph(tlp::Graph *graph, VisualPropertiesFlags propertiesToGet);

  /**
   * @brief property return the property corresponding to the given property
   * type.
   * @param property
   * @return
   */
  tlp::PropertyInterface *property(const VisualProperties property) const {
    return (_properties.find(property)!=_properties.end()) ? _properties.at(property) : nullptr;
  }

  void setProperty(VisualProperties type, tlp::PropertyInterface *property) {
    _properties[type] = property;
  }

  /**
    * @brief Overload visual properties stored in a GlGraphInputData by visual
    *properties in this object.
    **/
  void copyToInputData(tlp::GlGraphInputData *inputData) const;

  // Color properties
  tlp::ColorProperty *getElementColor() const;
  void setElementColor(tlp::ColorProperty *property);

  /**
   * @brief getElementBorderColor
   * @return
   */
  tlp::ColorProperty *getElementBorderColor() const;

  /**
   * @brief setElementBorderColor
   * @param property
   */
  void setElementBorderColor(tlp::ColorProperty *property);
  tlp::ColorProperty *getElementLabelColor() const;
  void setElementLabelColor(tlp::ColorProperty *property);

  // Layout prop
  tlp::LayoutProperty *getElementLayout() const;

  void setElementLayout(tlp::LayoutProperty *property);

  // Size property
  tlp::SizeProperty *getElementSize() const;
  void setElementSize(tlp::SizeProperty *property);

  tlp::DoubleProperty *getElementBorderWidth() const;
  void setElementBorderWidth(tlp::DoubleProperty *property);

  tlp::ColorProperty *getElementLabelBorderColor() const;
  void setElementLabelBorderColor(tlp::ColorProperty *property);

private:
  std::unordered_map<VisualProperties, tlp::PropertyInterface *> _properties;
};

#endif // GRAPHVISUALPROPERTIES_H
