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
#ifndef _PORTGRAPH_H
#define _PORTGRAPH_H

#include <tulip/GraphDecorator.h>
#include <tulip/StaticProperty.h>

#include <portgraph/PorgyConstants.h>
#include <portgraph/porgyconf.h>

namespace tlp {
class Graph;
class SizeProperty;
class LayoutProperty;
class PropertyInterface;
class IntegerProperty;
}

class Bridge;
class PortNode;
class Port;

class PORTGRAPH_SCOPE PortGraphDecorator : public tlp::GraphDecorator {
public:
  PortGraphDecorator(tlp::Graph *g);
  virtual tlp::BooleanProperty *getSelectionProperty() = 0;
  /**
   * @brief redraw Redraw the model using the given plugin. If errors occurs,
   * they are store in the errMsg variable.
   * The plugin must be in the redrawPlugins list.
   * @param errMsg The error message if the redraw process fail.
   * @return True if the redraw was done correctly.
   */
  virtual bool redraw(const std::string &plugin, tlp::DataSet &parameters, std::string &errMsg,
                      tlp::PluginProgress *progress = nullptr) = 0;
  /**
   * @brief layoutPluginsList returns the list of the plugins capable to redraw
   * a port graph. Call the redraw function with the selected plugins and it's
   * parameters.
   * @return
   */
  virtual std::list<std::string> layoutPluginsList() const = 0;
  // overloaded methods
  unsigned numberOfNodes() const override;

  static bool isCenter(tlp::Graph *g, const tlp::node n);
  PorgyConstants::PortNodeType getType(const tlp::node &n);
  PorgyConstants::PortNodeType getType(const tlp::edge &e);
  tlp::IntegerProperty *getTypeProperty();
};

class PORTGRAPH_SCOPE PortGraph {

  friend class Bridge;
  friend class PortNode;

public:
  PortGraph(tlp::Graph *);
  virtual ~PortGraph();

  Port *getPortContainer(const tlp::node &) const;

  /**
   * @brief getPortNodes
   * @param ban
   * @return a vector on all portnodes of the graph
   */
  virtual std::vector<PortNode *> getPortNodes(tlp::BooleanProperty *ban = nullptr,
                                               bool random_order = false) const;
  inline tlp::Graph *getGraph() const {
    return _graph;
  }

  void drawSameSizes(tlp::SizeProperty *nodeSize, tlp::LayoutProperty *layout) const;

  static bool isPortGraphEdge(const tlp::edge e, tlp::Graph *g);

  virtual std::vector<tlp::edge> getEdges(tlp::BooleanProperty *ban = nullptr) = 0;
  std::vector<const PortNode *> getInOutPortNodes(PortNode *pn, tlp::BooleanProperty *ban) const;
  // retourne le premier portnode
  PortNode *getOnePortNode() const;

  std::pair<const PortNode *, const PortNode *> ends(const tlp::edge e) const;
  const PortNode *operator[](const tlp::node n) const;
  //    PortNode*& operator[](const tlp::node n);

protected:
  tlp::Graph *_graph;
  tlp::NodeStaticProperty<PortNode *> *container; // relation entre node et PortNode *
  tlp::NodeStaticProperty<Port *> *container_port;
};

#endif
