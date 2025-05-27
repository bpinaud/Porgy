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
#include "SMView.h"
#include "../../StandardInteractorPriority.h"
#include "intermediatestates.h"
#include "porgysmallmultiplesconfigurationwidget.h"

#include <tulip/GlGraphComposite.h>
#include <tulip/GlOffscreenRenderer.h>
#include <tulip/GlTextureManager.h>
#include <tulip/Interactor.h>
#include <tulip/TlpQtTools.h>
#include <tulip/TlpTools.h>
#include <tulip/TulipMetaTypes.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortBase.h>
#include <portgraph/PortNodeBase.h>
#include <portgraph/Trace.h>

#include <porgy/PorgyInteractors.h>

#include <QGraphicsView>
#include <QMenu>

using namespace tlp;
using namespace std;

PorgySMView::PorgySMView(const PluginContext *)
    : AbstractTraceView(), _configurationWidget(nullptr), _bar(nullptr),
      _smallMultipleGraph(nullptr) {}

PorgySMView::~PorgySMView() {
  clearRedrawTriggers();
  if (!_textures.empty())
    clearTextures();

  delete _configurationWidget;
  delete _smallMultipleGraph;
}

void PorgySMView::fillContextMenu(QMenu *contextMenu, const QPointF &pos) {
  AbstractTraceView::fillContextMenu(contextMenu, pos);

  GlMainWidget *gl = getGlMainWidget();

  if (!gl->pickNodesEdges(pos.x(), pos.y(), entity))
    return;
  contextMenu->addSeparator();
  // Display a context menu
  node n(entity.getNode());
  IntegerProperty *indexp = _smallMultipleGraph->getProperty<IntegerProperty>("elementIndex");
  unsigned currentIndex(indexp->getNodeValue(n));

  QString str("Graph ");
  Graph *metaGraph(_states.getState(currentIndex).graph);
  str += tlp::tlpStringToQString(metaGraph->getName());
  contextMenu->addAction(str)->setEnabled(false);
  contextMenu->addSeparator();
  QAction *goAction =
      contextMenu->addAction(tr("Open in a new graph view"), this, SLOT(openModel()));
  goAction->setData(QVariant::fromValue<tlp::Graph *>(metaGraph));
  QAction *createNewAction = contextMenu->addAction(tr("Create a new derivation tree from here"),
                                                    this, SLOT(createNewAction()));
  Trace t(PorgyTlpGraphStructure::getMainTrace(metaGraph));
  n = t.findNode(metaGraph->getName());
  createNewAction->setData(QVariant::fromValue<tlp::node>(n));
}

tlp::DataSet PorgySMView::state() const {
  DataSet outDataSet(AbstractPorgyGraphView::state());

  // general configuration
  PorgyIntermediaryStateFlags flags(_bar->intermediateStates());
  outDataSet.set<bool>("ShowRHS", flags.testFlag(ViewRHS));
  outDataSet.set<bool>("ShowLHS", flags.testFlag(ViewLHS));
  outDataSet.set<bool>("ShowP", flags.testFlag(ViewLHSP));

  // Small Multiples
  outDataSet.set<double>("sm_space", _configurationWidget->spacing());
  outDataSet.set<unsigned>("sm_line_numbers", _configurationWidget->lineNumber());

  return outDataSet;
}

void PorgySMView::setupWidget() {
  AbstractTraceView::setupWidget();
  _smallMultipleGraph = tlp::newGraph();
  _configurationWidget = new PorgySMConfigurationWidget();
  connect(_configurationWidget, SIGNAL(smallMultiplesLayoutUpdated()), this,
          SLOT(updateSmallMultipleLayout()));
  connect(_configurationWidget, SIGNAL(spaceChanged()), this, SLOT(updateSmallMultipleLayout()));
  AbstractTraceView::graphChanged(_smallMultipleGraph);
}

void PorgySMView::setState(const tlp::DataSet &ds) {
  PorgyIntermediaryStateFlags flags;
  if (ds.exists("ShowRHS")) {
    bool boolflags = false;
    ds.get("ShowRHS", boolflags);
    if (boolflags) {
      flags = flags | ViewRHS;
    }
    ds.get("ShowLHS", boolflags);
    if (boolflags) {
      flags = flags | ViewLHSP;
    }
    ds.get("ShowP", boolflags);
    if (boolflags) {
      flags = flags | ViewLHS;
    }
    //  _bar->setintermediateStates(flags);
    updateIntermediateStates();
  }

  // Small Multiples
  if (ds.exists("sm_space")) {
    double spacing = 0.1;
    ds.get<double>("sm_space", spacing);
    _configurationWidget->setSpacing(spacing);
  }
  if (ds.exists("sm_line_numbers")) {
    unsigned line = 1;
    ds.get<unsigned>("sm_line_numbers", line);
    _configurationWidget->setLineNumber(line);
  }
  AbstractPorgyGraphView::setState(ds);
}

std::list<QWidget *> PorgySMView::configurationWidgets() const {
  return {_configurationWidget};
}

QuickAccessBar *PorgySMView::getQuickAccessBarImpl() {
  delete _bar;
  _bar = new IntermediateStatesQuickAccessBar(nullptr);
  connect(_bar, SIGNAL(settingsChanged()), this, SLOT(updateTexture()));
  connect(_bar, SIGNAL(intermediateStatesChanged()), this, SLOT(updateIntermediateStates()));
  connect(_bar, SIGNAL(toggleThumbLabels(bool)), this, SLOT(updateTexture()));

  return _bar;
}

void PorgySMView::graphChanged(tlp::Graph *graph) {
  clearRedrawTriggers();
  Observable::holdObservers();
  GlScene *scene = getGlMainWidget()->getScene();
  GlGraphInputData *inputData = scene->getGlGraphComposite()->getInputData();
  inputData->getElementColor()->setAllNodeValue(scene->getBackgroundColor());
  inputData->getElementShape()->setAllNodeValue(NodeShape::Square);
  inputData->getElementLabelPosition()->setAllNodeValue(LabelPosition::Bottom);
  inputData->getElementFontSize()->setAllNodeValue(2);
  inputData->getElementFont()->setAllNodeValue(TulipBitmapDir + "font.ttf");
  inputData->getElementFont()->setAllEdgeValue(TulipBitmapDir + "font.ttf");
  inputData->getElementBorderWidth()->setAllNodeValue(1);
  scene->getGlGraphComposite()->getRenderingParametersPointer()->setFontsType(1);
  scene->getGlGraphComposite()->getRenderingParametersPointer()->setLabelScaled(false);
  scene->getGlGraphComposite()->getRenderingParametersPointer()->setEdgeColorInterpolate(false);
  Observable::unholdObservers();
  _states.setGraphData(graph);
  if (_bar != nullptr)
    _states.updateStateList(_bar->intermediateStates());
  else
    _states.updateStateList();

  // Invalid state display empty graph
  if (_states.stateNumber() != 0) {
    updateSmallMultiples(Reset);
  } else {
    graphicsView()->setEnabled(false);
  }
  addRedrawTrigger(_smallMultipleGraph);
  const auto &properties = scene->getGlGraphComposite()->getInputData()->properties();
  for (const auto prop : properties) {
    addRedrawTrigger(prop);
  }
}

void PorgySMView::updateSmallMultiples(SmallMultipleUpdateTypeFlags updateType) {
  if (updateType.testFlag(Reset)) { // Reset all elements
    clearTextures();
    int diff = _states.stateNumber() - _smallMultipleGraph->numberOfNodes();
    if (diff < 0) {
      for (; diff < 0; ++diff)
        _smallMultipleGraph->delNode(_smallMultipleGraph->getOneNode());
    } else {
      vector<node> vect;
      _smallMultipleGraph->addNodes(diff, vect);
    }
    _textures.resize(_states.stateNumber());
  }
  int currentIndex = 0;
  // Generate elements
  IntegerProperty *indexp = _smallMultipleGraph->getProperty<IntegerProperty>("elementIndex");
  StringProperty *label = _smallMultipleGraph->getProperty<StringProperty>("viewLabel");
  StringProperty *texture = _smallMultipleGraph->getProperty<StringProperty>("viewTexture");
  LayoutProperty *layout = _smallMultipleGraph->getProperty<LayoutProperty>("viewLayout");

  Observable::holdObservers();
  for (node n : _smallMultipleGraph->nodes()) {
    if (updateType.testFlag(Reset)) {
      // Set element id
      indexp->setNodeValue(n, currentIndex);
      _states.getState(currentIndex)
          .graph->getProperty<BooleanProperty>("viewSelection")
          ->addListener(this);
    }
    if (updateType.testFlag(Update_Name)) {
      // Element name
      label->setNodeValue(n, _states.getState(currentIndex).name);
    }
    if (updateType.testFlag(Update_texture)) {
      // Element texture
      GlTextureManager::deleteTexture(_textures[currentIndex]);
      texture->setNodeValue(n, generateTexture(currentIndex));
    }

    if (updateType.testFlag(Update_Layout)) {
      // Element position
      int lineNumber = _configurationWidget->lineNumber();
      if (lineNumber == 0)
        lineNumber = ceil(_states.stateNumber() / round(sqrt(_states.stateNumber())));
      float elementsByRow =
          static_cast<float>(_states.stateNumber()) / static_cast<float>(lineNumber);
      // If the number of elements divided by the number of row is a whole
      // number use it as number of element by row else use the ceil.
      elementsByRow =
          ceil(elementsByRow) == floor(elementsByRow) ? elementsByRow : ceil(elementsByRow);
      int col = floor(static_cast<float>(currentIndex) / static_cast<float>(elementsByRow));
      int row = currentIndex % static_cast<int>(elementsByRow);
      // Compute spacing
      float s = 1.f + _configurationWidget->spacing();
      layout->setNodeValue(n, Coord(row * s, -col * s, 0));
    }
    ++currentIndex;
  }
  Observable::unholdObservers();
  getGlMainWidget()->centerScene();
}

// observer on viewSelection for each graph. If a modification is detected,
// redraw the layout
void PorgySMView::treatEvent(const Event &ev) {
  const tlp::PropertyEvent *pEvt = dynamic_cast<const tlp::PropertyEvent *>(&ev);
  if (pEvt != nullptr) {
    // Quick and dirty code: when selecting a portnode, edges between center and
    // ports are selected
    // after the nodes. So only check for edges selection. A better solution is
    // to rely on tlp::Event
    // but I need an object to observe
    //        if(pEvt->getType()==tlp::PropertyEvent::TLP_AFTER_SET_NODE_VALUE)
    //        {
    //                updateTexture();
    //        }
    //        Graph *g = pEvt->getProperty()->getGraph();
    if (pEvt->getType() == PropertyEvent::TLP_AFTER_SET_EDGE_VALUE) {
      //            edge e(pEvt->getEdge());
      //            const pair<node,node>& ends = g->ends(e);
      //            if(PortBase::isPort(ends.first,g)&&PortBase::isPort(ends.second,g))
      updateTexture();
    }
  }
}

string PorgySMView::generateTexture(unsigned int stateId) {
  assert(stateId < _states.stateNumber());
  assert(stateId < _textures.size());
  // Destruct old texture.
  string textureName = getTextureNameForId(stateId);
  GlTextureManager::deleteTexture(textureName);
  // Create graph preview texture
  GlOffscreenRenderer *renderer = GlOffscreenRenderer::getInstance();
  renderer->setSceneBackgroundColor(Color(255, 255, 255));
  renderer->setViewPortSize(512, 512);
  renderer->clearScene();
  GraphState &state = _states.getState(stateId);
  GlGraphComposite *composite = new GlGraphComposite(state.graph);
  GlGraphRenderingParameters *param(composite->getRenderingParametersPointer());
  GlGraphRenderingParameters *sm_param(
      getGlMainWidget()->getScene()->getGlGraphComposite()->getRenderingParametersPointer());
  param->setEdgeColorInterpolate(false);
  param->setEdgeSizeInterpolate(true);
  //param->setViewNodeLabel(sm_param->isViewNodeLabel());
  param->setViewNodeLabel(_bar&&_bar->isShowThumbLabels());
  param->setLabelScaled(sm_param->isLabelScaled());
  // Overwrite default properties with new one.
  state.properties.copyToInputData(composite->getInputData());
  renderer->addGraphCompositeToScene(composite);
#ifdef __APPLE__
  renderer->renderScene(true, false);
#else
  renderer->renderScene(true, true);
#endif
  GLuint textureId = renderer->getGLTexture();
  // Register texture.
  GlTextureManager::registerExternalTexture(textureName, textureId);
  _textures[stateId] = textureName;
  renderer->clearScene();
  return textureName;
}

string PorgySMView::getTextureNameForId(int stateId) {
  return QStringToTlpString(QString("porgysmallmultiples") + QString::number(stateId));
}

void PorgySMView::clearTextures() {
  for (const auto &str : _textures) {
    GlTextureManager::deleteTexture(str);
  }
}

void PorgySMView::updateTexture() {
  updateSmallMultiples(Update_texture);
}

void PorgySMView::updateSmallMultipleLayout() {
  updateSmallMultiples(Update_Layout);
}

void PorgySMView::updateIntermediateStates() {
  // Regenerate state list
  if (_bar != nullptr)
    _states.updateStateList(_bar->intermediateStates());
  else
    _states.updateStateList();
  updateSmallMultiples(Reset);
}

PLUGIN(PorgySMView)

INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(SMViewNavigation, "SMViewNavigation",
                                          "InteractorNavigation",
                                          PorgyConstants::SMALL_MULTIPLE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Navigate", "1.0",
                                          StandardInteractorPriority::Navigation)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(SMViewMagnifyingGlass, "SMViewMagnifyingGlass",
                                          "MouseMagnifyingGlassInteractor",
                                          PorgyConstants::SMALL_MULTIPLE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Magnyfiying glass", "1.0",
                                          StandardInteractorPriority::MagnifyingGlass)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(SMViewFishEye, "SMViewFishEye", "FishEyeInteractor",
                                          PorgyConstants::SMALL_MULTIPLE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Fisheye", "1.0",
                                          StandardInteractorPriority::FishEye)
INTERACTORPLUGINVIEWEXTENSIONWITHPRIORITY(SMViewRectangleZoom, "SMViewRectangleZoom",
                                          "InteractorRectangleZoom",
                                          PorgyConstants::SMALL_MULTIPLE_VIEW_NAME, "Tulip Team",
                                          "21/11/2008", "Rectangle zoom", "1.0",
                                          StandardInteractorPriority::ZoomOnRectangle)
