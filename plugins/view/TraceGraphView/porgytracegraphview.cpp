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
#include "porgytracegraphview.h"
#include "tracequickaccessbar.h"

#include <tulip/Algorithm.h>
#include <tulip/Camera.h>
#include <tulip/DrawingTools.h>
#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>
#include <tulip/GlMetaNodeRenderer.h>
#include <tulip/GlNode.h>
#include <tulip/GlRect.h>
#include <tulip/Interactor.h>
#include <tulip/TlpQtTools.h>
#include <tulip/TulipMetaTypes.h>
#include <tulip/TulipMimes.h>
#include <tulip/Perspective.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Trace.h>

#include <porgy/Strategy.h>
#include <porgy/pluginparametersconfigurationdialog.h>
#include <porgy/porgymimedata.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QGraphicsRectItem>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QPropertyAnimation>
#include <QTimer>
#include <QToolTip>
#include <QMainWindow>

#include "../../StandardInteractorPriority.h"

using namespace tlp;
using namespace std;

PorgyTraceGraphView::PorgyTraceGraphView(const PluginContext *) : _dragOverItem(nullptr) {
  addDependency("Sugiyama (OGDF)", "1.5");
  addDependency("Color Mapping", "2.2");
}

void PorgyTraceGraphView::setupWidget() {
  AbstractPorgyGraphView::setupWidget();

  graphicsView()->setAcceptDrops(true);
  graphicsView()->scene()->installEventFilter(this);
  graphicsView()->installEventFilter(this); // Treat tooltip events

  _dragOverItem = graphicsView()->scene()->addRect(0, 0, 0, 0);
  _dragOverItem->setVisible(false);
}

QuickAccessBar *PorgyTraceGraphView::getQuickAccessBarImpl() {
  return new TraceQuickAccessBar();
}

std::list<QWidget *> PorgyTraceGraphView::configurationWidgets() const {
    return {};
}

void PorgyTraceGraphView::initGlParameters(tlp::GlGraphRenderingParameters &parameters) const {
  parameters.setViewArrow(true);
  parameters.setEdgeColorInterpolate(false);
  parameters.setEdgeSizeInterpolate(true);
  parameters.setViewEdgeLabel(false);
  parameters.setViewNodeLabel(true);
  parameters.setViewMetaLabel(false);
  parameters.setLabelsDensity(0);
  parameters.setAntialiasing(true);
  parameters.setEdge3D(true);
}

void PorgyTraceGraphView::applyColorMapping() {
  Graph *g = graph();
  string err;
  ColorProperty *color = g->getProperty<ColorProperty>("viewColor");
  DataSet parameters;
  // plugin parameters dialog
  ParameterDescriptionList params = PluginLister::getPluginParameters("Color Mapping");
  DataSet dataSet;
  params.buildDefaultDataSet(dataSet, g);
  QString title = QString("Lauching Color Mapping on ") + tlp::tlpStringToQString(g->getName());
  bool resultBool = true;
  parameters = PluginParametersConfigurationDialog::getParameters(Perspective::instance()->mainWindow(), title, params,
                                                                  resultBool, &dataSet, g);

  g->applyPropertyAlgorithm("Color Mapping", color, err, &parameters);
}

void PorgyTraceGraphView::fillContextMenu(QMenu *contextMenu, const QPointF &position) {
  AbstractPorgyGraphView::fillContextMenu(contextMenu, position);
  contextMenu->addSeparator();
  contextMenu->addAction("&Compute a new layout", this, SLOT(redrawTraceGraph()));

  QMenu *m = contextMenu->addMenu("Apply a plugin");
  m->addAction("&Color Mapping", this, SLOT(applyColorMapping()));

  // Check if a node/edge is under the mouse pointer
  Graph *g = graph();
  GlMainWidget *gl = getGlMainWidget();
  Trace t(g);

  if (!gl->pickNodesEdges(position.x(), position.y(), entity))
    return;
  contextMenu->addSeparator();
  // Display a context menu
  node n(entity.getNode());
  if (n.isValid()) {
    Graph *metaGraph = g->getNodeMetaInfo(n);

    if (t.isModelNode(n)) {
      QString str("Graph ");
      str += tlp::tlpStringToQString(metaGraph->getName());
      contextMenu->addAction(str)->setEnabled(false);
      contextMenu->addSeparator();
      QAction *goAction =
          contextMenu->addAction(tr("Open in a new graph view"), this, SLOT(openModel()));
      goAction->setData(QVariant::fromValue<tlp::Graph *>(metaGraph));
      QAction *createNewAction = contextMenu->addAction(
          tr("Create a new derivation tree from here"), this, SLOT(createNewAction()));
      createNewAction->setData(QVariant::fromValue<tlp::node>(n));
    } else if (t.isFailureNode(n)) {
      contextMenu->addAction("Failure")->setEnabled(false);
      contextMenu->addSeparator();
    }
  } else {
    QString str("Edge ");
    str += QString::number(entity.getComplexEntityId());
    contextMenu->addAction(str)->setEnabled(false);
    contextMenu->addSeparator();
  }

  contextMenu->addAction(tr("Add to/Remove from selection"), this, SLOT(invertSelection()));
}

void PorgyTraceGraphView::redrawTraceGraph() {
  Trace t(graph());
  string errMsg;
  t.redraw(errMsg, nullptr);
  centerView();
}

void PorgyTraceGraphView::invertSelection() {
  Graph *g = graph();
  BooleanProperty *elementSelected = g->getProperty<BooleanProperty>("viewSelection");
  Observable::holdObservers();
  node n = entity.getNode();
  if (n.isValid()) {
    elementSelected->setNodeValue(n, !elementSelected->getNodeValue(n));
  } else {
    edge e(entity.getEdge());
    elementSelected->setEdgeValue(e, !elementSelected->getEdgeValue(e));
  }
  Observable::unholdObservers();
}

//void PorgyTraceGraphView::resetGraphicsScene() {
//  graphicsView()->scene()->installEventFilter(this);
//}

bool PorgyTraceGraphView::eventFilter(QObject *object, QEvent *event) {
  if (event->type() == QEvent::GraphicsSceneDragMove) {
    return checkDragEventValidity(static_cast<QGraphicsSceneDragDropEvent *>(event));
  } else if (event->type() == QEvent::GraphicsSceneDrop) {
    QGraphicsSceneDragDropEvent *dropEvent = static_cast<QGraphicsSceneDragDropEvent *>(event);
    const QMimeData *mimeData = dropEvent->mimeData();
    if (mimeData->hasFormat(GRAPH_MIME_TYPE)) {
      const GraphMimeType *gMimeData = static_cast<const GraphMimeType *>(mimeData);
      Graph *dragedGraph = gMimeData->graph();
      if (dragedGraph != graph() && PorgyTlpGraphStructure::isTraceGraph(dragedGraph)) {
        setGraph(dragedGraph);
        event->accept();
        centerView();
        return true;
      } else if (PorgyTlpGraphStructure::isRuleGraph(dragedGraph) && dragNode.isValid()) {
        // Clear displayed entities.
        node metaNode = dragNode;
        dragNode = node();
        updateEntities();
        //   emit drawNeeded();
        emit applyRuleOnModel(graph()->getNodeMetaInfo(metaNode), dragedGraph, graph());
        event->accept();
        return true;
      }
    } else if (mimeData->hasFormat(PorgyMimeData::StrategyMimeType) && dragNode.isValid()) {
      const PorgyMimeData *pMimeData = static_cast<const PorgyMimeData *>(mimeData);
      node metaNode = dragNode;
      // Clear displayed entities.
      dragNode = node();
      updateEntities();
      // emit drawNeeded();
      emit applyStrategyOnModel(graph()->getNodeMetaInfo(metaNode), pMimeData->getStrategy().code(),
                                graph());
      event->accept();
      return true;
    }
  } else if (event->type() == QEvent::GraphicsSceneDragLeave) {
    dragNode = node();
    updateEntities();
    emit drawNeeded();
  } else if (event->type() == QEvent::Close) {
    // The widget will be closed stop listening events from mainWidget.
    // Avoid next event but don't clear the current list.
    getGlMainWidget()->removeEventFilter(this);
  }

  return AbstractPorgyGraphView::eventFilter(object, event);
}

bool PorgyTraceGraphView::checkDragEventValidity(QGraphicsSceneDragDropEvent *event) {
  const QMimeData *mimeData = event->mimeData();

  if (mimeData->hasFormat(GRAPH_MIME_TYPE)) {
    const GraphMimeType *gMimeData = static_cast<const GraphMimeType *>(mimeData);
    Graph *dragedGraph = gMimeData->graph();
    if (PorgyTlpGraphStructure::isTraceGraph(dragedGraph)) {
      event->setDropAction(Qt::MoveAction);
      event->accept();
      return true;
    } else if (PorgyTlpGraphStructure::isRuleGraph(dragedGraph)) {
      return checkMetanodeUnderDragEvent(event);
    } else {
      // Block event
      event->setDropAction(Qt::IgnoreAction);
      event->ignore();
      return false;
    }
  } else if (mimeData->hasFormat(PorgyMimeData::StrategyMimeType)) {
    return checkMetanodeUnderDragEvent(event);
  }
  return false;
}

bool PorgyTraceGraphView::checkMetanodeUnderDragEvent(QGraphicsSceneDragDropEvent *event) {
  dragNode = pickMetaNodeUderCursor(event->scenePos().toPoint());
  if (!dragNode.isValid()) {
    // Avoid to send event to parent but block it.
    event->setDropAction(Qt::IgnoreAction);
  } else {
    event->setDropAction(Qt::MoveAction);
  }
  event->accept();
  updateEntities();
  draw();
  return true;
}

node PorgyTraceGraphView::pickMetaNodeUderCursor(const QPoint &cursorPosition) const {
  SelectedEntity selectedEntity;
  GlMainWidget *glWidget = getGlMainWidget();
  glWidget->pickNodesEdges(glWidget->screenToViewport(cursorPosition.x()),
                           glWidget->screenToViewport(cursorPosition.y()), selectedEntity, nullptr,
                           true, false);
  node n = selectedEntity.getNode();
  if (n.isValid() && glWidget->getScene()->getGlGraphComposite()->getGraph()->isMetaNode(n))
    return n;
  else
    return node();
}

void PorgyTraceGraphView::updateEntities() {
  if (dragNode.isValid()) {
    // Compute bounding bx of the node.
    GlNode glNode(dragNode.id);
    GlMainWidget *glWidget = getGlMainWidget();
    BoundingBox bb =
        glNode.getBoundingBox(glWidget->getScene()->getGlGraphComposite()->getInputData());
    // Update rect position.
    Size bbSize = Size(bb[1] - bb[0]) * 0.2f;
    Coord topLeft = glWidget->getScene()->getGraphCamera().worldTo2DViewport(
                        glWidget->screenToViewport(Coord(bb[0]))) -
                    bbSize;
    Coord bottomRight = glWidget->getScene()->getGraphCamera().worldTo2DViewport(
                            glWidget->screenToViewport(Coord(bb[1]))) +
                        bbSize;
    _dragOverItem->setRect(
        QRectF(QPointF(topLeft[0], graphicsView()->scene()->height() - topLeft[1]),
               QPointF(bottomRight[0],
                       graphicsView()->scene()->height() - bottomRight[1]))); //
    _dragOverItem->setBrush(QBrush(QColor(55, 255, 55, 100)));
    _dragOverItem->setVisible(true);
  } else {
    _dragOverItem->setVisible(false);
  }
}

void PorgyTraceGraphView::graphChanged(Graph *graph) {
  AbstractPorgyGraphView::graphChanged(graph);
  addRedrawTrigger(
      PorgyTlpGraphStructure::getTraceRoot(graph)->getProperty<BooleanProperty>("viewSelection"));
}

PLUGIN(PorgyTraceGraphView)

INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(TraceViewSelection, "TraceViewSelection",
                                          "InteractorSelection", PorgyConstants::TRACE_VIEW_NAME,
                                          "Tulip Team", "21/11/2008", "Select nodes and edges",
                                          "1.0", StandardInteractorPriority::RectangleSelection)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(TraceViewMagnifyingGlass, "TraceViewMagnifyingGlass",
                                          "MouseMagnifyingGlassInteractor",
                                          PorgyConstants::TRACE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Magnifying Glass", "1.0",
                                          StandardInteractorPriority::MagnifyingGlass)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(TraceViewFishEye, "TraceViewFishEye", "FishEyeInteractor",
                                          PorgyConstants::TRACE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "FishEye", "1.0",
                                          StandardInteractorPriority::FishEye)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(TraceViewRectangleZoom, "TraceViewRectangleZoom",
                                          "InteractorRectangleZoom",
                                          PorgyConstants::TRACE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Rectangle zoom", "1.0",
                                          StandardInteractorPriority::ZoomOnRectangle)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(TraceSelectionModifierInteractor,
                                          "TraceSelectionModifierInteractor",
                                          "InteractorSelectionModifier",
                                          PorgyConstants::TRACE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Selection Modifier Interactor", "1.0",
                                          StandardInteractorPriority::RectangleSelectionModifier)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(TraceEditEdgeBendsInteractor,
                                          "TraceEditEdgeBendsInteractor", "InteractorEditEdgeBends",
                                          PorgyConstants::TRACE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Edit Edge Bends Interactor", "1.0",
                                          StandardInteractorPriority::EditEdgeBends)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(TracePathFinder, "TracePathFinder", "PathFinder",
                                          PorgyConstants::TRACE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Path Finder Interactor", "1.0",
                                          StandardInteractorPriority::PathSelection)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(TraceMouseLassoNodesSelectorInteractor,
                                          "TraceMouseLassoNodesSelectorInteractor",
                                          "MouseLassoNodesSelectorInteractor",
                                          PorgyConstants::TRACE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Lasso selection", "1.0",
                                          StandardInteractorPriority::FreeHandSelection)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(TraceNeighborhoodHighlighterInteractor,
                                          "TraceNeighborhoodHighlighterInteractor",
                                          "NeighborhoodHighlighterInteractor",
                                          PorgyConstants::TRACE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Neighborhood Highlighter", "1.0",
                                          StandardInteractorPriority::NeighborhoodHighlighter)
