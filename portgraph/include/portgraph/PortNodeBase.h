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

#ifndef PORTNODEBASE_H_
#define PORTNODEBASE_H_

#include <tulip/IntegerProperty.h>
#include <tulip/Node.h>

#include <portgraph/PorgyConstants.h>
#include <portgraph/porgyconf.h>

class PortGraph;

namespace tlp {
class SizeProperty;
class ColorProperty;
class LayoutProperty;
}

class PORTGRAPH_SCOPE PortNodeBase {

protected:
  static const int sizeRate; // constant for drawing a PortNode
  const tlp::node centerNode;
  PortGraph &ParentPortGraph;

public:
  enum PortNodeType { PORTNODE, BRIDGE };

  PortNodeBase(PortGraph &pg, const tlp::node &);
  virtual ~PortNodeBase() {}

  inline tlp::node getCenter() const {
    return centerNode;
  }
  inline const PortGraph &getParentPortGraph() const {
    return ParentPortGraph;
  }
  std::string getName() const;
  void setName(std::string name) const;
  tlp::Color getColor() const;

  static bool isCenter(const tlp::node, tlp::Graph *g);

  virtual void resizeCenter(tlp::SizeProperty *nodeSize, tlp::LayoutProperty *layout) const = 0;
  virtual void draw(tlp::SizeProperty *nodeSize, tlp::ColorProperty *viewColor,
                    tlp::LayoutProperty *layout, tlp::ColorProperty *bordercolor) const = 0;
//  void setCenterAlphaValue(tlp::ColorProperty *colorProperty) const;
  void hidePortNodeEdges(tlp::ColorProperty *colorProperty) const;
  void setShape() const;

  virtual PortNodeType getType() = 0;

  // implicit conversion from Port to tlp::node
  operator tlp::node() const {
    return this->getCenter();
  }
  void select(const bool state, tlp::BooleanProperty *resultProperty) const;

  /**
   * @brief getTypeProperty
   * @param g
   * @return
   */
  static inline tlp::IntegerProperty *getTypeProperty(tlp::Graph *g) {
    return g->getProperty<tlp::IntegerProperty>(PorgyConstants::PORTNODE_TYPE);
  }

  /**
       * @brief getTypeProperty returns the property containing the node type
   * information for each node. Convenience function call the static
   * getTypeProperty function.
       * @return
       */
  tlp::IntegerProperty *getTypeProperty() const;
};

#endif /* PORTNODEBASE_H_ */
