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
#include <portgraph/Bridge.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>

#include <porgy/PortNodeQt.h>
#include <porgy/PortQt.h>

#include <tulip/BooleanProperty.h>
#include <tulip/ColorProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/TlpQtTools.h>

using namespace tlp;
using namespace std;

PortNodeQt::PortNodeQt(const QString &n, const QColor &c, const vector<PortQt> &ports)
    : name(n), color(c), ports(ports) {}

PortNodeQt::PortNodeQt(const PortNode *pn)
    : name(tlp::tlpStringToQString(pn->getName())), color(tlp::colorToQColor(pn->getColor())) {
  ports.reserve(pn->nbPorts());
  for (Port *p : pn->getPorts()) {
    PortQt port(tlp::tlpStringToQString(p->getName()), p->getShape(),
                tlp::colorToQColor(p->getColor()));
    if (PorgyTlpGraphStructure::isRuleGraph(pn->getParentPortGraph().getGraph()))
      port.setArity(p->getArity());
    ports.push_back(port);
  }
}

PortNodeQt::PortNodeQt(const PortNodeQt &portNode)
    : name(portNode.name), color(portNode.color), ports(portNode.ports) {}

PortNodeQt::~PortNodeQt() {}

node PortNodeQt::addToTlpGraph(Graph *g, const bool isRule, const Coord &posCenter,
                               const PorgyConstants::RuleSide side) const {
  IntegerProperty *shape = g->getProperty<IntegerProperty>("viewShape");
  StringProperty *label = g->getRoot()->getProperty<StringProperty>("viewLabel");
  IntegerProperty *type = PortNodeBase::getTypeProperty(g);
  ColorProperty *colorprop = g->getProperty<ColorProperty>("viewColor");
  // ColorProperty *bordercolor = g->getProperty<ColorProperty>
  // ("viewBorderColor");
  SizeProperty *nodeSize = g->getProperty<SizeProperty>("viewSize");
  LayoutProperty *layout = g->getProperty<LayoutProperty>("viewLayout");
  IntegerProperty *sideprop = nullptr;
  IntegerProperty *arity = nullptr;
  if (isRule) {
    PortGraphRuleDecorator dec(g);
    sideprop = dec.getSideProperty();
    arity = g->getProperty<IntegerProperty>(PorgyConstants::ARITY);
    g->getProperty<BooleanProperty>(PorgyConstants::M);
    g->getProperty<BooleanProperty>(PorgyConstants::N);
  }

  node n1 = g->addNode();
  label->setNodeValue(n1, tlp::QStringToTlpString(name));
  type->setNodeValue(n1, PorgyConstants::CENTER);
  shape->setNodeValue(n1, PorgyConstants::CENTER_SHAPE);
  Color c = tlp::QColorToColor(color);
  colorprop->setNodeValue(n1, c);

  if (isRule)
    sideprop->setNodeValue(n1, side);
  g->getRoot()->reserveNodes(ports.size());
  g->getRoot()->reserveEdges(ports.size());
  foreach (const PortQt &p, ports) {
    node n2 = g->addNode();
    shape->setNodeValue(n2, p.getShape());
    label->setNodeValue(n2, tlp::QStringToTlpString(p.getName()));
    colorprop->setNodeValue(n2, tlp::QColorToColor(p.getColor()));
    type->setNodeValue(n2, PorgyConstants::PORT);
    edge e = g->addEdge(n1, n2);
    colorprop->setEdgeValue(e, c);
    if (isRule) {
      sideprop->setNodeValue(n2, side);
      sideprop->setEdgeValue(e, side);
      arity->setNodeValue(n2, p.getArity());
    }
  }
  PortGraph *pg;
  if (isRule)
    pg = new PortGraphRule(g);
  else
    pg = new PortGraphModel(g);

  layout->setNodeValue(n1, posCenter);

  (*pg)[n1]->setShape();
  // p.draw(nodeSize, colorprop, layout, bordercolor );
  pg->drawSameSizes(nodeSize, layout);
  delete pg;
  return n1;
}

node PortNodeQt::addToTlpGraphUsingPosition(tlp::Graph *g, const tlp::Coord &posCenter) const {

  PorgyConstants::RuleSide side = PorgyConstants::NO_SIDE;
  bool isRule = PorgyTlpGraphStructure::isRuleGraph(g);
  if (isRule) {
    PortGraphRule rule(g);
    assert(rule.getBridge() != nullptr);
    node center = rule.getBridge()->getCenter();
    LayoutProperty *layout = g->getProperty<LayoutProperty>("viewLayout");
    // Get the position of the bridge.
    Coord bridgeCenter = layout->getNodeValue(center);
    // The center is at the left of the bridge
    side = posCenter[0] < bridgeCenter[0] ? PorgyConstants::SIDE_LEFT : PorgyConstants::SIDE_RIGHT;
  }
  return addToTlpGraph(g, isRule, posCenter, side);
}

void PortNodeQt::replacePorts(const vector<PortQt> &ql) {
  ports = ql;
}

bool PortNodeQt::operator==(const PortNodeQt *portNode) const {
  // Diferent name or different port number
  if (portNode->name != name || portNode->ports.size() != ports.size()) {
    return false;
  }

  vector<PortQt> portsList = portNode->ports;

  // Compare port two by two
  for (const PortQt &portQt : ports) {
    bool found = false;
    for (auto it = portsList.begin(); it != portsList.end(); ++it) {
      // The port was found in the list
      if (portQt == (*it)) {
        found = true;
        // remove the port from the list no need to check it again.
        portsList.erase(it);
        break;
      }
    }
    // The port was not found in the list the port list are different.
    if (!found) {
      return false;
    }
  }
  return true;
}
