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
#include "GlSceneMorphing.h"
#include "porgyAnimationtraceview.h"

#include <tulip/GlGraphComposite.h>
#include <tulip/GlScene.h>

using namespace tlp;
using namespace std;

GlSceneMorphing::GlSceneMorphing(PorgyAnimationTraceView *view, Graph *from,
                                 const GraphVisualProperties &fromVisualProperties,
                                 GraphVisualProperties &fromOutVisualProperties, Graph *to,
                                 const GraphVisualProperties &toVisualProperties,
                                 GraphVisualProperties &toOutVisualProperties, int frameNumber,
                                 GlScene *scene, VisualPropertiesFlags propertiesToAnimate,
                                 QObject *parent)
    : AbstractGraphMorphing(from, fromVisualProperties, fromOutVisualProperties, to,
                            toVisualProperties, toOutVisualProperties, frameNumber,
                            propertiesToAnimate, parent),
      _view(view), _scene(scene) {}
GlSceneMorphing::~GlSceneMorphing() {}

tlp::Graph *GlSceneMorphing::displayedGraph() {
  return _scene->getGlGraphComposite()->getGraph();
}

void GlSceneMorphing::switchDisplayedGraph(tlp::Graph *graphToDisplay,
                                           GraphVisualProperties &overloadedProperties) {
  // Replace widget properties by computed properties.
  _view->setAnimationData(graphToDisplay, overloadedProperties);
}
