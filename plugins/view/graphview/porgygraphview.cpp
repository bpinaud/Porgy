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
#include "porgygraphview.h"
#include "../../StandardInteractorPriority.h"

#include <tulip/Algorithm.h>
#include <tulip/DrawingTools.h>
#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>
#include <tulip/Interactor.h>
#include <tulip/QuickAccessBar.h>
#include <tulip/TlpQtTools.h>
#include <tulip/TulipMetaTypes.h>
#include <tulip/TulipMimes.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortNode.h>

#include <porgy/Strategy.h>
#include <porgy/porgymimedata.h>

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QToolTip>

using namespace tlp;
using namespace std;

PorgyGraphView::PorgyGraphView(const tlp::PluginContext *)
    : elementsColor(nullptr), overloadedElementsColor(nullptr) {
  addDependency(PorgyConstants::REDRAW_GRAPH, "1.0");
}

PorgyGraphView::~PorgyGraphView() {}

void PorgyGraphView::initGlParameters(tlp::GlGraphRenderingParameters &parameters) const {
  parameters.setViewArrow(false);
  parameters.setEdgeColorInterpolate(false);
  parameters.setEdgeSizeInterpolate(true);
  parameters.setLabelsDensity(0);
  parameters.setAntialiasing(true);
  parameters.setMinSizeOfLabel(8);
  parameters.setLabelScaled(false);
}

namespace {
bool isValidSelectionPropertyName(const std::string &name) {

  return name != "viewSelection" && name != PorgyConstants::NEW &&
         name != PorgyConstants::NEW_STRATEGY_APPLICATION;
}
}

bool PorgyGraphView::checkInteractors() {
  bool ret = false;
  foreach (Interactor *i, interactors()) {
    i->action()->setEnabled(true); // restore interactor
    //        if(PorgyTlpGraphStructure::isGraphUsed(graph())&&(i->group()==PorgyConstants::CATEGORY_NAME))
    //        {
    //                i->action()->setEnabled(false);
    //                ret = true;
    //        }
  }
  return ret;
}

PortGraph *PorgyGraphView::buildPortGraph(tlp::Graph *graph) const {
  return new PortGraphModel(graph);
}

PortGraphDecorator *PorgyGraphView::buildPortGraphDecorator(tlp::Graph *graph) const {
  return new PortGraphModelDecorator(graph);
}

//void PorgyGraphView::resetGraphicsScene() {
//  graphicsView()->scene()->installEventFilter(this);
//}

void PorgyGraphView::setupWidget() {
  AbstractPortGraphView::setupWidget();

  graphicsView()->setAcceptDrops(true);
  graphicsView()->scene()->installEventFilter(this);
  graphicsView()->installEventFilter(this); // get the tool tip events
}

QuickAccessBar *PorgyGraphView::getQuickAccessBarImpl() {
  return new QuickAccessBarImpl(
      nullptr, QuickAccessBarImpl::QuickAccessButtons(
                   QuickAccessBarImpl::SCREENSHOT | QuickAccessBarImpl::BACKGROUNDCOLOR |
                   QuickAccessBarImpl::SHOWLABELS | QuickAccessBarImpl::SIZEINTERPOLATION |
                   QuickAccessBarImpl::LABELSSCALED | QuickAccessBarImpl::SHOWEDGES));
}

void PorgyGraphView::fillContextMenu(QMenu *contextMenu, const QPointF &position) {
  AbstractPortGraphView::fillContextMenu(contextMenu, position);
  contextMenu->addSeparator();
  contextMenu->addAction(tr("&Compute a new layout"), this, SLOT(redrawGraph()));
  // Display boolean property management menu
  QMenu *saveBooleanPropertyMenu = contextMenu->addMenu("Save selection");
  QMenu *restoreBooleanPropertyMenu = contextMenu->addMenu("Restore selection");
  saveBooleanPropertyMenu->addAction("New property", this, SLOT(copySelectionInANewProperty()));
  saveBooleanPropertyMenu->addSeparator();

  BooleanProperty *selection =
      getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getElementSelected();
  for (BooleanProperty *p : availableSelectionProperties()) {
    QAction *sAction = saveBooleanPropertyMenu->addAction(tlpStringToQString(p->getName()), this,
                                                          SLOT(copySelectionToProperty()));
    sAction->setData(QVariant::fromValue(p));
    QAction *rAction = restoreBooleanPropertyMenu->addAction(tlpStringToQString(p->getName()), this,
                                                             SLOT(copyPropertyToSelection()));
    rAction->setData(QVariant::fromValue(p));
  }

  // If there is no selection disable copy to property
  // Search for selected elements.
  Iterator<node> *nodes = selection->getNodesEqualTo(true);
  Iterator<edge> *edges = selection->getEdgesEqualTo(true);
  bool haveSelection = nodes->hasNext() || edges->hasNext();
  delete nodes;
  delete edges;
  saveBooleanPropertyMenu->setEnabled(haveSelection);

  // If there is no boolean properties disable restore menu.
  restoreBooleanPropertyMenu->setEnabled(!restoreBooleanPropertyMenu->actions().isEmpty());

  // look if the mouse pointer is over a node or edge
  if (!getGlMainWidget()->pickNodesEdges(position.x(), position.y(), selectedEntity))
    return;

  contextMenu->addSeparator();
  // Display a context menu

  QString str =
      (selectedEntity.getEntityType() == SelectedEntity::NODE_SELECTED) ? "Node " : "Edge ";
  contextMenu->addAction(str)->setEnabled(false);
  contextMenu->addSeparator();
  contextMenu->addAction(tr("Add to/Remove from selection"), this, SLOT(invertSelection()));
}

void PorgyGraphView::invertSelection() {
  AbstractPortGraphView::invertSelection(selectedEntity);
}

bool PorgyGraphView::eventFilter(QObject *object, QEvent *event) {
  if (event->type() == QEvent::GraphicsSceneDragEnter ||
      event->type() == QEvent::GraphicsSceneDragMove) {
    //         cerr << __PRETTY_FUNCTION__ << " : " << event->type() << endl;
    QGraphicsSceneDragDropEvent *dragEvent = static_cast<QGraphicsSceneDragDropEvent *>(
        event); // Drag enter and drag move are the same kind of event
    const QMimeData *mimeData = dragEvent->mimeData();
    if (mimeData->hasFormat(GRAPH_MIME_TYPE)) {
      const GraphMimeType *gMimeData = static_cast<const GraphMimeType *>(mimeData);
      Graph *dragedGraph = gMimeData->graph();
      if ((dragedGraph != graph() && PorgyTlpGraphStructure::isModelGraph(dragedGraph)) ||
          PorgyTlpGraphStructure::isRuleGraph(dragedGraph)) {
        dragEvent->setDropAction(Qt::MoveAction);
      } else {
        // Block event
        dragEvent->setDropAction(Qt::IgnoreAction);
      }
      dragEvent->accept(); // Block the event propagation
      return true;
    } else if (mimeData->hasFormat(PorgyMimeData::StrategyMimeType)) {
      dragEvent->acceptProposedAction();
      return true;
    } else if (mimeData->hasFormat(PorgyMimeData::portNodeQtPtrListMimeTypes)) {
      dragEvent->acceptProposedAction();
      return true;
    }
    event->ignore();
    return false;
  } else if (event->type() == QEvent::GraphicsSceneDrop) {
    //            cerr << __PRETTY_FUNCTION__ << " : " << event->type() << endl;
    QGraphicsSceneDragDropEvent *dropEvent = static_cast<QGraphicsSceneDragDropEvent *>(event);
    const QMimeData *mimeData = dropEvent->mimeData();
    if (mimeData->hasFormat(GRAPH_MIME_TYPE)) {
      const GraphMimeType *gMimeData = static_cast<const GraphMimeType *>(mimeData);
      Graph *dragedGraph = gMimeData->graph();
      if (dragedGraph != graph() && PorgyTlpGraphStructure::isModelGraph(dragedGraph)) {
        setGraph(dragedGraph);
        dropEvent->accept(); // Block the event propagation
        centerView();
        return true;
      } else if (PorgyTlpGraphStructure::isRuleGraph(dragedGraph)) {
        emit applyRuleOnModel(graph(), dragedGraph, PorgyTlpGraphStructure::getMainTrace(graph()));
        dropEvent->acceptProposedAction();
        return true;
      }
    } else if (mimeData->hasFormat(PorgyMimeData::StrategyMimeType)) {
      const PorgyMimeData *pMimeData = static_cast<const PorgyMimeData *>(mimeData);
      emit applyStrategyOnModel(graph(), pMimeData->getStrategy().code(),
                                PorgyTlpGraphStructure::getMainTrace(graph()));
      dropEvent->acceptProposedAction();
      return true;
    } else if (mimeData->hasFormat(PorgyMimeData::portNodeQtPtrListMimeTypes)) {
      GlMainWidget *glWidget = getGlMainWidget();
      const PorgyMimeData *pMimeData = static_cast<const PorgyMimeData *>(mimeData);
      Coord posCenter(dropEvent->scenePos().x(), dropEvent->scenePos().y(), 0);
      posCenter = glWidget->getScene()->getGraphCamera().viewportTo3DWorld(
          glWidget->screenToViewport(posCenter));
      QList<PortNodeQt *> portNodes = pMimeData->getPortNodeQtPtrList();
      emit pastePortNodes(posCenter, graph(), portNodes);
      dropEvent->acceptProposedAction();
      return true;
    }
    dropEvent->ignore();
    return false;
  } else if (event->type() == QEvent::ToolTip) {
    QHelpEvent *he = static_cast<QHelpEvent *>(event);
    Graph *g = getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getGraph();
    SelectedEntity selectedEntity;
    if (getGlMainWidget()->pickNodesEdges(he->pos().x(), he->pos().y(), selectedEntity, nullptr, true, false)) {
      string label;
      node n(selectedEntity.getNode());
      if (n.isValid()) {
        if (PortGraphDecorator::isCenter(g, n)) {
          PortGraphModel pg(g);
          const PortNode *p = pg[n];
          label = "<b>Name: </b>" + p->getName();
        }
      }
      QToolTip::showText(he->globalPos(), tlpStringToQString(label), getGlMainWidget());
    }
  }
  return AbstractPorgyGraphView::eventFilter(object, event);
}

void PorgyGraphView::copySelectionInANewProperty() {
  QInputDialog dialog(getGlMainWidget());
  dialog.setWindowTitle(tr("Create a new property"));
  dialog.setLabelText(tr("Input the property name"));
  QStringList availableNames;
  for (BooleanProperty *p : availableSelectionProperties()) {
    availableNames << tlpStringToQString(p->getName());
  }

  dialog.setComboBoxItems(availableNames);
  dialog.setComboBoxEditable(true);
  dialog.setTextValue("");
  BooleanProperty *selection =
      getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getElementSelected();
  if (dialog.exec() == QDialog::Accepted) {
    QString name = dialog.textValue();
    if (!name.isEmpty()) {
      BooleanProperty *destination = nullptr;
      Graph *g = graph();
      PropertyInterface *property = g->existProperty(QStringToTlpString(name))
                                        ? g->getProperty(QStringToTlpString(name))
                                        : nullptr;
      if (!isValidSelectionPropertyName(QStringToTlpString(name))) {
        // Check if the name is not a system name
        QMessageBox::critical(getGlMainWidget(), tr("Invalid property name"),
                              tr("This is a system property it cannot be overloaded."));
      } else if (property != nullptr) {
        // If user want to overload a property with a different type.
        if (property->getTypename() != BooleanProperty::propertyTypename) {
          QMessageBox::critical(getGlMainWidget(), tr("Invalid property name"),
                                tr("A property with a different type and the same name already "
                                   "exists. We cannot erase property with a different type"));
        } else if (QMessageBox::question(
                       getGlMainWidget(), tr("Erase property?"),
                       tr("A property with the same name already exists. Erase it?"),
                       QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
                       QMessageBox::No) == QMessageBox::Yes) {
          // Erase existing property
          // Ask for confirmation
          destination = static_cast<BooleanProperty *>(property);
        }
      } else {
        // Create new one
        destination = g->getLocalProperty<BooleanProperty>(QStringToTlpString(name));
      }
      if (destination != nullptr) {
        destination->copy(selection);
      }
    } else {
      QMessageBox::critical(getGlMainWidget(), tr("Invalid property name"),
                            tr("Properties cannot have empty name."));
    }
  }
}

void PorgyGraphView::copySelectionToProperty() {
  QAction *a = qobject_cast<QAction *>(sender());
  // Get property associated to the action
  BooleanProperty *destination = a->data().value<BooleanProperty *>();
  BooleanProperty *selection =
      getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getElementSelected();
  destination->copy(selection);
}

void PorgyGraphView::copyPropertyToSelection() {
  QAction *a = qobject_cast<QAction *>(sender());
  // Get property associated to the action
  BooleanProperty *source = a->data().value<BooleanProperty *>();
  BooleanProperty *selection =
      getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getElementSelected();
  selection->copy(source);
}

vector<BooleanProperty *> PorgyGraphView::availableSelectionProperties() {
  vector<BooleanProperty *> result;
  for(auto p:graph()->getObjectProperties()) {
    if (p->getTypename() == BooleanProperty::propertyTypename) {
      // Ignore system properties.
      if (isValidSelectionPropertyName(p->getName())) {
        result.push_back(static_cast<BooleanProperty *>(p));
      }
    }
  }
  return result;
}

PLUGIN(PorgyGraphView)

INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(GraphViewNavigation, "GraphViewNavigation",
                                          "InteractorNavigation", PorgyConstants::GRAPH_VIEW_NAME,
                                          "Tulip Team", "21/11/2008", "Navigate", "1.0",
                                          StandardInteractorPriority::Navigation)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(GraphViewMagnifyingGlass, "GraphViewMagnifyingGlass",
                                          "MouseMagnifyingGlassInteractor",
                                          PorgyConstants::GRAPH_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Magnifying Glass", "1.0",
                                          StandardInteractorPriority::MagnifyingGlass)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(GraphViewFishEye, "GraphViewFishEye", "FishEyeInteractor",
                                          PorgyConstants::GRAPH_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "FishEye", "1.0",
                                          StandardInteractorPriority::FishEye)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(GraphViewRectangleZoom, "GraphViewRectangleZoom",
                                          "InteractorRectangleZoom",
                                          PorgyConstants::GRAPH_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Rectangle zoom", "1.0",
                                          StandardInteractorPriority::ZoomOnRectangle)
