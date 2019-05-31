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
#include "AbstractPorgyGraphView.h"

#include <QApplication>
#include <QPixmap>

#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>
#include <tulip/GlMetaNodeRenderer.h>
#include <tulip/GlScene.h>

#include <tulip/DrawingTools.h>
#include <tulip/Gl2DRect.h>
#include <tulip/GlOverviewGraphicsItem.h>
#include <tulip/GlVertexArrayManager.h>
#include <tulip/OpenGlConfigManager.h>
#include <tulip/QtGlSceneZoomAndPanAnimator.h>
#include <tulip/SceneConfigWidget.h>
#include <tulip/TlpTools.h>

using namespace tlp;
using namespace std;

AbstractPorgyGraphView::AbstractPorgyGraphView(const bool activateToolip) : GlMainView(activateToolip) {}

AbstractPorgyGraphView::~AbstractPorgyGraphView() {}

tlp::DataSet AbstractPorgyGraphView::state() const {
  GlScene *scene = getGlMainWidget()->getScene();
  DataSet outDataSet;
  outDataSet.set<DataSet>("Display",
                          scene->getGlGraphComposite()->getRenderingParameters().getParameters());
  std::string out;
  scene->getXML(out);
  size_t pos = out.find(TulipBitmapDir);

  while (pos != std::string::npos) {
    out.replace(pos, TulipBitmapDir.size(), "TulipBitmapDir/");
    pos = out.find(TulipBitmapDir);
  }

  outDataSet.set<std::string>("scene", out);
  // Overview
  outDataSet.set("overviewVisible", overviewVisible());
  if (needQuickAccessBar)
    outDataSet.set("quickAccessBarVisible", quickAccessBarVisible());
  return outDataSet;
}

void AbstractPorgyGraphView::setState(const tlp::DataSet &parameters) {

  // Overview
  bool overviewVisible = false;
  if (parameters.exists("overviewVisible")) {
    parameters.get<bool>("overviewVisible", overviewVisible);
  }
  setOverviewVisible(overviewVisible);
  bool quickAccessBarVisible = false;
  if (parameters.get<bool>("quickAccessBarVisible", quickAccessBarVisible)) {
    needQuickAccessBar = true;
    setQuickAccessBarVisible(quickAccessBarVisible);
  } else
    setQuickAccessBarVisible(true);
}

void AbstractPorgyGraphView::graphChanged(tlp::Graph *graph) {
  clearObservers();
  GlScene *scene = getGlMainWidget()->getScene();

  if (!scene->getLayer("Main")) { // No main layer initialize the scene
    initScene(graph, DataSet());
  } else {
    GlGraphComposite *oldGraphComposite =
        static_cast<GlGraphComposite *>(scene->getLayer("Main")->findGlEntity("graph"));
    if (!oldGraphComposite) { // No composite initialize the scene
      initScene(graph, DataSet());
    } else {

      GlGraphRenderingParameters param = oldGraphComposite->getRenderingParameters();
      GlMetaNodeRenderer *metaNodeRenderer =
          oldGraphComposite->getInputData()->getMetaNodeRenderer();
      // prevent deletion of MetaNodeRenderer when deleting oldGraphComposite
      oldGraphComposite->getInputData()->setMetaNodeRenderer(nullptr, false);
      GlGraphComposite *graphComposite = new GlGraphComposite(graph);
      graphComposite->setRenderingParameters(param);

      metaNodeRenderer->setInputData(graphComposite->getInputData());

      graphComposite->getInputData()->setMetaNodeRenderer(metaNodeRenderer);

      if (oldGraphComposite->getInputData()->graph == graph) {
        delete graphComposite->getInputData()->getGlVertexArrayManager();
        graphComposite->getInputData()->setGlVertexArrayManager(
            oldGraphComposite->getInputData()->getGlVertexArrayManager());
        oldGraphComposite->getInputData()->setGlVertexArrayManager(nullptr);
        graphComposite->getInputData()->getGlVertexArrayManager()->setInputData(
            graphComposite->getInputData());
      }

      scene->getLayer("Main")->addGlEntity(graphComposite, "graph");

      delete oldGraphComposite;
      getGlMainWidget()->emitGraphChanged();
      setViewOrtho(true);
      GlGraphRenderingParameters parameters;
      initGlParameters(parameters); // Initialize parameters
      getGlMainWidget()->getScene()->getGlGraphComposite()->setRenderingParameters(parameters);
    }
  }
  initObservers();
  emit drawNeeded();
}

void AbstractPorgyGraphView::initScene(Graph *graph, const DataSet dataSet) {

  GlScene *scene = getGlMainWidget()->getScene();
  scene->clearLayersList();

  std::string sceneInput = "";

  if (dataSet.exists("scene")) {
    dataSet.get("scene", sceneInput);
  }

  if (sceneInput.empty()) {
    // Default scene
    GlLayer *layer = new GlLayer("Main");
    GlLayer *backgroundLayer = new GlLayer("Background");
    backgroundLayer->setVisible(false);
    GlLayer *foregroundLayer = new GlLayer("Foreground");
    foregroundLayer->setVisible(true);

    backgroundLayer->set2DMode();
    foregroundLayer->set2DMode();
    std::string dir = TulipBitmapDir;
    Gl2DRect *labri = new Gl2DRect(5., 5., 50., 50., dir + "logolabri.jpg", true, false);
    labri->setVisible(false);
    foregroundLayer->addGlEntity(labri, "labrilogo");

    scene->addExistingLayer(backgroundLayer);
    scene->addExistingLayer(layer);
    scene->addExistingLayer(foregroundLayer);
    GlGraphComposite *graphComposite = new GlGraphComposite(graph, scene);
    scene->getLayer("Main")->addGlEntity(graphComposite, "graph");
    graphComposite->getRenderingParametersPointer()->setViewNodeLabel(true);
    graphComposite->getRenderingParametersPointer()->setEdgeColorInterpolate(false);
    graphComposite->getRenderingParametersPointer()->setNodesStencil(0x0002);
    graphComposite->getRenderingParametersPointer()->setNodesLabelStencil(0x0001);
    scene->centerScene();
  } else {
    size_t pos = sceneInput.find("TulipBitmapDir/");

    while (pos != std::string::npos) {
      sceneInput.replace(pos, 15, TulipBitmapDir);
      pos = sceneInput.find("TulipBitmapDir/");
    }

    pos = sceneInput.find("TulipLibDir/");

    while (pos != std::string::npos) {
      sceneInput.replace(pos, 12, TulipLibDir);
      pos = sceneInput.find("TulipLibDir/");
    }

    scene->setWithXML(sceneInput, graph);
  }
  GlGraphComposite* composite = scene->getGlGraphComposite();
  if (dataSet.exists("Display")) {
    DataSet renderingParameters;
    dataSet.get("Display", renderingParameters);  
    GlGraphRenderingParameters rp = composite->getRenderingParameters();
    rp.setParameters(renderingParameters);
    composite->setRenderingParameters(rp);
  }

  tlp::GlGraphRenderingParameters renderingParameters;
  initGlParameters(renderingParameters);
  composite->setRenderingParameters(renderingParameters);
  composite->getInputData()->setMetaNodeRenderer(new GlMetaNodeRenderer(composite->getInputData()));
  OpenGlConfigManager::setAntiAliasing(true);
  getGlMainWidget()->emitGraphChanged();
}

void AbstractPorgyGraphView::centerView(bool) {
    if (overviewVisible())
        overviewItem()->draw(false);
    GlScene *scene = getGlMainWidget()->getScene();
    GlGraphInputData *inputData = scene->getGlGraphComposite()->getInputData();
    if(!inputData->getGraph()->nodes().empty()) {
        BoundingBox bbox =
                tlp::computeBoundingBox(inputData->getGraph(), inputData->getElementLayout(),
                                        inputData->getElementSize(), inputData->getElementRotation());
        QtGlSceneZoomAndPanAnimator zoomAndPan(getGlMainWidget(), bbox);
        zoomAndPan.animateZoomAndPan();
    }
    scene->centerScene();
}

void AbstractPorgyGraphView::initObservers() {
    GlGraphComposite* composite = getGlMainWidget()->getScene()->getGlGraphComposite();
  if (composite != nullptr) {
    GlGraphInputData *inputData = composite->getInputData();
    std::set<tlp::PropertyInterface *> properties = inputData->properties();
    for (auto &it : properties) {
      addRedrawTrigger(it);
    }
    addRedrawTrigger(inputData->graph);
  }
}

void AbstractPorgyGraphView::clearObservers() {
    GlGraphComposite* composite = getGlMainWidget()->getScene()->getGlGraphComposite();
  if (composite != nullptr) {
    GlGraphInputData *inputData = composite->getInputData();
    std::set<tlp::PropertyInterface *> properties = inputData->properties();
    for (auto &it : properties) {
      removeRedrawTrigger(it);
    }
    removeRedrawTrigger(inputData->graph);
  }
}
