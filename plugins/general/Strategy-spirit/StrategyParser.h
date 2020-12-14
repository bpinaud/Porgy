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
#ifndef STRATEGYPARSER_CXX
#define STRATEGYPARSER_CXX

#include <boost/fusion/adapted.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_optional.hpp>

#include "StrategyElement.h"

#include <portgraph/Port.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortNode.h>

#include <tulip/DoubleProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/TlpQtTools.h>
#include <tulip/PluginLister.h>

#include <fstream>

// pour parser les stratégies qui se réduise à ID ou Fail
struct id_fail : boost::spirit::qi::symbols<char, StrategyElement::idfail> {
  id_fail(const std::string &name = std::string("id or fail"))
      : boost::spirit::qi::symbols<char, StrategyElement::idfail>(name) {
    add("id", StrategyElement::idfail::ID)("fail", StrategyElement::idfail::FAIL);
  }
} id_fail;

struct comparison_op : boost::spirit::qi::symbols<char, StrategyElement::comparison_operators> {
  comparison_op(const std::string &name = std::string("Comparison operator"))
      : boost::spirit::qi::symbols<char, StrategyElement::comparison_operators>(name) {
    add("=", StrategyElement::comparison_operators::EQUAL)(
        "[subSet]", StrategyElement::comparison_operators::SUBSET)(
        "!=", StrategyElement::comparison_operators::NEQ);
  }
} comparison_op;

// opérations pour le focus
struct focusing_op : boost::spirit::qi::symbols<char, boost::spirit::binary_string_type> {
  focusing_op(const std::string &name = std::string("crtGraph/crtPos/crtBan/emptySet"))
      : boost::spirit::qi::symbols<char, boost::spirit::binary_string_type>(name) {
    add("crtGraph", boost::spirit::binary_string_type(std::to_string(
                        static_cast<unsigned>(StrategyElement::focus_operation::CRTGRAPH))))(
        "crtPos", boost::spirit::binary_string_type(std::to_string(
                      static_cast<unsigned>(StrategyElement::focus_operation::CRTPOS))))(
        "crtBan", boost::spirit::binary_string_type(std::to_string(
                      static_cast<unsigned>(StrategyElement::focus_operation::CRTBAN))))(
        "[emptySet]", boost::spirit::binary_string_type(std::to_string(
                          static_cast<unsigned>(StrategyElement::focus_operation::EMPTYSET))));
  }
} focusing_op;

// opérations sur les éléments
struct graph_element : boost::spirit::qi::symbols<char, StrategyElement::Graph_Element> {
  graph_element(const std::string &name = std::string("Graph Element"))
      : boost::spirit::qi::symbols<char, StrategyElement::Graph_Element>(name) {
    add("node", StrategyElement::Graph_Element::NODE)("port", StrategyElement::Graph_Element::PORT)(
        "edge", StrategyElement::Graph_Element::EDGE);
  }
} graph_element;

// relational operators
struct relop : boost::spirit::qi::symbols<char, StrategyElement::relational_operators> {
  relop(const std::string &name = std::string("Relational operators"))
      : boost::spirit::qi::symbols<char, StrategyElement::relational_operators>(name) {
    add("==", StrategyElement::relational_operators::EQ)(
        "!=", StrategyElement::relational_operators::NEQ)(
        ">", StrategyElement::relational_operators::GT)(">=",
                                                        StrategyElement::relational_operators::GTE)(
        "<", StrategyElement::relational_operators::LT)("<=",
                                                        StrategyElement::relational_operators::LTE)(
        "=~", StrategyElement::relational_operators::REGEXP);
  }
} relop;

// position computation operators
struct position : boost::spirit::qi::symbols<char, StrategyElement::position_keywords> {
  position(const std::string &name = std::string("property or ngb keywords"))
      : boost::spirit::qi::symbols<char, StrategyElement::position_keywords>(name) {
    add("property", StrategyElement::position_keywords::PROPERTY)(
        "ngb", StrategyElement::position_keywords::NGB)("ngbIn",
                                                        StrategyElement::position_keywords::INNGB)(
        "ngbOut", StrategyElement::position_keywords::OUTNGB);
  }
} position;

// position computation operators
struct positionUpdate : boost::spirit::qi::symbols<char, PorgyConstants::strat_focus_op> {
  positionUpdate(const std::string &name = std::string("setPos/setBan"))
      : boost::spirit::qi::symbols<char, PorgyConstants::strat_focus_op>(name) {
    add("setPos", PorgyConstants::strat_focus_op::SETPOS)("setBan",
                                                          PorgyConstants::strat_focus_op::SETBAN);
  }
} positionUpdate;

// One / All for rules application
struct OneAllMatch : boost::spirit::qi::symbols<char, StrategyElement::rule_operators> {
  OneAllMatch(const std::string &name = std::string("one()/all()/match()"))
      : boost::spirit::qi::symbols<char, StrategyElement::rule_operators>(name) {
    add("all", StrategyElement::rule_operators::ALL)("one", StrategyElement::rule_operators::ONE)(
        "match", StrategyElement::rule_operators::MATCH);
  }
} OneAllMatch;

// try(), not(), repeat()
struct SyntacticSugar : boost::spirit::qi::symbols<char, StrategyElement::syntactic_sugar> {
  SyntacticSugar(const std::string &name = std::string("try(), not() or repeat()"))
      : boost::spirit::qi::symbols<char, StrategyElement::syntactic_sugar>(name) {
    add("try", StrategyElement::syntactic_sugar::TRY)("not", StrategyElement::syntactic_sugar::NOT);
  }
} SyntacticSugar;

struct expr_cup {
  template <typename T1, typename T2 = void>
  struct result {
    typedef void type;
  };

  void operator()(boost::spirit::utree &expr, boost::spirit::utree const &rhs) const {
    boost::spirit::utree lhs;
    lhs.swap(expr);
    expr.push_back(lhs);
    expr.push_back(rhs);
    expr.push_back(boost::spirit::utf8_symbol_type(
        std::to_string(static_cast<unsigned>(StrategyElement::set_theory_op::UNION))));
  }
};

struct expr_cap {
  template <typename T1, typename T2 = void>
  struct result {
    typedef void type;
  };

  void operator()(boost::spirit::utree &expr, boost::spirit::utree const &rhs) const {
    boost::spirit::utree lhs;
    lhs.swap(expr);
    expr.push_back(lhs);
    expr.push_back(rhs);
    expr.push_back(boost::spirit::utf8_symbol_type(
        std::to_string(static_cast<unsigned>(StrategyElement::set_theory_op::INTER))));
  }
};

struct expr_minus {
  template <typename T1, typename T2 = void>
  struct result {
    typedef void type;
  };

  void operator()(boost::spirit::utree &expr, boost::spirit::utree const &rhs) const {
    boost::spirit::utree lhs;
    lhs.swap(expr);
    expr.push_back(lhs);
    expr.push_back(rhs);
    expr.push_back(boost::spirit::utf8_symbol_type(
        std::to_string(static_cast<unsigned>(StrategyElement::set_theory_op::MINUS))));
  }
};

struct one_expr {
  template <typename T1>
  struct result {
    typedef boost::spirit::utree type;
  };

  boost::spirit::utree operator()(boost::spirit::utree const &rhs) const {
    boost::spirit::utree lhs;
    lhs.push_back(rhs);
    lhs.push_back(boost::spirit::utf8_string_type("one"));
    return lhs;
  }
};

struct all_expr {
  template <typename T1>
  struct result {
    typedef boost::spirit::utree type;
  };

  boost::spirit::utree operator()(boost::spirit::utree const &rhs) const {
    boost::spirit::utree lhs;
    lhs.push_back(rhs);
    lhs.push_back(boost::spirit::utf8_string_type("all"));
    return lhs;
  }
};

// parser for 0.?? style number
template <typename T>
struct proba : boost::spirit::qi::strict_ureal_policies<T> {
  static bool const expect_dot = true; // obligation d'avoir une virgule
  static bool const allow_leading_dot = false;
  static bool const allow_trailing_dot = false;
  // no exponent
  template <typename It>
  static bool parse_exp(It &, It const &) {
    return false;
  }
  //  No exponent
  template <typename It, typename Attribute>
  static bool parse_exp_n(It &, It const &, Attribute &) {
    return false;
  }
  // forcément 0 avant la virgule
  template <typename It, typename Attribute>
  static bool parse_n(It &first, It const &last, Attribute &) {
    return boost::spirit::qi::parse(first, last,
                                    boost::spirit::qi::uint_(0)); // needs at least boost 1.46.1
  }
};

// grammar to handle comments, tab or space
struct comments : boost::spirit::qi::grammar<std::string::const_iterator> {

  comments() : comments::base_type(start) {
    using boost::spirit::ascii::char_;
    using boost::spirit::qi::lit;

    start = boost::spirit::ascii::space                      // tab/space/cr/lf
            | lit("/*") >> *(char_ - lit("*/")) >> lit("*/") // C++-style comments
            | lit("//") >> *(char_ - boost::spirit::qi::eol);
  }
  boost::spirit::qi::rule<std::string::const_iterator> start;
};

// check if the string given to ppick is a regular and openable file
struct isfile {
  struct result {
    typedef bool type;
  };

  bool operator()(const std::string &v) const {
    std::ifstream ifile(v);
    return ifile.good();
  }
};

// phoenix functor for checking ppick probabilities
struct checkproba {

  template <typename T>
  struct result {
    typedef bool type;
  };

  template <typename T>
  bool operator()(const StrategyElement::ppick_proba_elt &v, const std::vector<T> &strat) const {
    if (strat.size() != v.size())
      return false;
    double sum = 0;
    for (auto elt : v) {
      sum += elt;
    }
    return (sum == 1.0);
  }
};

// graph plugin parameters
struct CheckParams {
  struct result {
    typedef void type;
  };

  void operator()(StrategyElement::PluginName &plugin_name,
                  boost::spirit::qi::symbols<char, tlp::ParameterDescription> &pluginParam) const {
    const tlp::ParameterDescriptionList &param =
        tlp::PluginLister::getPluginParameters(plugin_name.name);
    for (const tlp::ParameterDescription &p : param.getParameters()) {
      pluginParam.add(p.getName(), p);
    }
  }
};

// phoenix functor for checking that we have two tulip properties of the same
// type
struct checkprop {
  struct result {
    typedef bool type;
  };

  bool operator()(tlp::Graph *graph, StrategyElement::TulipProp &prop1,
                  StrategyElement::TulipProp &prop2) const {
    return (graph->getProperty(prop1.name)->getTypename() ==
            graph->getProperty(prop2.name)->getTypename());
  }
};

// when having a double as the value for property, checks if we have an
// IntegerProperty or a DoubleProperty
struct checkpropNum {
  struct result {
    typedef bool type;
  };

  bool operator()(tlp::Graph *graph, StrategyElement::TulipProp &p1) const {
    std::string str(p1.name);
    if (dynamic_cast<tlp::IntegerProperty *>(graph->getProperty(str)) ||
        dynamic_cast<tlp::DoubleProperty *>(graph->getProperty(str))) {
      return true;
    }
    return false;
  }
};

// when having a string as the value, checks if we do not have a DoubleProperty
// or an IntegerProperty
struct checkpropStr {
  struct result {
    typedef bool type;
  };

  bool operator()(tlp::Graph *graph, StrategyElement::TulipProp &p1) const {
    std::string str(p1.name);
    if (dynamic_cast<tlp::IntegerProperty *>(graph->getProperty(str)) ||
        dynamic_cast<tlp::DoubleProperty *>(graph->getProperty(str))) {
      return false;
    }
    return true;
  }
};

template <typename T>
class degFunctor {
  const T &_op;

public:
  explicit degFunctor(const T &n) : _op(n) {}
  unsigned operator()(const std::string &st) const {
    assert(_op != nullptr);
    if (st == PorgyConstants::ARITY) {
      return _op->deg();
    }
    if (st == PorgyConstants::INARITY) {
      return _op->indeg();
    }
    if (st == PorgyConstants::OUTARITY) {
      return _op->outdeg();
    }
    assert(false);
    return -1;
  }
};

boost::phoenix::function<expr_cup> const cup_e = expr_cup();
boost::phoenix::function<expr_cap> const cap_e = expr_cap();
boost::phoenix::function<expr_minus> const minus_e = expr_minus();
boost::phoenix::function<one_expr> const onef = one_expr();
boost::phoenix::function<all_expr> const allf = all_expr();
boost::phoenix::function<checkproba> const _checkProbaPpick_ = checkproba();
boost::phoenix::function<isfile> const _isFile_ = isfile();
boost::phoenix::function<checkprop> const _CheckProp_ = checkprop();
boost::phoenix::function<checkpropNum> const _CheckPropDouble_ = checkpropNum();
boost::phoenix::function<checkpropStr> const _CheckPropStr_ = checkpropStr();
boost::phoenix::function<CheckParams> const CheckParam = CheckParams();

BOOST_FUSION_ADAPT_STRUCT(StrategyElement::TulipProp, (std::string, name))

BOOST_FUSION_ADAPT_STRUCT(StrategyElement::PluginName, (std::string, name))

struct strategy_grammar
    : boost::spirit::qi::grammar<std::string::const_iterator,
                                 StrategyElement::vector_strategyelts(), comments> {

  // update arity values
  void updateArity(tlp::Graph *g, const StrategyElement::TulipProp &p) {
    // Handle special properties (Arity, InArity, OutArity)
    // get the property and update values
    if (PorgyConstants::ArityProps.find(p.name) != PorgyConstants::ArityProps.end()) {
      // compute values for each port and nodes
      tlp::IntegerProperty *propa = g->getLocalProperty<tlp::IntegerProperty>(p.name);
      PortGraphModel pg(g);
      for (PortNode *pn : pg.getPortNodes()) {
        degFunctor<PortNode *> df(pn);
        propa->setNodeValue(pn->getCenter(), df(p.name));
        for (Port *port : pn->getPorts()) {
          degFunctor<Port *> dfp(port);
          propa->setNodeValue(port->getNode(), dfp(p.name));
        }
      }
    }
  }

  strategy_grammar(std::string &errorMsg, const std::vector<std::string> &rules,
                   const std::vector<StrategyElement::TulipProp> &propertynames,
                   const std::vector<std::string> &property_names, tlp::Graph *g,
                   const std::vector<std::string> &porgy_plugin_names,
                   const std::vector<StrategyElement::PluginName> &porgyplugins)
      : strategy_grammar::base_type(start, "Porgy Grammar"), errMsg(errorMsg),
        sym_rules(rules, rules, "valid rule name"),
        sym_properties(property_names, propertynames, "valid property name"), graph(g),
        sym_porgy_plugins(porgy_plugin_names, porgyplugins, "Existing Porgy Tulip plugin") {
    using boost::spirit::qi::lit;
    using boost::spirit::qi::lexeme;
    using boost::spirit::qi::char_;
    using boost::spirit::qi::uint_;
    using boost::spirit::qi::double_;
    using boost::spirit::qi::eps;
    using boost::phoenix::construct;
    using boost::phoenix::val;
    using boost::phoenix::ref;
    using boost::spirit::qi::labels::_1;
    using boost::spirit::qi::labels::_2;
    using boost::spirit::qi::labels::_3;
    using boost::spirit::qi::labels::_4;
    using boost::spirit::qi::labels::_a;
    using boost::spirit::qi::labels::_val;
    using boost::spirit::qi::labels::_pass;

    // parser for float number style 0.xx
    boost::spirit::qi::real_parser<double, proba<double>> doublep_;

    // top level structure of a strategy => s;s;....;s
    start %= seq % ';';

    // all possible elements
    seq = one_all_match_T[_val = construct<StrategyElement>(_1)] |
          ppick[_val = construct<StrategyElement>(_1)] |
          isEmpty[_val = construct<StrategyElement>(_1)] |
          pwhile[_val = construct<StrategyElement>(_1)] |
          pif[_val = construct<StrategyElement>(_1)] |
          porelse[_val = construct<StrategyElement>(_1)] |
          id_fail[_val = construct<StrategyElement>(_1)] |
          ptry_not[_val = construct<StrategyElement>(_1)] |
          prepeat[_val = construct<StrategyElement>(_1)] |
          setpos_ban[_val = construct<StrategyElement>(_1)] |
          comparison[_val = construct<StrategyElement>(_1)] |
          update[_val = construct<StrategyElement>(_1)];

    // while loop
    pwhile %= lexeme[lit("while")] > lit('(') > start > lit(')') > lexeme[lit("do")] > lit('(') >
              start > lit(')') >
              -('(' > uint_ > ')'); // optional parameter to indicate the maximum number of loops;

    // orelse operator
    porelse %= lit('(') > start > lit(')') > lexeme[lit("orelse")] > lit('(') > start > lit(')');

    // if operator
    pif %= lexeme[lit("if")] > lit('(') > start > lit(')') > lexeme[lit("then")] > lit('(') >
           start > lit(')') > -(lexeme[lit("else")] > lit('(') > start > lit(')'));

    // try(), not()
    ptry_not %= lexeme[SyntacticSugar] > lit('(') > start > lit(')');

    // repeat()
    prepeat %= lexeme["repeat"] > lit('(') >> start > lit(')') >
               -('(' > uint_ > ')'); // optional parameter to indicate the maximum number of loops

    filename %= eps > quoted_string[_pass = _isFile_(_1)];

    list_proba %= '{' > doublep_ % ',' > '}';

    // ppick operator
    ppick %= lexeme[lit("ppick")] > '(' > (start % lit(','))[_a = _1] > ',' >
             (list_proba[_pass = _checkProbaPpick_(_1, _a)] | filename) > lit(')');

    ppickT_elt %= (parallel | sym_rules) % lit(',');

    // ppick operator for rules
    ppickT %= lexeme[lit("ppick")] > '(' > ppickT_elt[_a = _1] > ',' >
              (list_proba[_pass = _checkProbaPpick_(_1, _a)] | filename) > lit(')');

    quoted_string %= lexeme['"' >> *(char_ - '"') >> '"'];

    // for running rules in // => A||B (minimum 2, no maximum)
    parallel %= (sym_rules % lit("||"));

    // one(T) or all(T) operator
    one_all_match_T %= lexeme[OneAllMatch] > lit('(') > (parallel | sym_rules | ppickT) > lit(')');

    //////////////////////////
    expression =
        term[_val = _1] >> *((cup >> term[cup_e(_val, _1)]) | (cap >> term[cap_e(_val, _1)]));

    term = focusing[_val = _1] >> *(minus >> focusing[minus_e(_val, _1)]);

    focusing = focusing_op[_val = _1] |
               property_ngb[_val = construct<boost::spirit::any_ptr>(
                                boost::phoenix::new_<StrategyElement::property_datastruct>(
                                    _1))] // should be a utree
               | '(' >> expression[_val = _1] >> ')';

    scope %= one_F    //[_val = _1]
             | all_F; // [_val = _1];
    //   | expression [_val = allf(_1)]; //by default, it is all(F)

    cup = lit("[cup]");
    cap = lit("[cap]");
    minus = lit("\\");

    // One(F) => returns one element of F chosen uniformely at random
    one_F %= lexeme[lit("one")] > '(' > expression[_val = onef(_1)] > ')';

    // All(F) => returns all element of F
    all_F %= lexeme[lit("all")] > '(' > expression[_val = allf(_1)] > ')';

    property_ngb_expr_comp %=
        sym_properties[boost::phoenix::bind(&strategy_grammar::updateArity, this, graph, _1),
                       _a = _1] > relop >
        (double_[_pass = _CheckPropDouble_(graph, _a)] |
         quoted_string[_pass = _CheckPropStr_(graph, _a)] |
         sym_properties[_pass = _CheckProp_(graph, _a, _1),
                        boost::phoenix::bind(&strategy_grammar::updateArity, this, graph, _1)]);

    generatebool %= eps[_val = true];

    // property or ngb
    property_ngb %= lexeme[position] > '(' > expression > ',' > graph_element >
                    ((',' >> property_ngb_expr_comp % lit("&&")) | generatebool) > ')';

    // position update (setPos, setBan)
    setpos_ban %= lexeme[positionUpdate] > L'(' > scope > L')';

    // comparison
    comparison %= scope > comparison_op > scope;

    // isEmpty
    isEmpty %= lexeme[lit("isEmpty")] > L'(' > expression > L')';

    // set
    param %= pluginParam >> lit(':') >> *(char_("a-zA-Z_0-9 "));
    paramlist %= param % ',';
    update %= lexeme[lit("update")] > L'(' > L'"' >
              sym_porgy_plugins[CheckParam(_1, boost::phoenix::ref(pluginParam))] > lit('"') >>
              -(L'{' > paramlist > L'}') > L')';

    //////////////////////////////////////
    start.name("Valid strategy instruction");
    seq.name("seq");
    ppick.name("ppick (with strategies as parameters)");
    ppickT.name("ppick (with rules as parameters)");
    quoted_string.name("Quoted string");
    pwhile.name("while");
    porelse.name("orelse");
    ppickT_elt.name("ppick parameters: \"valid rulename\"");
    list_proba.name("ppick parameters: \"valid probability list\"");
    ptry_not.name("try(), not()");
    prepeat.name("repeat() operator");
    pif.name("if");
    parallel.name("parallel");
    filename.name("Valid filename");
    setpos_ban.name("setPos()/setBan()");
    property_ngb.name("property() or ngb()");
    one_all_match_T.name("one() or all()");
    focusing.name("focusing expression");
    scope.name("scope expression");
    term.name("scope term");
    expression.name("expression");
    cup.name("union operator");
    cap.name("inter operator");
    minus.name("minus operator");
    comparison.name("Equiv operator");
    one_F.name("one operator");
    update.name("update operator");
    pluginParam.name("Valid plugin parameter");
    paramlist.name("List of plugin's parameters");
    param.name("Plugin parameter");
    isEmpty.name("isEmpty operator");
    property_ngb_expr_comp.name("Optional comparison inside a property or ngb expression");
    all_F.name("All Focusing");

    boost::spirit::qi::on_error<boost::spirit::qi::fail>(
        start,
        boost::phoenix::ref(errMsg) =
            (construct<std::string>(val("Error! Expecting ")) +
             boost::phoenix::bind(&boost::spirit::info::tag, _4) // what failed
             + construct<std::string>(val(" here: \"")) + construct<std::string>(_3, _2) +
             construct<std::string>(val("\""))));
  }

  std::string &errMsg;
  boost::spirit::qi::symbols<char, std::string> sym_rules;
  boost::spirit::qi::symbols<char, tlp::ParameterDescription> pluginParam;
  boost::spirit::qi::symbols<char, StrategyElement::TulipProp> sym_properties;
  tlp::Graph *graph;
  boost::spirit::qi::symbols<char, StrategyElement::PluginName> sym_porgy_plugins;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::vector_strategyelts(),
                          comments>
      start;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::trynot_datastruct(),
                          comments>
      ptry_not;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::repeat_ds(), comments>
      prepeat;
  boost::spirit::qi::rule<
      std::string::const_iterator, StrategyElement::ppick_datastruct(), comments,
      boost::spirit::qi::locals<std::vector<StrategyElement::vector_strategyelts>>>
      ppick;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::ppick_proba_elt(), comments>
      list_proba;
  boost::spirit::qi::rule<std::string::const_iterator, std::vector<StrategyElement::ppickT_elt>(),
                          comments>
      ppickT_elt;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::ppickT_datastruct(),
                          comments,
                          boost::spirit::qi::locals<std::vector<StrategyElement::ppickT_elt>>>
      ppickT;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::one_all_match_datastruct(),
                          comments>
      one_all_match_T;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::parallel_datastruct(),
                          comments>
      parallel;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::setPosBan_datastruct(),
                          comments>
      setpos_ban;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::orelse_datastruct(),
                          comments>
      porelse;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::while_datastruct(),
                          comments>
      pwhile;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement(), comments> seq;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::if_datastruct(), comments>
      pif;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::comparison_datastruct(),
                          comments>
      comparison;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::property_datastruct(),
                          comments>
      property_ngb;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::ngbprop_ds(), comments,
                          boost::spirit::qi::locals<StrategyElement::TulipProp>>
      property_ngb_expr_comp;
  boost::spirit::qi::rule<std::string::const_iterator, bool(), comments> generatebool;
  boost::spirit::qi::rule<std::string::const_iterator, boost::spirit::utree(), comments> expression,
      focusing, term, one_F, all_F, scope, isEmpty;
  boost::spirit::qi::rule<std::string::const_iterator, boost::spirit::utf8_symbol_type(), comments>
      cup, cap, minus;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::parameterdesc(), comments>
      param;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::paramvect(), comments>
      paramlist;
  boost::spirit::qi::rule<std::string::const_iterator, StrategyElement::update_ds(), comments>
      update;
  boost::spirit::qi::rule<std::string::const_iterator, std::string(), comments> quoted_string,
      filename;
};
#endif // STRATEGYPARSER_CXX
