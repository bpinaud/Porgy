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
#ifndef GRAPHMORPHING_H
#define GRAPHMORPHING_H

#include "AbstractGraphMorphing.h"

namespace tlp {
class Graph;
class GlScene;
}

class PorgyAnimationTraceView;

/**
  * @brief Implement of graph morphing for a GlScene.
  **/
class GlSceneMorphing : public AbstractGraphMorphing {
  PorgyAnimationTraceView *_view;
  tlp::GlScene *_scene;

public:
  GlSceneMorphing(PorgyAnimationTraceView *view, tlp::Graph *fromGraph,
                  const GraphVisualProperties &fromVisualProperties,
                  GraphVisualProperties &fromOutVisualProperties, tlp::Graph *toGraph,
                  const GraphVisualProperties &toVisualProperties,
                  GraphVisualProperties &toOutVisualProperties, int frameNumber,
                  tlp::GlScene *scene,
                  VisualPropertiesFlags propertiesToAnimate = VisualPropertiesFlags(ElementColor |
                                                                                    ElementSize |
                                                                                    ElementLayout),
                  QObject *parent = nullptr);
  ~GlSceneMorphing() override;

protected:
  tlp::Graph *displayedGraph() override;
  void switchDisplayedGraph(tlp::Graph *graphToDisplay,
                            GraphVisualProperties &overloadedProperties) override;
};
#endif // GRAPHMORPHING_H
