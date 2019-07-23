#include "RuleConditionElement.h"

#include <iostream>

#include <tulip/Graph.h>
#include <tulip/IntegerProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/BooleanProperty.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortNode.h>
#include <portgraph/PortGraphModel.h>

using namespace ConditionParser;
using namespace std;
using namespace tlp;

namespace {
    string global_tmp_id;
    string global_tmp_prop;
    string global_tmp_element_type; // ndef, node, edge
    string global_type; // ndef, int, double, string
    string global_tmp_str_result;
    int global_tmp_int_result;
    double global_tmp_double_result;
    bool global_tmp_bool_result;
    map<std::string,std::string> global_lhsNodeMap, global_lhsEdgeMap;
    tlp::Graph *global_g_model;
}

mini_syntax_condition& mini_syntax_condition::operator+(mini_syntax_condition const& rhs) {
    expr = binary_op("+", expr, rhs);
    return *this;
}

mini_syntax_condition& mini_syntax_condition::operator-(mini_syntax_condition const& rhs) {
    expr = binary_op("-", expr, rhs);
    return *this;
}

mini_syntax_condition& mini_syntax_condition::operator*(mini_syntax_condition const& rhs) {
    expr = binary_op("*", expr, rhs);
    return *this;
}

mini_syntax_condition& mini_syntax_condition::operator/(mini_syntax_condition const& rhs) {
    expr = binary_op("/", expr, rhs);
    return *this;
}

mini_syntax_condition& mini_syntax_condition::operator%(mini_syntax_condition const& rhs) {
    expr = binary_op(".", expr, rhs);
    return *this;
}

mini_syntax_printer::mini_syntax_printer(tlp::Graph *g,
                    std::map<std::string, std::string> &lhsNMap,
                    std::map<std::string, std::string> &lhsEMap,
                    tlp::Graph *mdl)
                   : graph(g), lhsNodeMap(lhsNMap), lhsEdgeMap(lhsEMap), g_model(mdl)
{ }

mini_syntax_ast_printer::mini_syntax_ast_printer(tlp::Graph *g): _g(g) { }

bool mini_syntax_printer::operator()(mini_syntax const& syntax) const {
    global_lhsNodeMap = lhsNodeMap;
    global_lhsEdgeMap = lhsEdgeMap;
    global_g_model = g_model;
    
    bool res = false;
    for(auto const& node: syntax.children) {
        global_tmp_id = "";
        global_tmp_prop = "";
        global_tmp_element_type = "ndef"; // ndef, node, edge
        global_type = "ndef"; // ndef, int, double, string, bool
        global_tmp_str_result = "";
        global_tmp_int_result = 0;
        global_tmp_double_result = 0;
        global_tmp_bool_result = false;
        res = boost::apply_visitor(mini_syntax_ast_printer(graph), node.expr);
        if (!res)
            return false;
    }
    return res;
}

bool mini_syntax_ast_printer::operator()(mini_syntax_condition const& cond) const {
    //tlp::debug() << "mini_syntax_condition found." << endl;
    return boost::apply_visitor(mini_syntax_ast_printer(_g), cond.expr);
}

bool mini_syntax_ast_printer::operator()(binary_op const& bin) const {
    mini_syntax_ast_printer lhs(_g);
    mini_syntax_ast_printer rhs(_g);

    boost::apply_visitor(lhs, bin.left.expr);

    std::string lhs_id = global_tmp_id;
    std::string lhs_prop = global_tmp_prop;
    std::string lhs_element_type = global_tmp_element_type;
    std::string lhs_type = global_type;
    std::string lhs_str_result = global_tmp_str_result;
    int lhs_int_result = global_tmp_int_result;
    double lhs_double_result = global_tmp_double_result;

    boost::apply_visitor(rhs, bin.right.expr);

    //std::string rhs_id = global_tmp_id;
    std::string rhs_prop = global_tmp_prop;
    //std::string rhs_element_type =  global_tmp_element_type;
    std::string rhs_type = global_type;
    std::string rhs_str_result = global_tmp_str_result;
    int rhs_int_result = global_tmp_int_result;
    double rhs_double_result = global_tmp_double_result;

    if (lhs_type != "ndef" && lhs_type != rhs_type) {
        std::cerr << "binary_op: Incompatible types: " << lhs_type << " and " << rhs_type << "!" << std::endl;
        return false;
    }

    if (bin.op ==  "+")
    {
        //operation or concatenation
        if (lhs_type == "int")
            global_tmp_int_result = lhs_int_result + rhs_int_result;
        if (lhs_type == "double")
            global_tmp_double_result = lhs_double_result + rhs_double_result;
        if (lhs_type == "string")
            global_tmp_str_result = lhs_str_result + rhs_str_result;        
    }
    else if (bin.op ==  "-")
    {
        //operation
        if (lhs_type == "int")
            global_tmp_int_result = lhs_int_result - rhs_int_result;
        if (lhs_type == "double")
            global_tmp_double_result = lhs_double_result - rhs_double_result;        
    }
    else if (bin.op ==  "*")
    {
        //operation
        if (lhs_type == "int")
            global_tmp_int_result = lhs_int_result * rhs_int_result;
        if (lhs_type == "double")
            global_tmp_double_result = lhs_double_result * rhs_double_result;        
    }
    else if (bin.op ==  "/")
    {
        //operation
        if (lhs_type == "int")
            global_tmp_int_result = lhs_int_result / rhs_int_result;
        if (lhs_type == "double")
            global_tmp_double_result = lhs_double_result / rhs_double_result;        
    }
    else if (bin.op ==  ">") // else if (bin.op ==  Max
    {
        //operation
        if (lhs_type == "int") {
            if (lhs_int_result > rhs_int_result)
                global_tmp_int_result = lhs_int_result;
            else
                global_tmp_int_result = rhs_int_result;
        }
        if (lhs_type == "double") {
            if (lhs_double_result > rhs_double_result)
                global_tmp_double_result = lhs_double_result;
            else
                global_tmp_double_result = rhs_double_result;
        }
        if (lhs_type == "string") {
            if (lhs_str_result > rhs_str_result)
                global_tmp_str_result = lhs_str_result;
            else
                global_tmp_str_result = rhs_str_result;
        }        
    }
    else if (bin.op ==  "<") // else if (bin.op ==  Min
    {
        //operation
        if (lhs_type == "int") {
            if (lhs_int_result < rhs_int_result)
                global_tmp_int_result = lhs_int_result;
            else
                global_tmp_int_result = rhs_int_result;
        }
        if (lhs_type == "double") {
            if (lhs_double_result < rhs_double_result)
                global_tmp_double_result = lhs_double_result;
            else
                global_tmp_double_result = rhs_double_result;
        }
        if (lhs_type == "string") {
            if (lhs_str_result < rhs_str_result)
                global_tmp_str_result = lhs_str_result;
            else
                global_tmp_str_result = rhs_str_result;
        }        
    }
    else if (bin.op ==  ".")
    {
        //lhs: e/n, rhs: p
        tlp::PropertyInterface* propI = _g->getProperty(rhs_prop);
        std::string result_string;
        int lhs_id_int;

        if (lhs_element_type == "node") {
            if (global_lhsNodeMap.find(lhs_id) == global_lhsNodeMap.end()) {
                // element from the rhs of the rule
                result_string = "0";
            } else {
                std::istringstream(global_lhsNodeMap.find(lhs_id)->second) >> lhs_id_int;
                result_string = propI->getNodeStringValue(tlp::node(lhs_id_int));
                global_type = propI->getTypename();
            }
        }
        if (lhs_element_type == "edge") {
            if (global_lhsEdgeMap.find(lhs_id) == global_lhsEdgeMap.end()) {
                // element from the rhs of the rule
                result_string = "0";
            } else {
                std::istringstream(global_lhsEdgeMap.find(lhs_id)->second) >> lhs_id_int;
                result_string = propI->getEdgeStringValue(tlp::edge(lhs_id_int));
                global_type = propI->getTypename();
            }
        }

        if (global_type == tlp::IntegerProperty::propertyTypename)
            std::istringstream(result_string) >> global_tmp_int_result;
        if (global_type == tlp::DoubleProperty::propertyTypename)
            std::istringstream(result_string) >> global_tmp_double_result;
        if (global_type == tlp::StringProperty::propertyTypename)
            global_tmp_str_result = result_string;    
    }
    
    return true;
}

bool mini_syntax_ast_printer::operator()(unary_op const& un) const {
    mini_syntax_ast_printer mhs(_g);
    std::string tmp_type = global_type;

    boost::apply_visitor(mhs, un.subject.expr);

    switch(un.op)
    {
        case 'e':
        {   // case "edge"
            global_tmp_element_type = "edge";
            global_tmp_id = global_tmp_str_result;
            global_type = tmp_type;
            break;
        }
        case 'n':
        {   // case "node"
            global_tmp_element_type = "node";
            global_tmp_id = global_tmp_str_result;
            global_type = tmp_type;
            break;
        }
        case 'p':
        {   // case "property"
            global_tmp_prop = global_tmp_str_result;
            tlp::PropertyInterface* pI = _g->getProperty(global_tmp_prop);
            global_type = pI->getTypename();
            if (tmp_type != "ndef" && tmp_type != global_type)
                std::cerr << "unary_op: Incompatible types: " << tmp_type << " and " << global_type << std::endl;
            break;
        }
        case '-':
        {   // case operator "minus"
            if (global_type == tlp::IntegerProperty::propertyTypename)
                global_tmp_int_result = -global_tmp_int_result;
            if (global_type == tlp::DoubleProperty::propertyTypename)
                global_tmp_double_result = -global_tmp_double_result;
            break;
        }
        case 't':
        {
            return true;
            break;
        }
        case 'f':
        {
            return false;
            break;
        }
        case 'r':
        {   // case "random"
            uniform_real_distribution<double> choose(0, global_tmp_double_result);
            global_tmp_double_result = choose(PorgyTlpGraphStructure::gen);
            global_type = tlp::DoubleProperty::propertyTypename;
            break;
        }
    }
    
    return true;
}

bool mini_syntax_ast_printer::operator()(std::string const& text) const {
    global_tmp_str_result = text;
    global_type = tlp::StringProperty::propertyTypename;
    return true;
}

bool mini_syntax_ast_printer::operator()(int const& i) const {
    global_tmp_int_result = i;
    global_type = tlp::IntegerProperty::propertyTypename;
    return true;
}

bool mini_syntax_ast_printer::operator()(double const& dbl) const {
    global_tmp_double_result = dbl;
    global_type = tlp::DoubleProperty::propertyTypename;
    return true;
}

bool mini_syntax_ast_printer::operator()(bool const& bl) const {
    global_tmp_bool_result = bl;
    global_type = tlp::BooleanProperty::propertyTypename;
    return bl;
}

bool mini_syntax_ast_printer::operator()(comp_op const& comp) const {
    //tlp::debug() << "comp_op found." << endl;
    
    mini_syntax_ast_printer lhs(_g);
    mini_syntax_ast_printer rhs(_g);

    bool lhs_bool_result = boost::apply_visitor(lhs, comp.left.expr);    
    std::string lhs_type = global_type;
    std::string lhs_str_result = global_tmp_str_result;
    int lhs_int_result = global_tmp_int_result;
    double lhs_double_result = global_tmp_double_result;

    bool rhs_bool_result = boost::apply_visitor(rhs, comp.right.expr);
    std::string rhs_type = global_type;
    std::string rhs_str_result = global_tmp_str_result;
    int rhs_int_result = global_tmp_int_result;
    double rhs_double_result = global_tmp_double_result;

    if (lhs_type != "ndef" && lhs_type != rhs_type) {
        std::cerr << "comp_op: Incompatible types: " << lhs_type << " and " << rhs_type << "!" << std::endl;
        return false;
    }
    
    if (comp.op == "==") {
        if (lhs_type == tlp::IntegerProperty::propertyTypename)
            return (lhs_int_result == rhs_int_result);
        else if (lhs_type == tlp::DoubleProperty::propertyTypename)
            return (lhs_double_result == rhs_double_result);
        else if (lhs_type == tlp::StringProperty::propertyTypename)
            return (lhs_str_result == rhs_str_result);
        else if (lhs_type == tlp::BooleanProperty::propertyTypename)
            return ( (lhs_bool_result && rhs_bool_result) || (!lhs_bool_result && !rhs_bool_result) );
    }
    else if (comp.op == "!=") {
        if (lhs_type == tlp::IntegerProperty::propertyTypename)
            return (lhs_int_result != rhs_int_result);
        else if (lhs_type == tlp::DoubleProperty::propertyTypename)
            return (lhs_double_result != rhs_double_result);
        else if (lhs_type == tlp::StringProperty::propertyTypename)
            return (lhs_str_result != rhs_str_result);
        else if (lhs_type == tlp::BooleanProperty::propertyTypename)
            return (lhs_bool_result ^ rhs_bool_result);
    }
    else if (comp.op == ">=") {
        if (lhs_type == tlp::IntegerProperty::propertyTypename)
            return (lhs_int_result >= rhs_int_result);
        else if (lhs_type == tlp::DoubleProperty::propertyTypename)
            return (lhs_double_result >= rhs_double_result);
        else if (lhs_type == tlp::StringProperty::propertyTypename)
            return (lhs_str_result >= rhs_str_result);
    }
    else if (comp.op == ">") {
        if (lhs_type == tlp::IntegerProperty::propertyTypename)
            return (lhs_int_result > rhs_int_result);
        else if (lhs_type == tlp::DoubleProperty::propertyTypename)
            return (lhs_double_result > rhs_double_result);
        else if (lhs_type == tlp::StringProperty::propertyTypename)
            return (lhs_str_result > rhs_str_result);
    }
    else if (comp.op == "<=") {
        if (lhs_type == tlp::IntegerProperty::propertyTypename)
            return (lhs_int_result <= rhs_int_result);
        else if (lhs_type == tlp::DoubleProperty::propertyTypename)
            return (lhs_double_result <= rhs_double_result);
        else if (lhs_type == tlp::StringProperty::propertyTypename)
            return (lhs_str_result <= rhs_str_result);
    }
    else if (comp.op == "<") {
        if (lhs_type == tlp::IntegerProperty::propertyTypename)
            return (lhs_int_result < rhs_int_result);
        else if (lhs_type == tlp::DoubleProperty::propertyTypename)
            return (lhs_double_result < rhs_double_result);
        else if (lhs_type == tlp::StringProperty::propertyTypename)
            return (lhs_str_result < rhs_str_result);
    }
 
    return false;
}

bool mini_syntax_ast_printer::operator()(logical_binary_op const& log_bin) const {
    //tlp::debug() << "logical_binary_op found." << endl;
    
    mini_syntax_ast_printer lhs(_g);
    mini_syntax_ast_printer rhs(_g);

    bool res_left = boost::apply_visitor(lhs, log_bin.left.expr);
    std::string lhs_type = global_type;
    
    bool res_right = boost::apply_visitor(lhs, log_bin.right.expr);
    std::string rhs_type = global_type;
    
    if (lhs_type != "ndef" && lhs_type != rhs_type) {
        std::cerr << "logical_binary_op: Incompatible types: " << lhs_type << " and " << rhs_type << "!" << std::endl;
        return false;
    }
    
    global_type = tlp::BooleanProperty::propertyTypename;
    
    if (log_bin.op == "&&") {
        return (res_left && res_right);
    }
    else if (log_bin.op == "||") {
        return (res_left || res_right);
    }
    
    return false;
}

bool mini_syntax_ast_printer::operator()(logical_unary_op const& log_un) const {
    //tlp::debug() << "logical_unary_op found." << endl;
    
    mini_syntax_ast_printer mhs(_g);
    std::string tmp_type = global_type;

    bool res = boost::apply_visitor(mhs, log_un.subject.expr);
    
    if (tmp_type != tlp::BooleanProperty::propertyTypename) {
        std::cerr << "logical_unary_op: Incompatible types: " << tmp_type << " and bool!" << std::endl;
        return false;
    }
    
    global_type = tlp::BooleanProperty::propertyTypename;
    
    if (log_un.op == '!')
        return !res;
        
    return false;    
}

bool mini_syntax_ast_printer::operator()(not_node_op const& notnode) const {
    if (_debug)
      tlp::debug() << "NotNode() found." << endl;
    /*
    This function evaluates NotNode() conditions.
    It will return TRUE if on every node the condition evaluates to FALSE. (There isn't a node.)
    It will return FALSE if it finds at least one node on which the condition expression evaluates to TRUE.
    */
    mini_syntax_ast_printer prop_name_eval(_g);
    boost:apply_visitor(prop_name_eval, notnode.prop_name.expr);
    std::string prop_name = global_tmp_str_result;
    
    mini_syntax_ast_printer expr_eval(_g);
    boost::apply_visitor(expr_eval, notnode.value_exp.expr);  
    
    std::string expr_type = global_type;
    std::string expr_str_result = global_tmp_str_result;
    double expr_double_int_result = std::numeric_limits<double>::max();
    if (expr_type == tlp::IntegerProperty::propertyTypename)
        expr_double_int_result = global_tmp_int_result;
    else if (expr_type == tlp::DoubleProperty::propertyTypename)
        expr_double_int_result = global_tmp_double_result;
    
    tlp::PropertyInterface* propI = _g->getProperty(prop_name);
    std::string prop_type = propI->getTypename();

    if (expr_type != "ndef" && expr_type != prop_type) {
        std::cerr << "NotNode: Incompatible types! Expression: " << expr_type 
                  << ", Property: " << prop_type << "." << std::endl;
        return false;
    }
    
    PortGraphModel pg(global_g_model);
    
    if (prop_type == tlp::IntegerProperty::propertyTypename || prop_type == tlp::DoubleProperty::propertyTypename) {
        for (PortNode *pn : pg.getPortNodes()) {
            node c(pn->getCenter());
            double node_val = static_cast<NumericProperty*>(propI)->getNodeDoubleValue(c);
            if (notnode.op == "==") {
                if (node_val == expr_double_int_result)
                    return false;
            }
            else if (notnode.op == "!=") {
                if (node_val != expr_double_int_result)
                    return false;
            }
            else if (notnode.op == ">=") {
                if (node_val >= expr_double_int_result)
                    return false;
            }
            else if (notnode.op == ">") {
                if (node_val > expr_double_int_result)
                    return false;
            }
            else if (notnode.op == "<=") {
                if (node_val <= expr_double_int_result)
                    return false;
            }
            else if (notnode.op == "<") {
                if (node_val < expr_double_int_result)
                    return false;
            }
        }
    }
    else if (prop_type == tlp::StringProperty::propertyTypename) {
        for (PortNode *pn : pg.getPortNodes()) {
            node c(pn->getCenter());
            string node_str_val = propI->getNodeStringValue(c);
            if (notnode.op == "==") {
                if (node_str_val == expr_str_result)
                    return false;
            }
            else if (notnode.op == "!=") {
                if (node_str_val != expr_str_result)
                    return false;
            }
            else if (notnode.op == ">=") {
                if (node_str_val >= expr_str_result)
                    return false;
            }
            else if (notnode.op == ">") {
                if (node_str_val > expr_str_result)
                    return false;
            }
            else if (notnode.op == "<=") {
                if (node_str_val <= expr_str_result)
                    return false;
            }
            else if (notnode.op == "<") {
                if (node_str_val < expr_str_result)
                    return false;
            }
        }
    }
            
    return true;
}

bool mini_syntax_ast_printer::operator()(nil const& n) const {
    tlp::debug() << "nil found." << endl;
    return false;
}