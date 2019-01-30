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
#ifndef TRACEINFORMATIONMETANODERENDERER_H
#define TRACEINFORMATIONMETANODERENDERER_H
#include <porgy/GraphVisualProperties.h>
#include <tulip/GlMetaNodeRenderer.h>
#include <tulip/tuliphash.h>

namespace tlp {
class GlGraphInputData;
}

class TraceInformationMetanodeRenderer : public tlp::GlMetaNodeRenderer {
public:
  TraceInformationMetanodeRenderer(tlp::GlGraphInputData *_originalInputData,
                                   VisualPropertiesFlags originalPropertiesToUse);
  ~TraceInformationMetanodeRenderer() override;

  void clearStoredInputData();
  void setOverloadedPropertiesForGraph(tlp::Graph *, const GraphVisualProperties &data);

protected:
private:
  void copyOriginalInputDataPropertiesToMetaNodeInputDataProperties(tlp::GlGraphInputData *orig,
                                                                    tlp::GlGraphInputData *dest);
  VisualPropertiesFlags originalPropertiesToUse;

  TLP_HASH_MAP<tlp::Graph *, GraphVisualProperties> _inputData;
};

#endif // TRACEINFORMATIONMETANODERENDERER_H
