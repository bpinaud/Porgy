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
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * last modification : $Id: $
 */

#include <portgraph/Bridge.h>
#include <portgraph/BridgePort.h>
#include <portgraph/PorgyConstants.h>
#include <portgraph/PortGraph.h>
#include <portgraph/PortGraphRule.h>

using namespace tlp;
using namespace std;

BridgePort::BridgePort(const node n, const Bridge *pp)
    : PortBase(n, (PortNode *)(pp)), greenedge(false) {
  // je cherche les sommets qui correspondent dans les parties droite et gauche
  // de la règle
  PortGraphRuleDecorator dec(pp->getParentPortGraph().getGraph());
  for (node tmp : dec.getInOutNodes(n)) {
    if (dec.getSide(tmp) == PorgyConstants::SIDE_LEFT) {
      left = tmp;
    } else if (dec.getSide(tmp) == PorgyConstants::SIDE_RIGHT) {
      right = tmp;
    }
  }
  if (!right.isValid()) { // arête verte
    greenedge = true;
    for (edge e : dec.allEdges(n)) {
      if (dec.isElement(e) && dec.getSide(e) == PorgyConstants::SIDE_BRIDGE_OPP) {
        const pair<node, node> &ends = dec.ends(e);
        if ((ends.second != n) && (left != ends.second))
          right = ends.second;
        else if ((ends.first != n) && (left != ends.first))
          right = ends.first;
      }
    }
  }

  assert(left.isValid() && right.isValid());
}
