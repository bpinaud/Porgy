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
#include <portgraph/PorgyConstants.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Trace.h>

#include <porgy/PropertyValueToQVariantConverter.h>
#include <porgy/PropertyValueToQVariantManager.h>
#include <porgy/TraceElementValueModel.h>

#include <tulip/IntegerProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/TlpQtTools.h>

#include <tulip/Graph.h>

using namespace tlp;
using namespace std;

/**
 * @brief The TransformationTypePropertyConverter class converter for
 * Trace::TransformationType enum type.
 */
class TransformationTypePropertyConverter : public AbstractEnumEdgePropertyEditor {
public:
  QString label(const unsigned int &value) const override {
    return Trace::TransformationTypeToString.at(static_cast<Trace::TransformationType>(value));
  }

  Qt::ItemFlags defaultFlags(tlp::PropertyInterface *) override {
    return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  }
};

/**
 * @brief The ParallelTypePropertyConverter class is and editor for the
 * Trace::TransformationParallelInformation enum type.
 */
class ParallelTypePropertyConverter : public AbstractEnumEdgePropertyEditor {
public:
  QString label(const unsigned int &value) const override {
    return Trace::TransformationParallelInformationToString.at(
        static_cast<Trace::TransformationParallelInformation>(value));
  }

  Qt::ItemFlags defaultFlags(tlp::PropertyInterface *) override {
    return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  }
};

/**
 * @brief The TraceNodeTypePropertyConverter class is an editor for the values
 * of the Trace::NodeType enum.
 */
class TraceNodeTypePropertyConverter : public AbstractEnumNodePropertyEditor {
public:
  QString label(const unsigned int &value) const override {
    return Trace::NodeTypeToString.at(static_cast<Trace::NodeType>(value));
  }

  Qt::ItemFlags defaultFlags(tlp::PropertyInterface *) override {
    return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  }
};

TraceElementValueModel::TraceElementValueModel(Graph *graph, ElementType elementType,
                                               unsigned int elementId, QObject *parent)
    : GraphElementValueModel(graph, elementType, elementId, unordered_set<string>(), parent) {
  // By default all string properties will be unmodifiable
  _propertyDataConverter->setDefaultEdgePropertyConverter(
      StringProperty::propertyTypename, new UneditableEdgeValueToQVariantConverter);
  _propertyDataConverter->setDefaultNodePropertyConverter(
      StringProperty::propertyTypename, new UneditableEdgeValueToQVariantConverter);
  // Overload properties nodes converters
  _propertyDataConverter->overloadNodePropertyConverter(PorgyConstants::TYPE_PROPERTY,
                                                        new TraceNodeTypePropertyConverter);
  _propertyDataConverter->overloadNodePropertyConverter(
      PorgyConstants::TRANSFORMATION_RULE_PROPERTY, new NodeGraphIdPropertyConverter());

  // Overload properties edges converters
  _propertyDataConverter->overloadEdgePropertyConverter(
      PorgyConstants::TRANSFORMATION_PARALLEL_INFORMATION_PROPERTY,
      new ParallelTypePropertyConverter);
  _propertyDataConverter->overloadEdgePropertyConverter(PorgyConstants::TYPE_PROPERTY,
                                                        new TransformationTypePropertyConverter());
  _propertyDataConverter->overloadEdgePropertyConverter(
      PorgyConstants::TRANSFORMATION_RULE_PROPERTY, new EdgeGraphIdPropertyConverter());
  _propertyDataConverter->overloadEdgePropertyConverter(
      PorgyConstants::TRANSFORMATION_INSTANCE_PROPERTY, new EdgeGraphIdPropertyConverter());

  // Set properties to dipslay
  initPropertiesList(buildPropertiesList());
}

unordered_set<string> TraceElementValueModel::buildPropertiesList() {
  unordered_set<string> propertiesNames;
  Trace traceGraph(_graph);
  // The type property is used for both nodes and edges
  propertiesNames.insert(PorgyConstants::TYPE_PROPERTY);
  if (_displayType == EDGE) {
    propertiesNames.insert(PorgyConstants::TRANSFORMATION_PPROPERTYNAME_PROPERTY);
    propertiesNames.insert(PorgyConstants::TRANSFORMATION_BANPROPERTYNAME_PROPERTY);

    switch (traceGraph.getTransformationType(edge(_eltId))) {
    case Trace::TransformationType::Simple_Transformation:
      propertiesNames.insert(PorgyConstants::TRANSFORMATION_RULE_PROPERTY);
      propertiesNames.insert(PorgyConstants::TRANSFORMATION_INSTANCE_PROPERTY);
      propertiesNames.insert(PorgyConstants::TRANSFORMATION_PARALLEL_INFORMATION_PROPERTY);
      break;
    case Trace::TransformationType::Strategy_Transformation:
      propertiesNames.insert(PorgyConstants::TRANSFORMATION_STRATEGY_PROPERTY);
      break;
    case Trace::TransformationType::Focus_Transformation:
      propertiesNames.insert(PorgyConstants::TRANSFORMATION_RULE_PROPERTY);
      propertiesNames.insert(PorgyConstants::TRANSFORMATION_INSTANCE_PROPERTY);
      propertiesNames.insert(PorgyConstants::TRANSFORMATION_PARALLEL_INFORMATION_PROPERTY);
      break;
    case Trace::TransformationType::Ban_Transformation:
    case Trace::TransformationType::Update_Transformation:
      break;
    }

  } else {
    propertiesNames.insert("viewLabel");
    if (traceGraph.isFailureNode(node(_eltId))) {
      propertiesNames.insert(PorgyConstants::TRANSFORMATION_RULE_PROPERTY);
    }
  }
  return propertiesNames;
}
