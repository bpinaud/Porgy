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
#include "SimpleCameraMorphing.h"

#include <tulip/Camera.h>

using namespace tlp;
using namespace std;

SimpleCameraMorphing::SimpleCameraMorphing(Camera &camera, const BoundingBox &to, int frameCount,
                                           QObject *parent)
    : Animation(frameCount, parent), _camera(&camera) {
  assert(frameCount != 0);
  Coord center;
  Coord eye;
  float sceneRadius;

  Vector<int, 4> viewPort = camera.getViewport();
  computeAjustSceneToSize(to, viewPort[2], viewPort[3], &center, &eye, &sceneRadius);

  _centerOrig = camera.getCenter();
  _eyerOrig = camera.getEyes();
  _radiusOrig = camera.getSceneRadius();

  _centerStep = (center - _centerOrig) / static_cast<float>(frameCount);
  _eyeStep = (eye - _eyerOrig) / static_cast<float>(frameCount);
  _radiusStep = (sceneRadius - _radiusOrig) / static_cast<float>(frameCount);
}

void SimpleCameraMorphing::frameChanged(int frame) {
  _camera->setCenter(_centerOrig + (_centerStep * static_cast<float>(frame)));
  _camera->setSceneRadius(_radiusOrig + (_radiusStep * static_cast<float>(frame)));
  _camera->setEyes(_eyerOrig + (_eyeStep * static_cast<float>(frame)));
  _camera->setUp(Coord(0, 1., 0));
  _camera->setZoomFactor(1.);
}

void SimpleCameraMorphing::computeAjustSceneToSize(BoundingBox boundingBox, int width, int height,
                                                   Coord *center, Coord *eye, float *sceneRadius) {
  if (!boundingBox.isValid()) {
    if (center)
      *center = Coord(0, 0, 0);
    if (sceneRadius)
      *sceneRadius = sqrt(300.0);

    if (eye && center && sceneRadius) {
      *eye = Coord(0, 0, *sceneRadius);
      *eye = *eye + *center;
    }
    return;
  }

  Coord maxC(boundingBox[1]);
  Coord minC(boundingBox[0]);

  double dx = maxC[0] - minC[0];
  double dy = maxC[1] - minC[1];
  double dz = maxC[2] - minC[2];

  Coord centerTmp = (maxC + minC) / 2.f;
  if (center) {
    *center = centerTmp;
  }

  if ((dx == 0) && (dy == 0) && (dz == 0)) {
    //   dx = dy = dz = 10.0;
    dx = dy = 10.0;
  }
  double wdx = width / dx;
  double hdy = height / dy;

  float sceneRadiusTmp;
  if (dx < dy) {
    if (wdx < hdy) {
      sceneRadiusTmp = dx;
    } else {
      if (width < height)
        sceneRadiusTmp = dx * wdx / hdy;
      else
        sceneRadiusTmp = dy;
    }
  } else {
    if (wdx > hdy) {
      sceneRadiusTmp = dy;
    } else {
      if (height < width)
        sceneRadiusTmp = dy * hdy / wdx;
      else
        sceneRadiusTmp = dx;
    }
  }

  if (sceneRadius) {
    *sceneRadius = sceneRadiusTmp;
  }

  if (eye) {
    *eye = Coord(0, 0, sceneRadiusTmp);
    *eye = *eye + centerTmp;
  }
}
