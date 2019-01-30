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
#include "InteractorPortGraphAddEdgeComponent.h"

#include <QMouseEvent>

#include <tulip/Color.h>
#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>
#include <tulip/Observable.h>
#include <tulip/StringProperty.h>
#include <tulip/TulipViewSettings.h>
#include <tulip/View.h>

#include <portgraph/Bridge.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortBase.h>
#include <portgraph/PortGraphRule.h>

using namespace tlp;
using namespace std;

PortGraphAddEdgeComponent::PortGraphAddEdgeComponent() {}

void PortGraphAddEdgeComponent::addLink(const node &source, const node &dest) {
  Graph *g = view()->graph();
  if (!PorgyTlpGraphStructure::isRuleGraph(g)) {
    MouseEdgeBuilder::addLink(source, dest);
  } else {
    // the graph is a rule
    assert(PorgyTlpGraphStructure::isRuleGraph(g));
    PortGraphRuleDecorator dec(g);
    IntegerProperty *side = dec.getSideProperty();
    IntegerProperty *arity = dec.getArityProperty();
    LayoutProperty *mLayout = g->getProperty<LayoutProperty>("viewLayout");
    Observable::holdObservers();
    if ((dec.getSide(source) == PorgyConstants::SIDE_LEFT) &&
        (dec.getSide(dest) == PorgyConstants::SIDE_RIGHT)) {
      setBends(vector<Coord>());
      PortGraphRule pgr(g);
      Bridge *b = pgr.getBridge();
      b->addPort(source, dest); // add one node and 3 edges to the graph
      b->draw(g->getProperty<SizeProperty>("viewSize"), g->getProperty<ColorProperty>("viewColor"),
              mLayout, g->getProperty<ColorProperty>("viewBorderColor"));
      b->setPositions(mLayout);
    } else if ((dec.getSide(source) == PorgyConstants::SIDE_LEFT) &&
               (dec.getSide(dest) == PorgyConstants::SIDE_LEFT)) {
      edge e = g->addEdge(source, dest);
      side->setEdgeValue(e, PorgyConstants::SIDE_LEFT);
      arity->setNodeValue(source, arity->getNodeValue(source) + 1);
      arity->setNodeValue(dest, arity->getNodeValue(dest) + 1);
      mLayout->setEdgeValue(e, bends());

    } else if ((dec.getSide(source) == PorgyConstants::SIDE_RIGHT) &&
               (dec.getSide(dest) == PorgyConstants::SIDE_RIGHT)) {
      edge e = g->addEdge(source, dest);
      side->setEdgeValue(e, PorgyConstants::SIDE_RIGHT);
      arity->setNodeValue(source, arity->getNodeValue(source) + 1);
      arity->setNodeValue(dest, arity->getNodeValue(dest) + 1);
      mLayout->setEdgeValue(e, bends());
    }
    setBends(vector<Coord>());
    Observable::unholdObservers();
  }
}

bool PortGraphAddEdgeComponent::eventFilter(QObject *widget, QEvent *e) {
  QMouseEvent *qMouseEv = static_cast<QMouseEvent *>(e);
  GlMainWidget *glMainWidget = static_cast<GlMainWidget *>(widget);
  Graph *g = view()->graph();
  SelectedEntity selectedEntity;
  PortGraphRuleDecorator dec(g);
  IntegerProperty *arity = dec.getArityProperty();
  if ((e->type() == QEvent::MouseButtonDblClick) && (qMouseEv->button() == Qt::LeftButton) &&
      (PorgyTlpGraphStructure::isRuleGraph(g))) {
    glMainWidget->pickNodesEdges(qMouseEv->x(), qMouseEv->y(), selectedEntity, nullptr, false,
                                 true);
    edge e(selectedEntity.getEdge());
    ColorProperty *col = g->getProperty<ColorProperty>("viewColor");
    DoubleProperty *width = g->getProperty<DoubleProperty>("viewBorderWidth");
    if (e.isValid()) {
      IntegerProperty *dstshape = g->getProperty<IntegerProperty>("viewTgtAnchorShape");
      const pair<node, node> &ends = g->ends(e);
      if ((dec.getType(e) == PorgyConstants::NO_TYPE) &&
          (dec.getSide(e) == PorgyConstants::SIDE_LEFT)) {
        dec.getTypeProperty()->setEdgeValue(e, PorgyConstants::ANTI);
        dstshape->setEdgeValue(e, EdgeExtremityShape::EdgeExtremityShapes::FontAwesomeIcon);
        StringProperty *icon = g->getProperty<StringProperty>("viewIcon");
        icon->setEdgeValue(e, "fa-times");
        arity->setNodeValue(ends.second, arity->getNodeValue(ends.second) - 1);
        arity->setNodeValue(ends.first, arity->getNodeValue(ends.first) - 1);
        col->setEdgeValue(e, tlp::Color::Gray);
        width->setEdgeValue(e, 1.0);
        return true;
      } else if (dec.getType(e) == PorgyConstants::ANTI) {
        dec.getTypeProperty()->setEdgeValue(e, PorgyConstants::NO_TYPE);
        dstshape->setEdgeValue(e, EdgeExtremityShape::EdgeExtremityShapes::None);
        arity->setNodeValue(ends.second, arity->getNodeValue(ends.second) + 1);
        arity->setNodeValue(ends.first, arity->getNodeValue(ends.first) + 1);
        col->setEdgeValue(e, col->getEdgeDefaultValue());
        width->setEdgeValue(e, 0.0);
        return true;
      }
    }
  } else if ((e->type() == QEvent::MouseButtonPress) && (qMouseEv->button() == Qt::LeftButton)) {

    glMainWidget->pickNodesEdges(qMouseEv->x(), qMouseEv->y(), selectedEntity, nullptr, true,
                                 false);
    node n(selectedEntity.getNode());
    if (n.isValid()) {
      if ((PortBase::isPort(n, g)) && (!Bridge::isBridge(n, g))) {
        if (PorgyTlpGraphStructure::isRuleGraph(g)) {

          if (!source().isValid())
            return MouseEdgeBuilder::eventFilter(widget, e);
          else if (!((dec.getSide(source()) == PorgyConstants::SIDE_RIGHT) &&
                     (dec.getSide(n) == PorgyConstants::SIDE_LEFT)))
            return MouseEdgeBuilder::eventFilter(widget, e);
          else
            return false;
        } else
          return MouseEdgeBuilder::eventFilter(widget, e);
      } else
        return false;
    }
  } else
    return MouseEdgeBuilder::eventFilter(widget, e);

  return MouseEdgeBuilder::eventFilter(widget, e);
}
