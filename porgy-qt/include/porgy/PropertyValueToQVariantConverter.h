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
#ifndef PROPERTYVALUETOQVARIANTCONVERTER_H
#define PROPERTYVALUETOQVARIANTCONVERTER_H

#include <QVariant>

#include <tulip/PropertyInterface.h>

namespace tlp {
class Graph;
}

/**
 * @brief The PropertyValueToQVariantConverter class convert properties values
 * to QVariant object
 */
struct PropertyValueToQVariantConverter {
  virtual ~PropertyValueToQVariantConverter();
  /**
   * @brief value Returns the QVariant object for the given type
   * @param id
   * @param property
   * @return
   */
  virtual QVariant value(unsigned int id, tlp::PropertyInterface *property,
                         int role = Qt::EditRole) const = 0;
  /**
   * @brief defaultValue
   * @param property
   * @return
   */
  virtual QVariant defaultValue(tlp::PropertyInterface *property,
                                int role = Qt::EditRole) const = 0;
  /**
   * @brief setValue Set the value in the qvariant
   * @param id
   * @param property
   * @param data
   * @return
   */
  virtual bool setValue(unsigned int id, tlp::PropertyInterface *property, const QVariant &data,
                        int role = Qt::EditRole) const = 0;
  /**
   * @brief setAllValue
   * @param property
   * @param data
   * @return
   */
  virtual bool setAllValue(tlp::PropertyInterface *property, const QVariant &data,
                           int role = Qt::EditRole) const = 0;

  virtual Qt::ItemFlags defaultFlags(tlp::PropertyInterface *) {
    return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
  }

  virtual Qt::ItemFlags flags(unsigned int, tlp::PropertyInterface *p) {
    return defaultFlags(p);
  }
};

/**
 * @brief The EmptyPropertyValueToQVariantConverter class is an implementation
 * of the PropertyValueToQVariantConverter that return invalid values.
 */
struct EmptyPropertyValueToQVariantConverter : public PropertyValueToQVariantConverter {
  QVariant value(unsigned int, tlp::PropertyInterface *, int) const override {
    return QVariant();
  }
  QVariant defaultValue(tlp::PropertyInterface *, int) const override {
    return QVariant();
  }
  bool setValue(unsigned int, tlp::PropertyInterface *, const QVariant &, int) const override {
    return false;
  }
  bool setAllValue(tlp::PropertyInterface *, const QVariant &, int) const override {
    return false;
  }
  Qt::ItemFlags defaultFlags(tlp::PropertyInterface *) override {
    return Qt::ItemFlags();
  }
  Qt::ItemFlags flags(unsigned int, tlp::PropertyInterface *) override {
    return Qt::ItemFlags();
  }
};

/**
 * @brief Template converter for nodes.
 */
struct NodeValueToQVariantConverter : public PropertyValueToQVariantConverter {
  QVariant value(unsigned int id, tlp::PropertyInterface *property, int role) const override;
  bool setValue(unsigned int id, tlp::PropertyInterface *property, const QVariant &data,
                int role) const override;
  QVariant defaultValue(tlp::PropertyInterface *property, int role) const override;
  bool setAllValue(tlp::PropertyInterface *property, const QVariant &data, int role) const override;
};

/**
 * @brief Uneditable converter for nodes.
 */
struct UneditableNodeValueToQVariantConverter : public NodeValueToQVariantConverter {
  Qt::ItemFlags defaultFlags(tlp::PropertyInterface *) override {
    return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  }
};

/**
 * @brief Template converter for nodes.
 */
struct EdgeValueToQVariantConverter : public PropertyValueToQVariantConverter {
  QVariant value(unsigned int id, tlp::PropertyInterface *property, int role) const override;
  bool setValue(unsigned int id, tlp::PropertyInterface *property, const QVariant &data,
                int role) const override;
  QVariant defaultValue(tlp::PropertyInterface *property, int role) const override;
  bool setAllValue(tlp::PropertyInterface *property, const QVariant &data, int role) const override;
};

/**
 * Uneditable value converter.
 */
struct UneditableEdgeValueToQVariantConverter : public EdgeValueToQVariantConverter {
  Qt::ItemFlags defaultFlags(tlp::PropertyInterface *) override {
    return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  }
};

/**
 * @brief The AbstractEnumPropertyEditor class
 */
class AbstractEnumPropertyEditor : public PropertyValueToQVariantConverter {

  bool _editable;

public:
  AbstractEnumPropertyEditor(bool isEditable = true) : _editable(isEditable) {}

  bool setValue(unsigned int, tlp::PropertyInterface *, const QVariant &, int) const override {
    return false;
  }

  bool setAllValue(tlp::PropertyInterface *, const QVariant &, int) const override {
    return false;
  }
  Qt::ItemFlags defaultFlags(tlp::PropertyInterface *) override {
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (_editable)
      f |= Qt::ItemIsEditable;
    return f;
  }

  virtual QString label(const unsigned int &value) const = 0;
};

/**
 * @brief The AbstractEnumNodePropertyEditor class handles enum types for nodes.
 */
struct AbstractEnumNodePropertyEditor : public AbstractEnumPropertyEditor {
  AbstractEnumNodePropertyEditor(bool isEditable = true) : AbstractEnumPropertyEditor(isEditable) {}
  QVariant value(unsigned int id, tlp::PropertyInterface *property, int role) const override;
  QVariant defaultValue(tlp::PropertyInterface *property, int role = Qt::EditRole) const override;
};

/**
 * @brief The AbstractEnumNodePropertyEditor class handles enum types for Edge.
 */
struct AbstractEnumEdgePropertyEditor : public AbstractEnumPropertyEditor {
  AbstractEnumEdgePropertyEditor(bool isEditable = true) : AbstractEnumPropertyEditor(isEditable) {}

  QVariant value(unsigned int id, tlp::PropertyInterface *property, int role) const override;
  QVariant defaultValue(tlp::PropertyInterface *property, int role = Qt::EditRole) const override;
};

struct PortNodeTypeNodePropertyConverter : public AbstractEnumNodePropertyEditor {
  PortNodeTypeNodePropertyConverter();
  QString label(const unsigned int &value) const override;
};

/**
 * @brief The AbstractGraphIdPropertyConverter class is an abstract class to
 * transform graph id stored in an integer property to the graph name.
 */
struct AbstractGraphIdPropertyConverter : public PropertyValueToQVariantConverter {
  bool setValue(unsigned int, tlp::PropertyInterface *, const QVariant &, int) const override {
    return false;
  }

  bool setAllValue(tlp::PropertyInterface *, const QVariant &, int) const override {
    return false;
  }
  Qt::ItemFlags defaultFlags(tlp::PropertyInterface *) override {
    return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  }

  QString idToGraph(tlp::Graph *graph, const int &value) const;
};

/**
 * @brief The NodeGraphIdPropertyConverter class Transform id stored in nodes in
 * graph names
 */
struct NodeGraphIdPropertyConverter : public AbstractGraphIdPropertyConverter {
  QVariant value(unsigned int id, tlp::PropertyInterface *property, int role) const override;
  QVariant defaultValue(tlp::PropertyInterface *property, int role = Qt::EditRole) const override;
};

/**
 * @brief The NodeGraphIdPropertyConverter class Transform id stored in edges in
 * graph names
 */
struct EdgeGraphIdPropertyConverter : public AbstractGraphIdPropertyConverter {
  QVariant value(unsigned int id, tlp::PropertyInterface *property, int role) const override;
  QVariant defaultValue(tlp::PropertyInterface *property, int role = Qt::EditRole) const override;
};

/**
 * @brief The TransformationTypePropertyConverter class converter for
 * PorgyConstants::RuleSide enum type.
 */
struct RuleSideNodePropertyConverter : public AbstractEnumNodePropertyEditor {
  RuleSideNodePropertyConverter();
  QString label(const unsigned int &value) const override;
};

struct RuleSideEdgePropertyConverter : public AbstractEnumEdgePropertyEditor {
  RuleSideEdgePropertyConverter();
  QString label(const unsigned int &value) const override;
};

/**
 * @brief The TransformationTypePropertyConverter class converter for
 * PorgyConstants::RuleComparison enum type.
 */
struct RuleComparisonNodePropertyConverter : public AbstractEnumNodePropertyEditor {

  QString label(const unsigned int &value) const override;
  QVariant value(unsigned int id, tlp::PropertyInterface *property, int role) const override;
  bool setValue(unsigned int id, tlp::PropertyInterface *pr, const QVariant &v,
                int role) const override;
};

struct RuleComparisonEdgePropertyConverter : public AbstractEnumEdgePropertyEditor {
  QString label(const unsigned int &value) const override;
  QVariant value(unsigned int id, tlp::PropertyInterface *property, int role) const override;
  bool setValue(unsigned int id, tlp::PropertyInterface *pr, const QVariant &v,
                int role) const override;
};

#endif // PROPERTYVALUETOQVARIANTCONVERTER_H
