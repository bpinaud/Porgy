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
#include <tulip/GlGraphComposite.h>
#include <tulip/GlGraphInputData.h>
#include <tulip/GlScene.h>

#include "traceinformationmetanoderenderer.h"

using namespace tlp;
using namespace std;

TraceInformationMetanodeRenderer::TraceInformationMetanodeRenderer(
    GlGraphInputData *inputData, VisualPropertiesFlags originalPropertiesToUse)
    : GlMetaNodeRenderer(inputData), originalPropertiesToUse(originalPropertiesToUse) {}
TraceInformationMetanodeRenderer::~TraceInformationMetanodeRenderer() {
  clearStoredInputData();
}

void TraceInformationMetanodeRenderer::copyOriginalInputDataPropertiesToMetaNodeInputDataProperties(
    GlGraphInputData *orig, GlGraphInputData *dest) {
  if (originalPropertiesToUse.testFlag(ElementColor)) {
    dest->setElementColor(orig->getElementColor());
  }
  if (originalPropertiesToUse.testFlag(ElementBorderColor)) {
    dest->setElementBorderColor(orig->getElementBorderColor());
  }
  if (originalPropertiesToUse.testFlag(ElementLabelColor)) {
    dest->setElementLabelColor(orig->getElementLabelColor());
  }
  if (originalPropertiesToUse.testFlag(ElementLabelBorderColor)) {
    dest->setElementLabelBorderColor(orig->getElementLabelBorderColor());
  }
}

void TraceInformationMetanodeRenderer::clearStoredInputData() {
  for (TLP_HASH_MAP<tlp::Graph *, GraphVisualProperties>::const_iterator it = _inputData.begin();
       it != _inputData.end(); ++it) {
    GlScene *scene = getSceneForMetaGraph(it->first);
    if (scene != nullptr) // if the metanode is hidden, the scene is not
                          // initialized, so do nothing
      scene->getGlGraphComposite()->getInputData()->reloadGraphProperties();
  }
  _inputData.clear();
}

void TraceInformationMetanodeRenderer::setOverloadedPropertiesForGraph(
    tlp::Graph *graph, const GraphVisualProperties &data) {
  _inputData[graph] = data;
  GlScene *scene = getSceneForMetaGraph(graph);
  if (scene != nullptr) // if the metanode is hidden, the scene is not
                        // initialized, so do nothing
    data.copyToInputData(scene->getGlGraphComposite()->getInputData()); // Overload input data
}
