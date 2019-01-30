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
#include <tulip/ColorProperty.h>
#include <tulip/Graph.h>

using namespace tlp;

template <typename PropertyType, typename NodeType, typename EdgeType>
void AbstractGraphMorphing::computeMiddleValueForProperty(tlp::Graph *fromGraph, PropertyType *from,
                                                          tlp::Graph *toGraph, PropertyType *to,
                                                          PropertyType *outFrom,
                                                          PropertyType *outTo) const {
  for (node n : fromGraph->nodes()) {
    if (toGraph->isElement(n)) {
      NodeType r = computeMiddleValue<NodeType>(from->getNodeValue(n), to->getNodeValue(n));
      outFrom->setNodeValue(n, r);
      outTo->setNodeValue(n, r);
    }
  }
  for (edge e : fromGraph->edges()) {
    if (toGraph->isElement(e)) {
      EdgeType r = computeMiddleValue<EdgeType>(from->getEdgeValue(e), to->getEdgeValue(e));
      outFrom->setEdgeValue(e, r);
      outTo->setEdgeValue(e, r);
    }
  }
}

template <typename T>
T AbstractGraphMorphing::computeMiddleValue(const T &from, const T &to) const {
  return from + ((to - from) / 2.f);
}
