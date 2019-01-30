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
#include <porgy/PropertyValueToQVariantManager.h>
#include <porgy/PropertyValueToQVariantConverter.h>

#include <tulip/BooleanProperty.h>
#include <tulip/ColorProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/GraphProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/TulipMetaTypes.h>

using namespace tlp;
using namespace std;

template <>
unsigned int qHash(const string &s) {
  return qHash(QString(s.c_str()));
}

#define REGISTER_CONVERTER(map, KEY, converter) map[KEY] = converter;
#define REGISTER_NODE_DEFAULT_CONVERTER(PROPERTYTYPE)                                              \
  REGISTER_CONVERTER(_defaultNodeValueConverters, PROPERTYTYPE::propertyTypename,                  \
                     (new NodeValueToQVariantConverter()))
#define REGISTER_EDGE_DEFAULT_CONVERTER(PROPERTYTYPE)                                              \
  REGISTER_CONVERTER(_defaultEdgeValueConverters, PROPERTYTYPE::propertyTypename,                  \
                     (new EdgeValueToQVariantConverter()))
#define REGISTER_NODE_OVERLOAD_CONVERTER(PROPERTYNAME, CONVERTER)                                  \
  REGISTER_CONVERTER(_overloadedPropertyNodeValueConverters, PROPERTYNAME, CONVERTER)
#define REGISTER_EDGE_OVERLOAD_CONVERTER(PROPERTYNAME, CONVERTER)                                  \
  REGISTER_CONVERTER(_overloadedPropertyEdgeValueConverters, PROPERTYNAME, CONVERTER)

PropertyValueToQVariantManager::PropertyValueToQVariantManager(){

    // Register default properties converter
    REGISTER_NODE_DEFAULT_CONVERTER(BooleanProperty) REGISTER_NODE_DEFAULT_CONVERTER(
        ColorProperty) REGISTER_NODE_DEFAULT_CONVERTER(DoubleProperty)
        REGISTER_NODE_DEFAULT_CONVERTER(GraphProperty) REGISTER_NODE_DEFAULT_CONVERTER(
            IntegerProperty) REGISTER_NODE_DEFAULT_CONVERTER(LayoutProperty)
            REGISTER_NODE_DEFAULT_CONVERTER(SizeProperty) REGISTER_NODE_DEFAULT_CONVERTER(
                StringProperty) REGISTER_NODE_DEFAULT_CONVERTER(BooleanVectorProperty)
                REGISTER_NODE_DEFAULT_CONVERTER(ColorVectorProperty) REGISTER_NODE_DEFAULT_CONVERTER(
                    CoordVectorProperty) REGISTER_NODE_DEFAULT_CONVERTER(DoubleVectorProperty)
                    REGISTER_NODE_DEFAULT_CONVERTER(IntegerVectorProperty) REGISTER_NODE_DEFAULT_CONVERTER(
                        SizeVectorProperty) REGISTER_NODE_DEFAULT_CONVERTER(StringVectorProperty)

                        REGISTER_EDGE_DEFAULT_CONVERTER(
                            BooleanProperty) REGISTER_EDGE_DEFAULT_CONVERTER(ColorProperty)
                            REGISTER_EDGE_DEFAULT_CONVERTER(
                                DoubleProperty) REGISTER_EDGE_DEFAULT_CONVERTER(GraphProperty)
                                REGISTER_EDGE_DEFAULT_CONVERTER(
                                    IntegerProperty) REGISTER_EDGE_DEFAULT_CONVERTER(LayoutProperty)
                                    REGISTER_EDGE_DEFAULT_CONVERTER(SizeProperty)
                                        REGISTER_EDGE_DEFAULT_CONVERTER(StringProperty)
                                            REGISTER_EDGE_DEFAULT_CONVERTER(BooleanVectorProperty)
                                                REGISTER_EDGE_DEFAULT_CONVERTER(ColorVectorProperty)
                                                    REGISTER_EDGE_DEFAULT_CONVERTER(
                                                        CoordVectorProperty)
                                                        REGISTER_EDGE_DEFAULT_CONVERTER(
                                                            DoubleVectorProperty)
                                                            REGISTER_EDGE_DEFAULT_CONVERTER(
                                                                IntegerVectorProperty)
                                                                REGISTER_EDGE_DEFAULT_CONVERTER(
                                                                    SizeVectorProperty)
                                                                    REGISTER_EDGE_DEFAULT_CONVERTER(
                                                                        StringVectorProperty)}

PropertyValueToQVariantManager::~PropertyValueToQVariantManager() {
  qDeleteAll(_overloadedPropertyNodeValueConverters.begin(),
             _overloadedPropertyNodeValueConverters.end());
  qDeleteAll(_overloadedPropertyEdgeValueConverters.begin(),
             _overloadedPropertyEdgeValueConverters.end());
  qDeleteAll(_defaultNodeValueConverters.begin(), _defaultNodeValueConverters.end());
  qDeleteAll(_defaultEdgeValueConverters.begin(), _defaultEdgeValueConverters.end());
}

void PropertyValueToQVariantManager::setDefaultNodePropertyConverter(
    const string &propertyTypeName, PropertyValueToQVariantConverter *converter) {
  replaceConverter(_defaultNodeValueConverters, propertyTypeName, converter);
}

void PropertyValueToQVariantManager::setDefaultEdgePropertyConverter(
    const string &propertyTypeName, PropertyValueToQVariantConverter *converter) {
  replaceConverter(_defaultEdgeValueConverters, propertyTypeName, converter);
}

void PropertyValueToQVariantManager::overloadNodePropertyConverter(
    const string &propertyName, PropertyValueToQVariantConverter *converter) {
  replaceConverter(_overloadedPropertyNodeValueConverters, propertyName, converter);
}

void PropertyValueToQVariantManager::overloadEdgePropertyConverter(
    const string &propertyName, PropertyValueToQVariantConverter *converter) {
  replaceConverter(_overloadedPropertyEdgeValueConverters, propertyName, converter);
}

void PropertyValueToQVariantManager::replaceConverter(
    QHash<string, PropertyValueToQVariantConverter *> &map, const string &key,
    PropertyValueToQVariantConverter *converter) {
  QHash<string, PropertyValueToQVariantConverter *>::iterator it = map.find(key);
  if (it != map.end()) {
    delete it.value();
    map.erase(it);
  }
  if (converter != nullptr) {
    map[key] = converter;
  }
}

PropertyValueToQVariantConverter *PropertyValueToQVariantManager::findConverter(
    QHash<string, PropertyValueToQVariantConverter *> &overloadedProperties,
    QHash<string, PropertyValueToQVariantConverter *> &defaultProperties,
    tlp::PropertyInterface *p) {
  assert(p != nullptr);
  // Search for overloaded properties
  PropertyValueToQVariantConverter *converter = nullptr;
  if (!p->getName().empty()) {
    converter = findConverter(overloadedProperties, string(p->getName()));
  }
  if (converter == nullptr) {
    // Search for default converter
    converter = findConverter(defaultProperties, p->getTypename());
  }
  return converter;
}

QVariant PropertyValueToQVariantManager::nodeData(tlp::node n, tlp::PropertyInterface *p,
                                                  int role) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyNodeValueConverters, _defaultNodeValueConverters, p);
  return converter != nullptr ? converter->value(n.id, p, role) : QVariant();
}

QVariant PropertyValueToQVariantManager::defaultNodeData(tlp::PropertyInterface *p, int role) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyNodeValueConverters, _defaultNodeValueConverters, p);
  return converter != nullptr ? converter->defaultValue(p, role) : QVariant();
}

bool PropertyValueToQVariantManager::setNodeData(const QVariant &data, tlp::node n,
                                                 tlp::PropertyInterface *p, int role) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyNodeValueConverters, _defaultNodeValueConverters, p);
  return converter != nullptr ? converter->setValue(n.id, p, data, role) : false;
}

bool PropertyValueToQVariantManager::setAllNodeData(const QVariant &data, tlp::PropertyInterface *p,
                                                    int role) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyNodeValueConverters, _defaultNodeValueConverters, p);
  return converter != nullptr ? converter->setAllValue(p, data, role) : false;
}

QVariant PropertyValueToQVariantManager::edgeData(tlp::edge e, tlp::PropertyInterface *p,
                                                  int role) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyEdgeValueConverters, _defaultEdgeValueConverters, p);
  return converter != nullptr ? converter->value(e.id, p, role) : QVariant();
}

QVariant PropertyValueToQVariantManager::defaultEdgeData(PropertyInterface *p, int role) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyEdgeValueConverters, _defaultEdgeValueConverters, p);
  return converter != nullptr ? converter->defaultValue(p, role) : QVariant();
}

bool PropertyValueToQVariantManager::setEdgeData(const QVariant &data, tlp::edge e,
                                                 tlp::PropertyInterface *p, int role) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyEdgeValueConverters, _defaultEdgeValueConverters, p);
  return converter != nullptr ? converter->setValue(e.id, p, data, role) : false;
}

bool PropertyValueToQVariantManager::setDefaultEdgeData(const QVariant &data,
                                                        tlp::PropertyInterface *p, int role) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyEdgeValueConverters, _defaultEdgeValueConverters, p);
  return converter != nullptr ? converter->setAllValue(p, data, role) : false;
}

PropertyValueToQVariantConverter *PropertyValueToQVariantManager::findConverter(
    const QHash<string, PropertyValueToQVariantConverter *> &map, const std::string &key) const {
  QHash<string, PropertyValueToQVariantConverter *>::const_iterator it = map.find(key);
  return it == map.end() ? nullptr : it.value();
}

Qt::ItemFlags PropertyValueToQVariantManager::nodeFlags(tlp::node n, tlp::PropertyInterface *p) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyNodeValueConverters, _defaultNodeValueConverters, p);
  return converter != nullptr ? converter->flags(n.id, p) : Qt::ItemFlags();
}

Qt::ItemFlags PropertyValueToQVariantManager::nodeDefaultFlags(tlp::PropertyInterface *p) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyNodeValueConverters, _defaultNodeValueConverters, p);
  return converter != nullptr ? converter->defaultFlags(p) : Qt::ItemFlags();
}

Qt::ItemFlags PropertyValueToQVariantManager::edgeFlags(edge e, tlp::PropertyInterface *p) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyEdgeValueConverters, _defaultEdgeValueConverters, p);
  return converter != nullptr ? converter->flags(e.id, p) : Qt::ItemFlags();
}

Qt::ItemFlags PropertyValueToQVariantManager::edgeDefaultFlags(tlp::PropertyInterface *p) {
  PropertyValueToQVariantConverter *converter =
      findConverter(_overloadedPropertyEdgeValueConverters, _defaultEdgeValueConverters, p);
  return converter != nullptr ? converter->defaultFlags(p) : Qt::ItemFlags();
}
