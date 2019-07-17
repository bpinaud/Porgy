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
#ifndef SIMPLECAMERAMORPHING_H
#define SIMPLECAMERAMORPHING_H

#include <porgy/Animation.h>

#include <tulip/Coord.h>

namespace tlp {
class Camera;
struct BoundingBox;
}

/**
  * @brief Simple camera animation class..
  **/
class SimpleCameraMorphing : public Animation {
public:
  SimpleCameraMorphing(tlp::Camera &camera, const tlp::BoundingBox &to, int frameCount,
                       QObject *parent = nullptr);
  void frameChanged(int frame) override;

private:
  void computeAjustSceneToSize(tlp::BoundingBox boundingBox, int width, int height,
                               tlp::Coord *center, tlp::Coord *eye, float *sceneRadius);
  tlp::Camera *_camera;
  tlp::Coord _centerStep;
  tlp::Coord _eyeStep;
  float _radiusStep;

  tlp::Coord _centerOrig;
  tlp::Coord _eyerOrig;
  float _radiusOrig;
};

#endif // SIMPLECAMERAMORPHING_H
