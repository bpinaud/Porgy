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
#include <tulip/IntegerProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>
#include <tulip/StringProperty.h>

#include <portgraph/Bridge.h>
#include <portgraph/BridgePort.h>
#include <portgraph/PorgyConstants.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraph.h>
#include <portgraph/PortNode.h>

using namespace std;
using namespace tlp;

Bridge::Bridge(PortGraph &pg, const node n) : PortNodeBase(pg, n) {
  Graph *g = getParentPortGraph().getGraph();
  for (node v : g->getInOutNodes(getCenter())) {
    if ((PortBase::isPort(v, g)) && Bridge::isBridge(v, g)) {
      BridgePort *p = new BridgePort(v, this);
      ports.push_back(p);
      container_left_bridge[p->getLeft()].push_front(p->getRight());
      if (p->getSpecial())
        container_left_bridge[p->getRight()].push_front(p->getLeft());
    }
  }
}

Bridge::~Bridge() {
  for (auto p : ports)
    delete p;
}

static double PORT_SIZE = 0.1;

///////////////////////////////
void Bridge::resizeCenter(SizeProperty *nodeSize, LayoutProperty *) const {
  int size = (max(ports.size(), static_cast<size_t>(5)) - 1) * PorgyConstants::SPACE / 2 + 1;
  nodeSize->setNodeValue(centerNode, Size(size, size, 1));
}

void Bridge::update() {
  Graph *g(getParentPortGraph().getGraph());
  g->getProperty<IntegerProperty>("viewShape")
      ->setNodeValue(centerNode, PorgyConstants::BRIDGE_CENTER_SHAPE);
  g->getProperty<StringProperty>("viewIcon")->setNodeValue(centerNode, CENTER_ICON);
  g->getProperty<StringProperty>("viewLabel")->setNodeValue(centerNode, "");
}

void Bridge::resizePorts(SizeProperty *nodeSize) const {
  for (BridgePort *bp : getBridgePorts()) {
    nodeSize->setNodeValue(bp->getNode(), Size(PORT_SIZE, PORT_SIZE, 1));
  }
}

void Bridge::draw(SizeProperty *nodeSize, ColorProperty *viewColor, LayoutProperty *,
                  ColorProperty *bordercolor) const {
  setEdgesColor(viewColor, bordercolor);
  hidePortNodeEdges(viewColor);
  resizePorts(nodeSize);
  resizeCenter(nodeSize);
}

void Bridge::addPort(node source, node target) {
  Graph *g = getParentPortGraph().getGraph();
  PortGraphRuleDecorator dec(g);
  assert(dec.getSide(source) == PorgyConstants::SIDE_LEFT);
  assert(dec.getSide(target) == PorgyConstants::SIDE_RIGHT);
  IntegerProperty *side = dec.getSideProperty();
  IntegerProperty *type = PortNodeBase::getTypeProperty(g);
  IntegerProperty *shape = g->getProperty<IntegerProperty>("viewShape");
  ColorProperty *color = g->getProperty<ColorProperty>("viewColor");

  g->getRoot()->reserveEdges(3);

  node port = g->addNode();
  type->setNodeValue(port, PorgyConstants::PORT);
  side->setNodeValue(port, PorgyConstants::SIDE_BRIDGE);
  shape->setNodeValue(port, NodeShape::Hexagon);
  color->setNodeValue(port, Color(0, 0, 0, 255));
  edge e = g->addEdge(centerNode, port);
  color->setEdgeValue(e, Color(0, 0, 0, 0));
  side->setEdgeValue(e, PorgyConstants::SIDE_BRIDGE_PORT);
  edge e1 = g->addEdge(source, port);
  edge e2 = g->addEdge(port, target);
  side->setEdgeValue(e1, PorgyConstants::SIDE_BRIDGE);
  side->setEdgeValue(e2, PorgyConstants::SIDE_BRIDGE);
  BridgePort *p = new BridgePort(port, this);
  ports.push_back(p);
}

const forward_list<node> Bridge::getRight(const Port *n) const {
  auto it = container_left_bridge.find(n->getNode());
  return (it != container_left_bridge.end()) ? it->second : forward_list<node>();
}

node Bridge::createEmptyBridge(Graph *rule) {
  PortGraphRuleDecorator dec(rule);
  node bridge = rule->addNode();
  dec.getSideProperty()->setNodeValue(bridge, PorgyConstants::SIDE_BRIDGE);
  PortNodeBase::getTypeProperty(rule)->setNodeValue(bridge, PorgyConstants::CENTER);
  rule->getProperty<IntegerProperty>("viewShape")
      ->setNodeValue(bridge, PorgyConstants::BRIDGE_CENTER_SHAPE);
  rule->getProperty<StringProperty>("viewIcon")->setNodeValue(bridge, CENTER_ICON);
  rule->getProperty<ColorProperty>("viewColor")->setNodeValue(bridge, Color(0, 0, 0, 100));
  rule->getProperty<ColorProperty>("viewLabelColor")->setNodeValue(bridge, Color::White);
  rule->getProperty<SizeProperty>("viewSize")->setNodeValue(bridge, Size(5, 5, 1));
  rule->getProperty<LayoutProperty>("viewLayout")->setNodeValue(bridge, Coord(0, 0, 0));
  return bridge;
}

// returns the Y location of the intersection between Y axis and the line
// passing by the x and y coordinates
static float meanYStepAux(const float x1, const float x2, const float y1, const float y2) {
  return (x2 - x1 == 0) ? 0 : (y1 - ((y2 - y1) * x1) / (x2 - x1));
}

void Bridge::setPositions(LayoutProperty *layout) {
  PortGraphRuleDecorator dec(ParentPortGraph.getGraph());
  // Size of bridge : (nbPorts-1) * PortNodeBase::space + nbPorts * 1
  if (!ports.empty()) {
    float min = 0, sum = 0;
    Coord c1, c2;
    for (BridgePort *bp : getBridgePorts()) {
      c1 = layout->getNodeValue(ParentPortGraph[bp->getLeft()]->getCenter());
      if (dec.getSide(bp->getRight()) == PorgyConstants::SIDE_RIGHT)
        c2 = layout->getNodeValue(ParentPortGraph[bp->getRight()]->getCenter());
      else
        c2 = Coord(0, 0, 0);
      sum += meanYStepAux(c1.getX(), c2.getX(), c1.getY(), c2.getY());
    }

    min = sum / ports.size();
    mySortPorts(ports, min, layout); // ports triés par portnodes connectés en
                                     // fonction de la somme de leurs ordonnées
    min -= ((ports.size() - 1) / 2.0f) * PorgyConstants::SPACE / 2; // Min Y of the display
    float new_pos = min;
    for (BridgePort *bp : getBridgePorts()) {
      layout->setNodeValue(bp->getNode(), Coord(0, new_pos, 0));
      new_pos += PorgyConstants::SPACE / 2;
    }
    float max = new_pos - PorgyConstants::SPACE / 2;

    // déplacement du centre pour le recentrer
    layout->setNodeValue(getCenter(), Coord(0, (max + min) / 2, 0));
  } else
    layout->setNodeValue(getCenter(), Coord(0, 0, 0));
}

// Puts the ports of the bridge in a good order to avoid edge crossing
void Bridge::mySortPorts(vector<BridgePort *> &sidePorts, const float yBridgeCenter,
                         LayoutProperty *layout) {
  bool b = false;
  Coord myCenter = Coord(0, yBridgeCenter, 0);

  while (!b && sidePorts.size() > 1) {
    b = true;
    for (unsigned int i = 0; i < sidePorts.size() - 1; i++) {
      node c1L = ParentPortGraph[sidePorts[i]->getLeft()]->getCenter();
      node n = sidePorts[i]->getRight();
      Coord posR;
      node c1R;
      if (n.isValid()) {
        c1R = ParentPortGraph[n]->getCenter();
        posR = layout->getNodeValue(c1R);
      } else
        posR = Coord(0, 0, 1);
      node c2L = ParentPortGraph[sidePorts[i + 1]->getLeft()]->getCenter();
      n = sidePorts[i + 1]->getRight();
      Coord pos_nextR;
      node c2R;
      if (n.isValid()) {
        c2R = ParentPortGraph[n]->getCenter();
        pos_nextR = layout->getNodeValue(c2R);
      } else
        pos_nextR = Coord(0, 0, 1);
      if ((c1L == c2L) && (c1R == c2R))
        continue;

      Coord posL = layout->getNodeValue(c1L);
      Coord pos_nextL = layout->getNodeValue(c2L);

      // WEST or EAST -> DeltaY/DeltaX
      float deltaL = (myCenter[1] - posL[1]) / (myCenter[0] - posL[0]);
      float deltaR = (myCenter[1] - posR[1]) / (myCenter[0] - posR[0]);
      float delta = deltaL - deltaR;

      float delta_nextL = (myCenter[1] - pos_nextL[1]) / (myCenter[0] - pos_nextL[0]);
      float delta_nextR = (myCenter[1] - pos_nextR[1]) / (myCenter[0] - pos_nextR[0]);
      float delta_next = delta_nextL - delta_nextR;

      if (delta < delta_next) {
        BridgePort *tmp = sidePorts[i];
        sidePorts[i] = sidePorts[i + 1];
        sidePorts[i + 1] = tmp;
        b = false;
      }
    }
  }
}

void Bridge::setEdgesColor(ColorProperty *color, ColorProperty *bordercolor) const {
  Graph *g = ParentPortGraph.getGraph();
  PortGraphRuleDecorator dec(ParentPortGraph.getGraph());
  hidePortNodeEdges(color);
  for (BridgePort *bp : getBridgePorts()) {
    for (edge x : dec.allEdges(bp->getNode())) {
      if (dec.isElement(x)) {
        const pair<node, node> &ends = dec.ends(x);
        if (!PortNodeBase::isCenter(ends.first, g) && (!PortNodeBase::isCenter(ends.second, g))) {
          if (dec.getSide(x) == PorgyConstants::SIDE_BRIDGE_OPP) {
            color->setEdgeValue(x, Color(0, 255, 0, 255));
            bordercolor->setEdgeValue(x, Color(0, 255, 0, 255));
          } else {
            color->setEdgeValue(x, Color(255, 0, 0, 255));
            bordercolor->setEdgeValue(x, Color(255, 0, 0, 255));
          }
        }
      }
    }
  }
}
