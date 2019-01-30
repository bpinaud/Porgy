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
#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>

#include <portgraph/Bridge.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>
#include <portgraph/PortNodeBase.h>

#include <porgy/SelectPortNodeInteractorComponent.h>

#include <QMouseEvent>

using namespace tlp;
using namespace std;

SelectPortNodeInteractorComponent::SelectPortNodeInteractorComponent() {}

SelectPortNodeInteractorComponent::~SelectPortNodeInteractorComponent() {}

bool SelectPortNodeInteractorComponent::eventFilter(QObject *widget, QEvent *e) {
  Observable::holdObservers();
  bool result = MouseSelector::eventFilter(widget, e);
  if (e->type() == QEvent::MouseButtonRelease) {
    QMouseEvent *qMouseEv = static_cast<QMouseEvent *>(e);
    GlMainWidget *glMainWidget = static_cast<GlMainWidget *>(widget);
    if ((qMouseEv->button() == Qt::LeftButton) && ((qMouseEv->modifiers() == Qt::NoModifier) ||
                                                   qMouseEv->modifiers() ==
#if defined(__APPLE__)
                                                       Qt::AltModifier
#else
                                                       Qt::ControlModifier
#endif
                                                   )) {
      selectOp(glMainWidget, true);
    } else { // deselect
      selectOp(glMainWidget, false);
    }
  }
  Observable::unholdObservers();
  return result;
}

void SelectPortNodeInteractorComponent::selectOp(GlMainWidget *glMainWidget,
                                                 const bool state) const {
  Graph *g = glMainWidget->getScene()->getGlGraphComposite()->getGraph();
  int xPos = x;
  int yPos = y;
  int wPos = w;
  int hPos = h;
  if (wPos == 0) {
    xPos = x - 1;
    wPos = 3;
  }
  if (hPos == 0) {
    yPos = y - 1;
    hPos = 3;
  }

  std::vector<SelectedEntity> nodes;
  std::vector<SelectedEntity> edges;
  glMainWidget->pickNodesEdges(xPos, yPos, wPos, hPos, nodes, edges);
  Observable::holdObservers();
  BooleanProperty *selection = nullptr;
  PortGraph *pg = nullptr;
  if (PorgyTlpGraphStructure::isModelGraph(g)) {
    pg = new PortGraphModel(g);
    PortGraphModelDecorator dec(g);
    selection = dec.getSelectionProperty();
  } else {
    pg = new PortGraphRule(g);
    PortGraphRuleDecorator dec(g);
    selection = dec.getSelectionProperty();
  }
  for (const auto &it : edges) {
    edge e(it.getEdge());
    if (PorgyTlpGraphStructure::isRuleGraph(g) && Bridge::isBridge(e, g)) {
      selection->setEdgeValue(e, false);
    }
  }
  for (const auto &it : nodes) {
    node n(it.getNode());
    if (PorgyTlpGraphStructure::isRuleGraph(g) && Bridge::isBridge(n, g)) {
      selection->setNodeValue(n, false);
    } else {
      const PortNode *p((*pg)[n]);
      p->select(state, selection);
    }
  }

  Observable::unholdObservers();
  delete pg;
}
