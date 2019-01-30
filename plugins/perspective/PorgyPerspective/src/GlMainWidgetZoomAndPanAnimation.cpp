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
#include "GlMainWidgetZoomAndPanAnimation.h"

#include <tulip/GlMainWidget.h>

using namespace tlp;

GlMainWidgetZoomAndPanAnimation::GlMainWidgetZoomAndPanAnimation(
    tlp::GlMainWidget *widget, const tlp::BoundingBox &newViewPort, int frameCount,
    bool optimalPath, double p, QObject *parent)
    : GlSceneZoomAndPanAnimation(widget->getScene(), newViewPort, frameCount, optimalPath, p,
                                 parent),
      _glWidget(widget) {}

void GlMainWidgetZoomAndPanAnimation::frameChanged(int frame) {
  GlSceneZoomAndPanAnimation::frameChanged(frame);
  _glWidget->draw();
}

GlSceneZoomAndPanAnimation::GlSceneZoomAndPanAnimation(tlp::GlScene *glScene,
                                                       const BoundingBox &newViewPort,
                                                       int frameCount, bool optimalPath, double p,
                                                       QObject *parent)
    : Animation(frameCount, parent),
      sceneZoomAndPan(GlSceneZoomAndPan(glScene, newViewPort, "Main", frameCount, optimalPath, p)) {
}

void GlSceneZoomAndPanAnimation::frameChanged(int frame) {
  sceneZoomAndPan.zoomAndPanAnimationStep(frame);
}
