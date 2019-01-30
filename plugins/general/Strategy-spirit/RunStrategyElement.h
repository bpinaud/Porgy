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
#ifndef RUNSTRATEGYELEMENT_H
#define RUNSTRATEGYELEMENT_H

#include "StrategyElement.h"

#include <boost/variant/static_visitor.hpp>

#include <vector>

namespace tlp {
class PluginProgress;
class Graph;
class DataSet;
}

class PortNode;

/**
 * @brief The runElement class is used to execute a strategy element
 */
class runStrategyElement : public boost::static_visitor<bool> {

  tlp::PluginProgress *_pp;
  tlp::Graph *_g;
  std::string &_P;
  std::string &_Ban;
  const PorgyConstants::state _state;
  static unsigned step;
  StrategyElement::stackStrat &_st;
  StrategyElement::vector_strategyelts &_r;
  bool &_doBreak;
  StrategyElement::vectNewGraphs &_newGraphAll;
  bool _debug;

  friend class utree_walk;

public:
  runStrategyElement(tlp::PluginProgress *pp, tlp::Graph *g, std::string &P, std::string &Ban,
                     const PorgyConstants::state state, StrategyElement::stackStrat &s,
                     StrategyElement::vector_strategyelts &v, bool &b,
                     StrategyElement::vectNewGraphs &newGraphAll, bool debug);
  static bool run_strat(StrategyElement::stackStrat &s, const PorgyConstants::state &,
                        tlp::PluginProgress *pp, bool debug,
                        boost::optional<StrategyElement::vectNewGraphs &> res = boost::none);
  /**
   * @brief operator () To execute a All() or a One()
   * @param
   * @return true if the rule has been applied successfully
   */
  bool operator()(const StrategyElement::one_all_match_datastruct &ds);

  /**
   * @brief operator () Execute an "orelse" or a "while" statement
   * @param ifds The datastructure which contains the if to execute
   * @return true if the statement was applied successfully (no failure)
   */
  bool operator()(const StrategyElement::orelse_datastruct &ifds);
  bool operator()(const StrategyElement::while_datastruct &ifds);
  /**
   * @brief operator () Execute a "if" statement
   * @param ifds The datastructure which contains the if to execute
   * @return true if the statement was applied successfully (no failure)
   */
  bool operator()(const StrategyElement::if_datastruct &ifds);

  /**
   * @brief operator () Execute a "ppick" statement
   * @param ds the datastructure of the ppick
   * @return  true if the statement has been applied successfully
   */
  bool operator()(const StrategyElement::ppick_datastruct &ds);

  /**
   * @brief operator () Execute a "repeat" statement
   * @param ds The "repeat" datastructure
   * @return  true if the statement has been applied successfully
   */
  bool operator()(const StrategyElement::repeat_ds &ds);

  /**
   * @brief operator () The strategy element is FAIL or ID
   * @param pu
   * @return true if ID, false if FAIL
   */
  bool operator()(const StrategyElement::idfail &pu);

  /**
   * @brief operator () TO execute a "setBan" or a "setPos" statement
   * @param fd the datastructure to use
   * @return true if the statement has been applied successfully
   */
  bool operator()(const StrategyElement::setPosBan_datastruct &fd);

  /**
   * @brief operator () TO execute a "Comparison" statement. It checks if both
   * expression are equivalent
   * @param fd the datastructure to use
   * @return true if the statement has been applied successfully
   */
  bool operator()(const StrategyElement::comparison_datastruct &fd);

  /**
   * @brief operator () TO execute a "set" statement. It applies the given porgy
   * tulip plugin to the subgraph Sc
   * @param fd the datastructure to use
   * @return true if the statement has been applied successfully
   */
  bool operator()(const StrategyElement::update_ds &fd);
};

#endif // RUNSTRATEGYELEMENT_H
