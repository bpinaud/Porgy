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
#include "StrategyElement.h"
#include "RunStrategyElement.h"

#include <tulip/BooleanProperty.h>

using namespace std;
using namespace tlp;

// Id/Fail
StrategyElement::StrategyElement(const idfail op) : data(op) {}

// All/One
StrategyElement::StrategyElement(const StrategyElement::one_all_match_datastruct &rules)
    : data(rules) {}

// setPos, setBan
StrategyElement::StrategyElement(const setPosBan_datastruct &op) : data(op) {}

// ppick
StrategyElement::StrategyElement(const ppick_datastruct &vect) : data(vect) {}

// while
StrategyElement::StrategyElement(const while_datastruct &v) : data(v) {}

// orelse
StrategyElement::StrategyElement(const orelse_datastruct &v) : data(v) {}

// if
StrategyElement::StrategyElement(const if_datastruct &v) : data(v) {}

// equiv
StrategyElement::StrategyElement(const comparison_datastruct &v) : data(v) {}

// update
StrategyElement::StrategyElement(const update_ds &v) : data(v) {}

// isEmpty(F) rewritten to F = \emptyset
StrategyElement::StrategyElement(const boost::spirit::utree &u) {
  cerr << "__ISEMPTY__ rewritten to F = \\emptyset" << endl;
  comparison_datastruct v;
  get<0>(v) = u;
  get<1>(v) = comparison_operators::EQUAL;
  get<2>(v) = boost::spirit::utree(boost::spirit::binary_string_type(
      to_string(static_cast<unsigned>(focus_operation::EMPTYSET))));
  data = v;
}

// try(S) equiv. à (S)orelse(Id)
// not(s) equiv. à if(S)then(Fail)else(Id)
// repeat(S) est implémenté en natif
StrategyElement::StrategyElement(const trynot_datastruct &ds) {
  // strat ID
  vector_strategyelts id;
  id.push_back(StrategyElement(idfail::ID));

  // strat Fail
  vector_strategyelts fail;
  fail.push_back(StrategyElement(idfail::FAIL));

  switch (get<0>(ds)) {
  case syntactic_sugar::TRY: {
// fabrication du orelse
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__TRY__: rewritten to an ORELSE" << endl;
#endif
    orelse_datastruct ods;
    get<0>(ods) = get<1>(ds);
    get<1>(ods) = id;
    data = ods;
  } break;
  case syntactic_sugar::NOT: {
    if_datastruct ifds;
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__NOT__: rewritten to an IF" << endl;
#endif
    get<0>(ifds) = get<1>(ds);
    get<1>(ifds) = fail;
    get<2>(ifds) = id;
    data = ifds;
  } break;
  }
}

StrategyElement::StrategyElement(const repeat_ds &ds) : data(ds) {}

///////////////////////////////
bool StrategyElement::run(PluginProgress *pp, Graph *g, string &P, string &Ban,
                          const PorgyConstants::state state, StrategyElement::stackStrat &s,
                          StrategyElement::vector_strategyelts &r, bool &doBreak,
                          StrategyElement::vectNewGraphs &newGraphAll, bool debug) const {
  // execute strategy element
  runStrategyElement visitor(pp, g, P, Ban, state, s, r, doBreak, newGraphAll, debug);
  return data.apply_visitor(visitor);
}
