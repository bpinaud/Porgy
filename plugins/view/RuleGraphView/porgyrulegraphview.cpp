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
#include "porgyrulegraphview.h"
#include "../../StandardInteractorPriority.h"
#include "RuleAlgorithmWidget.h"
#include "RuleAttributesWidget.h"
#include "porgyRuleViewQuickAccessBar.h"

#include <tulip/Algorithm.h>
#include <tulip/Camera.h>
#include <tulip/DrawingTools.h>
#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>
#include <tulip/Graph.h>
#include <tulip/Interactor.h>
#include <tulip/Perspective.h>
#include <tulip/TlpQtTools.h>
#include <tulip/TulipMimes.h>

#include <portgraph/Bridge.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>

#include <porgy/porgymimedata.h>

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMessageBox>
#include <QToolTip>

using namespace tlp;
using namespace std;

PorgyRuleGraphView::PorgyRuleGraphView(const tlp::PluginContext *)
    : AbstractPortGraphView(), _RuleAttributes(nullptr), _RuleAlgorithm(nullptr), _bar(nullptr) {
  addDependency(PorgyConstants::REDRAW_RULE, "1.0");
}

PorgyRuleGraphView::~PorgyRuleGraphView() {
  delete _RuleAttributes;
  delete _RuleAlgorithm;
}

QList<QWidget *> PorgyRuleGraphView::configurationWidgets() const {
  return AbstractPortGraphView::configurationWidgets() << _RuleAttributes << _RuleAlgorithm;
}

void PorgyRuleGraphView::graphChanged(tlp::Graph *g) {
  AbstractPortGraphView::graphChanged(g);
  _RuleAttributes->setRuleAttributes(g);
  _RuleAlgorithm->setRuleAlgorithm(g);
  bool state = false;
  if (g->existAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED))
    g->getAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED, state);
  else
    g->setAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED, state);
  if (state) {
    getGlMainWidget()
        ->getScene()
        ->getGlGraphComposite()
        ->getRenderingParametersPointer()
        ->setViewArrow(state);
  }
}

bool PorgyRuleGraphView::checkInteractors() {
  bool ret = false;
  foreach (Interactor *i, interactors()) {
    i->action()->setEnabled(true); // restore interactor
    //        if(PorgyTlpGraphStructure::isRuleUsed(graph())&&(i->group()==PorgyConstants::CATEGORY_NAME))
    //        {
    //                i->action()->setEnabled(false);
    //                ret = true;
    //        }
  }
  return ret;
}

void PorgyRuleGraphView::setupWidget() {
  AbstractPortGraphView::setupWidget();
  if (_RuleAttributes == nullptr)
    _RuleAttributes = new RuleAttributesWidget();
  if (_RuleAlgorithm == nullptr)
    _RuleAlgorithm = new RuleAlgorithmWidget();
  graphicsView()->setAcceptDrops(true);
  getGlMainWidget()->setAcceptDrops(false);
  graphicsView()->scene()->installEventFilter(this);
  graphicsView()->installEventFilter(this); // Handle tooltip events

  connect(_RuleAttributes, SIGNAL(SignalViewArrows(bool)), this, SLOT(ViewArrows(bool)));
}

//void PorgyRuleGraphView::resetGraphicsScene() {
//  graphicsView()->scene()->installEventFilter(this);
//}

PortGraph *PorgyRuleGraphView::buildPortGraph(tlp::Graph *graph) const {
  if (!graph->existAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED))
    graph->setAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED, false);
  return new PortGraphRule(graph);
}

PortGraphDecorator *PorgyRuleGraphView::buildPortGraphDecorator(tlp::Graph *graph) const {
  return new PortGraphRuleDecorator(graph);
}

void PorgyRuleGraphView::initGlParameters(tlp::GlGraphRenderingParameters &parameters) const {
  // Default viewArrow should be depending of the value of
  // EDGE_ORIENTATION_ENABLED
  parameters.setViewArrow(true);
  parameters.setEdgeColorInterpolate(false);
  parameters.setEdgeSizeInterpolate(true);
  parameters.setLabelsDensity(0);
}

void PorgyRuleGraphView::redrawRuleGraph() {
  Graph *gr = (_bar != nullptr) ? _bar->getGraph() : nullptr;
  redrawGraph(gr);
}

void PorgyRuleGraphView::fillContextMenu(QMenu *contextMenu, const QPointF &cursorPosition) {
  AbstractPortGraphView::fillContextMenu(contextMenu, cursorPosition);
  contextMenu->addSeparator();
  contextMenu->addAction(tr("&Compute a new layout"), this, SLOT(redrawRuleGraph()));
  QMenu *menu = new QMenu(tr("Selection"), contextMenu);
  if (((_bar != nullptr) && (_bar->LHSVisible())) || _bar == nullptr)
    menu->addAction("Select the left hand side", this, SLOT(selectLHS()));
  if (((_bar != nullptr) && (_bar->RHSVisible())) || _bar == nullptr)
    menu->addAction("Select the right hand side", this, SLOT(selectRHS()));
  if (((_bar != nullptr) && _bar->LHSVisible() && _bar->RHSVisible()) || _bar == nullptr) {
    menu->addSeparator();
    menu->addAction("Copy/Paste selection to the left hand side", this,
                    SLOT(pasteSelectionToLHS()));
    menu->addAction("Copy/Paste selection to the right hand side", this,
                    SLOT(pasteSelectionToRHS()));
  }
  if (!menu->isEmpty())
    contextMenu->addMenu(menu);

  bool result;
  // look if the mouse pointer is over a node or edge
  result =
      getGlMainWidget()->pickNodesEdges(cursorPosition.x(), cursorPosition.y(), selectedEntity);
  node n(selectedEntity.getNode());
  if (!result || ((n.isValid()) && (Bridge::isBridge(n, graph()))))
    return;

  contextMenu->addSeparator();
  // Display a context menu
  QString str = n.isValid() ? "Node " : "Edge ";
  contextMenu->addAction(str)->setEnabled(false);
  contextMenu->addSeparator();
  contextMenu->addAction(tr("Add to/Remove from selection"), this, SLOT(invertSelection()));
}

QuickAccessBar *PorgyRuleGraphView::getQuickAccessBarImpl() {
  _bar = new PorgyQuickAccessBar(graph());
  connect(_bar, SIGNAL(new_graph_to_display(tlp::BooleanProperty*)), this, SLOT(filterGraphLayout(tlp::BooleanProperty*)));
  return _bar;
}

void PorgyRuleGraphView::filterGraphLayout(tlp::BooleanProperty* b) {
    getGlMainWidget()
         ->getScene()
         ->getGlGraphComposite()
         ->getRenderingParametersPointer()
         ->setDisplayFilteringProperty(b);
    emit(drawNeeded());
}

void PorgyRuleGraphView::invertSelection() {
  AbstractPortGraphView::invertSelection(selectedEntity);
}

void PorgyRuleGraphView::pasteSelectionToLHS() {
  emit copy_paste_other_side(PorgyConstants::SIDE_LEFT);
}
void PorgyRuleGraphView::pasteSelectionToRHS() {
  emit copy_paste_other_side(PorgyConstants::SIDE_RIGHT);
}

void PorgyRuleGraphView::selectRHS() {
  selectSide(PorgyConstants::SIDE_RIGHT);
}
void PorgyRuleGraphView::selectLHS() {
  selectSide(PorgyConstants::SIDE_LEFT);
}

void PorgyRuleGraphView::selectSide(PorgyConstants::RuleSide side) {
  PortGraphRuleDecorator dec(graph());
  PortGraphRule pg(graph());
  BooleanProperty *selP = dec.getSelectionProperty();
  for (PortNode *pn : pg.getMember(side)) {
    pn->select(true, selP);
  }
  for (edge e : pg.getMemberEdges(side)) {
    selP->setEdgeValue(e, true);
  }
}

DataSet PorgyRuleGraphView::state() const {
  // hack: when closing the view, reset the quickaccessbar to show the rule
  // entirely if it is not already the case
  if (_bar&&!_bar->allVisible()) {
    QMessageBox::information(Perspective::instance()->mainWindow(),
                             "Cannot save file with a filtered view",
                             "Project cannot be saved with a filtered view. "
                             "Resetting QuickAccessBar");
    _bar->reset();
  }
  return AbstractPortGraphView::state();
}

bool PorgyRuleGraphView::eventFilter(QObject *object, QEvent *event) {

  if (event->type() == QEvent::GraphicsSceneDragEnter) {
    return treatGraphicsDragMoveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
  } else if (event->type() == QEvent::GraphicsSceneDragMove) {
    return treatGraphicsDragMoveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
  } else if (event->type() == QEvent::GraphicsSceneDrop) {
    QGraphicsSceneDragDropEvent *dropEvent = static_cast<QGraphicsSceneDragDropEvent *>(event);
    const QMimeData *mimeData = dropEvent->mimeData();
    if (mimeData->hasFormat(GRAPH_MIME_TYPE)) {
      const GraphMimeType *gMimeData = static_cast<const GraphMimeType *>(mimeData);
      Graph *dragedGraph = gMimeData->graph();
      if (dragedGraph != graph() && PorgyTlpGraphStructure::isRuleGraph(dragedGraph)) {
        if (_bar) {
          _bar->reset();
          _bar->setGraph(dragedGraph);
        }
        setGraph(dragedGraph); // Update the current view graph
        centerView();
      }
      event->accept();
      return true;
    } else if (mimeData->hasFormat(PorgyMimeData::portNodeQtPtrListMimeTypes)) {
      const PorgyMimeData *pMimeData = static_cast<const PorgyMimeData *>(mimeData);
      GlMainWidget *glWidget = getGlMainWidget();
      Coord posCenter(glWidget->width() - dropEvent->pos().x(), dropEvent->pos().y(), 0);
      posCenter = glWidget->getScene()->getGraphCamera().viewportTo3DWorld(
          glWidget->screenToViewport(posCenter));
      emit pastePortNodes(posCenter, graph(), pMimeData->getPortNodeQtPtrList());
      dropEvent->accept();
      return true;
    }
  } else if (event->type() == QEvent::ToolTip) {
    SelectedEntity selectedEntity;
    QHelpEvent *he = static_cast<QHelpEvent *>(event);
    Graph *g = getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getGraph();
    if (getGlMainWidget()->pickNodesEdges(he->x(), he->y(), selectedEntity, nullptr, true, false)) {
      std::string label;
      node n(selectedEntity.getNode());
      if (n.isValid()) {
        if (PortGraphDecorator::isCenter(g, n) && !Bridge::isBridge(n, g)) {
          PortGraphRule pg(g);
          const PortNode *p = pg[n];
          label = "<b>Name: </b>" + p->getName();
        }
        QToolTip::showText(he->globalPos(), tlpStringToQString(label), getGlMainWidget());
      }
    }
  }
  return GlMainView::eventFilter(object, event);
}

bool PorgyRuleGraphView::treatDragMoveEvent(QDragMoveEvent *evt) {
  const QMimeData *mimeData = evt->mimeData();
  if (mimeData->hasFormat(GRAPH_MIME_TYPE)) {
    const GraphMimeType *gMimeData = static_cast<const GraphMimeType *>(mimeData);
    Graph *dragedGraph = gMimeData->graph();
    if (dragedGraph != graph() && PorgyTlpGraphStructure::isRuleGraph(dragedGraph)) {
      evt->setDropAction(Qt::MoveAction);
      evt->accept();
      return true;
    } else {
      // Block event
      evt->setDropAction(Qt::IgnoreAction);
      evt->accept();
      return true;
    }
  } else if (mimeData->hasFormat(PorgyMimeData::portNodeQtPtrListMimeTypes)) {
    evt->setDropAction(Qt::MoveAction);
    evt->accept();
    return true;
  }
  return false;
}

bool PorgyRuleGraphView::treatGraphicsDragMoveEvent(QGraphicsSceneDragDropEvent *evt) {
  const QMimeData *mimeData = evt->mimeData();
  if (mimeData->hasFormat(GRAPH_MIME_TYPE)) {
    const GraphMimeType *gMimeData = static_cast<const GraphMimeType *>(mimeData);
    Graph *dragedGraph = gMimeData->graph();
    if (dragedGraph != graph() && PorgyTlpGraphStructure::isRuleGraph(dragedGraph)) {
      evt->setDropAction(Qt::MoveAction);
      evt->accept();
      return true;
    } else {
      // Block event
      evt->setDropAction(Qt::IgnoreAction);
      evt->accept();
      return true;
    }
  } else if (mimeData->hasFormat(PorgyMimeData::portNodeQtPtrListMimeTypes)) {
    evt->setDropAction(Qt::MoveAction);
    evt->accept();
    return true;
  }
  return false;
}

void PorgyRuleGraphView::ViewArrows(bool state) {
  getGlMainWidget()
      ->getScene()
      ->getGlGraphComposite()
      ->getRenderingParametersPointer()
      ->setViewArrow(state);
  emit drawNeeded();
}

PLUGIN(PorgyRuleGraphView)

INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(RuleViewNavigation, "RuleViewNavigation",
                                          "InteractorNavigation", PorgyConstants::RULE_VIEW_NAME,
                                          "Tulip Team", "21/11/2008", "Navigate", "1.0",
                                          StandardInteractorPriority::Navigation)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(RuleViewMagnifyingGlass, "RuleViewMagnifyingGlass",
                                          "MouseMagnifyingGlassInteractor",
                                          PorgyConstants::RULE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Magnyfiying glass", "1.0",
                                          StandardInteractorPriority::MagnifyingGlass)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(RuleViewFishEye, "RuleViewFishEye", "FishEyeInteractor",
                                          PorgyConstants::RULE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Fisheye", "1.0",
                                          StandardInteractorPriority::FishEye)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(RuleViewRectangleZoom, "RuleViewRectangleZoom",
                                          "InteractorRectangleZoom", PorgyConstants::RULE_VIEW_NAME,
                                          "Tulip Team", "21/11/2008", "Rectangle zoom", "1.0",
                                          StandardInteractorPriority::ZoomOnRectangle)
