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
#include "RunStrategyElement.h"
#include "RunStrategyElementVisitors.h"
#include "UtreeWalk.h"
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
#include "UtreePrint.h"
#endif

#include <tulip/BooleanProperty.h>
#include <tulip/ColorProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/SizeProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/PluginLister.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Trace.h>

using namespace tlp;
using namespace std;
using namespace boost;

unsigned runStrategyElement::step = 1;

// to run a strategy
bool runStrategyElement::run_strat(StrategyElement::stackStrat &s,
                                   const PorgyConstants::state &state, PluginProgress *pp,
                                   bool debug,
                                   boost::optional<StrategyElement::vectNewGraphs &> res) {
  bool ret(false);
  step = 0;
  while (!s.empty()) {
    ret = true;
    pp->setError("");
    StrategyElement::stratElem elt = s.top();
    s.pop();
    StrategyElement::vector_strategyelts cond(get<3>(elt));
    Graph *g = get<2>(elt);
    string P(get<0>(elt));
    string Ban(get<1>(elt));
    if (cond.empty() && res) {
      (*res).push_back(g);
      continue;
    }
    for (auto it = cond.begin(); ((it != cond.end()) && (pp->state() == TLP_CONTINUE)); ++it) {
      const StrategyElement &elt = *it;
      StrategyElement::vector_strategyelts remaining_op(it + 1, cond.end());
      bool doBreak(false);
      ++step;
      pp->setComment(string("Running strategy step ").append(to_string(step)));
      StrategyElement::vectNewGraphs newGraphAll;
      ret = elt.run(pp, g, P, Ban, state, s, remaining_op, doBreak, newGraphAll, debug) &&
            (pp->state() == TLP_CONTINUE);
      if (ret) {
        if (doBreak)
          break;
        if (res && (it + 1 == cond.end())) // when reaching the end of a
                                           // strategy, keep a trace of its
                                           // result for future branching (for
                                           // repeat and orelse operators)
          (*res).insert((*res).end(), newGraphAll.begin(), newGraphAll.end());
        if (newGraphAll.empty())
          break;
        else if (newGraphAll.size() == 1) {
          g = newGraphAll[0];
        } else { // a all(R) has been used
          // associate for each graph the remaining operations to do
          if (it + 1 == cond.end()) // if we reach the end of the strategy,
                                    // there is no need to continue
            break;
          StrategyElement::vector_strategyelts v(it + 1,
                                                 cond.end()); // copy all remaining operations
          for (auto g : newGraphAll) {
            s.push(StrategyElement::stratElem(P, Ban, g, v));
          }
          break;
        }
        if (pp->state() == TLP_STOP) {
          return true;
        }
        if (pp->state() == TLP_CANCEL) {
          // errorMsg = "Cancel button pressed";
          return false;
        }

        if (pp->getError() == "error") {
          cerr << "Error when running the strategy" << endl;
          return false;
        }
      } else {
        break;
      }
    }
  }
  return ret;
}

/////////////
runStrategyElement::runStrategyElement(PluginProgress *pp, Graph *g, string &P, string &Ban,
                                       const PorgyConstants::state st,
                                       StrategyElement::stackStrat &s,
                                       StrategyElement::vector_strategyelts &v, bool &d,
                                       StrategyElement::vectNewGraphs &newGraphAll, bool debug)
    : _pp(pp), _g(g), _P(P), _Ban(Ban), _state(st), _st(s), _r(v), _doBreak(d),
      _newGraphAll(newGraphAll), _debug(debug) {}

///////////////
bool runStrategyElement::operator()(const StrategyElement::one_all_match_datastruct &ds) {

  OneAllVisitor visit(_pp, _g, _P, _Ban, _state, get<0>(ds), _newGraphAll, _debug);
  return get<1>(ds).apply_visitor(visit);
}

////////////
/// \brief runStrategyElement::operator ()
/// \param ifds
/// \return
bool runStrategyElement::operator()(const StrategyElement::if_datastruct &ifds) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  cerr << "____IF____" << endl;
#endif
  // exécution de la condition du if
  _g->push(false);
  Graph *g = _g;
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  cerr << "__IF__: test condition" << endl;
#endif
  StrategyElement::stackStrat s;
  StrategyElement::stratElem elt(_P, _Ban, g, get<0>(ifds));
  s.push(elt);
  bool cond_status = run_strat(s, PorgyConstants::state::IF, _pp, _debug);
  _pp->setError("");
  _g->pop(false);
  // la seule chose intéressante de la condition est le code de retour
  StrategyElement::vector_strategyelts v;
  if (cond_status) { // test de la condition
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__IF__: then" << endl;
#endif
    v = get<1>(ifds);
  } else if (get<2>(ifds)) { // Do we have an else?
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__IF__: else" << endl;
#endif
    v = *get<2>(ifds);
  }
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  else {
    cerr << "__IF__: no else" << endl;
  }
#endif
  v.insert(v.end(), _r.begin(), _r.end());
  elt = StrategyElement::stratElem(_P, _Ban, _g, v);
  _st.push(elt);
  _doBreak = true;
  return true;
}
//////////////////////////////////
bool runStrategyElement::operator()(const StrategyElement::orelse_datastruct &ods) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  cerr << "____ORELSE____" << endl;
#endif
  StrategyElement::vector_strategyelts v;
  StrategyElement::stratElem elt;
  // We try to execute the 1st strategy and keep a trace of the 2nd in case of a
  // Fail on the 1st strategy
  _g->push(false);
  elt = StrategyElement::stratElem(_P, _Ban, _g, get<0>(ods));
  StrategyElement::stackStrat s;
  s.push(elt);
  StrategyElement::vectNewGraphs newGraphAll;
  if (!run_strat(s, PorgyConstants::state::ORELSE, _pp, _debug, newGraphAll)) {
    _g->pop(false);
    v = get<1>(ods);
    _doBreak = true;
    v.insert(v.end(), _r.begin(), _r.end());
    elt = StrategyElement::stratElem(_P, _Ban, _g, v);
    _st.push(elt);
  } else {
    v.insert(v.end(), _r.begin(), _r.end());
    if (newGraphAll.empty()) {
      elt = StrategyElement::stratElem(_P, _Ban, _g, v);
      _st.push(elt);
      _doBreak = true;
    }

    else if (newGraphAll.size() == 1) {
      elt = StrategyElement::stratElem(_P, _Ban, newGraphAll[0], v);
      _st.push(elt);
      _doBreak = true;
    } else { // a all(R) has been used
      // associate for each graph the remaining operations to do
      for (auto g : newGraphAll) {
        _st.push(StrategyElement::stratElem(_P, _Ban, g, v));
        _doBreak = true;
      }
    }
  }
  return true;
}

bool runStrategyElement::operator()(const StrategyElement::while_datastruct &wds) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  cerr << "____WHILE____" << endl; // while(S)do(S1)[(max)] =>
                                   // if(S)then(S1;while(S)do(S1)[(max-1)])else(Id)
#endif
  StrategyElement::stratElem elt;
  if (get<2>(wds) && (*get<2>(wds) == 0)) { // operation counter
    _doBreak = true;
    elt = StrategyElement::stratElem(_P, _Ban, _g, _r);
    _st.push(elt);
  } else {
    StrategyElement::vector_strategyelts v;
    StrategyElement::if_datastruct ifds;
    get<0>(ifds) = get<0>(wds);
    StrategyElement::vector_strategyelts v1(get<1>(wds));
    if (get<2>(wds)) {
      unsigned num = *get<2>(wds);
      --num;
      v1.push_back(
          StrategyElement(StrategyElement::while_datastruct(get<0>(wds), get<1>(wds), num)));
    } else {
      v1.push_back(StrategyElement(
          StrategyElement::while_datastruct(get<0>(wds), get<1>(wds), optional<unsigned>())));
    }
    get<1>(ifds) = v1;
    v.push_back(StrategyElement(ifds));
    _doBreak = true;
    v.insert(v.end(), _r.begin(), _r.end());
    elt = StrategyElement::stratElem(_P, _Ban, _g, v);
    _st.push(elt);
  }
  return true;
}

//////////
bool runStrategyElement::operator()(const StrategyElement::ppick_datastruct &ds) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  cerr << "__PPICK__" << endl;
#endif

  const std::vector<StrategyElement::vector_strategyelts> &v_of_v = ds.first;
  const StrategyElement::ppick_proba &prob = ds.second;
  unsigned rnd = 0;
  if (prob.type() != typeid(std::string)) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__PPICK__: probabilities given with numbers" << endl;
#endif
    // compute a random number with the given probabilities then run the
    // associated strategy
    const StrategyElement::ppick_proba_elt &v = boost::get<StrategyElement::ppick_proba_elt>(prob);
    std::discrete_distribution<> dist(v.begin(), v.end());
    rnd = dist(PorgyTlpGraphStructure::gen);
  } else {
// probabilities computed from a Python script
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__PPICK__: probabilities given with a Python script" << endl;
#endif
    cerr << "__PPICK__: Choosing a strategy based on a Python script is not "
            "yet implemented"
         << endl;
  }

  StrategyElement::vector_strategyelts v(v_of_v[rnd]);
  _doBreak = true;
  v.insert(v.end(), _r.begin(), _r.end());
  StrategyElement::stratElem elt(_P, _Ban, _g, v);
  _st.push(elt);
  return true;
}

////////////
/// \brief runStrategyElement::operator () runs a repeat(S) which is equivalent
/// to S;repeat(S)
/// execute S, then go back one step inside the remaining strategy operations
/// (i.e. execute repeat(S) again.
/// \param ds
/// \return
///
bool runStrategyElement::operator()(const StrategyElement::repeat_ds &ds) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  cerr << "____REPEAT starting on " << _g->getName() << "____" << endl;
#endif
  // We try to execute the strategy. If Id => do it again, else if Fail =>
  // remove the error and continue the strategy
  _g->push(false);
  StrategyElement::stratElem elt(_P, _Ban, _g, get<0>(ds));
  StrategyElement::stackStrat s;
  s.push(elt);
  StrategyElement::vectNewGraphs newGraphAll;
  if (get<1>(ds) && (*get<1>(ds) == 0)) { // operation counter terminated
    _doBreak = true;
    elt = StrategyElement::stratElem(_P, _Ban, _g, _r);
    _st.push(elt);
  } else {
    if (!run_strat(s, PorgyConstants::state::OTHER, _pp, _debug, newGraphAll)) {
      _g->pop(false);
      _doBreak = true;
      elt = StrategyElement::stratElem(_P, _Ban, _g, _r);
      _st.push(elt);
    } else {
      StrategyElement::vector_strategyelts v;
      if (get<1>(ds)) {
        unsigned num = *get<1>(ds);
        --num;
        v.push_back(StrategyElement(StrategyElement::repeat_ds(get<0>(ds), num)));
      } else {
        v.push_back(
            StrategyElement(StrategyElement::repeat_ds(get<0>(ds), boost::optional<unsigned>())));
      }
      v.insert(v.end(), _r.begin(), _r.end());
      if (newGraphAll.empty()) {
        elt = StrategyElement::stratElem(_P, _Ban, _g, v);
        _st.push(elt);
        _doBreak = true;
      } else if (newGraphAll.size() == 1) {
        elt = StrategyElement::stratElem(_P, _Ban, newGraphAll[0], v);
        _st.push(elt);
        _doBreak = true;
      } else { // a all(R) has been used
        // associate for each graph the remaining operations to do
        for (auto g1 : newGraphAll) {
          _st.push(StrategyElement::stratElem(_P, _Ban, g1, v));
        }
        _doBreak = true;
      }
    }
  }
  return true;
}
/////////////
// ID/FAIL
bool runStrategyElement::operator()(const StrategyElement::idfail &pu) {
  bool ret = false;
  switch (pu) {
  case StrategyElement::idfail::ID: {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "____ID____" << endl;
#endif
    ret = true;
    StrategyElement::stratElem elt(_P, _Ban, _g, _r);
    _st.push(elt);
  } break;
  case StrategyElement::idfail::FAIL: {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "____FAIL____" << endl;
#endif
    Trace(PorgyTlpGraphStructure::getMainTrace(_g))
        .addFailureNode(_g->getName(), -1, _P, _Ban);
    ret = false;
    break;
  }
  }
  return ret;
}

static BooleanProperty *make_prop(Graph *new_model, const string &prop_name, booleanptr prop_new) {
  // resetting current value;
  BooleanProperty *b = new_model->getLocalProperty<BooleanProperty>(prop_name);
  for (auto n : b->getNodesEqualTo(true)) {
    b->setNodeValue(n, false);
  }
  for (auto e : b->getEdgesEqualTo(true)) {
    b->setEdgeValue(e, false);
  }
  b->copy(prop_new.get());
  return b;
}

////////////
bool runStrategyElement::
operator()(const StrategyElement::setPosBan_datastruct &fd) { // setPos, setBan
  Graph *new_model = PorgyTlpGraphStructure::clone(_g, _P, _Ban);
  spirit::utree ut(get<1>(fd));
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  cerr << "utree: " << ut << endl;
#endif

  PorgyConstants::strat_focus_op operation(get<0>(fd));
  vector<booleanptr> v; //shared_ptr necessary?
  spirit::utree::visit(ut, utree_walk(v, _P, _Ban, new_model));
  assert(v.size() == 1); // should have only one result if the computation is successfull
  booleanptr prop_new = v[0];
  v.clear();
  if (operation == PorgyConstants::strat_focus_op::SETPOS) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "____SETPOS on " << _g->getName() << "____" << endl;
#endif
    make_prop(new_model, _P, prop_new);
  } else if (operation == PorgyConstants::strat_focus_op::SETBAN) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "____SETBAN____" << endl;
#endif
    assert(!_Ban.empty());
//    if (_Ban.empty()) {
//      unsigned i = 0;
//      _Ban = PorgyConstants::BAN_PREFIX;

//      while (new_model->existProperty(_Ban)) {
//        _Ban = "Ban" + to_string(i++);
//      }
//    }
    make_prop(new_model, _Ban, prop_new);
  }

  Trace traceobj(PorgyTlpGraphStructure::getMainTrace(new_model));
  node tgt = traceobj.createMetaNode(new_model);
  traceobj.addFocusTransformationEdge(traceobj.findNode(_g->getName()), tgt, prop_new->getName(), _g,
                                      operation);
  _newGraphAll.push_back(new_model);
  return true;
}

////////////////////////
bool runStrategyElement::operator()(const StrategyElement::update_ds &fd) {
  string name(get<0>(fd).name);
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  cerr << "__UPDATE__" << endl;
  cerr << "Lauching tulip plugin " << name << endl;
#endif
  DataSet parameters;
  Graph *new_model = PorgyTlpGraphStructure::clone(_g, _P, _Ban);
  // plugin parameters
  ParameterDescriptionList params = PluginLister::getPluginParameters(name);
  params.buildDefaultDataSet(parameters, new_model);
  if (get<1>(fd)) {
    for (auto p : *get<1>(fd)) {
      ParameterDescription pa(get<0>(p));
      if (pa.getTypeName() == typeid(StringCollection).name()) {
        StringCollection scp;
        parameters.get(pa.getName(), scp);
        scp.setCurrent(get<1>(p));
        parameters.set(pa.getName(), scp);
      } else if (pa.getTypeName() == typeid(DoubleProperty).name()) {
        DoubleProperty *dp = new_model->getRoot()->getProperty<DoubleProperty>(get<1>(p));
        parameters.set(pa.getName(), dp);
      } else if (pa.getTypeName() == typeid(BooleanProperty).name()) {
        BooleanProperty *dp = new_model->getRoot()->getProperty<BooleanProperty>(get<1>(p));
        parameters.set(pa.getName(), dp);
      } else if (pa.getTypeName() == typeid(IntegerProperty).name()) {
        IntegerProperty *dp = new_model->getRoot()->getProperty<IntegerProperty>(get<1>(p));
        parameters.set(pa.getName(), dp);
      } else if (pa.getTypeName() == typeid(ColorProperty).name()) {
        ColorProperty *dp = new_model->getRoot()->getProperty<ColorProperty>(get<1>(p));
        parameters.set(pa.getName(), dp);
      } else if (pa.getTypeName() == typeid(StringProperty).name()) {
        StringProperty *dp = new_model->getRoot()->getProperty<StringProperty>(get<1>(p));
        parameters.set(pa.getName(), dp);
      } else if (pa.getTypeName() == typeid(LayoutProperty).name()) {
        LayoutProperty *dp = new_model->getRoot()->getProperty<LayoutProperty>(get<1>(p));
        parameters.set(pa.getName(), dp);
      } else if (pa.getTypeName() == typeid(SizeProperty).name()) {
        SizeProperty *dp = new_model->getRoot()->getProperty<SizeProperty>(get<1>(p));
        parameters.set(pa.getName(), dp);
      } else
        parameters.set(pa.getName(), get<1>(p));
    }
  }
  string err;
  PropertyInterface *prop = nullptr;
  parameters.get("result", prop);
  bool ret = new_model->applyPropertyAlgorithm(name, prop, err, &parameters,_pp);
  Trace traceobj(PorgyTlpGraphStructure::getMainTrace(new_model));
  node tgt = traceobj.createMetaNode(new_model);
  traceobj.addOtherEdge(traceobj.findNode(_g->getName()), tgt, Trace::Update_Transformation);
  _newGraphAll.push_back(new_model);
  return ret;
}

static bool check_equality(booleanptr prop1, booleanptr prop2) {
  bool ret(true);
  // check if prop1 and prop2 are equals
  for (auto n : prop1->getNonDefaultValuatedNodes()) {
    if (prop2->getNodeValue(n) == prop1->getNodeDefaultValue()) {
      ret = false;
      break;
    }
  }
  if (ret) {
    for (auto n : prop2->getNonDefaultValuatedNodes()) {
      if (prop2->getNodeValue(n) == prop1->getNodeDefaultValue()) {
        ret = false;
        break;
      }
    }
    if (ret) {
      for (auto e : prop1->getNonDefaultValuatedEdges()) {
        if (prop2->getEdgeValue(e) == prop1->getEdgeDefaultValue()) {
          ret = false;
          break;
        }
      }
      if (ret) {
        for (auto e : prop2->getNonDefaultValuatedEdges()) {
          if (prop1->getEdgeValue(e) == prop1->getEdgeDefaultValue()) {
            ret = false;
            break;
          }
        }
      }
    }
  }
  return ret;
}

////////////////////////
bool runStrategyElement::operator()(const StrategyElement::comparison_datastruct &fd) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  cerr << "__COMPARISON__" << endl;
#endif
  // get utrees
  spirit::utree ut1(get<0>(fd));
  spirit::utree ut2(get<2>(fd));
  vector<booleanptr> v1, v2;
  // execute both utrees and retrieve the resulting Boolean property
  spirit::utree::visit(ut1, utree_walk(v1, _P, _Ban, _g));
  spirit::utree::visit(ut2, utree_walk(v2, _P, _Ban, _g));
  assert((v1.size() == 1) && (v2.size() == 1));
  booleanptr prop1 = v1[0], prop2 = v2[0];
  v1.clear();
  v2.clear();
  // prop1 and prop2 are the properties to compare.
  StrategyElement::comparison_operators op(get<1>(fd));
  bool correct(true);
  switch (op) {
  case StrategyElement::comparison_operators::EQUAL: {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__EQUAL__" << endl;
#endif
    correct = check_equality(prop1, prop2);
  } break;
  case StrategyElement::comparison_operators::SUBSET: {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__SUBSET__" << endl;
#endif
    // check if prop1 is a subset of prop2
    for (auto n : prop1->getNonDefaultValuatedNodes()) {
      if (prop2->getNodeValue(n) == prop1->getNodeDefaultValue()) {
        correct = false;
        break;
      }
    }
    if (correct) {
      for (auto e : prop1->getNonDefaultValuatedEdges()) {
        if (prop2->getEdgeValue(e) == prop1->getEdgeDefaultValue()) {
          correct = false;
          break;
        }
      }
    }
  } break;
  case StrategyElement::comparison_operators::NEQ: {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__NOT_EQUAL__" << endl;
#endif
    correct = !check_equality(prop1, prop2);
  } break;
  }

  if (correct) {
    if (_state != PorgyConstants::state::IF) {
      Graph *new_model = PorgyTlpGraphStructure::clone(_g, _P, _Ban);
      Trace traceobj(PorgyTlpGraphStructure::getMainTrace(new_model));
      node tgt = traceobj.createMetaNode(new_model);
      traceobj.addFocusTransformationEdge(traceobj.findNode(_g->getName()), tgt, nullptr, _g,
                                          PorgyConstants::strat_focus_op::COMPARISON);
      _newGraphAll.push_back(new_model);
    }
  } else {
    if (_state != PorgyConstants::state::IF) {
      Trace(PorgyTlpGraphStructure::getMainTrace(_g))
          .addFailureNode(_g->getName(), -1, _P, _Ban);
    }
  }
  return correct;
}
