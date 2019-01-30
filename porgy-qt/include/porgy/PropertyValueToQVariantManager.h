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
#ifndef PROPERTYVALUETOQVARIANTMANAGER_H
#define PROPERTYVALUETOQVARIANTMANAGER_H

template <class T>
unsigned int qHash(const T &);

#include <QHash>
#include <QVariant>

#include <tulip/Edge.h>

struct PropertyValueToQVariantConverter;
namespace tlp {
class PropertyInterface;
struct node;
}
class PropertyValueToQVariantManager {

  QHash<std::string, PropertyValueToQVariantConverter *> _defaultNodeValueConverters;
  QHash<std::string, PropertyValueToQVariantConverter *> _defaultEdgeValueConverters;

  QHash<std::string, PropertyValueToQVariantConverter *> _overloadedPropertyNodeValueConverters;
  QHash<std::string, PropertyValueToQVariantConverter *> _overloadedPropertyEdgeValueConverters;

public:
  PropertyValueToQVariantManager();
  ~PropertyValueToQVariantManager();

  /**
   * @brief setDefaultNodePropertyConverter Replace the default converter for
   * the property type by the converter in parameter. The object take the
   * responsibility of the converter.
   * @param propertyTypeName
   * @param converter
   */
  void setDefaultNodePropertyConverter(const std::string &propertyTypeName,
                                       PropertyValueToQVariantConverter *converter);
  void setDefaultEdgePropertyConverter(const std::string &propertyTypeName,
                                       PropertyValueToQVariantConverter *converter);

  void overloadNodePropertyConverter(const std::string &propertyName,
                                     PropertyValueToQVariantConverter *converter);
  void overloadEdgePropertyConverter(const std::string &propertyName,
                                     PropertyValueToQVariantConverter *converter);

  QVariant nodeData(tlp::node n, tlp::PropertyInterface *p, int role = Qt::EditRole);
  Qt::ItemFlags nodeFlags(tlp::node n, tlp::PropertyInterface *p);
  Qt::ItemFlags nodeDefaultFlags(tlp::PropertyInterface *p);
  QVariant defaultNodeData(tlp::PropertyInterface *p, int role = Qt::EditRole);
  bool setNodeData(const QVariant &data, tlp::node n, tlp::PropertyInterface *p,
                   int role = Qt::EditRole);
  bool setAllNodeData(const QVariant &data, tlp::PropertyInterface *p, int role = Qt::EditRole);

  QVariant edgeData(tlp::edge e, tlp::PropertyInterface *p, int role = Qt::EditRole);
  Qt::ItemFlags edgeFlags(tlp::edge e, tlp::PropertyInterface *p);
  Qt::ItemFlags edgeDefaultFlags(tlp::PropertyInterface *p);
  QVariant defaultEdgeData(tlp::PropertyInterface *p, int role = Qt::EditRole);
  bool setEdgeData(const QVariant &data, tlp::edge e, tlp::PropertyInterface *p,
                   int role = Qt::EditRole);
  bool setDefaultEdgeData(const QVariant &data, tlp::PropertyInterface *p, int role = Qt::EditRole);

private:
  void replaceConverter(QHash<std::string, PropertyValueToQVariantConverter *> &map,
                        const std::string &key, PropertyValueToQVariantConverter *converter);

  PropertyValueToQVariantConverter *
  findConverter(QHash<std::string, PropertyValueToQVariantConverter *> &overloadedProperties,
                QHash<std::string, PropertyValueToQVariantConverter *> &defaultProperties,
                tlp::PropertyInterface *p);
  PropertyValueToQVariantConverter *
  findConverter(const QHash<std::string, PropertyValueToQVariantConverter *> &map,
                const std::string &key) const;
};

#endif // PROPERTYVALUETOQVARIANTMANAGER_H
