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
#include <tulip/GraphModel.h>
#include <tulip/IntegerProperty.h>
#include <tulip/TlpQtTools.h>
#include <tulip/TulipMetaTypes.h>

#include <portgraph/PortGraphModel.h>
#include <portgraph/PortNode.h>

#include <porgy/PorgyQtConstants.h>
#include <porgy/PropertyValueToQVariantConverter.h>

using namespace tlp;

PropertyValueToQVariantConverter::~PropertyValueToQVariantConverter() {}

QVariant AbstractEnumNodePropertyEditor::value(unsigned int id, tlp::PropertyInterface *property,
                                               int role) const {
  if (role == Qt::DisplayRole) {
    tlp::IntegerProperty *intProp = static_cast<tlp::IntegerProperty *>(property);
    return label(intProp->getNodeValue(tlp::node(id)));
  } else {
    return QVariant();
  }
}
QVariant AbstractEnumNodePropertyEditor::defaultValue(tlp::PropertyInterface *property,
                                                      int role) const {
  if (role == Qt::DisplayRole) {
    tlp::IntegerProperty *intProp = static_cast<tlp::IntegerProperty *>(property);
    return label(intProp->getNodeDefaultValue());
  } else {
    return QVariant();
  }
}

QVariant AbstractEnumEdgePropertyEditor::value(unsigned int id, tlp::PropertyInterface *property,
                                               int role) const {
  if (role == Qt::DisplayRole) {
    tlp::IntegerProperty *intProp = static_cast<tlp::IntegerProperty *>(property);
    return label(intProp->getEdgeValue(tlp::edge(id)));
  } else {
    return QVariant();
  }
}
QVariant AbstractEnumEdgePropertyEditor::defaultValue(tlp::PropertyInterface *property,
                                                      int role) const {
  if (role == Qt::DisplayRole) {
    tlp::IntegerProperty *intProp = static_cast<tlp::IntegerProperty *>(property);
    return label(intProp->getEdgeDefaultValue());
  } else {
    return QVariant();
  }
}

QString AbstractGraphIdPropertyConverter::idToGraph(tlp::Graph *graph, const int &value) const {
  if (graph != nullptr) {
    tlp::Graph *child =
        graph->getId() == unsigned(value) ? graph : graph->getRoot()->getDescendantGraph(value);
    if (child != nullptr) {
      return tlp::tlpStringToQString(child->getName());
    }
  }
  return QString();
}

QVariant NodeGraphIdPropertyConverter::value(unsigned int id, tlp::PropertyInterface *property,
                                             int role) const {
  if (role == Qt::DisplayRole) {
    tlp::IntegerProperty *intProp = static_cast<tlp::IntegerProperty *>(property);
    return idToGraph(intProp->getGraph(), intProp->getNodeValue(tlp::node(id)));
  } else {
    return QVariant();
  }
}

QVariant NodeGraphIdPropertyConverter::defaultValue(tlp::PropertyInterface *property,
                                                    int role) const {
  if (role == Qt::DisplayRole) {
    tlp::IntegerProperty *intProp = static_cast<tlp::IntegerProperty *>(property);
    return idToGraph(intProp->getGraph(), intProp->getNodeDefaultValue());
  } else {
    return QVariant();
  }
}

QVariant EdgeGraphIdPropertyConverter::value(unsigned int id, tlp::PropertyInterface *property,
                                             int role) const {
  if (role == Qt::DisplayRole) {
    tlp::IntegerProperty *intProp = static_cast<tlp::IntegerProperty *>(property);
    return idToGraph(intProp->getGraph(), intProp->getEdgeValue(tlp::edge(id)));
  } else {
    return QVariant();
  }
}
QVariant EdgeGraphIdPropertyConverter::defaultValue(tlp::PropertyInterface *property,
                                                    int role) const {
  if (role == Qt::DisplayRole) {
    tlp::IntegerProperty *intProp = static_cast<tlp::IntegerProperty *>(property);
    return idToGraph(intProp->getGraph(), intProp->getEdgeDefaultValue());
  } else {
    return QVariant();
  }
}

PortNodeTypeNodePropertyConverter::PortNodeTypeNodePropertyConverter()
    : AbstractEnumNodePropertyEditor(false) {}

QString PortNodeTypeNodePropertyConverter::label(const unsigned int &value) const {
  return PorgyConstants::PortNodeTypeToString.at(static_cast<PorgyConstants::PortNodeType>(value));
}

RuleSideNodePropertyConverter::RuleSideNodePropertyConverter()
    : AbstractEnumNodePropertyEditor(false) {}

QString RuleSideNodePropertyConverter::label(const unsigned int &value) const {
  return PorgyConstants::RuleSideToString.at(static_cast<PorgyConstants::RuleSide>(value));
}

RuleSideEdgePropertyConverter::RuleSideEdgePropertyConverter()
    : AbstractEnumEdgePropertyEditor(false) {}

QString RuleSideEdgePropertyConverter::label(const unsigned int &value) const {
  return PorgyConstants::RuleSideToString.at(static_cast<PorgyConstants::RuleSide>(value));
}

QString RuleComparisonNodePropertyConverter::label(const unsigned int &value) const {
  return PorgyConstants::RuleComparisonToString.at(
      static_cast<PorgyConstants::ComparisonOp>(value));
}

QVariant RuleComparisonNodePropertyConverter::value(unsigned int id,
                                                    tlp::PropertyInterface *property,
                                                    int role) const {
  if (role == Qt::DisplayRole) {
    IntegerProperty *p = static_cast<IntegerProperty *>(property);
    PorgyConstants::ComparisonOp op =
        static_cast<PorgyConstants::ComparisonOp>(p->getNodeValue(node(id)));
    return QVariant::fromValue<PorgyConstants::ComparisonOp>(op);
  } else
    return QVariant();
}

QString RuleComparisonEdgePropertyConverter::label(const unsigned int &value) const {
  return PorgyConstants::RuleComparisonToString.at(
      static_cast<PorgyConstants::ComparisonOp>(value));
}

QVariant RuleComparisonEdgePropertyConverter::value(unsigned int id,
                                                    tlp::PropertyInterface *property,
                                                    int role) const {
  if (role == Qt::DisplayRole) {
    IntegerProperty *p = static_cast<IntegerProperty *>(property);
    PorgyConstants::ComparisonOp op =
        static_cast<PorgyConstants::ComparisonOp>(p->getEdgeValue(edge(id)));
    return QVariant::fromValue<PorgyConstants::ComparisonOp>(op);
  } else
    return QVariant();
}

bool RuleComparisonEdgePropertyConverter::setValue(unsigned int id, PropertyInterface *pr,
                                                   const QVariant &v, int role) const {
  if (role == Qt::EditRole) {
    IntegerProperty *p = static_cast<IntegerProperty *>(pr);
    PorgyConstants::ComparisonOp op = v.value<PorgyConstants::ComparisonOp>();
    p->setEdgeValue(edge(id), static_cast<int>(op));
    return true;
  }
  return false;
}

bool RuleComparisonNodePropertyConverter::setValue(unsigned int id, PropertyInterface *pr,
                                                   const QVariant &v, int role) const {
  if (role == Qt::EditRole) {
    IntegerProperty *p = static_cast<IntegerProperty *>(pr);
    PorgyConstants::ComparisonOp op = v.value<PorgyConstants::ComparisonOp>();
    p->setNodeValue(node(id), static_cast<int>(op));
    return true;
  }
  return false;
}

QVariant NodeValueToQVariantConverter::value(unsigned int id, tlp::PropertyInterface *property,
                                             int role) const {
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return tlp::GraphModel::nodeValue(id, property);
  }
  return QVariant();
}

bool NodeValueToQVariantConverter::setValue(unsigned int id, tlp::PropertyInterface *property,
                                            const QVariant &data, int role) const {
  if (role == Qt::EditRole) {
    return tlp::GraphModel::setNodeValue(id, property, data);
  }
  return false;
}

QVariant NodeValueToQVariantConverter::defaultValue(tlp::PropertyInterface *property,
                                                    int role) const {
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return tlp::GraphModel::nodeDefaultValue(property);
  }
  return QVariant();
}

bool NodeValueToQVariantConverter::setAllValue(tlp::PropertyInterface *property,
                                               const QVariant &data, int role) const {
  if (role == Qt::EditRole) {
    return tlp::GraphModel::setAllNodeValue(property, data);
  }
  return false;
}

QVariant EdgeValueToQVariantConverter::value(unsigned int id, tlp::PropertyInterface *property,
                                             int role) const {
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return tlp::GraphModel::edgeValue(id, property);
  }
  return QVariant();
}

bool EdgeValueToQVariantConverter::setValue(unsigned int id, tlp::PropertyInterface *property,
                                            const QVariant &data, int role) const {
  if (role == Qt::EditRole) {
    return tlp::GraphModel::setEdgeValue(id, property, data);
  }
  return false;
}

QVariant EdgeValueToQVariantConverter::defaultValue(tlp::PropertyInterface *property,
                                                    int role) const {
  if (role == Qt::EditRole) {
    return tlp::GraphModel::edgeDefaultValue(property);
  }
  return QVariant();
}

bool EdgeValueToQVariantConverter::setAllValue(tlp::PropertyInterface *property,
                                               const QVariant &data, int role) const {
  if (role == Qt::EditRole) {
    return tlp::GraphModel::setAllEdgeValue(property, data);
  }
  return false;
}
