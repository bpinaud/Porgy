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
#include <tulip/Camera.h>
#include <tulip/GlBoundingBoxSceneVisitor.h>
#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>
#include <tulip/QtGlSceneZoomAndPanAnimator.h>

#include <porgy/MetaNodeZoomInteractorComponent.h>

#include <QEvent>
#include <QMouseEvent>

#include <portgraph/PorgyTlpGraphStructure.h>

using namespace std;
using namespace tlp;

MetaNodeZoomInteractorComponent::MetaNodeZoomInteractorComponent()
    : GLInteractorComponent(), zoom(false) {}

bool MetaNodeZoomInteractorComponent::eventFilter(QObject *obj, QEvent *e) {
  GlMainWidget *glw = static_cast<GlMainWidget *>(obj);
  Graph *graph = glw->getScene()->getGlGraphComposite()->getInputData()->getGraph();
  SelectedEntity selectedEntity;
  if (e->type() == QEvent::MouseMove) {
    QMouseEvent *qMouseEv = static_cast<QMouseEvent *>(e);
    if (glw->pickNodesEdges(qMouseEv->pos().x(), qMouseEv->pos().y(), selectedEntity, nullptr, true, false)) {
      node meta(selectedEntity.getNode());
      if (meta.isValid() && (graph->isMetaNode(meta))) {
        glw->setCursor(Qt::CrossCursor);
        return true;
      }
    }
    glw->setCursor(Qt::ArrowCursor);
  } else if (e->type() == QEvent::MouseButtonDblClick) {
    QMouseEvent *qMouseEv = static_cast<QMouseEvent *>(e);
    if (qMouseEv->button() == Qt::LeftButton) {
      node metaNode;
      if (glw->pickNodesEdges(qMouseEv->pos().x(), qMouseEv->pos().y(), selectedEntity, nullptr, true, false)) {
        metaNode = selectedEntity.getNode();
        if ((metaNode.isValid()) && (graph->isMetaNode(metaNode))) {
          if (zoom == false) {
            Graph *metaGraph = graph->getNodeMetaInfo(metaNode);
            if (metaGraph) {
              Size size = glw->getScene()
                              ->getGlGraphComposite()
                              ->getInputData()
                              ->getElementSize()
                              ->getNodeValue(metaNode);
              Coord coord = glw->getScene()
                                ->getGlGraphComposite()
                                ->getInputData()
                                ->getElementLayout()
                                ->getNodeValue(metaNode);
              BoundingBox bb;
              bb.expand(coord - size / 2.f);
              bb.expand(coord + size / 2.f);
              QtGlSceneZoomAndPanAnimator zoomAnPan(glw, bb);
              zoomAnPan.animateZoomAndPan();
              zoom = true;
              return true;
            }
          } else { // zoom==true
            Camera camera = glw->getScene()->getLayer("Main")->getCamera();
            glw->getScene()->getLayer("Main")->getCamera().setCenter(camera.getCenter());
            glw->getScene()->getLayer("Main")->getCamera().setEyes(camera.getEyes());
            glw->getScene()->getLayer("Main")->getCamera().setSceneRadius(camera.getSceneRadius());
            glw->getScene()->getLayer("Main")->getCamera().setUp(camera.getUp());
            glw->getScene()->getLayer("Main")->getCamera().setZoomFactor(camera.getZoomFactor());
            glw->draw(false);

            GlBoundingBoxSceneVisitor visitor(
                glw->getScene()->getGlGraphComposite()->getInputData());
            glw->getScene()->getLayer("Main")->acceptVisitor(&visitor);
            BoundingBox bb = visitor.getBoundingBox();
            QtGlSceneZoomAndPanAnimator zoomAnPan(glw, bb);
            zoomAnPan.animateZoomAndPan();
            zoom = false;
            return true;
          }
        }
      }
    }
  }
  return GLInteractorComponent::eventFilter(obj, e);
}
