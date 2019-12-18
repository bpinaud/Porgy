#ifndef RULECONDITIONELEMENT_H
#define RULECONDITIONELEMENT_H

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include <map>
#include <string>
#include <vector>

namespace tlp {
  class Graph;
}

namespace ConditionParser {
    struct nil {};
    struct mini_syntax;
    struct mini_syntax_condition;
    struct binary_op;
    struct unary_op;
    struct comp_op;
    struct logical_unary_op;
    struct logical_binary_op;
    struct not_node_op;

    typedef
        boost::variant<
            nil,
            boost::recursive_wrapper<mini_syntax>,
            boost::recursive_wrapper<mini_syntax_condition>,
            boost::recursive_wrapper<binary_op>,
            boost::recursive_wrapper<unary_op>,
            boost::recursive_wrapper<comp_op>,
            boost::recursive_wrapper<logical_binary_op>,
            boost::recursive_wrapper<logical_unary_op>,
            boost::recursive_wrapper<not_node_op>,
            int,
            double,
            bool,
            std::string
        >
    type;
    
    struct mini_syntax_condition
    {
        type expr;

        mini_syntax_condition() : expr(nil()) {}

        template <typename Expr>
        mini_syntax_condition(Expr const& expr) : expr(expr) {}

        mini_syntax_condition& operator+(mini_syntax_condition const& rhs);
        mini_syntax_condition& operator-(mini_syntax_condition const& rhs);
        mini_syntax_condition& operator*(mini_syntax_condition const& rhs);
        mini_syntax_condition& operator/(mini_syntax_condition const& rhs);
        mini_syntax_condition& operator%(mini_syntax_condition const& rhs);
    };

    struct mini_syntax
    {
        std::vector<mini_syntax_condition> children;
    };

    struct binary_op
    {
        std::string op;
        mini_syntax_condition left;
        mini_syntax_condition right;
        binary_op(std::string op, mini_syntax_condition const& left, mini_syntax_condition const& right) : op(op), left(left), right(right) {}
    };

    struct unary_op
    {
        char op;
        mini_syntax_condition subject;
        unary_op(char op, mini_syntax_condition const& subject) : op(op), subject(subject) {}
    };
    
    struct comp_op
    {
        std::string op;
        mini_syntax_condition left;
        mini_syntax_condition right;
        comp_op(std::string op, mini_syntax_condition const& left, mini_syntax_condition const& right) : op(op), left(left), right(right) {}
    };
    
    struct logical_unary_op
    {
        char op;
        mini_syntax_condition subject;
        logical_unary_op(char op, mini_syntax_condition const& subject) : op(op), subject(subject) {}
    };
    
    struct logical_binary_op
    {
        std::string op;
        mini_syntax_condition left;
        mini_syntax_condition right;
        logical_binary_op(std::string op, mini_syntax_condition const& left, mini_syntax_condition const& right) : op(op), left(left), right(right) {}
    };
    
    struct not_node_op
    {
        std::string op;
        mini_syntax_condition prop_name;
        mini_syntax_condition value_exp;
        not_node_op(std::string op, mini_syntax_condition const& pn, mini_syntax_condition const& e) : op(op), prop_name(pn), value_exp(e) {}
    };
   
}

// We need to tell fusion about our mini_syntax struct
// to make it a first-class fusion citizen
BOOST_FUSION_ADAPT_STRUCT( ConditionParser::mini_syntax, (std::vector<ConditionParser::mini_syntax_condition>, children) )
BOOST_FUSION_ADAPT_STRUCT( ConditionParser::mini_syntax_condition, (ConditionParser::type, expr) )
BOOST_FUSION_ADAPT_STRUCT( ConditionParser::binary_op, (std::string, op)(ConditionParser::mini_syntax_condition, left)(ConditionParser::mini_syntax_condition, right) )
BOOST_FUSION_ADAPT_STRUCT( ConditionParser::unary_op, (char, op)(ConditionParser::mini_syntax_condition, subject) )
BOOST_FUSION_ADAPT_STRUCT( ConditionParser::comp_op, (std::string, op)(ConditionParser::mini_syntax_condition, left)(ConditionParser::mini_syntax_condition, right) )
BOOST_FUSION_ADAPT_STRUCT( ConditionParser::logical_unary_op, (char, op)(ConditionParser::mini_syntax_condition, subject) )
BOOST_FUSION_ADAPT_STRUCT( ConditionParser::logical_binary_op, (std::string, op)(ConditionParser::mini_syntax_condition, left)(ConditionParser::mini_syntax_condition, right) )
BOOST_FUSION_ADAPT_STRUCT( ConditionParser::not_node_op, (std::string, op)(ConditionParser::mini_syntax_condition, prop_name)(ConditionParser::mini_syntax_condition, value_exp) )

namespace ConditionParser {

    // printer
    struct mini_syntax_printer {
        tlp::Graph *graph;
        std::map<std::string,std::string> lhsNodeMap;
        std::map<std::string,std::string> lhsEdgeMap;
        tlp::Graph *g_model;
        
        mini_syntax_printer(tlp::Graph *g, std::map<std::string,std::string> &lhsNMap, std::map<std::string,std::string> &lhsEMap, tlp::Graph *mdl);
        
        bool operator()(mini_syntax const& syntax) const;
    };

    struct mini_syntax_ast_printer : boost::static_visitor<bool> {
        tlp::Graph *_g;
        
        explicit mini_syntax_ast_printer(tlp::Graph *g);
        
        bool operator()(mini_syntax_condition const&) const;
        bool operator()(binary_op const&) const;
        bool operator()(unary_op const&) const;
        bool operator()(comp_op const&) const;
        bool operator()(logical_binary_op const&) const;
        bool operator()(logical_unary_op const&) const;
        bool operator()(not_node_op const&) const;
        bool operator()(std::string const&) const;
        bool operator()(int const&) const;
        bool operator()(double const&) const;
        bool operator()(bool const&) const;
        bool operator()(nil const&) const;
    };
    
}

#endif // RULECONDITIONELEMENT_H
