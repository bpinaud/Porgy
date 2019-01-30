#include "UtreePrint.h"
#include "StrategyElement.h"

using namespace boost;
using namespace std;

// visitor for the last parameter of the property construct
class ValueVisitorPrint : public static_visitor<> {
  ostream &out;

public:
  explicit ValueVisitorPrint(ostream &out) : out(out) {}
  void operator()(const StrategyElement::TulipProp &prop) {
    out << "Prop: " << prop.name;
  }
  void operator()(const std::string &str) {
    out << "StrValue: " << str;
  }

  void operator()(const double &v) {
    out << "DoubleValue: " << v;
  }
};

class CompVisit : public static_visitor<> {
  ostream &out;

public:
  explicit CompVisit(ostream &out) : out(out) {}

  void operator()(const bool) {}

  void operator()(const StrategyElement::ngbprop_ds_vect &p_ex) {
    out << ", ";
    for (StrategyElement::ngbprop_ds_vect::const_iterator p_it = p_ex.begin();
         p_it != p_ex.end();) {
      StrategyElement::ngbprop_ds p_element = *p_it;
      StrategyElement::TulipProp prop(get<0>(p_element));
      out << prop.name << " ";
      switch (get<1>(p_element)) {
      case StrategyElement::relational_operators::EQ:
        out << "EQ";
        break;
      case StrategyElement::relational_operators::NEQ:
        out << "NEQ";
        break;
      case StrategyElement::relational_operators::GT:
        out << "GT";
        break;
      case StrategyElement::relational_operators::GTE:
        out << "GTE";
        break;
      case StrategyElement::relational_operators::LT:
        out << "LT";
        break;
      case StrategyElement::relational_operators::LTE:
        out << "LTE";
        break;
      case StrategyElement::relational_operators::REGEXP:
        out << "REGEXP";
        break;
      }

      out << " ";
      ValueVisitorPrint visit(out);
      get<2>(p_element).apply_visitor(visit);

      if (++p_it != p_ex.end())
        out << " && ";
    }
  }
};

utree_print::utree_print(std::ostream &out) : out(out) {}

void utree_print::operator()(spirit::utree::invalid_type) const {
  out << "<invalid> ";
}

void utree_print::operator()(spirit::utree::nil_type) const {
  out << "<nil> ";
}

void utree_print::operator()(bool b) const {
  out << (b ? "true" : "false") << ' ';
}

// used for crtban, crtpos, crtgraph, emptyset
void utree_print::operator()(spirit::binary_range_type const &b) const {
  typedef spirit::binary_range_type::const_iterator iterator;
  std::string st = "";
  for (iterator i = b.begin(); i != b.end(); ++i)
    st += i;

  StrategyElement::focus_operation op = static_cast<StrategyElement::focus_operation>(stoul(st));
  switch (op) {
  case StrategyElement::focus_operation::CRTBAN:
    out << " CRTBAN";
    break;
  case StrategyElement::focus_operation::CRTPOS:
    out << " CRTPOS";
    break;
  case StrategyElement::focus_operation::CRTGRAPH:
    out << " CRTGRAPH";
    break;
  case StrategyElement::focus_operation::EMPTYSET:
    out << " EMPTYSET";
    break;
  }
}

void utree_print::operator()(spirit::utf8_string_range_type const &str) const {
  typedef spirit::utf8_string_range_type::const_iterator iterator;
  out << '"';
  for (iterator i = str.begin(); i != str.end(); ++i)
    out << *i;
  out << "\" ";
}

void utree_print::operator()(spirit::utf8_symbol_range_type const &str) const {
  out << "op(";
  typedef spirit::utf8_symbol_range_type::const_iterator iterator;
  string st = "";
  for (iterator i = str.begin(); i != str.end(); ++i)
    st += *i;
  StrategyElement::set_theory_op op = static_cast<StrategyElement::set_theory_op>(stoul(st));
  switch (op) {
  case StrategyElement::set_theory_op::MINUS:
    out << "MINUS";
    break;
  case StrategyElement::set_theory_op::UNION:
    out << "UNION";
    break;
  case StrategyElement::set_theory_op::INTER:
    out << "INTER";
    break;
  }

  out << ") ";
}

void utree_print::operator()(spirit::any_ptr const &a) const {
  // right now, I am using any_ptr only for StrategyElement::property_datastruct
  // => very ugly
  StrategyElement::property_datastruct *v_ptr = a.get<StrategyElement::property_datastruct *>();
  StrategyElement::property_datastruct v(*v_ptr);
  // delete v_ptr; => memory leak but I do not know how to solve it. the pointer
  // cannot be deleted if the strategy uses a loop operator
  // operation
  switch (get<0>(v)) {
  case StrategyElement::position_keywords::PROPERTY:
    out << "property(";
    break;
  case StrategyElement::position_keywords::NGB:
    out << "ngb(";
    break;
  case StrategyElement::position_keywords::INNGB:
    out << "inNgb(";
    break;
  case StrategyElement::position_keywords::OUTNGB:
    out << "outNgb(";
    break;
  }
  // starting set
  spirit::utree::visit(get<1>(v), *this);
  out << ", ";

  // graph element
  switch (get<2>(v)) {
  case StrategyElement::Graph_Element::NODE:
    out << "node";
    break;
  case StrategyElement::Graph_Element::EDGE:
    out << "edge";
    break;
  case StrategyElement::Graph_Element::PORT:
    out << "port";
    break;
  }

  // operation
  //  if(at_c<3>(v)) {
  CompVisit visit_val(out);
  get<3>(v).apply_visitor(visit_val);

  // }

  out << ") ";
}

void utree_print::operator()(spirit::function_base const &) const {
  return (*this)("<function>");
}

std::ostream &operator<<(std::ostream &out, spirit::utree const &x) {
  spirit::utree::visit(x, utree_print(out));
  return out;
}
