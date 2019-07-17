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
#include <tulip/ColorProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>
#include <tulip/StringCollection.h>

#include <portgraph/Bridge.h>
#include <portgraph/PorgyConstants.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraph.h>
#include <portgraph/PortNode.h>

using namespace std;
using namespace tlp;

PortNode::PortNode(PortGraph &pg, const node n) : PortNodeBase(pg, n) {
  assert(pg.getGraph()->isElement(n));
  // trouver tous les ports
  Graph *g = pg.getGraph();
  for (node v : g->getOutNodes(n)) {
    assert(PortBase::isPort(v, g));
    Port *p = new Port(v, this);
    ports.push_back(p);
    (*pg.container_port)[g->nodePos(v)] = p;
  }
}

PortNode::~PortNode() {
  // Ports deletion done in PortGraph
}

PortNodeBase::PortNodeType PortNode::getType() {
  return PortNodeBase::PORTNODE;
}

vector<const PortNode *> PortNode::getInOutConnectedPortNodes() const {
  vector<const PortNode *> v;
  for (Port *p : getPorts()) {
    for (Port *p1 : p->getConnectedPorts()) {
      v.push_back(p1->getParentPortNode());
    }
  }
  return v;
}

vector<const PortNode *> PortNode::getConnectedInPortNodes() const {
  vector<const PortNode *> v;
  for (Port *p : getPorts()) {
    for (Port *p1 : p->getConnectedInPorts()) {
      v.push_back(p1->getParentPortNode());
    }
  }
  return v;
}

vector<const PortNode *> PortNode::getConnectedOutPortNodes() const {
  vector<const PortNode *> v;
  for (Port *p : getPorts()) {
    for (Port *p1 : p->getConnectedOutPorts()) {
      v.push_back(p1->getParentPortNode());
    }
  }
  return v;
}

///////////////////////////////
unsigned PortNode::indeg() const {
  unsigned degree = 0;

  for (Port *p : getPorts()) {
    degree += p->indeg();
  }
  return degree;
}

///////////////////////////////
unsigned PortNode::outdeg() const {
  unsigned degree = 0;

  for (Port *p : getPorts()) {
    degree += p->outdeg();
  }
  return degree;
}

///////////////////////////////
unsigned PortNode::deg() const {
  return indeg() + outdeg();
}

///////////////////////////////
void PortNode::setOptimalSize(const int sizeRate, node n, SizeProperty *nodeSize) const {
  float xrate = .4f;
  float yrate = 1.0f;
  float xwidth = max(xrate, yrate);
  nodeSize->setNodeValue(n, Size(sizeRate * xwidth, sizeRate * yrate, 1));
}

void PortNode::draw(SizeProperty *nodeSize, ColorProperty *viewColor, LayoutProperty *layout,
                    tlp::ColorProperty *) const {
  for (Port *p : getPorts()) {
    setOptimalSize(PortNode::sizeRate, p->getNode(), nodeSize);
    //remove this code. Let user decides about the alpha value
//    p->setPortAlphaValue(viewColor);
  }
  setPositions(layout);
  resizeCenter(nodeSize, layout);
  //remove this code. Let users decide about the alpha value
//  setCenterAlphaValue(viewColor);
  hidePortNodeEdges(viewColor);
}

bool PortNode::check(const PortNode *rule_portnode, const PortNode *modele_portnode,
                     const bool exact,
                     const PorgyTlpGraphStructure::matchpropvector &matchingPropertiesList) {

#ifdef PORGY_RULE_DEBUG_MESSAGES
  cerr << __PRETTY_FUNCTION__ << ": model portnode: " << modele_portnode->getCenter() << endl;
  cerr << __PRETTY_FUNCTION__ << ": rule portnode: " << rule_portnode->getCenter() << endl;
#endif

  // number of ports
  if (rule_portnode->nbPorts() == modele_portnode->nbPorts()) {
    if ((exact && rule_portnode->deg() == modele_portnode->deg()) ||
        ((!exact) && (rule_portnode->deg() <= modele_portnode->deg()))) {
      // checking the rest of the matching properties
      Graph *modele_portnode_graph = modele_portnode->getParentPortGraph().getGraph();
      Graph *rule_portnode_graph = rule_portnode->getParentPortGraph().getGraph();
      node rule_center = rule_portnode->getCenter();
      node model_center = modele_portnode->getCenter();
      bool ret =
          PorgyTlpGraphStructure::checkNode(modele_portnode_graph, rule_portnode_graph, rule_center,
                                            model_center, matchingPropertiesList);
#ifdef PORGY_RULE_DEBUG_MESSAGES
      if (!ret)
        cerr << __PRETTY_FUNCTION__ << ": portnode center : " << model_center
             << " and rule portnode center " << rule_center << " not compatible" << endl;
#endif
      return ret;
    }

#ifdef PORGY_RULE_DEBUG_MESSAGES
    else {
      cerr << __PRETTY_FUNCTION__ << ": portnode degrees problem (" << modele_portnode->deg()
           << ", " << rule_portnode->deg() << ")" << endl;
    }
#endif
  }
#ifdef PORGY_RULE_DEBUG_MESSAGES
  else
    cerr << __PRETTY_FUNCTION__ << ": problem on the number of ports: " << rule_portnode->nbPorts()
         << " for the rules and " << modele_portnode->nbPorts() << " for the model" << endl;
#endif

  return false;
}

pair<unsigned, unsigned>
PortNode::getOptimalSize(LayoutProperty *layout) const { // Call a draw_position step1 first!
  return drawPosition_aux(false, layout);
}

pair<unsigned, unsigned> PortNode::drawPosition_aux(const bool draw, LayoutProperty *layout,
                                                    const bool specifyWidths, const int xWidth,
                                                    const int yWidth) const {
  assert(specifyWidths == false || (specifyWidths == true && draw == true));
  vector<Port *> northPorts, westPorts, eastPorts, southPorts;
  vector<Port *> freePorts;

  for (Port *port : getPorts()) {
    // if any, take an arbitrary attached PortNode
    node neighbour;
    const Port *p = port->getOneConnectedPort();
    if (p != nullptr) {
      neighbour = p->getParentPortNode()->centerNode; // take the center node
    }

    if (neighbour.isValid()) {
      Coord myPos = layout->getNodeValue(centerNode);
      Coord neighbourPos = layout->getNodeValue(neighbour);
      float deltaX = neighbourPos[0] - myPos[0];
      float deltaY = neighbourPos[1] - myPos[1];
      float ratio = 0;

      if (deltaX == 0)
        ratio = HUGE_VAL; // +INF
      else
        ratio = fabs(deltaY / deltaX);
      // NORTH
      if (ratio >= 1 && deltaY >= 0)
        northPorts.push_back(port);
      // EAST
      else if (ratio < 1 && deltaX >= 0)
        eastPorts.push_back(port);
      // SOUTH
      else if (ratio >= 1 && deltaY < 0)
        southPorts.push_back(port);
      // WEST
      else if (ratio < 1 && deltaX < 0)
        westPorts.push_back(port);
    } else
      freePorts.push_back(port);
  }

  if (draw == true) {
    // sort the four vectors by X or Y coord, in reversed or normal order
    if (!northPorts.empty())
      mySortPorts(northPorts, true, false, layout);
    if (!westPorts.empty())
      mySortPorts(westPorts, false, true, layout);
    if (!southPorts.empty())
      mySortPorts(southPorts, true, false, layout);
    if (!eastPorts.empty())
      mySortPorts(eastPorts, false, true, layout);
  }
  // add the free ports with attempting to equilibriate nr of ports per
  // dimension
  for (unsigned i = 0; i < freePorts.size(); ++i) {
    size_t sumX = northPorts.size() + southPorts.size();
    size_t sumY = westPorts.size() + eastPorts.size();

    size_t minX = min(northPorts.size(), southPorts.size());
    size_t minY = min(westPorts.size(), eastPorts.size());

    // put free ports into west or east
    if (minX < minY || (minX == minY && sumX < sumY)) {
      if (northPorts.size() > southPorts.size())
        southPorts.push_back(freePorts[i]);
      else
        northPorts.push_back(freePorts[i]);
    } else { // put free ports into south or north
      if (westPorts.size() > eastPorts.size())
        eastPorts.push_back(freePorts[i]);
      else
        westPorts.push_back(freePorts[i]);
    }
  }
  if (draw == false)
    return pair<unsigned, unsigned>(max(northPorts.size(), southPorts.size()),
                                    max(westPorts.size(), eastPorts.size()));

  // effectively draw ports
  if (specifyWidths == true)
    placePorts(layout, northPorts, westPorts, southPorts, eastPorts, xWidth, yWidth);
  else
    placePorts(layout, northPorts, westPorts, southPorts, eastPorts);
  return pair<unsigned, unsigned>(0, 0);
}

void PortNode::setPositions(const int xWidth, const int yWidth, LayoutProperty *layout) const {
  drawPosition_aux(true, layout, true, xWidth, yWidth);
}

void PortNode::setPositions(LayoutProperty *layout) const {
  drawPosition_aux(true, layout);
}

// helper sorting method
void PortNode::mySortPorts(vector<Port *> &sidePorts, const bool x, const bool reverse,
                           LayoutProperty *layout) const {
  bool b = false;

  Coord myCenter = layout->getNodeValue(centerNode);
  float delta = 0, delta_next = 0;

  while (!b && sidePorts.size() > 1) {
    b = true;
    for (unsigned int i = 0; i < sidePorts.size() - 1; ++i) {
      node c1 = sidePorts[i]->getOneConnectedPort()->getParentPortNode()->getCenter();
      node c2 = sidePorts[i + 1]->getOneConnectedPort()->getParentPortNode()->getCenter();

      if (c1 == c2) { // Connected to the same PortNode; puts the ports
                      // connected to main ports in c1 and c2
        c1 = sidePorts[i]->getOneConnectedPort()->getNode();
        c2 = sidePorts[i + 1]->getOneConnectedPort()->getNode();
      }

      Coord pos = layout->getNodeValue(c1);
      Coord pos_next = layout->getNodeValue(c2);

      if (x) // NORTH or SOUTH -> DeltaX/DeltaY
      {
        delta = (myCenter[0] - pos[0]) / (myCenter[1] - pos[1]);
        delta_next = (myCenter[0] - pos_next[0]) / (myCenter[1] - pos_next[1]);
      } else // WEST or EAST -> DeltaY/DeltaX
      {
        delta = (myCenter[1] - pos[1]) / (myCenter[0] - pos[0]);
        delta_next = (myCenter[1] - pos_next[1]) / (myCenter[0] - pos_next[0]);
      }

      if ((!reverse && delta > delta_next) || (reverse && delta < delta_next)) {
        Port *tmp = sidePorts[i];
        sidePorts[i] = sidePorts[i + 1];
        sidePorts[i + 1] = tmp;
        b = false;
      }
    }
  }
}

void PortNode::resizeCenter(SizeProperty *nodeSize, LayoutProperty *layout) const {
  // trouve les coordonnées de ports les plus éloignés du centre en abcisse et
  // ordonnée
  Coord myPos = layout->getNodeValue(centerNode);
  float max_diff_y = 0, max_diff_x = 0;
  Size lg_x;
  for (Port *p : getPorts()) {
    Coord pos = layout->getNodeValue(p->getNode());
    if (fabs(pos[1] - myPos[1]) > max_diff_y) {
      max_diff_y = fabs(pos[1] - myPos[1]);
    }
    if (fabs(pos[0] - myPos[0]) > max_diff_x) {
      max_diff_x = fabs(pos[0] - myPos[0]);
      lg_x = nodeSize->getNodeValue(p->getNode());
    }
  }
  Size centersize = nodeSize->getNodeValue(centerNode);

  // le +1 c'est pour la taille du port
  centersize.setH(2.0f * max_diff_y + 1);
  centersize.setW(2.0f * max_diff_x + lg_x.getW());
  if (centersize.getW() == 0)
    centersize.setW(1);
  centersize.setD(1); // obligatoire en 2D

  nodeSize->setNodeValue(centerNode, centersize);
}

void PortNode::resizeCenter(const float xWidth, const float yWidth,
                            SizeProperty *nodeSize) const { // Bound to size max of placePorts
  nodeSize->setNodeValue(centerNode, Size(max(xWidth, .5f) * PorgyConstants::SPACE + 2.0f,
                                          max(yWidth, .5f) * PorgyConstants::SPACE + 2.0f, 1));
  // 2 <=> 1 for decalage (offset) + 1 for PortSize
}

////////////////////////////
void PortNode::placePorts(LayoutProperty *layout, vector<Port *> &northPorts,
                          vector<Port *> &westPorts, vector<Port *> &southPorts,
                          vector<Port *> &eastPorts) const {
  size_t nbX = max(northPorts.size(), southPorts.size());
  size_t nbY = max(westPorts.size(), eastPorts.size());
  placePorts(layout, northPorts, westPorts, southPorts, eastPorts, nbX, nbY);
}

void PortNode::placePorts(LayoutProperty *layout, std::vector<Port *> &northPorts,
                          std::vector<Port *> &westPorts, std::vector<Port *> &southPorts,
                          std::vector<Port *> &eastPorts, const float Xwidth,
                          const float Ywidth) const {
  float width = max(Xwidth, .5f);
  float height = max(Ywidth, .5f);
  // Think to modify recizeCenter in the same time
  assert(width >= 0 && height >= 0);
  width *= PorgyConstants::SPACE;
  height *= PorgyConstants::SPACE;

  // distance between first(last) port and corner
  float decalage = 0.5; // +PortNode::space -SizePort
  // CALCULATE DESIRED WIDTHS

  // coords of central node
  Coord center = layout->getNodeValue(centerNode);
  // PLACE PORTS
  float ratio = 0;
  node n;
  Coord newC;
  newC.setZ(0);

  // NORTH
  if (northPorts.size() != 0) {
    ratio = width / northPorts.size();
    newC.setY(center.getY() + height / 2.0f + decalage);
    float X = center.getX() - width / 2.0f + ratio / 2.0f;
    for (unsigned i = 0; i < northPorts.size(); ++i) {
      n = northPorts[i]->getNode();
      newC.setX(X + i * ratio);
      layout->setNodeValue(n, newC);
    }
  }
  // EAST
  if (eastPorts.size() != 0) {
    ratio = height / eastPorts.size();
    newC.setX(center.getX() + width / 2.0f + decalage);
    float Y = center.getY() + height / 2.0f - ratio / 2.0f;
    for (unsigned i = 0; i < eastPorts.size(); ++i) {
      n = eastPorts[i]->getNode();
      newC.setY(Y - i * ratio);
      layout->setNodeValue(n, newC);
    }
  }
  // SOUTH
  if (southPorts.size() != 0) {
    ratio = width / southPorts.size();
    float X = center.getX() + width / 2.0f - ratio / 2.0f;
    newC.setY(center.getY() - height / 2.0f - decalage);
    for (unsigned i = 0; i < southPorts.size(); ++i) {
      n = southPorts[i]->getNode();
      newC.setX(X - i * ratio);
      layout->setNodeValue(n, newC);
    }
  }
  // WEST
  if (westPorts.size() != 0) {
    ratio = height / westPorts.size();
    newC.setX(center.getX() - width / 2.0f - decalage);
    float Y = center.getY() - height / 2.0f + ratio / 2.0f;
    for (unsigned i = 0; i < westPorts.size(); ++i) {
      n = westPorts[i]->getNode();
      newC.setY(Y + i * ratio);
      layout->setNodeValue(n, newC);
    }
  }
}
