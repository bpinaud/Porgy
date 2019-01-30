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
#ifndef _PORTGRAPHRULE_H
#define _PORTGRAPHRULE_H

#include <portgraph/PorgyConstants.h>
#include <portgraph/PortGraph.h>
#include <portgraph/porgyconf.h>

#include <tulip/StringCollection.h>

#include <forward_list>

class PortNode;
class Bridge;

namespace tlp {
class IntegerProperty;
}

class PORTGRAPH_SCOPE PortGraphRule : public PortGraph {

  Bridge *bridge;

public:
  // iterators
  std::vector<PortNode *> getMember(const PorgyConstants::RuleSide);
  /**
   * @brief getPortNodes
   * @return An iterator on the portnodes of the left- and right-hand side
   */
  std::vector<PortNode *> getPortNodes(tlp::BooleanProperty *ban = nullptr,
                                       bool random_order = false) const override;
  tlp::StringCollection getLayoutAlgorithm();
  std::vector<tlp::edge> getMemberEdges(const PorgyConstants::RuleSide);
  /**
   * @brief getAllEdges
   * @return Returns edges of the left and right members
   */
  std::vector<tlp::edge> getEdges(tlp::BooleanProperty *ban = nullptr) override;

  inline Bridge *getBridge() const {
    return bridge;
  }

  static std::tuple<tlp::Graph *, tlp::Graph *, tlp::Graph *>
  extractLHSRHS(tlp::Graph *g, tlp::PluginProgress *pp, std::string &errMsg);
  static void cleanLHSRHS(tlp::Graph *g);

  PortGraphRule(tlp::Graph *);
  ~PortGraphRule() override;
  const std::forward_list<tlp::node> findRightEquiv(const tlp::node &n) const;
};

class PORTGRAPH_SCOPE PortGraphRuleDecorator : public PortGraphDecorator {
public:
  PortGraphRuleDecorator(tlp::Graph *g = nullptr);
  tlp::BooleanProperty *getSelectionProperty() override;
  bool redraw(const std::string &plugin, tlp::DataSet &parameters, std::string &errMsg,
              tlp::PluginProgress *progress = nullptr) override;
  std::list<std::string> layoutPluginsList() const override;

  PorgyConstants::RuleSide getSide(const tlp::node &n);
  bool isEmpty() const override;
  PorgyConstants::RuleSide getSide(const tlp::edge &e);
  tlp::IntegerProperty *getSideProperty();
  tlp::IntegerProperty *getArityProperty();
};

#endif
