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

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortBase.h>
#include <portgraph/PortGraph.h>
#include <portgraph/PortNode.h>

#include <tulip/ColorProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/StringProperty.h>

using namespace tlp;

PortBase::PortBase(const tlp::node n, const PortNode *pp) : port(n), ParentPortNode(pp) {}

std::string PortBase::getName() const {
  return ParentPortNode->getParentPortGraph()
      .getGraph()
      ->getProperty<StringProperty>("viewLabel")
      ->getNodeValue(port);
}

NodeShape::NodeShapes PortBase::getShape() const {
  return static_cast<NodeShape::NodeShapes>(ParentPortNode->getParentPortGraph()
                                                .getGraph()
                                                ->getProperty<IntegerProperty>("viewShape")
                                                ->getNodeValue(port));
}

// static method
bool PortBase::isPort(const node n, Graph *g) {
  assert(g->isElement(n));
  return (PortNodeBase::getTypeProperty(g)->getNodeValue(n) == PorgyConstants::PORT);
}

Color PortBase::getColor() const {
  return ParentPortNode->getParentPortGraph()
      .getGraph()
      ->getProperty<ColorProperty>("viewColor")
      ->getNodeValue(port);
}

//void PortBase::setPortAlphaValue(ColorProperty *colorP) const {
//  Color c = colorP->getNodeValue(port);
//  c.setA(PorgyConstants::PORT_ALPHA);
//  colorP->setNodeValue(port, c);
//}
