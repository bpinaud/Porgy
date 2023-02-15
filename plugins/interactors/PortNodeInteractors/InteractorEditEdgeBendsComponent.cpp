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
#include "InteractorEditEdgeBendsComponent.h"

#include <QMouseEvent>

#include <portgraph/Bridge.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortBase.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>

#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>
#include <tulip/View.h>

using namespace tlp;
using namespace std;

EditEdgeBendsPorgyComponent::EditEdgeBendsPorgyComponent()
    : MouseEdgeBendEditor(), oldNode(node()) {}

bool EditEdgeBendsPorgyComponent::eventFilter(QObject *widget, QEvent *e) {
  QMouseEvent *qMouseEv = static_cast<QMouseEvent *>(e);
  GlMainWidget *glMainWidget = static_cast<GlMainWidget *>(widget);
  Graph *_graph = view()->graph();
  if ((e->type() == QEvent::MouseButtonPress) && (qMouseEv->button() == Qt::LeftButton)) {
    SelectedEntity pickedEntity;
    glMainWidget->pickNodesEdges(qMouseEv->pos().x(), qMouseEv->pos().y(), pickedEntity, nullptr, true, false);
    node n = pickedEntity.getNode();
    if (n.isValid()) {
      if ((PortBase::isPort(n, _graph)) && (!Bridge::isBridge(n, _graph))) {
        if (PorgyTlpGraphStructure::isRuleGraph(_graph)) {
          PortGraphRuleDecorator dec(_graph);
          if ((dec.getSide(n) == PorgyConstants::SIDE_RIGHT) ||
              (dec.getSide(n) == PorgyConstants::SIDE_LEFT)) {
            oldNode = n;
            return MouseEdgeBendEditor::eventFilter(widget, e);
          } else
            return false;
        } else {
          return MouseEdgeBendEditor::eventFilter(widget, e);
        }
      } else
        return true;
    }
  } else if (e->type() == QEvent::MouseButtonRelease && qMouseEv->button() == Qt::LeftButton &&
             operation() != NONE_OP) {
    SelectedEntity pickedEntity;
    glMainWidget->pickNodesEdges(qMouseEv->pos().x(), qMouseEv->pos().y(), pickedEntity, nullptr, true, false);
    node n = pickedEntity.getNode();
    if (n.isValid()) {
      if (!PortNode::isCenter(n, _graph) && (!Bridge::isBridge(n, _graph))) {
        if (PorgyTlpGraphStructure::isRuleGraph(_graph)) {
          PortGraphRuleDecorator pgr(_graph);
          if ((pgr.getSide(n) == pgr.getSide(getEdge())) ||
              (pgr.getSide(getEdge()) == PorgyConstants::SIDE_BRIDGE &&
               (pgr.getSide(n) == pgr.getSide(oldNode))))
            return MouseEdgeBendEditor::eventFilter(widget, e);
          else {
            stopEdition();
            glMainWidget->setCursor(QCursor(Qt::PointingHandCursor));
            glMainWidget->redraw();
            return true;
          }
        }
      } else {
        stopEdition();
        glMainWidget->setCursor(QCursor(Qt::PointingHandCursor));
        glMainWidget->redraw();
        return true;
      }
    }

  } else
    return MouseEdgeBendEditor::eventFilter(widget, e);

  return MouseEdgeBendEditor::eventFilter(widget, e);
}
