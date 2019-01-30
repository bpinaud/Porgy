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
#ifndef _PORTGRAPHMODEL_H
#define _PORTGRAPHMODEL_H

#include <portgraph/PortGraph.h>
#include <portgraph/porgyconf.h>

#include <tulip/GraphDecorator.h>

class PORTGRAPH_SCOPE PortGraphModel : public PortGraph {

public:
  PortGraphModel(tlp::Graph *);
  bool find(tlp::Graph *left, tlp::Graph *left_anti, tlp::DataSet *dataset, bool exact,
            tlp::PluginProgress *pp, bool debug);
  std::vector<tlp::edge> getEdges(tlp::BooleanProperty *ban = nullptr) override;
};

class PORTGRAPH_SCOPE PortGraphModelDecorator : public PortGraphDecorator {
public:
  PortGraphModelDecorator(tlp::Graph *g);

  tlp::BooleanProperty *getSelectionProperty() override;
  bool redraw(const std::string &plugin, tlp::DataSet &parameters, std::string &errMsg,
              tlp::PluginProgress *progress = nullptr) override;
  std::list<std::string> layoutPluginsList() const override;

  // overloaded methods
  unsigned numberOfEdges() const override;
};

#endif
