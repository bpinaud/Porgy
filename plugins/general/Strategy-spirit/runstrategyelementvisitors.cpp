#include "RunStrategyElementVisitors.h"

#include <tulip/Graph.h>
#include <tulip/PythonInterpreter.h>
#include <tulip/TlpQtTools.h>

#include <portgraph/Bridge.h>
#include <portgraph/PortBase.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PorgyTlpGraphStructure.h>

#include <numeric>
#include <typeinfo>
#include <string>

#include <QFile>
#include <QFileInfo>

using namespace std;
using namespace tlp;
using namespace boost;

/////////////
static bool run_rule(const string &rulename, Graph *g, const string &P, const string &Ban,
                     const PorgyConstants::state state, PluginProgress *pp, bool isAll,
                     StrategyElement::vectNewGraphs &newGraphs, bool isMatch, bool debug) {
  DataSet ds;
  ds.set(PorgyConstants::DEBUG, debug);
  ds.set(PorgyConstants::POSITION, g->getProperty<BooleanProperty>(P));
  ds.set(PorgyConstants::BAN, g->getProperty<BooleanProperty>(Ban));
  ds.set<string>(PorgyConstants::RuleName, rulename);
  ds.set<unsigned int>(PorgyConstants::minimum_number_of_instances, 1);
  ds.set<bool>(PorgyConstants::applyConnectedComponentPacking, false);
  ds.set<PorgyConstants::state>(PorgyConstants::isCondition, state);
  ds.set<bool>(PorgyConstants::checkRule, true);
  ds.set<int>(PorgyConstants::maximum_number_of_instances, isAll ? (-1) : 1);
  bool ret(false);
  string errMsg;
  if (!isMatch) {
      assert(!rulename.empty());
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "Running " << rulename << " on " << g->getName() << endl;
#endif
    ret = g->applyAlgorithm(PorgyConstants::CHECK_APPLY_RULE, errMsg, &ds, pp);
    ds.get(PorgyConstants::newModelAll, newGraphs);
  } else {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "Trying to match " << rulename << " on " << g->getName() << endl;
#endif
    ds.set(PorgyConstants::applyRule, false);
    g->push(false);
    if (g->applyAlgorithm(PorgyConstants::CHECK_APPLY_RULE, errMsg, &ds, pp)) {
      ret = true;
      newGraphs.push_back(g);
    }
    g->pop();
  }
  return ret;
}

//////////////////////////////////////
static std::string build_parallel_rule(const StrategyElement::parallel_datastruct &rule,
                                       tlp::Graph *g, PluginProgress *pluginProgress) {
  if (rule.size() == 1)
    return rule[0];
  // build the rule
  vector<Graph *> rulesg;
  string parallel_rulename = "parallel";
  for (const string &s : rule) {
    parallel_rulename += "_";
    rulesg.push_back(PorgyTlpGraphStructure::findRule(g, s));
    parallel_rulename += s;
  }

  // Is the rule already exist?
  if (PorgyTlpGraphStructure::findRule(g, parallel_rulename) != nullptr) {
    return parallel_rulename;
  }

  // Building rule: it is the union of all LHS and RHS
  Graph *parallel_rule = PorgyTlpGraphStructure::getRulesRoot(g)->addSubGraph(parallel_rulename);
  parallel_rule->setAttribute(PorgyConstants::IS_PARALLEL_RULE, true);

  // Copy last rule entirely (to have a bridge)
  Graph *g1 = rulesg.back();
  rulesg.pop_back();
  Observable::holdObservers();
  tlp::copyToGraph(parallel_rule, g1);
  // for all other rules, only LHS, RHS and bridge ports have to be copied
  for (auto r : rulesg) {
    PortGraphRuleDecorator dec(r);
    BooleanProperty sel(r);
    sel.setAllNodeValue(false);
    sel.setAllEdgeValue(false);
    for (node n : r->nodes()) {
      if ((dec.getSide(n) == PorgyConstants::SIDE_LEFT) ||
          (dec.getSide(n) == PorgyConstants::SIDE_RIGHT)) {
        sel.setNodeValue(n, true);
      }
      if (Bridge::isBridge(n, r) && (PortBase::isPort(n, r))) {
        sel.setNodeValue(n, true);
      }
    }
    for (edge e : r->edges()) {
      if ((dec.getSide(e) == PorgyConstants::SIDE_LEFT) ||
          (dec.getSide(e) == PorgyConstants::SIDE_RIGHT) ||
          (dec.getSide(e) == PorgyConstants::SIDE_BRIDGE) ||
          (dec.getSide(e) == PorgyConstants::SIDE_BRIDGE_OPP)) {
        sel.setEdgeValue(e, true);
      }
    }
    tlp::copyToGraph(parallel_rule, r, &sel);
  }

  // je rattache les ports orphelins des bridges
  PortGraphRuleDecorator dec(parallel_rule);
  IntegerProperty *side_parallel = dec.getSideProperty();
  node bridge_center;

  // je cherche le centre du bridge
  for (node n : parallel_rule->nodes()) {
    if (Bridge::isBridge(n, parallel_rule) && (PortNodeBase::isCenter(n, parallel_rule)))
      bridge_center = n;
    if (bridge_center.isValid() && (PortBase::isPort(n, parallel_rule))) {
      edge e = parallel_rule->addEdge(bridge_center, n);
      side_parallel->setEdgeValue(e, PorgyConstants::SIDE_BRIDGE_PORT);
    }
  }

  // settings redraw algorithm attribute: use the algorithm of the first rule
  // used
  string str;
  g->getAttribute(PorgyConstants::LAYOUTALGORITHM, str);
  parallel_rule->setAttribute(PorgyConstants::LAYOUTALGORITHM, str);

  // layouting the nule rule
  string errMsg;
  parallel_rule->applyPropertyAlgorithm(PorgyConstants::REDRAW_RULE,
                                        parallel_rule->getProperty<LayoutProperty>("viewLayout"),
                                        errMsg, nullptr,pluginProgress);

  Observable::unholdObservers();

  return parallel_rulename;
}
////////////////
RuleNameVisitor::RuleNameVisitor(tlp::PluginProgress *pp, tlp::Graph *g) : _pp(pp), _g(g) {}

std::string RuleNameVisitor::operator()(const std::string &str) const {
  return str;
}
std::string RuleNameVisitor::operator()(const StrategyElement::parallel_datastruct &ds) const {
  std::string parallel_rulename(build_parallel_rule(ds, _g, _pp));
  return parallel_rulename;
}
////////////////////
OneAllVisitor::OneAllVisitor(tlp::PluginProgress *pp, tlp::Graph *g, std::string &P,
                             std::string &Ban, const PorgyConstants::state &s,
                             StrategyElement::rule_operators op,
                             StrategyElement::vectNewGraphs &newGraphs, bool debug)
    : _pp(pp), _g(g), _P(P), _Ban(Ban), _state(s), _newGraphs(newGraphs), _isAll(false),
      _isMatch(false), _debug(debug), _op(op) {
  switch (op) {
  case StrategyElement::rule_operators::ALL:
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    std::cerr << "__ALL__" << std::endl;
#endif
    _isAll = true;
    _isMatch = false;
    break;
  case StrategyElement::rule_operators::ONE:
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    std::cerr << "__ONE__" << std::endl;
#endif
    _isAll = false;
    _isMatch = false;
    break;
  case StrategyElement::rule_operators::MATCH:
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    std::cerr << "__MATCH__" << std::endl;
#endif
    _isAll = false;
    _isMatch = true;
    break;
  }
}

// for running a simple rule
bool OneAllVisitor::operator()(const std::string &rulename) {
  return run_rule(rulename, _g, _P, _Ban, _state, _pp, _isAll, _newGraphs, _isMatch, _debug);
}

// for running rules in //
bool OneAllVisitor::operator()(const StrategyElement::parallel_datastruct &ds) {
  std::string parallel_rulename(build_parallel_rule(ds, _g, _pp));
  return run_rule(parallel_rulename, _g, _P, _Ban, _state, _pp, _isAll, _newGraphs, _isMatch,
                  _debug);
}

// ppick based choice of rule
bool OneAllVisitor::operator()(const StrategyElement::ppickT_datastruct &ds) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  std::cerr << "__PPICKT__" << std::endl;
#endif
  const std::vector<StrategyElement::ppickT_elt> &v_of_r = ds.first;
  const StrategyElement::ppick_proba &prob = ds.second;
  if (prob.type() != typeid(std::string)) {
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__PPICKT__: probabilities given with numbers" << endl;
#endif
    // compute a random number with the given probabilities
    const StrategyElement::ppick_proba_elt &v_p =
        boost::get<StrategyElement::ppick_proba_elt>(prob);
    std::discrete_distribution<> dist(v_p.begin(), v_p.end());
    unsigned rnd = dist(PorgyTlpGraphStructure::gen);
    StrategyElement::ppickT_elt v(v_of_r[rnd]);
    OneAllVisitor visit(_pp, _g, _P, _Ban, _state, _op, _newGraphs, _debug);
    return v.apply_visitor(visit);
  } else {
// probabilities computed from a Python script
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
    cerr << "__PPICKT__: probabilities given with a Python script" << endl;
#endif
    // build rule list
    std::list<string> rulelist;
    for (auto &v : v_of_r) {
      // extract rule name
      RuleNameVisitor visitor(_pp, _g);
      string rule(v.apply_visitor(visitor));
      rulelist.push_back(rule);
    }
    // load Python script
    vector<string> ret;
    QFile file(tlp::tlpStringToQString(boost::get<std::string>(prob)));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QString func(file.readAll());
      QFileInfo f(file);
      file.close();
      // execute Python script
      PythonInterpreter *inst = PythonInterpreter::getInstance();
      _g->push(false);
      if (inst->runString(func)) {
        if (inst->callFunctionWithParamsAndGetReturnValue("__main__", f.baseName(), ret, _g,
                                                          rulelist, _P, _Ban)) {
          _g->pop(false);
          if(ret.empty()) {
              cerr << "Python script returned nothing" << endl;
              return false;
          }
          // The Python script must return a list with a rule and its
          // application probability (one list element each)
          std::vector<double> pr;
          for (unsigned i = 1; i < ret.size(); i += 2) {
            pr.push_back(std::stod(ret[i]));
          }
          std::discrete_distribution<> dist(pr.begin(), pr.end()); // choose the rule to run
          unsigned num(dist(PorgyTlpGraphStructure::gen) * 2);
          return run_rule(ret[num], _g, _P, _Ban, _state, _pp, _isAll, _newGraphs, _isMatch,
                          _debug);
        } else {
          _g->pop(false);
          cerr << "problem when running python script (stdout): "
               << tlp::QStringToTlpString(inst->getStandardOutput()) << endl;
          cerr << "problem when running python script (stderr): "
               << tlp::QStringToTlpString(inst->getStandardErrorOutput()) << endl;
          return false;
        }
      } else {
        cerr << "error when reading Python script" << endl;
        return false;
      }
    }
    cerr << "Cannot open Python script" << endl;
    return false;
  }
  return false;
}
