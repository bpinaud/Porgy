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
#include <porgy/PortNodeQt.h>
#include <porgy/PropertyValueToQVariantConverter.h>
#include <porgy/PropertyValueToQVariantManager.h>
#include <porgy/RuleElementValueModel.h>

#include <portgraph/PorgyTlpGraphStructure.h>

#include <tulip/BooleanProperty.h>
#include <tulip/Graph.h>
#include <tulip/IntegerProperty.h>
#include <tulip/StringProperty.h>

#include <unordered_set>

using namespace tlp;
using namespace std;

RuleElementValueModel::RuleElementValueModel(Graph *graph, ElementType elementType,
                                             unsigned int elementId, QObject *parent)
    : GraphElementValueModel(graph, elementType, elementId, unordered_set<string>(), parent) {
  _propertyDataConverter->overloadNodePropertyConverter(PorgyConstants::RULE_SIDE,
                                                        new RuleSideNodePropertyConverter());
  _propertyDataConverter->overloadEdgePropertyConverter(PorgyConstants::RULE_SIDE,
                                                        new RuleSideEdgePropertyConverter());
  _propertyDataConverter->overloadNodePropertyConverter(PorgyConstants::PORTNODE_TYPE,
                                                        new PortNodeTypeNodePropertyConverter());
  if (PorgyTlpGraphStructure::isRuleUsed(graph)) {
    // Avoid modification to all properties
    _propertyDataConverter->overloadNodePropertyConverter(
        PorgyConstants::M, new UneditableNodeValueToQVariantConverter());
    _propertyDataConverter->overloadNodePropertyConverter(
        PorgyConstants::N, new UneditableNodeValueToQVariantConverter());
    _propertyDataConverter->overloadNodePropertyConverter(
        PorgyConstants::W, new UneditableNodeValueToQVariantConverter());
    _propertyDataConverter->overloadNodePropertyConverter(
        PorgyConstants::ARITY, new UneditableNodeValueToQVariantConverter());
    _propertyDataConverter->overloadEdgePropertyConverter(
        PorgyConstants::M, new UneditableEdgeValueToQVariantConverter());
    _propertyDataConverter->overloadEdgePropertyConverter(
        PorgyConstants::N, new UneditableEdgeValueToQVariantConverter());
    _propertyDataConverter->overloadEdgePropertyConverter(
        PorgyConstants::W, new UneditableEdgeValueToQVariantConverter());
    _propertyDataConverter->overloadEdgePropertyConverter(
        "viewLabel", new UneditableEdgeValueToQVariantConverter());
    _propertyDataConverter->overloadNodePropertyConverter(
        "viewLabel", new UneditableNodeValueToQVariantConverter());
    _propertyDataConverter->overloadEdgePropertyConverter(
        "viewColor", new UneditableEdgeValueToQVariantConverter());
    _propertyDataConverter->overloadNodePropertyConverter(
        "viewColor", new UneditableNodeValueToQVariantConverter());
    _propertyDataConverter->overloadNodePropertyConverter(
        "viewBorderWidth", new UneditableNodeValueToQVariantConverter());
    _propertyDataConverter->overloadEdgePropertyConverter(
        "viewBorderWidth", new UneditableEdgeValueToQVariantConverter());
    _propertyDataConverter->overloadNodePropertyConverter(
        "viewBorderColor", new UneditableNodeValueToQVariantConverter());
    _propertyDataConverter->overloadEdgePropertyConverter(
        "viewBorderColor", new UneditableEdgeValueToQVariantConverter());
  } else {
    _propertyDataConverter->overloadNodePropertyConverter(PorgyConstants::ARITY,
                                                          new NodeValueToQVariantConverter());
  }
  // add the RuleComparisonNode/Edge custom field
  unordered_set<string> propertiesNames;
  PorgyTlpGraphStructure::getMatchingPropertyNames(graph, propertiesNames);
  for (const std::string &st : propertiesNames) {
    _propertyDataConverter->overloadNodePropertyConverter(
        st + PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX,
        new RuleComparisonNodePropertyConverter());
    _propertyDataConverter->overloadEdgePropertyConverter(
        st + PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX,
        new RuleComparisonEdgePropertyConverter());
  }
  initPropertiesList(buildPropertiesList());
}

unordered_set<string> RuleElementValueModel::buildPropertiesList() {
  std::unordered_set<std::string> matchingPropertiesList, matchingPropertiesListex;
  PorgyTlpGraphStructure::getMatchingPropertyNames(_graph, matchingPropertiesList);

  // Extend the normal output with the "*_comp" values
  for (const std::string &st : matchingPropertiesList) {
    matchingPropertiesListex.insert(st);
    matchingPropertiesListex.insert(st + PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX);
  }
  PorgyTlpGraphStructure::getPropertiesToDisplay(_graph, matchingPropertiesListex);
  matchingPropertiesListex.insert(PorgyConstants::M);
  matchingPropertiesListex.insert(PorgyConstants::N);
  matchingPropertiesListex.insert(PorgyConstants::W);
  matchingPropertiesListex.insert(PorgyConstants::RULE_SIDE);
  switch (_displayType) {
  case NODE:
    matchingPropertiesListex.insert(PorgyConstants::PORTNODE_TYPE);
    matchingPropertiesListex.insert(PorgyConstants::ARITY);
    break;
  case EDGE:
    break;
  }

  return matchingPropertiesListex;
}

bool RuleElementValueModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (hasIndex(index.row(), index.column())) {
    if (role == Qt::CheckStateRole) {
      PropertyInterface *p = _propertyTable[index.row()];
      string name = p->getName();
      string prop = name.substr(
          0, name.length() - PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX.length());
      BooleanProperty *isused =
          _graph->getProperty<BooleanProperty>(prop + PorgyConstants::MATCHING_PROPERTIES_SUFFIX);
      if (_displayType == NODE) {
        node n(_eltId);
        isused->setNodeValue(n, !isused->getNodeValue(n));
      } else {
        edge e(_eltId);
        isused->setEdgeValue(e, !isused->getEdgeValue(e));
      }
      emit dataChanged(index, index);
      return true;

    } else
      return GraphElementValueModel::setData(index, value, role);
  }
  return false;
}

Qt::ItemFlags RuleElementValueModel::flags(const QModelIndex &index) const {
  if (hasIndex(index.row(), index.column())) {
    Qt::ItemFlags f = GraphElementValueModel::flags(index);
    unordered_set<string> propertiesNames;
    PorgyTlpGraphStructure::getMatchingPropertyNames(_graph, propertiesNames, true);
    PropertyInterface *p = _propertyTable[index.row()];
    if (propertiesNames.find(p->getName()) != propertiesNames.end()) {
      f |= Qt::ItemIsUserCheckable;
    }
    return f;
  }
  return Qt::ItemFlags();
}

QVariant RuleElementValueModel::data(const QModelIndex &index, int role) const {
  if (hasIndex(index.row(), index.column())) {
    if (role == Qt::CheckStateRole) {
      unordered_set<string> propertiesNames;
      PorgyTlpGraphStructure::getMatchingPropertyNames(_graph, propertiesNames, true);
      PropertyInterface *p = _propertyTable[index.row()];
      if (propertiesNames.find(p->getName()) != propertiesNames.end()) {
        string name = p->getName();
        string prop = name.substr(
            0, name.length() - PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX.length());
        BooleanProperty *isused =
            _graph->getProperty<BooleanProperty>(prop + PorgyConstants::MATCHING_PROPERTIES_SUFFIX);
        if (_displayType == NODE) {
          if (isused->getNodeValue(node(_eltId)))
            return Qt::Checked;
          else
            return Qt::Unchecked;
        } else {
          if (isused->getEdgeValue(edge(_eltId)))
            return Qt::Checked;
          else
            return Qt::Unchecked;
        }
      }
    } else if ((role == Qt::ToolTipRole) && flags(index).testFlag(Qt::ItemIsUserCheckable)) {
      return "Check/Uncheck the box to use/to not use element for matching for "
             "this property";
    } else {
      return GraphElementValueModel::data(index, role);
    }
  }
  return QVariant();
}
