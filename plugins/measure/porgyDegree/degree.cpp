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

#include <tulip/DoubleProperty.h>
#include <tulip/PropertyAlgorithm.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>

using namespace tlp;
using namespace std;

namespace {
const char *paramHelp[] = {
    // Degree type
    "Type of degree to compute (in/out/inout)."};
}

#define DEGREE_TYPE "type"
#define DEGREE_TYPES "InOut;In;Out;"
#define INOUT 0
#define IN 1
#define OUT 2

class PortNodeDegreeMetric : public tlp::DoubleAlgorithm {

public:
  PLUGININFORMATION("Portnode Degree", "Bruno Pinaud", "01/09/2009", "Alpha", "1.0",
                    PorgyConstants::CATEGORY_NAME)

  PortNodeDegreeMetric(const PluginContext *context) : DoubleAlgorithm(context) {
    addInParameter<StringCollection>(DEGREE_TYPE, paramHelp[0], DEGREE_TYPES);
  }

  bool check(string &errMsg) override {
    if (!(PorgyTlpGraphStructure::isModelGraph(graph) ||
          PorgyTlpGraphStructure::isRuleGraph(graph))) {
      errMsg = "Active Tulip graph (" + graph->getName() + ") is not a Graph nor a rule";
      return false;
    }
    return true;
  }

  bool run() override {
    StringCollection degreeTypes(DEGREE_TYPES);
    degreeTypes.setCurrent(0);

    if (dataSet != nullptr) {
      dataSet->get(DEGREE_TYPE, degreeTypes);
    }

    PortGraph *p = nullptr;
    if (PorgyTlpGraphStructure::isModelGraph(graph))
      p = new PortGraphModel(graph);
    else
      p = new PortGraphRule(graph);
    switch (degreeTypes.getCurrent()) {
    case INOUT: {
      for (PortNode *pn : p->getPortNodes()) {
        result->setNodeValue(pn->getCenter(), pn->deg());
        for (Port *port : pn->getPorts()) {
          result->setNodeValue(port->getNode(), port->deg());
        }
      }
    } break;
    case IN: {
      for (PortNode *pn : p->getPortNodes()) {
        result->setNodeValue(pn->getCenter(), pn->indeg());
        for (Port *port : pn->getPorts()) {
          result->setNodeValue(port->getNode(), port->indeg());
        }
      }
    } break;
    case OUT: {
      for (PortNode *pn : p->getPortNodes()) {
        result->setNodeValue(pn->getCenter(), pn->outdeg());
        for (Port *port : pn->getPorts()) {
          result->setNodeValue(port->getNode(), port->outdeg());
        }
      }
    } break;
    }

    delete p;

    return true;
  }
};
PLUGIN(PortNodeDegreeMetric)
