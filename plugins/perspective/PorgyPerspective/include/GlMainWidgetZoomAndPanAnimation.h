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
#ifndef GLMAINWIDGETZOOMANDPANANIMATION_H
#define GLMAINWIDGETZOOMANDPANANIMATION_H

#include <tulip/GlSceneZoomAndPan.h>

#include <porgy/Animation.h>

namespace tlp {
class GlMainWidget;
}

class GlSceneZoomAndPanAnimation : public Animation {
public:
  GlSceneZoomAndPanAnimation(tlp::GlScene *glScene, const tlp::BoundingBox &newViewPort,
                             int frameCount = 1, bool optimalPath = true, double p = sqrt(1.6),
                             QObject *parent = nullptr);

  void setFrameCount(int frameCount) override {
    Animation::setFrameCount(frameCount);
    sceneZoomAndPan.setNbAnimationSteps(frameCount);
  }

  void frameChanged(int frame) override;

private:
  tlp::GlSceneZoomAndPan sceneZoomAndPan;
};

/**
 * @brief The GlMainWidgetZoomAndPanAnimation class is an extention of the
 * GlSceneZoomAndPanAnimation class to automatically redraw the GlMainWidget at
 * each frame.
 */
class GlMainWidgetZoomAndPanAnimation : public GlSceneZoomAndPanAnimation {
public:
  GlMainWidgetZoomAndPanAnimation(tlp::GlMainWidget *widget, const tlp::BoundingBox &newViewPort,
                                  int frameCount = 1, bool optimalPath = true, double p = sqrt(1.6),
                                  QObject *parent = nullptr);
  void frameChanged(int frame) override;

private:
  tlp::GlMainWidget *_glWidget;
};

#endif // GLMAINWIDGETZOOMANDPANANIMATION_H
