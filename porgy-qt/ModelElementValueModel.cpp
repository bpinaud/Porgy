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
#include <porgy/ModelElementValueModel.h>
#include <porgy/PropertyValueToQVariantConverter.h>
#include <porgy/PropertyValueToQVariantManager.h>

#include <portgraph/PorgyConstants.h>
#include <portgraph/PorgyTlpGraphStructure.h>

#include <tulip/BooleanProperty.h>
#include <tulip/Graph.h>
#include <tulip/StringProperty.h>

#include <unordered_set>

using namespace tlp;
using namespace std;

ModelElementValueModel::ModelElementValueModel(Graph *graph, ElementType elementType,
                                               unsigned int elementId, QObject *parent)
    : GraphElementValueModel(graph, elementType, elementId, unordered_set<string>(), parent) {
  _propertyDataConverter->overloadNodePropertyConverter(PorgyConstants::PORTNODE_TYPE,
                                                        new PortNodeTypeNodePropertyConverter());
  _propertyDataConverter->overloadNodePropertyConverter(
      PorgyConstants::TAG, new UneditableNodeValueToQVariantConverter());
  _propertyDataConverter->overloadEdgePropertyConverter(
      PorgyConstants::TAG, new UneditableEdgeValueToQVariantConverter());

  initPropertiesList(buildPropertiesList());
}

unordered_set<string> ModelElementValueModel::buildPropertiesList() {
  unordered_set<string> propertiesNames;
  PorgyTlpGraphStructure::getPropertiesToDisplay(_graph, propertiesNames);
  propertiesNames.insert(PorgyConstants::TAG);
  propertiesNames.insert("viewSelection");

  // Search for boolean properties : P properties.
  for (PropertyInterface *property : _graph->getObjectProperties()) {
    if (property->getTypename() == BooleanProperty::propertyTypename &&
        property->getName()[0] == 'P') {
      propertiesNames.insert(property->getName());
    }
  }
  switch (_displayType) {
  case NODE: {
    propertiesNames.insert(PorgyConstants::PORTNODE_TYPE);
    break;
  }
  case EDGE: {
    break;
  }
  }

  return propertiesNames;
}
