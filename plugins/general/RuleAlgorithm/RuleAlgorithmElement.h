#ifndef RULEALGORITHMELEMENT_H
#define RULEALGORITHMELEMENT_H

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include <map>
#include <string>
#include <vector>

namespace tlp {
class Graph;
}

namespace AlgoParser {
struct nil {};
struct mini_syntax;
struct mini_syntax_instruction;
struct binary_op;
struct unary_op;

typedef boost::variant<nil, boost::recursive_wrapper<mini_syntax>,
                       boost::recursive_wrapper<mini_syntax_instruction>,
                       boost::recursive_wrapper<binary_op>, boost::recursive_wrapper<unary_op>,
                       unsigned int, double, std::string>
    type;

struct mini_syntax_instruction {
  type expr;

  mini_syntax_instruction() : expr(nil()) {}

  template <typename Expr>
  mini_syntax_instruction(Expr const &expr) : expr(expr) {}

  mini_syntax_instruction &operator+(mini_syntax_instruction const &rhs);
  mini_syntax_instruction &operator-(mini_syntax_instruction const &rhs);
  mini_syntax_instruction &operator*(mini_syntax_instruction const &rhs);
  mini_syntax_instruction &operator/(mini_syntax_instruction const &rhs);
  mini_syntax_instruction &operator%(mini_syntax_instruction const &rhs);
  mini_syntax_instruction &operator|=(mini_syntax_instruction const &rhs);
};

struct mini_syntax {
  std::vector<mini_syntax_instruction> children;
};

struct binary_op {
  binary_op(char op, mini_syntax_instruction const &left, mini_syntax_instruction const &right)
      : op(op), left(left), right(right) {}

  char op;
  mini_syntax_instruction left;
  mini_syntax_instruction right;
};

struct unary_op {
  unary_op(char op, mini_syntax_instruction const &subject) : op(op), subject(subject) {}

  char op;
  mini_syntax_instruction subject;
};
}

// We need to tell fusion about our mini_syntax struct
// to make it a first-class fusion citizen
BOOST_FUSION_ADAPT_STRUCT(AlgoParser::mini_syntax,
                          (std::vector<AlgoParser::mini_syntax_instruction>, children))

BOOST_FUSION_ADAPT_STRUCT(AlgoParser::mini_syntax_instruction, (AlgoParser::type, expr))
//]

namespace AlgoParser {

// Algo printer
struct mini_syntax_printer {
  mini_syntax_printer(tlp::Graph *g, std::map<std::string, std::string> &lhsNMap,
                      std::map<std::string, std::string> &lhsEMap,
                      std::map<std::string, std::string> &rhsNMap,
                      std::map<std::string, std::string> &rhsEMap);

  void operator()(mini_syntax const &syntax) const;

  tlp::Graph *graph;
  std::map<std::string, std::string> lhsNodeMap, lhsEdgeMap, rhsNodeMap, rhsEdgeMap;
};

struct mini_syntax_ast_printer : boost::static_visitor<> {
  tlp::Graph *_g;
  mini_syntax_ast_printer(tlp::Graph *g);
  void operator()(mini_syntax_instruction const &inst) const;
  void operator()(binary_op const &bin) const;
  void operator()(unary_op const &un) const;
  void operator()(std::string const &text) const;
  void operator()(unsigned int const &uint) const;
  void operator()(double const &dbl) const;
};
}

// test :
// node(101).p("Arity") = edge(77).p("Arity") + 3;

#endif // RULEALGORITHMELEMENT_H
