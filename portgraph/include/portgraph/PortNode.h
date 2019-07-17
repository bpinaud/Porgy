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
#ifndef _PORTNODE_H
#define _PORTNODE_H

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortNodeBase.h>
#include <portgraph/porgyconf.h>

namespace tlp {
class Graph;
class SizeProperty;
class ColorProperty;
class LayoutProperty;
}

class Port;

class PORTGRAPH_SCOPE PortNode : public PortNodeBase {

  std::vector<Port *> ports;

  void setOptimalSize(const int, tlp::node, tlp::SizeProperty *nodeSize) const;
  void mySortPorts(std::vector<Port *> &, const bool, const bool,
                   tlp::LayoutProperty *layout) const;
  void placePorts(tlp::LayoutProperty *layout, std::vector<Port *> &, std::vector<Port *> &,
                  std::vector<Port *> &, std::vector<Port *> &, const float, const float) const;
  std::pair<unsigned, unsigned> drawPosition_aux(const bool, tlp::LayoutProperty *layout,
                                                 const bool specifyWidths = false,
                                                 const int xWidth = 0, const int yWidth = 0) const;
  // If draw && specifyWidhts, it draws the portnode with xWidth and yWidth.
  // If !draw, it calculates the desired withs iof the portnode
  void placePorts(tlp::LayoutProperty *layout, std::vector<Port *> &, std::vector<Port *> &,
                  std::vector<Port *> &, std::vector<Port *> &) const;

public:
  // implicit conversion from PortNode to tlp::node
  operator tlp::node() const {
    return this->getCenter();
  }

  PortNode(PortGraph &, const tlp::node);
  ~PortNode() override;
  std::vector<const PortNode *> getInOutConnectedPortNodes() const;
  std::vector<const PortNode *> getConnectedInPortNodes() const;
  std::vector<const PortNode *> getConnectedOutPortNodes() const;

  PortNodeType getType() override;

  inline const std::vector<Port *> &getPorts() const {
    return ports;
  }

  inline size_t nbPorts() const {
    return ports.size();
  }

  void resizeCenter(tlp::SizeProperty *nodeSize, tlp::LayoutProperty *layout) const override;
  void resizeCenter(const float, const float, tlp::SizeProperty *nodeSize) const;
  virtual void setPositions(tlp::LayoutProperty *layout) const;
  void setPositions(const int, const int, tlp::LayoutProperty *layout) const;
  std::pair<unsigned, unsigned> getOptimalSize(tlp::LayoutProperty *layout)
      const; // gives the optimal number of ports in x and y of the portnode

  unsigned deg() const;
  unsigned indeg() const;
  unsigned outdeg() const;

  void draw(tlp::SizeProperty *nodeSize, tlp::ColorProperty *viewColor, tlp::LayoutProperty *layout,
            tlp::ColorProperty *bordercolor) const override;

  static bool check(const PortNode *rule_portnode, const PortNode *modele_portnode,
                    const bool exact,
                    const PorgyTlpGraphStructure::matchpropvector &matchingPropertiesList);
};

#endif
