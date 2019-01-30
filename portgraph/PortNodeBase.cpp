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
 * last modification : $Id:
 */
#include <tulip/BooleanProperty.h>
#include <tulip/ColorProperty.h>
#include <tulip/StringProperty.h>

#include <portgraph/Bridge.h>
#include <portgraph/PorgyConstants.h>
#include <portgraph/PortGraph.h>
#include <portgraph/PortNodeBase.h>

using namespace std;
using namespace tlp;

// constants for drawing the PortNode
const int PortNodeBase::sizeRate = 1;

PortNodeBase::PortNodeBase(PortGraph &pg, const node &n) : centerNode(n), ParentPortGraph(pg) {
  assert(PortNodeBase::isCenter(n, pg.getGraph()));
}

// renvoie le label du centre
string PortNodeBase::getName() const {
  return ParentPortGraph.getGraph()
      ->getProperty<StringProperty>("viewLabel")
      ->getNodeValue(centerNode);
}

void PortNodeBase::setName(std::string name) const {
  ParentPortGraph.getGraph()
      ->getProperty<StringProperty>("viewLabel")
      ->setNodeValue(centerNode, name);
}

Color PortNodeBase::getColor() const {
  return ParentPortGraph.getGraph()
      ->getProperty<ColorProperty>("viewColor")
      ->getNodeValue(centerNode);
}

tlp::IntegerProperty *PortNodeBase::getTypeProperty() const {
  return getTypeProperty(ParentPortGraph.getGraph());
}

// static method
bool PortNodeBase::isCenter(const tlp::node n, tlp::Graph *g) {
  assert(g->isElement(n));
  return (PortNodeBase::getTypeProperty(g)->getNodeValue(n) == PorgyConstants::CENTER);
}

// change le alpha des sommets centraux
//void PortNodeBase::setCenterAlphaValue(ColorProperty *colorProperty) const {
//  Color col = colorProperty->getNodeValue(centerNode);
//  col.setA(PorgyConstants::CENTER_ALPHA);
//  colorProperty->setNodeValue(centerNode, col);
//}

// rend invisible les arêtes
void PortNodeBase::hidePortNodeEdges(tlp::ColorProperty *colorProperty) const {
  Graph *g(ParentPortGraph.getGraph());
  ColorProperty *BorderColor = g->getProperty<ColorProperty>("viewBorderColor");
  for (edge e : g->allEdges(centerNode)) {
    if (g->isElement(e)) {
      Color col = colorProperty->getEdgeValue(e);
      col.setA(PorgyConstants::PORTNODEEDGE_ALPHA);
      colorProperty->setEdgeValue(e, col);
      BorderColor->setEdgeValue(e, col);
    }
  }
}

void PortNodeBase::setShape() const {
  ParentPortGraph.getGraph()
      ->getProperty<IntegerProperty>("viewShape")
      ->setNodeValue(centerNode, PorgyConstants::CENTER_SHAPE);
}

void PortNodeBase::select(const bool state, BooleanProperty *resultProperty) const {
  Graph *g = ParentPortGraph.getGraph();
  Observable::holdObservers();
  if (!Bridge::isBridge(centerNode, g)) {
    resultProperty->setNodeValue(centerNode, state);
    // edges are only going from the center to the ports. Checked when loading a
    // project
    for (edge e : g->allEdges(centerNode)) {
      if (g->isElement(e)) {
        resultProperty->setEdgeValue(e, state);
        resultProperty->setNodeValue(g->target(e), state);
      }
    }
  }
  Observable::unholdObservers();
}
