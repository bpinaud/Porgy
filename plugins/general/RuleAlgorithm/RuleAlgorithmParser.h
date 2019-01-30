#ifndef RULEALGORITHMPARSER_H
#define RULEALGORITHMPARSER_H

#include "RuleAlgorithmElement.h"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/qi.hpp>

namespace AlgoParser {

// grammar to handle comments, tab or space
template <typename Iterator>
struct comments : boost::spirit::qi::grammar<Iterator> {

  comments() : comments::base_type(start) {
    using boost::spirit::ascii::char_;
    using boost::spirit::qi::lit;

    start = boost::spirit::ascii::space                      // tab/space/cr/lf
            | lit("/*") >> *(char_ - lit("*/")) >> lit("*/") // C++-style comments
            | lit("//") >> *(char_ - boost::spirit::qi::eol);
  }
  boost::spirit::qi::rule<Iterator> start;
};

struct negate_expr {
  template <typename T>
  struct result {
    typedef T type;
  };

  mini_syntax_instruction operator()(mini_syntax_instruction const &expr) const {
    return mini_syntax_instruction(unary_op('-', expr));
  }
};

boost::phoenix::function<negate_expr> neg;

struct node_expr {
  template <typename T>
  struct result {
    typedef T type;
  };

  mini_syntax_instruction operator()(mini_syntax_instruction const &expr) const {
    return mini_syntax_instruction(unary_op('n', expr));
  }
};

boost::phoenix::function<node_expr> set_node;

struct edge_expr {
  template <typename T>
  struct result {
    typedef T type;
  };

  mini_syntax_instruction operator()(mini_syntax_instruction const &expr) const {
    return mini_syntax_instruction(unary_op('e', expr));
  }
};

boost::phoenix::function<edge_expr> set_edge;

struct property_expr {
  template <typename T>
  struct result {
    typedef T type;
  };

  mini_syntax_instruction operator()(mini_syntax_instruction const &expr) const {
    return mini_syntax_instruction(unary_op('p', expr));
  }
};

boost::phoenix::function<property_expr> set_prop;

struct true_expr {
  template <typename T>
  struct result {
    typedef T type;
  };

  mini_syntax_instruction operator()(mini_syntax_instruction const &expr) const {
    return mini_syntax_instruction(unary_op('t', expr));
  }
};

boost::phoenix::function<true_expr> set_true;

struct false_expr {
  template <typename T>
  struct result {
    typedef T type;
  };

  mini_syntax_instruction operator()(mini_syntax_instruction const &expr) const {
    return mini_syntax_instruction(unary_op('f', expr));
  }
};

boost::phoenix::function<false_expr> set_false;

struct max_expr {
  template <typename T1, typename T2>
  struct result {
    typedef T1 type;
  };

  mini_syntax_instruction operator()(mini_syntax_instruction const &expr1,
                                     mini_syntax_instruction const &expr2) const {
    return mini_syntax_instruction(binary_op('>', expr1, expr2));
  }
};

boost::phoenix::function<max_expr> set_max;

struct min_expr {
  template <typename T1, typename T2>
  struct result {
    typedef T1 type;
  };

  mini_syntax_instruction operator()(mini_syntax_instruction const &expr1,
                                     mini_syntax_instruction const &expr2) const {
    return mini_syntax_instruction(binary_op('<', expr1, expr2));
  }
};

boost::phoenix::function<min_expr> set_min;

bool parse(const std::string &instr, std::string &errMsg, std::vector<std::string> propertynames,
           std::vector<std::string> lhsNodesId, std::vector<std::string> rhsNodesId,
           std::vector<std::string> lhsEdgesId, std::vector<std::string> rhsEdgesId,
           AlgoParser::mini_syntax &ast);

struct random_expr {
  template <typename T>
  struct result {
    typedef T type;
  };

  mini_syntax_instruction operator()(mini_syntax_instruction const &expr) const {
    return mini_syntax_instruction(unary_op('r', expr));
  }
};

boost::phoenix::function<random_expr> set_random;

// Grammar definition
template <typename Iterator>
struct mini_syntax_grammar
    : boost::spirit::qi::grammar<Iterator, mini_syntax(), comments<Iterator>> {

  typedef comments<Iterator> comments_;

  mini_syntax_grammar(std::string &errMsg, const std::vector<std::string> &propertynames,
                      const std::vector<std::string> &lhsNodesId,
                      const std::vector<std::string> &rhsNodesId,
                      const std::vector<std::string> &lhsEdgesId,
                      const std::vector<std::string> &rhsEdgesId)
      : mini_syntax_grammar::base_type(start), errMsg(errMsg),
        sym_properties(propertynames, propertynames, "valid property names"),
        sym_lhs_nodes(lhsNodesId, lhsNodesId, "valid lhs node Ids"),
        sym_rhs_nodes(rhsNodesId, rhsNodesId, "valid rhs node Ids"),
        sym_lhs_edges(lhsEdgesId, lhsEdgesId, "valid lhs edge Ids"),
        sym_rhs_edges(rhsEdgesId, rhsEdgesId, "valid rhs edge Ids") {
    using boost::spirit::qi::lit;
    using boost::spirit::qi::lexeme;
    using boost::spirit::qi::uint_;
    using boost::spirit::qi::double_;
    using boost::spirit::ascii::char_;
    using namespace boost::spirit::qi::labels;

    using boost::phoenix::at_c;
    using boost::phoenix::push_back;

    node_id = sym_lhs_nodes[_val = _1];

    node_rhs_id = sym_rhs_nodes[_val = _1];

    edge_id = sym_lhs_edges[_val = _1];

    edge_rhs_id = sym_rhs_edges[_val = _1];

    prop_id = sym_properties[_val = _1];

    quoted_text = '"' >> lexeme[+(char_ - '"')[_val += _1]] >> '"';

    quoted_text_str = quoted_text[at_c<0>(_val) = _1];

    node = (lit("node") | lit("n")) >> '(' >> node_id[_val = set_node(_1)] >> ')';

    node_rhs = (lit("node") | lit("n")) >> '(' >> node_rhs_id[_val = set_node(_1)] >> ')';

    edge = (lit("edge") | lit("e")) >> '(' >> edge_id[_val = set_edge(_1)] >> ')';

    edge_rhs = (lit("edge") | lit("e")) >> '(' >> edge_rhs_id[_val = set_edge(_1)] >> ')';

    property =
        //       (lit("property") | lit("p")) >> '(' >>
        '"' >> prop_id[_val = set_prop(_1)] >> '"' // >> ')'
        ;

    element_property = (node | edge)[_val = _1] >> '.' >> property[_val = _val % _1];

    element_property_rhs = (node_rhs | edge_rhs)[_val = _1] >> '.' >> property[_val = _val % _1];

    expression =
        term[_val = _1] >> *(('+' >> term[_val = _val + _1]) | ('-' >> term[_val = _val - _1]));

    term = factor[_val = _1] >>
           *(('*' >> factor[_val = _val * _1]) | ('/' >> factor[_val = _val / _1]));

    factor =
        //    uint_                       [_val = _1]
        double_[_val = _1] | ('(' >> expression[_val = _1] >> ')') |
        ('-' >> factor[_val = neg(_1)]) | ('+' >> factor[_val = _1]) |
        (element_property[_val = _1]) | (quoted_text_str[_val = _1]) |
        ((lit("max(") >> expression >> ',' >> expression >> ')')[_val = set_max(_1, _2)]) |
        ((lit("min(") >> expression >> ',' >> expression >> ')')[_val = set_min(_1, _2)]) |
        (lit("random(") >> factor[_val = set_random(_1)] >> ')');

    instruction = element_property_rhs[_val = _1] >> '=' >> expression[_val |= _1];

    start = *(instruction[push_back(at_c<0>(_val), _1)] >> ';');

    start.name("start");
    instruction.name("instruction");
    expression.name("expression");
    term.name("term");
    factor.name("factor");
    node.name("node");
    edge.name("edge");
    property.name("property");
    element_property.name("element_property");
    element_property_rhs.name("element_property");
    edge_id.name("edge_id");
    node_id.name("node_id");
    prop_id.name("prop_id");
    quoted_text_str.name("quoted_text_str");
    quoted_text.name("quoted_text");

    // debug(start);
    // TODO implement the ostream

    boost::spirit::qi::on_error<boost::spirit::qi::fail>(
        start,
        boost::phoenix::ref(errMsg) =
            (boost::phoenix::construct<std::string>(boost::phoenix::val("Error! Expecting ")) +
             boost::phoenix::bind(&boost::spirit::info::tag, _4) // what failed
             + boost::phoenix::construct<std::string>(boost::phoenix::val(" here: \"")) +
             boost::phoenix::construct<std::string>(_3, _2) +
             boost::phoenix::construct<std::string>(boost::phoenix::val("\""))));
  }

  std::string &errMsg;

  boost::spirit::qi::symbols<char, std::string> sym_properties, sym_lhs_nodes, sym_rhs_nodes,
      sym_lhs_edges, sym_rhs_edges;
  boost::spirit::qi::rule<Iterator, mini_syntax(), comments_> start;
  boost::spirit::qi::rule<Iterator, mini_syntax_instruction(), comments_> instruction;
  boost::spirit::qi::rule<Iterator, mini_syntax_instruction(), comments_> expression, term, factor;
  boost::spirit::qi::rule<Iterator, mini_syntax_instruction(), comments_> node, edge, node_rhs,
      edge_rhs, property, element_property, element_property_rhs;
  boost::spirit::qi::rule<Iterator, mini_syntax_instruction(), comments_> quoted_text_str, node_id,
      edge_id, prop_id, node_rhs_id, edge_rhs_id;
  boost::spirit::qi::rule<Iterator, std::string(), comments_> quoted_text;
};
}

#endif // RULEALGORITHMPARSER_H
