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

#ifndef PORTBASE_H_
#define PORTBASE_H_

#include <tulip/Node.h>
#include <tulip/TulipViewSettings.h>

#include <portgraph/porgyconf.h>

class PortNode;

namespace tlp {
class Color;
//class ColorProperty;
class Graph;
}

class PORTGRAPH_SCOPE PortBase {

protected:
  tlp::node port;
  const PortNode *ParentPortNode;

public:
  PortBase(const tlp::node, const PortNode *);
  virtual ~PortBase() {}

  std::string getName() const;
  tlp::NodeShape::NodeShapes getShape() const;
  tlp::Color getColor() const;
  static bool isPort(const tlp::node, tlp::Graph *);
//  void setPortAlphaValue(tlp::ColorProperty *colorP) const;

  const inline PortNode *getParentPortNode() const {
    return ParentPortNode;
  }
  const inline tlp::node getNode() const {
    return port;
  }
};

#endif /* PORTBASE_H_ */
