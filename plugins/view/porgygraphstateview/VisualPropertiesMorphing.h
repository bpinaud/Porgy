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
#ifndef VISUALPROPERTIESMORPHING_H
#define VISUALPROPERTIESMORPHING_H

#include <porgy/Animation.h>
#include <porgy/GraphVisualProperties.h>

#include <vector>

class VisualPropertiesMorphing : public Animation {
public:
  VisualPropertiesMorphing(tlp::Graph *graph, const GraphVisualProperties &fromVisualProperties,
                           const GraphVisualProperties &toVisualProperties,
                           GraphVisualProperties &outVisualProperties, int frameNumber,
                           VisualPropertiesFlags propertiesToAnimate =
                               VisualPropertiesFlags(ElementColor | ElementSize | ElementLayout),
                           QObject *parent = nullptr);
  ~VisualPropertiesMorphing() override;
  void frameChanged(int frame) override;

private:
  void setFrameCount(int frameCount) override;
  std::vector<Animation *> propertiesAnimations;
};
#endif // VISUALPROPERTIESMORPHING_H
