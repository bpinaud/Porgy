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
#ifndef _BRIDGE_H
#define _BRIDGE_H

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNodeBase.h>

class BridgePort;
class PortNode;

class PORTGRAPH_SCOPE Bridge : public PortNodeBase {

  /**
   * key: a LHS node
   * value: all linked nodes in RHS with a red edges
   **/
  std::map<tlp::node, std::forward_list<tlp::node>> container_left_bridge;
  std::vector<BridgePort *> ports;

  void mySortPorts(std::vector<BridgePort *> &, const float, tlp::LayoutProperty *layout);

public:
  Bridge(PortGraph &, const tlp::node);
  ~Bridge() override;
  inline PortNodeBase::PortNodeType getType() override {
    return PortNodeBase::BRIDGE;
  }
  void update();

  inline const std::vector<BridgePort *> &getBridgePorts() const {
    return ports;
  }
  inline unsigned numportsbridge() const {
    return ports.size();
  }
  void setPositions(tlp::LayoutProperty *layout);
  void setEdgesColor(tlp::ColorProperty *color, tlp::ColorProperty *bordercolor) const;
  const std::forward_list<tlp::node> getRight(const Port *n) const;
  void resizeCenter(tlp::SizeProperty *nodeSize,
                    tlp::LayoutProperty *layout = nullptr) const override;
  void resizePorts(tlp::SizeProperty *nodeSize) const;
  void draw(tlp::SizeProperty *nodeSize, tlp::ColorProperty *color, tlp::LayoutProperty *layout,
            tlp::ColorProperty *bordercolor) const override;
  void addPort(tlp::node source, tlp::node dest);
  template <typename TYPE>
  static bool isBridge(TYPE elt, tlp::Graph *);
  static tlp::node createEmptyBridge(tlp::Graph *rule);
};

// is given node a member of a bridge structure?
template <typename TYPE>
bool Bridge::isBridge(TYPE elt, tlp::Graph *g) {
  assert(g->isElement(elt));
  PortGraphRuleDecorator dec(g);
  return PorgyTlpGraphStructure::isRuleGraph(g) &&
         (dec.getSide(elt) == PorgyConstants::SIDE_BRIDGE ||
          dec.getSide(elt) == PorgyConstants::SIDE_BRIDGE_OPP ||
          dec.getSide(elt) == PorgyConstants::SIDE_BRIDGE_PORT);
}

#endif
