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
#include <tulip/BooleanProperty.h>

#include <portgraph/PortNodeBase.h>

using namespace tlp;

template <typename PROPTYPE, typename NODETYPE, typename EDGETYPE>
void GraphStateManager::setValueForElements(tlp::Graph *graph, tlp::BooleanProperty *elements,
                                            const NODETYPE &nodeValue, const EDGETYPE &edgeValue,
                                            PROPTYPE *propertyToUpdate, bool treatNodes,
                                            bool treatEdges) const {
  if (treatNodes) {
    for (node n : elements->getNodesEqualTo(true, graph)) {
      if (PortNodeBase::isCenter(n, graph)) {
        propertyToUpdate->setNodeValue(n, nodeValue);
      }
    }
  }
  if (treatEdges) {
    for (edge e : elements->getEdgesEqualTo(true, graph)) {
      std::pair<tlp::node, tlp::node> ends = graph->ends(e);
      if (!PortNodeBase::isCenter(ends.first, graph) &&
          !PortNodeBase::isCenter(ends.second, graph)) {
        propertyToUpdate->setEdgeValue(e, edgeValue);
      }
    }
  }
}
