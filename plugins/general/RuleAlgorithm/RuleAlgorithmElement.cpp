#include "RuleAlgorithmElement.h"

#include <iostream>

#include <tulip/DoubleProperty.h>
#include <tulip/Graph.h>
#include <tulip/IntegerProperty.h>
#include <tulip/StringProperty.h>

#include <portgraph/PorgyTlpGraphStructure.h>

using namespace AlgoParser;

namespace {
  std::string global_tmp_id;
  std::string global_tmp_prop;
  std::string global_tmp_element_type; // ndef, node, edge
  std::string global_type;             // ndef, int, double, string
  std::string global_tmp_str_result;
  int global_tmp_int_result;
  double global_tmp_double_result;
  std::map<std::string, std::string> global_lhsNodeMap, global_lhsEdgeMap, global_rhsNodeMap,
    global_rhsEdgeMap;
}

mini_syntax_instruction &mini_syntax_instruction::operator+(mini_syntax_instruction const &rhs) {
  expr = binary_op('+', expr, rhs);
  return *this;
}

mini_syntax_instruction &mini_syntax_instruction::operator-(mini_syntax_instruction const &rhs) {
  expr = binary_op('-', expr, rhs);
  return *this;
}

mini_syntax_instruction &mini_syntax_instruction::operator*(mini_syntax_instruction const &rhs) {
  expr = binary_op('*', expr, rhs);
  return *this;
}

mini_syntax_instruction &mini_syntax_instruction::operator/(mini_syntax_instruction const &rhs) {
  expr = binary_op('/', expr, rhs);
  return *this;
}

mini_syntax_instruction &mini_syntax_instruction::operator%(mini_syntax_instruction const &rhs) {
  expr = binary_op('.', expr, rhs);
  return *this;
}

mini_syntax_instruction &mini_syntax_instruction::operator|=(mini_syntax_instruction const &rhs) {
  expr = binary_op('=', expr, rhs);
  return *this;
}

mini_syntax_printer::mini_syntax_printer(tlp::Graph *g, std::map<std::string, std::string> &lhsNMap,
                                         std::map<std::string, std::string> &lhsEMap,
                                         std::map<std::string, std::string> &rhsNMap,
                                         std::map<std::string, std::string> &rhsEMap)
    : graph(g), lhsNodeMap(lhsNMap), lhsEdgeMap(lhsEMap), rhsNodeMap(rhsNMap), rhsEdgeMap(rhsEMap) {
}

void mini_syntax_printer::operator()(mini_syntax const &syntax) const {
  global_lhsNodeMap = lhsNodeMap;
  global_lhsEdgeMap = lhsEdgeMap;
  global_rhsNodeMap = rhsNodeMap;
  global_rhsEdgeMap = rhsEdgeMap;

  for (auto node : syntax.children) {
    global_tmp_id = "";
    global_tmp_prop = "";
    global_tmp_element_type = "ndef"; // ndef, node, edge
    global_type = "ndef";             // ndef, int, double, string
    global_tmp_str_result = "";
    global_tmp_int_result = 0;
    global_tmp_double_result = 0;
    boost::apply_visitor(mini_syntax_ast_printer(graph), node.expr);
  }
}

void mini_syntax_ast_printer::operator()(binary_op const &bin) const {
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

  // std::string rhs_id = global_tmp_id;
  std::string rhs_prop = global_tmp_prop;
  // std::string rhs_element_type =  global_tmp_element_type;
  std::string rhs_type = global_type;
  std::string rhs_str_result = global_tmp_str_result;
  int rhs_int_result = global_tmp_int_result;
  double rhs_double_result = global_tmp_double_result;

  if (lhs_type != "ndef" && lhs_type != rhs_type) {
    std::cerr << "BinaryOp Uncompatible types: " << lhs_type << " and " << rhs_type << std::endl;
    return;
  }

  switch (bin.op) {
  case '=': {
    // affectation
    tlp::PropertyInterface *propI = _g->getProperty(lhs_prop);
    std::stringstream rhs_result_stream;
    if (rhs_type == "int")
      rhs_result_stream << rhs_int_result;
    if (rhs_type == "double")
      rhs_result_stream << rhs_double_result;
    if (rhs_type == "string")
      rhs_result_stream << rhs_str_result;

    int lhs_id_int;
    // The result is affected to an element from the rhs of the rule (proved)
    if (lhs_element_type == "node") {
      std::istringstream(global_rhsNodeMap.find(lhs_id)->second) >> lhs_id_int;
      propI->setNodeStringValue(tlp::node(lhs_id_int), rhs_result_stream.str());
    }
    if (lhs_element_type == "edge") {
      std::istringstream(global_rhsEdgeMap.find(lhs_id)->second) >> lhs_id_int;
      propI->setEdgeStringValue(tlp::edge(lhs_id_int), rhs_result_stream.str());
    }

    break;
  }
  case '+': {
    // operation or concatenation
    if (lhs_type == "int")
      global_tmp_int_result = lhs_int_result + rhs_int_result;
    if (lhs_type == "double")
      global_tmp_double_result = lhs_double_result + rhs_double_result;
    if (lhs_type == "string")
      global_tmp_str_result = lhs_str_result + rhs_str_result;
    break;
  }
  case '-': {
    // operation
    if (lhs_type == "int")
      global_tmp_int_result = lhs_int_result - rhs_int_result;
    if (lhs_type == "double")
      global_tmp_double_result = lhs_double_result - rhs_double_result;
    break;
  }
  case '*': {
    // operation
    if (lhs_type == "int")
      global_tmp_int_result = lhs_int_result * rhs_int_result;
    if (lhs_type == "double")
      global_tmp_double_result = lhs_double_result * rhs_double_result;
    break;
  }
  case '/': {
    // operation
    if (lhs_type == "int")
      global_tmp_int_result = lhs_int_result / rhs_int_result;
    if (lhs_type == "double")
      global_tmp_double_result = lhs_double_result / rhs_double_result;
    break;
  }
  case '>': // case Max
  {
    // operation
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
    break;
  }
  case '<': // case Min
  {
    // operation
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
    break;
  }
  case '.': {
    // lhs: e/n, rhs: p
    tlp::PropertyInterface *propI = _g->getProperty(rhs_prop);
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

    break;
  }
  }
}

void mini_syntax_ast_printer::operator()(unary_op const &un) const {
  mini_syntax_ast_printer mhs(_g);
  std::string tmp_type = global_type;

  boost::apply_visitor(mhs, un.subject.expr);

  switch (un.op) {
  case 'e': { // case "edge"
    global_tmp_element_type = "edge";
    global_tmp_id = global_tmp_str_result;
    global_type = tmp_type;
    break;
  }
  case 'n': { // case "node"
    global_tmp_element_type = "node";
    global_tmp_id = global_tmp_str_result;
    global_type = tmp_type;
    break;
  }
  case 'p': { // case "property"
    global_tmp_prop = global_tmp_str_result;
    tlp::PropertyInterface *pI = _g->getProperty(global_tmp_prop);
    global_type = pI->getTypename();
    if (tmp_type != "ndef" && tmp_type != global_type)
      std::cerr << "UnaryOp Uncompatible types: " << tmp_type << " and " << global_type << std::endl;
    break;
  }
  case '-': { // case operator "minus"
    if (global_type == tlp::IntegerProperty::propertyTypename)
      global_tmp_int_result = -global_tmp_int_result;
    if (global_type == tlp::DoubleProperty::propertyTypename)
      global_tmp_double_result = -global_tmp_double_result;
    break;
  }
  case 't': {
    // TODO: case "true"
    break;
  }
  case 'f': {
    // TODO: case "false"
    break;
  }
  case 'r': { // case "random"
    std::uniform_real_distribution<double> choose(0, global_tmp_double_result);
    global_tmp_double_result = choose(PorgyTlpGraphStructure::gen);
    global_type = tlp::DoubleProperty::propertyTypename;
    break;
  }
  }
}

void mini_syntax_ast_printer::operator()(std::string const &text) const {
  global_tmp_str_result = text;
  global_type = tlp::StringProperty::propertyTypename;
}

void mini_syntax_ast_printer::operator()(unsigned int const &uint) const {
  global_tmp_int_result = uint;
  global_type = tlp::IntegerProperty::propertyTypename;
}

void mini_syntax_ast_printer::operator()(double const &dbl) const {
  global_tmp_double_result = dbl;
  global_type = tlp::DoubleProperty::propertyTypename;
}

mini_syntax_ast_printer::mini_syntax_ast_printer(tlp::Graph *g) : _g(g) {}

void mini_syntax_ast_printer::operator()(mini_syntax_instruction const &inst) const {
  boost::apply_visitor(mini_syntax_ast_printer(_g), inst.expr);
}
