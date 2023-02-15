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
#include "InteractorPortGraphDelElementComponent.h"

#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>
#include <tulip/MouseInteractors.h>
#include <tulip/StableIterator.h>
#include <tulip/View.h>

#include <portgraph/Bridge.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortBase.h>
#include <portgraph/PortGraph.h>
#include <portgraph/PortGraphRule.h>

#include <QMouseEvent>

using namespace tlp;
using namespace std;

PortGraphDelElementComponent::PortGraphDelElementComponent()
    : MouseElementDeleter(), started(false) {}

void PortGraphDelElementComponent::viewChanged(View *view) {
  MouseElementDeleter::viewChanged(view);
  if (view == nullptr)
    return;
  if (PorgyTlpGraphStructure::isRuleGraph(view->graph())) {
    view->graph()->addListener(this);
    view->graph()->addObserver(this);
  }
}

// if one deletes an edge connected to the bridge, we have to delete the bridge
// port also
void PortGraphDelElementComponent::delEdge(Graph *g, const edge e) {
  const pair<node, node> &ends = g->ends(e);
  if (!started)
    return;
  if (PorgyTlpGraphStructure::isRuleGraph(g)) {
    PortGraphRuleDecorator dec(g);
    if (Bridge::isBridge(ends.second, g)) {
      nodetodelete.push_back(ends.second);
    } else if (Bridge::isBridge(ends.first, g)) {
      nodetodelete.push_back(ends.first);
    }
    // lhs or rhs
    else if (((dec.getSide(ends.first) == PorgyConstants::SIDE_LEFT) &&
              (dec.getSide(ends.second) == PorgyConstants::SIDE_LEFT)) ||
             ((dec.getSide(ends.first) == PorgyConstants::SIDE_RIGHT) &&
              (dec.getSide(ends.second) == PorgyConstants::SIDE_RIGHT))) {
      IntegerProperty *arity = g->getProperty<IntegerProperty>(PorgyConstants::ARITY);
      arity->setNodeValue(ends.first, arity->getNodeValue(ends.first) - 1);
      arity->setNodeValue(ends.second, arity->getNodeValue(ends.second) - 1);
    }
  }
}

void PortGraphDelElementComponent::delElement(Graph *graph, SelectedEntity &selectedEntity) {
  switch (selectedEntity.getEntityType()) {
  case SelectedEntity::NODE_SELECTED: {
    node n(selectedEntity.getComplexEntityId());
    if (PortNodeBase::isCenter(n, graph) && !Bridge::isBridge(n, graph)) {
      started = true;
      delPortNode(n, graph);
    } else if (PortBase::isPort(n, graph) && !Bridge::isBridge(n, graph)) {
      for(auto center:graph->getInNodes(n)) {
        if (PortNodeBase::isCenter(center, graph)) {
          started = true;
          delPortNode(center, graph);
        }
      }
    } else if (PortBase::isPort(n, graph) && Bridge::isBridge(n, graph)) {
      started = true;
      graph->delNode(n, true);
    }
    break;
  }
  case SelectedEntity::EDGE_SELECTED: {
    edge ed(selectedEntity.getComplexEntityId());
    if ((PortGraph::isPortGraphEdge(ed, graph)) || (Bridge::isBridge(ed, graph)))
      started = true;
    graph->delEdge(ed, true);
    break;
  }
  default:
    break;
  }
}

bool PortGraphDelElementComponent::eventFilter(QObject *widget, QEvent *e) {
  GlMainWidget *glMainWidget = static_cast<GlMainWidget *>(widget);
  Graph *_graph = view()->graph();
  if (e->type() == QEvent::MouseMove) {
    SelectedEntity selectedEntity;
    QMouseEvent *qMouseEv = static_cast<QMouseEvent *>(e);
    glMainWidget->pickNodesEdges(qMouseEv->pos().x(), qMouseEv->pos().y(), selectedEntity, nullptr, true,
                                 false);
    node n(selectedEntity.getNode());
    if (n.isValid() && (Bridge::isBridge(n, _graph) && (PortNodeBase::isCenter(n, _graph)))) {
      glMainWidget->setCursor(Qt::ArrowCursor);
      return true;
    }
  }

  return MouseElementDeleter::eventFilter(widget, e);
}

void PortGraphDelElementComponent::treatEvent(const tlp::Event &message) {
  const tlp::GraphEvent *gEvt = dynamic_cast<const tlp::GraphEvent *>(&message);
  if (gEvt) {
    if (gEvt->getType() == tlp::GraphEvent::TLP_DEL_EDGE) {
      delEdge(gEvt->getGraph(), gEvt->getEdge());
    }
  }
}

void PortGraphDelElementComponent::treatEvents(const std::vector<tlp::Event> &) {
  started = false;
  if (!nodetodelete.empty()) {
    Graph *g = view()->graph();
    for (auto n :  nodetodelete) {
      if ((g->isElement(n)) && (PortBase::isPort(n, g))) {
        // nodes and edges have to be deleted inside the whole hierarchy
        view()->graph()->delNode(n, true);
      }
    }
    nodetodelete.clear();
    view()->draw();
  }
}

void PortGraphDelElementComponent::delPortNode(node center, Graph *_graph) {
  _graph->push(); // allow to undo
  Observable::holdObservers();
  Iterator<node> *it = new StableIterator<node>(_graph->getOutNodes(center));
  _graph->delNodes(it, true);
  delete it;
  _graph->delNode(center, true);
  Observable::unholdObservers();
}
