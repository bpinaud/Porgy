#ifndef RULECONDITIONPARSER_H
#define RULECONDITIONPARSER_H

#include "RuleConditionElement.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace qi  = boost::spirit::qi;
namespace phx = boost::phoenix;

namespace ConditionParser
{
    //grammar to handle comments, tab or space
    template <typename Iterator>
    struct comments : boost::spirit::qi::grammar<Iterator> {
        comments():comments::base_type(start) 
        {
            using boost::spirit::ascii::char_;
            using boost::spirit::qi::lit;

            start = boost::spirit::ascii::space  // tab/space/cr/lf
                    | lit("/*") >> *(char_ - lit("*/")) >> lit("*/")                 // C++-style comments
                    | lit("//") >> *(char_ - boost::spirit::qi::eol);
        }
        boost::spirit::qi::rule<Iterator> start;
    };

    struct negate_expr {
      template <typename T>
      struct result { typedef T type; };

      mini_syntax_condition operator()(mini_syntax_condition const& expr) const
      {
          return mini_syntax_condition(unary_op('!', expr));
      }
    };
    phx::function<negate_expr> neg;

    struct node_expr {
      template <typename T>
      struct result { typedef T type; };

      mini_syntax_condition operator()(mini_syntax_condition const& expr) const
      {
          return mini_syntax_condition(unary_op('n', expr));
      }
    };
    phx::function<node_expr> set_node;

    struct edge_expr {
      template <typename T>
      struct result { typedef T type; };

      mini_syntax_condition operator()(mini_syntax_condition const& expr) const
      {
          return mini_syntax_condition(unary_op('e', expr));
      }
    };
    phx::function<edge_expr> set_edge;

    struct property_expr {
      template <typename T>
      struct result { typedef T type; };

      mini_syntax_condition operator()(mini_syntax_condition const& expr) const
      {
          return mini_syntax_condition(unary_op('p', expr));
      }
    };
    phx::function<property_expr> set_prop;

    struct true_expr {
      template <typename T>
      struct result { typedef T type; };

      mini_syntax_condition operator()(mini_syntax_condition const& expr) const
      {
          return mini_syntax_condition(unary_op('t', expr));
      }
    };
    phx::function<true_expr> set_true;

    struct false_expr {
      template <typename T>
      struct result { typedef T type; };

      mini_syntax_condition operator()(mini_syntax_condition const& expr) const
      {
          return mini_syntax_condition(unary_op('f', expr));
      }
    };
    phx::function<false_expr> set_false;

    struct max_expr {
      template <typename T1, typename T2>
      struct result { typedef T1 type; };

      mini_syntax_condition operator()(mini_syntax_condition const& expr1, mini_syntax_condition const& expr2) const
      {
          return mini_syntax_condition(binary_op(">", expr1, expr2));
      }
    };
    phx::function<max_expr> set_max;

    struct min_expr {
      template <typename T1, typename T2>
      struct result { typedef T1 type; };

      mini_syntax_condition operator()(mini_syntax_condition const& expr1, mini_syntax_condition const& expr2) const
      {
          return mini_syntax_condition(binary_op("<", expr1, expr2));
      }
    };
    phx::function<min_expr> set_min;

    struct random_expr {
      template <typename T>
      struct result { typedef T type; };

      mini_syntax_condition operator()(mini_syntax_condition const& expr) const
      {
          return mini_syntax_condition(unary_op('r', expr));
      }
    };
    phx::function<random_expr> set_random;
    
    struct comparison_operator {
        template <typename T>
        struct result { typedef T type; };

        mini_syntax_condition operator()(std::string op, mini_syntax_condition const& expr1, mini_syntax_condition const& expr2) const
        {
            return mini_syntax_condition(comp_op(op, expr1, expr2));
        }
    };
    phx::function<comparison_operator> fn_comp_op;
    
    struct op_not {
        template <typename T> struct result { typedef T type; };

        mini_syntax_condition operator()(mini_syntax_condition const& expr) const
        {
            return mini_syntax_condition(logical_unary_op('!', expr));
        }
    };
    phx::function<op_not> log_op_not;
    
    struct logical_operator {
        template <typename T> struct result { typedef T type; };

        mini_syntax_condition operator()(std::string op, mini_syntax_condition const& expr1, mini_syntax_condition const& expr2) const
        {
            return mini_syntax_condition(logical_binary_op(op, expr1, expr2));
        }
    };
    phx::function<logical_operator> fn_log_op;
    
    struct not_node {
        template <typename T> struct result { typedef T type; };
        
        mini_syntax_condition operator()(std::string op, mini_syntax_condition const& propname, mini_syntax_condition const& expr) const
        {
            return mini_syntax_condition(not_node_op(op, propname, expr));
        }
    };
    phx::function<not_node> fn_not_node;
    
    bool parse(const std::string &cond,
                     std::string &errMsg,
                     std::vector<std::string> propertynames,
                     std::vector<std::string> lhsNodesId,
                     std::vector<std::string> lhsEdgesId,
                     ConditionParser::mini_syntax &ast);

    //Grammar definition
    template <typename Iterator>
    struct mini_syntax_grammar : qi::grammar<Iterator, mini_syntax(), comments<Iterator>> {
        typedef comments<Iterator> comments_;
    
        mini_syntax_grammar(std::string& errMsg,
                            const std::vector<std::string> &propertynames,
                            const std::vector<std::string> &lhsNodesId,
                            const std::vector<std::string> &lhsEdgesId
            ) :
            mini_syntax_grammar::base_type(start),
            errMsg(errMsg),
            sym_properties(propertynames, propertynames, "valid property names"),
            sym_lhs_nodes(lhsNodesId, lhsNodesId, "valid lhs node Ids"),
            sym_lhs_edges(lhsEdgesId, lhsEdgesId, "valid lhs edge Ids")
        {
            using namespace qi;
            using namespace phx;

            node_id =
                    sym_lhs_nodes               [_val = _1]
                ;

            edge_id =
                    sym_lhs_edges               [_val = _1]
                ;

            prop_id =
                    sym_properties              [_val = _1]
                ;

            quoted_text =
                    '"'
                >>     lexeme[+(char_ - '"')    [_val += _1]]
                >>     '"'
                ;

            quoted_text_str =
                    quoted_text                 [at_c<0>(_val) = _1];

            node =
                    (lit("node") | lit("n"))
                >>  '('
                >>     node_id                  [_val = set_node(_1)]
                >>  ')'
                ;

            edge =
                    (lit("edge") | lit("e"))
                >>  '('
                >>     edge_id                  [_val = set_edge(_1)]
                >>  ')'
                ;

            property =
                '"'
                >>     prop_id                  [_val = set_prop(_1)]
                >>  '"'
                ;

            element_property =
                    (node | edge)               [_val = _1]
                >>    '.'
                >>     property                 [_val = _val % _1]
                ;
           
            expression =
                term                            [_val = _1]
                >> *(   ('+' >> term            [_val = _val + _1])
                      | ('-' >> term            [_val = _val - _1])
                    )
                ;

            term =
                factor                          [_val = _1]
                >> *(   ('*' >> factor          [_val = _val * _1])
                      | ('/' >> factor          [_val = _val / _1])
                      | ('%' >> factor          [_val = _val % _1])
                    )
                ;

            factor =
                    int_                        [_val = _1]
                |   double_                     [_val = _1]
                |   ('(' >> expression          [_val = _1] >> ')')
                |   ('!' >> factor              [_val = neg(_1)])
                |   ('+' >> factor              [_val = _1])
                |   (element_property           [_val = _1])
                |   (quoted_text_str            [_val = _1])
                |   ((lit("max(") >> expression >> ',' >> expression >> ')')    [_val = set_max(_1, _2)])
                |   ((lit("min(") >> expression >> ',' >> expression >> ')')    [_val = set_min(_1, _2)])
                |   (lit("random(") >> factor   [_val = set_random(_1)] >> ')')
                ;

            comparison = 
                   (expression                  [ _a = _1 ] 
                >> (
                    (lit("==") >> expression)   [ _val = fn_comp_op("==", _a, _1)] 
                |   (lit("!=") >> expression)   [ _val = fn_comp_op("!=", _a, _1)]
                |   (lit(">") >> expression)    [ _val = fn_comp_op(">", _a, _1) ] 
                |   (lit("<") >> expression)    [ _val = fn_comp_op("<", _a, _1) ] 
                |   (lit(">=") >> expression)   [ _val = fn_comp_op(">=", _a, _1)] 
                |   (lit("<=") >> expression)   [ _val = fn_comp_op("<=", _a, _1)]
                ))
                |   (lit("NotNode(") >> property             [ _a = _1 ] 
                >> (
                        (lit("==") >> expression >> ')')     [ _val = fn_not_node("==", _a, _1) ]
                    |   (lit("!=") >> expression >> ')')     [ _val = fn_not_node("!=", _a, _1) ]
                    |   (lit(">=") >> expression >> ')')     [ _val = fn_not_node(">=", _a, _1) ]
                    |   (lit(">") >> expression >> ')')      [ _val = fn_not_node(">", _a, _1)  ]
                    |   (lit("<=") >> expression >> ')')     [ _val = fn_not_node("<=", _a, _1) ]
                    |   (lit("<") >> expression >> ')')      [ _val = fn_not_node("<", _a, _1)  ]
                ))
                ;
                
            // TODO: implement this as logical parsing: 
            // https://cs.stackexchange.com/questions/10558/grammar-for-describing-boolean-expressions-with-and-or-and-not
            //condition =
            //        ('!' >> condition)          [ _val = log_op_not(_1) ] 
            //    |   comparison                  [ _a = _1] 
            //    >> *(
            //       | (lit("||") >> condition    [ _val = fn_log_op("||", _a, _1) ])
            //    );
                            
            start = *(comparison                   [push_back(at_c<0>(_val), _1)] >> ';');
            
            start.name("start");
            comparison.name("comparison");
            condition.name("condition");
            expression.name("expression");
            term.name("term");
            factor.name("factor");
            node.name("node");
            edge.name("edge");
            property.name("property");
            element_property.name("element_property");
            edge_id.name("edge_id");
            node_id.name("node_id");
            prop_id.name("prop_id");
            quoted_text_str.name("quoted_text_str");
            quoted_text.name("quoted_text");

            qi::on_error<qi::fail>
            (
                start,
                phx::ref(errMsg) =
                    (phx::construct<std::string>(phx::val("Error! Expecting "))
                    + phx::bind(&info::tag, qi::_4)      //what failed
                    + phx::construct<std::string>(phx::val(" here: \"") )
                    + phx::construct<std::string>(qi::_3, qi::_2)
                    + phx::construct<std::string>(phx::val("\"") )
                    )
            );
        }

        std::string& errMsg;

        qi::symbols<char, std::string> sym_properties, sym_lhs_nodes, sym_lhs_edges;
        qi::rule<Iterator, mini_syntax(), comments_> start;
        qi::rule<Iterator, mini_syntax_condition(), comments_, qi::locals<type>> comparison, condition, expression, term, factor, node, edge, property, element_property, quoted_text_str, node_id, edge_id, prop_id;
        qi::rule<Iterator, std::string(), comments_> quoted_text;
    };
}


#endif // RULECONDITIONPARSER_H
