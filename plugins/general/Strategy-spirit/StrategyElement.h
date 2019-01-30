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
#ifndef STRATEGYELEMENT_H
#define STRATEGYELEMENT_H

#define BOOST_SPIRIT_DISABLE_UTREE_IO

#include <boost/spirit/include/support_utree.hpp>

#include <portgraph/PorgyConstants.h>

#include <tulip/WithParameter.h>

#include <stack>
#include <tuple>

namespace tlp {
class Graph;
class PluginProgress;
class DataSet;
}

class StrategyElement {

public:
  struct TulipProp {
    std::string name;
  };

  struct PluginName {
    std::string name;
  };

  enum class strat_operators { RULE, VECT, FOR, IDFAIL, PPICK };
  enum class rule_operators { ALL, ONE, MATCH };
  enum class syntactic_sugar { TRY, NOT };
  enum class idfail { ID, FAIL };
  enum class Graph_Element { NODE, PORT, EDGE };
  enum class relational_operators { EQ, NEQ, GT, GTE, LT, LTE, REGEXP };
  enum class comparison_operators { EQUAL, SUBSET, NEQ };
  enum class property_check { VALUE, PROP };
  enum class position_keywords { NGB, PROPERTY, INNGB, OUTNGB };
  enum class set_theory_op : unsigned { MINUS = 0, UNION = 1, INTER = 2 };
  enum class focus_operation : unsigned { CRTGRAPH = 0, CRTPOS = 1, CRTBAN = 2, EMPTYSET = 3 };

  typedef std::vector<StrategyElement> vector_strategyelts;
  typedef std::tuple<std::string, std::string, tlp::Graph *, vector_strategyelts> stratElem;
  typedef std::stack<stratElem> stackStrat;
  typedef std::tuple<syntactic_sugar, vector_strategyelts> trynot_datastruct;
  typedef std::vector<std::string> parallel_datastruct;
  typedef boost::variant<TulipProp, std::string, double> comp_value_variant;
  typedef std::tuple<TulipProp, StrategyElement::relational_operators, comp_value_variant>
      ngbprop_ds;
  typedef std::vector<ngbprop_ds> ngbprop_ds_vect;
  typedef boost::variant<ngbprop_ds_vect, bool> compvar;
  typedef std::tuple<position_keywords, boost::spirit::utree, StrategyElement::Graph_Element,
                     compvar>
      property_datastruct;
  typedef boost::variant<focus_operation, boost::recursive_wrapper<property_datastruct>>
      focusop_datastruct;
  typedef std::tuple<PorgyConstants::strat_focus_op, boost::spirit::utree> setPosBan_datastruct;
  typedef std::tuple<vector_strategyelts, vector_strategyelts, boost::optional<vector_strategyelts>>
      if_datastruct;
  typedef std::tuple<vector_strategyelts, vector_strategyelts> orelse_datastruct;
  typedef std::tuple<vector_strategyelts, vector_strategyelts, boost::optional<unsigned>>
      while_datastruct;
  typedef std::tuple<boost::spirit::utree, comparison_operators, boost::spirit::utree>
      comparison_datastruct;
  typedef std::vector<double> ppick_proba_elt;
  typedef boost::variant<ppick_proba_elt, std::string> ppick_proba;
  typedef std::pair<std::vector<vector_strategyelts>, ppick_proba> ppick_datastruct;
  typedef boost::variant<std::string, parallel_datastruct> ppickT_elt;
  typedef std::pair<std::vector<ppickT_elt>, ppick_proba> ppickT_datastruct;
  typedef std::tuple<vector_strategyelts, boost::optional<unsigned>> repeat_ds;
  typedef std::tuple<tlp::ParameterDescription, std::string> parameterdesc;
  typedef std::vector<StrategyElement::parameterdesc> paramvect;
  typedef std::tuple<PluginName, boost::optional<paramvect>> update_ds;
  typedef std::tuple<rule_operators,
                     boost::variant<std::string, parallel_datastruct, ppickT_datastruct>>
      one_all_match_datastruct;

  typedef std::vector<tlp::Graph *> vectNewGraphs;

  /**
   * @brief StrategyElement Constructor for one() and all() operators.
   * @brief by default, a rule R (or R||R) alone is considered as one(R)
   * @param rules
   * @param op Operation type
   */
  explicit StrategyElement(const one_all_match_datastruct &rules);
  // Id/Fail
  explicit StrategyElement(const idfail op = idfail::FAIL);
  // setPos, setBan
  explicit StrategyElement(const setPosBan_datastruct &op);
  // ppick
  explicit StrategyElement(const ppick_datastruct &vect);
  // orelse
  explicit StrategyElement(const orelse_datastruct &v);
  // while
  explicit StrategyElement(const while_datastruct &v);
  // if
  explicit StrategyElement(const if_datastruct &v);
  // try, not
  explicit StrategyElement(const trynot_datastruct &ds);
  // repeat
  explicit StrategyElement(const repeat_ds &ds);
  // vector of elements
  explicit StrategyElement(const vector_strategyelts &v);
  // equiv
  explicit StrategyElement(const comparison_datastruct &v);
  // set
  explicit StrategyElement(const update_ds &v);
  // isEmpty
  explicit StrategyElement(const boost::spirit::utree &u);

  // run an element of the strategy
  bool run(tlp::PluginProgress *pluginProgress, tlp::Graph *g, std::string &P, std::string &Ban,
           const PorgyConstants::state state, stackStrat &s, vector_strategyelts &r, bool &doBreak,
           vectNewGraphs &newGraphAll, bool debug) const;

private:
  // boost::variant to store all data structures
  boost::variant<idfail, one_all_match_datastruct, orelse_datastruct, while_datastruct,
                 if_datastruct, repeat_ds, ppick_datastruct, setPosBan_datastruct,
                 comparison_datastruct, update_ds>
      data;
};

#endif // STRATEGYELEMENT_H
