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
#include <porgy/ModelElementValueModel.h>
#include <porgy/RuleElementValueModel.h>
#include <porgy/TraceElementValueModel.h>
#include <porgy/elementinformationinteractorcomponent.h>
#include <porgy/porgyitemdelegate.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphRule.h>

#include <tulip/GlMainView.h>
#include <tulip/GlMainWidget.h>
#include <tulip/IntegerProperty.h>
#include <tulip/View.h>

#include <QMouseEvent>
#include <QTableView>

using namespace tlp;

ElementInformationRuleInteractorComponent::ElementInformationRuleInteractorComponent()
    : MouseShowElementInfo(false) {
  delete tableView()->itemDelegate();
  tableView()->setItemDelegate(new PorgyItemDelegate(tableView()));
}

ElementInformationInteractorComponent::ElementInformationInteractorComponent()
    : MouseShowElementInfo(false) {}

QAbstractItemModel *ElementInformationInteractorComponent::buildModel(ElementType elementType,
                                                                      unsigned int elementId,
                                                                      QObject *parent) const {
  Graph *graph = view()->graph();
  GraphElementValueModel *model = nullptr;
  if (PorgyTlpGraphStructure::isTraceGraph(graph)) {
    model = new TraceElementValueModel(graph, elementType, elementId, parent);
  } else if (PorgyTlpGraphStructure::isModelGraph(graph)) {
    model = new ModelElementValueModel(graph, elementType, elementId, parent);
  }
  if (model != nullptr) {
    model->sort(0);
  }
  return model;
}

QAbstractItemModel *ElementInformationRuleInteractorComponent::buildModel(ElementType elementType,
                                                                          unsigned int elementId,
                                                                          QObject *parent) const {
  Graph *graph = view()->graph();
  GraphElementValueModel *model = nullptr;
  assert(PorgyTlpGraphStructure::isRuleGraph(graph));
  model = new RuleElementValueModel(graph, elementType, elementId, parent);

  if (model != nullptr) {
    model->sort(0);
  }
  return model;
}

bool ElementInformationInteractorComponent::eventFilter(QObject *widget, QEvent *e) {
  if (widget == _informationWidget &&
      (e->type() == QEvent::Wheel || e->type() == QEvent::MouseButtonPress))
    return true;

  return MouseShowElementInfo::eventFilter(widget, e);
}

bool ElementInformationRuleInteractorComponent::eventFilter(QObject *widget, QEvent *e) {
  if (widget == _informationWidget &&
      (e->type() == QEvent::Wheel || e->type() == QEvent::MouseButtonPress))
    return true;

  QMouseEvent *qMouseEv = dynamic_cast<QMouseEvent *>(e);
  if (((e->type() == QEvent::MouseButtonPress) && (qMouseEv->button() == Qt::LeftButton)) ||
      (e->type() == QEvent::MouseMove)) {
    GlMainWidget *glMainWidget = static_cast<GlMainView *>(view())->getGlMainWidget();
    Graph *g = view()->graph();
    PortGraphRuleDecorator dec(g);
    SelectedEntity pickedEntity;
    glMainWidget->pickNodesEdges(qMouseEv->x(), qMouseEv->y(), pickedEntity, nullptr, true, true);
    IntegerProperty *rule_side = dec.getSideProperty();
    edge ed = pickedEntity.getEdge();
    if (ed.isValid()) {
      if (rule_side->getEdgeValue(ed) == PorgyConstants::SIDE_BRIDGE) {
        e->ignore();
        return true;
      }
    }
    // bypass the limit and enable the modification of Port NodeType
    node n = pickedEntity.getNode();
    if (n.isValid()) {
      if (rule_side->getNodeValue(n) == PorgyConstants::SIDE_BRIDGE) {
        e->ignore();
        return true;
      }
    }
  }
  return MouseShowElementInfo::eventFilter(widget, e);
}
