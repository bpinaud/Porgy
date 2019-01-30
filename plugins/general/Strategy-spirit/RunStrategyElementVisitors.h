#ifndef RUNSTRATEGYELEMENTVISITORS_H
#define RUNSTRATEGYELEMENTVISITORS_H

#include <boost/variant/static_visitor.hpp>

#include <string>

#include "StrategyElement.h"

namespace tlp {
class Graph;
class PluginProgress;
}

class RuleNameVisitor : public boost::static_visitor<std::string> {
  tlp::PluginProgress *_pp;
  tlp::Graph *_g;

public:
  RuleNameVisitor(tlp::PluginProgress *pp, tlp::Graph *g);
  std::string operator()(const std::string &str) const;
  std::string operator()(const StrategyElement::parallel_datastruct &ds) const;
};

class OneAllVisitor : public boost::static_visitor<bool> {
  tlp::PluginProgress *_pp;
  tlp::Graph *_g;
  std::string &_P;
  std::string &_Ban;
  const PorgyConstants::state &_state;
  StrategyElement::vectNewGraphs &_newGraphs;
  bool _isAll, _isMatch, _debug;
  StrategyElement::rule_operators _op;

public:
  OneAllVisitor(tlp::PluginProgress *pp, tlp::Graph *g, std::string &P, std::string &Ban,
                const PorgyConstants::state &s, StrategyElement::rule_operators op,
                StrategyElement::vectNewGraphs &newGraphs, bool debug);

  // for running a simple rule
  bool operator()(const std::string &rulename);
  // for running rules in //
  bool operator()(const StrategyElement::parallel_datastruct &ds);
  // ppick based choice of rule
  bool operator()(const StrategyElement::ppickT_datastruct &ds);
  // newer version of ppick: a list of rules then a Python script which returns
  // a distribution of probability to use
  //    bool operator() (const StrategyElement::ppickT2_datastruct &ds);
};
#endif // RUNSTRATEGYELEMENTVISITORS_H
