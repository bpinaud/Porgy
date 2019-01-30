#include "UtreeWalk.h"

#include <tulip/BooleanProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/IntegerProperty.h>

#include <portgraph/Port.h>
#include <portgraph/PortNode.h>

#include <boost/lexical_cast.hpp>

#include <regex>

using namespace tlp;
using namespace std;
using namespace boost;

// execute the utree structure to compute the booleanproperty
utree_walk::utree_walk(vector<booleanptr> &vect, string &p, string &ban, Graph *new_model)
    : vect(vect), _p(p), _ban(ban), new_model(new_model) {}

void utree_walk::operator()(spirit::utree::invalid_type) const {
  assert(false);
}

void utree_walk::operator()(spirit::utree::nil_type) const {
  assert(false);
}

void utree_walk::operator()(bool) const {
  assert(false);
}

// used for crtban, crtpos, crtgraph, emptyset
void utree_walk::operator()(spirit::binary_range_type const &b) const {
  std::string st = "";
  for (spirit::binary_range_type::const_iterator i = b.begin(); i != b.end(); ++i)
    st += i;

  StrategyElement::focus_operation op = static_cast<StrategyElement::focus_operation>(stoul(st));
  switch (op) {
  case StrategyElement::focus_operation::CRTBAN: {
    booleanptr banp(new BooleanProperty(new_model));
    if (!_ban.empty())
      banp->copy(new_model->getProperty<BooleanProperty>(_ban));
    vect.push_back(banp);
  } break;
  case StrategyElement::focus_operation::CRTPOS: {
    booleanptr pp(new BooleanProperty(new_model));
    pp->copy(new_model->getProperty<BooleanProperty>(_p));
    vect.push_back(pp);
  } break;
  case StrategyElement::focus_operation::CRTGRAPH: {
    // cannot use setAllNodeValue because it changes the default value
    booleanptr ptr(new BooleanProperty(new_model));
    for (node n : new_model->nodes()) {
      ptr->setNodeValue(n, true);
    }
    for (edge e : new_model->edges()) {
      ptr->setEdgeValue(e, true);
    }
    vect.push_back(ptr);
  } break;
  case StrategyElement::focus_operation::EMPTYSET: {
    // cannot use setAllNodeValue because it changes the default value
    booleanptr ptr(new BooleanProperty(new_model));
    for (node n : new_model->nodes()) {
      ptr->setNodeValue(n, false);
    }
    for (edge e : new_model->edges()) {
      ptr->setEdgeValue(e, false);
    }
    vect.push_back(ptr);
  } break;
  }
}

// one(F) and all(F)
void utree_walk::operator()(spirit::utf8_string_range_type const &type) const {
  string st = "";
  for (spirit::utf8_string_range_type::const_iterator i = type.begin(); i != type.end(); ++i)
    st += *i;
  if (st == "one") {
    booleanptr p1(vect.back());
    vect.pop_back();
    booleanptr p2(new BooleanProperty(new_model));
    vect.push_back(p2);
    vector<node> v;
    for (node n : p1->getNodesEqualTo(true)) {
      v.push_back(n);
    }
    if (!v.empty()) {
      uniform_int_distribution<int> choose(0, v.size() - 1);
      node n = v[choose(PorgyTlpGraphStructure::gen)]; // chose one element
      PortGraphModel pg(new_model); // one returns a graph, so select the whole portnode
      const PortNode *pn(pg[n]);
      pn->select(true, p2.get());
    }
  }
#ifdef PORGY_STRATEGY_DEBUG_MESSAGES
  else if (st == "all") {
    cerr << __PRETTY_FUNCTION__ << ": nothing to do for all(t)" << endl;
  }
#endif
}

void utree_walk::operator()(spirit::utf8_symbol_range_type const &str) const {
  using iterator = spirit::utf8_symbol_range_type::const_iterator;
  string st = "";
  for (iterator i = str.begin(); i != str.end(); ++i)
    st += *i;
  booleanptr p1(vect.back());
  vect.pop_back();
  booleanptr p2(vect.back());
  vect.pop_back();
  booleanptr p3(new BooleanProperty(new_model));
  p3->copy(p2.get());
  StrategyElement::set_theory_op op =
      static_cast<StrategyElement::set_theory_op>(lexical_cast<unsigned>(st));
  switch (op) {
  case StrategyElement::set_theory_op::MINUS:
    // A \ B : keeps all elements of A set to true which are not true in B
    for (node n : p3->getNodesEqualTo(true)) {
      if (p1->getNodeValue(n)) {
        p3->setNodeValue(n, false);
      }
    }
    for (edge e : p3->getEdgesEqualTo(true)) {
      if (p2->getEdgeValue(e))
        p3->setEdgeValue(e, false);
    }
    break;
  case StrategyElement::set_theory_op::UNION:
    // A \cup B = all elements set to true in A or B
    for (node n : p1->getNodesEqualTo(true)) {
      p3->setNodeValue(n, true);
    }
    for (edge e : p1->getEdgesEqualTo(true)) {
      p3->setEdgeValue(e, true);
    }
    break;
  case StrategyElement::set_theory_op::INTER:
    // A \cap B = all elements set to true in A and B
    for (node n : p3->getNodesEqualTo(true)) {
      if (!p1->getNodeValue(n))
        p3->setNodeValue(n, false);
    }
    for (edge e : p3->getEdgesEqualTo(true)) {
      if (!p1->getEdgeValue(e))
        p3->setEdgeValue(e, false);
    }
    break;
  }
  vect.push_back(p3);
}

/////////////////////////
static void select_edge(booleanptr fromOp, const pair<node, node> &ends, PortGraphModel &pg,
                        BooleanProperty *prop_new) {
  if (fromOp->getNodeValue(ends.first) && (!fromOp->getNodeValue(ends.second))) {
    pg[ends.second]->select(true, prop_new);
    return;
  }
  if (fromOp->getNodeValue(ends.second) && (!fromOp->getNodeValue(ends.first))) {
    pg[ends.first]->select(true, prop_new);
    return;
  }
}

bool utree_walk::test_node(const StrategyElement::ngbprop_ds_vect &expr, node c,
                           unsigned id) const {
  // stop recursion
  if (id == expr.size() || expr.empty())
    return true;

  StrategyElement::comp_value_variant comp_value = get<2>(expr[id]);
  StrategyElement::relational_operators op = get<1>(expr[id]);
  PropertyInterface *p2 = nullptr;
  if (comp_value.type() == typeid(StrategyElement::TulipProp)) {
    StrategyElement::TulipProp p = boost::get<StrategyElement::TulipProp>(comp_value);
    p2 = new_model->getProperty(p.name);
  }

  StrategyElement::TulipProp check_property(get<0>(expr[id]));
  PropertyInterface *p = new_model->getProperty(check_property.name); // property MUST exist.
  bool res;
  if ((p->getTypename() == DoubleProperty::propertyTypename) ||
      (p->getTypename() == IntegerProperty::propertyTypename)) {
    double val = (p2 != nullptr) ? static_cast<NumericProperty *>(p2)->getNodeDoubleValue(c)
                                 : boost::get<double>(comp_value);
    NumericProperty *prop = static_cast<NumericProperty *>(p);
    switch (op) {
    case StrategyElement::relational_operators::EQ:
      res = test_node(expr, c, ++id);
      return res && prop->getNodeDoubleValue(c) == val;
    case StrategyElement::relational_operators::NEQ:
      res = test_node(expr, c, ++id);
      return res && prop->getNodeDoubleValue(c) != val;
    case StrategyElement::relational_operators::GT:
      res = test_node(expr, c, ++id);
      return res && prop->getNodeDoubleValue(c) > val;
    case StrategyElement::relational_operators::GTE:
      res = test_node(expr, c, ++id);
      return res && prop->getNodeDoubleValue(c) >= val;
    case StrategyElement::relational_operators::LT:
      res = test_node(expr, c, ++id);
      return res && prop->getNodeDoubleValue(c) < val;
    case StrategyElement::relational_operators::LTE:
      res = test_node(expr, c, ++id);
      return res && prop->getNodeDoubleValue(c) <= val;
    case StrategyElement::relational_operators::REGEXP:
      return false;
    }
  } else {
    string val = (p2 != nullptr) ? p2->getNodeStringValue(c) : boost::get<string>(comp_value);
    switch (op) {
    case StrategyElement::relational_operators::EQ:
      res = test_node(expr, c, ++id);
      return res && p->getNodeStringValue(c) == val;
    case StrategyElement::relational_operators::NEQ:
      res = test_node(expr, c, ++id);
      return res && p->getNodeStringValue(c) != val;
    case StrategyElement::relational_operators::GT:
      res = test_node(expr, c, ++id);
      return res && p->getNodeStringValue(c) > val;
    case StrategyElement::relational_operators::GTE:
      res = test_node(expr, c, ++id);
      return res && p->getNodeStringValue(c) >= val;
    case StrategyElement::relational_operators::LT:
      res = test_node(expr, c, ++id);
      return res && p->getNodeStringValue(c) < val;
    case StrategyElement::relational_operators::LTE:
      res = test_node(expr, c, ++id);
      return res && p->getNodeStringValue(c) <= val;
    case StrategyElement::relational_operators::REGEXP:
      res = test_node(expr, c, ++id);
      regex val_regex(val);
      return res && regex_match(p->getNodeStringValue(c), val_regex);
    }
  }
  return false;
}

bool utree_walk::test_edge(const StrategyElement::ngbprop_ds_vect &expr, edge c,
                           unsigned id) const {
  // stop recursion
  if (id == expr.size() || expr.empty())
    return true;

  StrategyElement::comp_value_variant comp_value = get<2>(expr[id]);
  StrategyElement::relational_operators op = get<1>(expr[id]);
  PropertyInterface *p2 = nullptr;
  if (comp_value.type() == typeid(StrategyElement::TulipProp)) {
    StrategyElement::TulipProp p = boost::get<StrategyElement::TulipProp>(comp_value);
    p2 = new_model->getProperty(p.name);
  }

  StrategyElement::TulipProp check_property(get<0>(expr[id]));
  PropertyInterface *p = new_model->getProperty(check_property.name); // property MUST exist.
  bool res;
  if ((p->getTypename() == DoubleProperty::propertyTypename) ||
      (p->getTypename() == IntegerProperty::propertyTypename)) {
    double val = (p2 != nullptr) ? static_cast<NumericProperty *>(p2)->getEdgeDoubleValue(c)
                                 : boost::get<double>(comp_value);
    NumericProperty *prop = static_cast<NumericProperty *>(p);
    switch (op) {
    case StrategyElement::relational_operators::EQ:
      res = test_edge(expr, c, ++id);
      return res && prop->getEdgeDoubleValue(c) == val;
    case StrategyElement::relational_operators::NEQ:
      res = test_edge(expr, c, ++id);
      return res && prop->getEdgeDoubleValue(c) != val;
    case StrategyElement::relational_operators::GT:
      res = test_edge(expr, c, ++id);
      return res && prop->getEdgeDoubleValue(c) > val;
    case StrategyElement::relational_operators::GTE:
      res = test_edge(expr, c, ++id);
      return res && prop->getEdgeDoubleValue(c) >= val;
    case StrategyElement::relational_operators::LT:
      res = test_edge(expr, c, ++id);
      return res && prop->getEdgeDoubleValue(c) < val;
    case StrategyElement::relational_operators::LTE:
      res = test_edge(expr, c, ++id);
      return res && prop->getEdgeDoubleValue(c) <= val;
    case StrategyElement::relational_operators::REGEXP:
      return false;
    }
  } else {
    string val = (p2 != nullptr) ? p2->getEdgeStringValue(c) : boost::get<string>(comp_value);
    switch (op) {
    case StrategyElement::relational_operators::EQ:
      res = test_edge(expr, c, ++id);
      return res && p->getEdgeStringValue(c) == val;
    case StrategyElement::relational_operators::NEQ:
      res = test_edge(expr, c, ++id);
      return res && p->getEdgeStringValue(c) != val;
    case StrategyElement::relational_operators::GT:
      res = test_edge(expr, c, ++id);
      return res && p->getEdgeStringValue(c) > val;
    case StrategyElement::relational_operators::GTE:
      res = test_edge(expr, c, ++id);
      return res && p->getEdgeStringValue(c) >= val;
    case StrategyElement::relational_operators::LT:
      res = test_edge(expr, c, ++id);
      return res && p->getEdgeStringValue(c) < val;
    case StrategyElement::relational_operators::LTE:
      res = test_edge(expr, c, ++id);
      return res && p->getEdgeStringValue(c) <= val;
    case StrategyElement::relational_operators::REGEXP:
      res = test_edge(expr, c, ++id);
      regex val_regex(val);
      return res && regex_match(p->getEdgeStringValue(c), val_regex);
    }
  }
  return false;
}

void utree_walk::operator()(spirit::any_ptr const &a) const {
  // right now, I am using any_ptr only for StrategyElement::property_datastruct
  StrategyElement::property_datastruct *v_ptr = a.get<StrategyElement::property_datastruct *>();
  StrategyElement::property_datastruct v(*v_ptr);
  // do not delete in case of a while. I have a memory leak here.
  //  delete v_ptr;

  // starting set
  spirit::utree::visit(get<1>(v), *this);
  booleanptr fromOp(vect.back());
  vect.pop_back();
  PortGraphModel pg(new_model);
  booleanptr prop_new_ptr(new BooleanProperty(new_model));
  BooleanProperty *prop_new = prop_new_ptr.get();
  StrategyElement::compvar variant_comp(get<3>(v));
  StrategyElement::ngbprop_ds_vect expr;
  if (variant_comp.type() != typeid(bool))
    expr = boost::get<StrategyElement::ngbprop_ds_vect>(variant_comp);
  const StrategyElement::Graph_Element &elem = get<2>(v);
  switch (get<0>(v)) { // property/ngb/inNgb/outNgb
  case StrategyElement::position_keywords::NGB: {
    switch (elem) {
    case StrategyElement::Graph_Element::NODE: {
      for (PortNode *pn : pg.getPortNodes()) {
        node c(pn->getCenter());
        if (fromOp->getNodeValue(c) && test_node(expr, c, 0)) {
          for (const PortNode *pn2 : pn->getInOutConnectedPortNodes()) {
            if (!fromOp->getNodeValue(pn2->getCenter()))
              pn2->select(true, prop_new);
          }
        }
      }
    } break;
    case StrategyElement::Graph_Element::PORT: {
      for (PortNode *pn : pg.getPortNodes()) {
        for (Port *pt : pn->getPorts()) {
          node c(pt->getNode());
          if (fromOp->getNodeValue(c) && test_node(expr, c, 0)) {
            for (Port *p : pt->getConnectedPorts()) {
              if (!fromOp->getNodeValue(p->getParentPortNode()->getCenter()))
                p->getParentPortNode()->select(true, prop_new);
            }
          }
        }
      }
    } break;
    case StrategyElement::Graph_Element::EDGE: {
      for (edge e : pg.getEdges()) {
        const pair<node, node> &ends = new_model->ends(e);
        if (test_edge(expr, e, 0))
          select_edge(fromOp, ends, pg, prop_new);
      }
    } break;
    }
  } break;
  case StrategyElement::position_keywords::PROPERTY: {
    switch (elem) {
    case StrategyElement::Graph_Element::NODE: {
      for (PortNode *pn : pg.getPortNodes()) {
        node c(pn->getCenter());
        if (fromOp->getNodeValue(c) && test_node(expr, c, 0)) {
          pn->select(true, prop_new);
        }
      }
    } break;
    case StrategyElement::Graph_Element::PORT: {
      for (PortNode *pn : pg.getPortNodes()) {
        for (Port *pt : pn->getPorts()) {
          node c(pt->getNode());
          if (fromOp->getNodeValue(c) && test_node(expr, c, 0)) {
            pn->select(true, prop_new);
          }
        }
      }
    } break;
    case StrategyElement::Graph_Element::EDGE: {
      for (edge e : pg.getEdges()) {
        const pair<node, node> &ends = new_model->ends(e);
        if (fromOp->getEdgeValue(e) && test_edge(expr, e, 0)) {
          prop_new->setEdgeValue(e, true);
          pg[ends.first]->select(true, prop_new);
          pg[ends.second]->select(true, prop_new);
        }
      }
    } break;
    }
  } break;
  case StrategyElement::position_keywords::INNGB: {
    switch (elem) {
    case StrategyElement::Graph_Element::NODE: {
      for (PortNode *pn : pg.getPortNodes()) {
        node c(pn->getCenter());
        if (fromOp->getNodeValue(c) && test_node(expr, c, 0)) {
          for (const PortNode *pn2 : pn->getConnectedInPortNodes()) {
            if (!fromOp->getNodeValue(pn2->getCenter()))
              pn2->select(true, prop_new);
          }
        }
      }
    } break;
    case StrategyElement::Graph_Element::PORT: {
      for (PortNode *pn : pg.getPortNodes()) {
        for (Port *pt : pn->getPorts()) {
          node c(pt->getNode());
          if (fromOp->getNodeValue(c) && test_node(expr, c, 0)) {
            for (Port *p : pt->getConnectedInPorts()) {
              if (!fromOp->getNodeValue(p->getParentPortNode()->getCenter()))
                p->getParentPortNode()->select(true, prop_new);
            }
          }
        }
      }
    } break;
    case StrategyElement::Graph_Element::EDGE: { // if ends.second is in P,
                                                 // ends.first is the solution.
                                                 // The edge is going to P
      for (edge e : pg.getEdges()) {
        const pair<node, node> &ends = new_model->ends(e);
        if (fromOp->getNodeValue(ends.second) && !fromOp->getNodeValue(ends.first) &&
            test_edge(expr, e, 0)) {
          pg[ends.first]->select(true, prop_new);
        }
      }
    } break;
    }
  } break;
  case StrategyElement::position_keywords::OUTNGB: {
    switch (elem) {
    case StrategyElement::Graph_Element::NODE: {
      for (PortNode *pn : pg.getPortNodes()) {
        node c(pn->getCenter());
        if (fromOp->getNodeValue(c) && test_node(expr, c, 0)) {
          for (auto pn2 : pn->getConnectedOutPortNodes()) {
            if (!fromOp->getNodeValue(pn2->getCenter()))
              pn2->select(true, prop_new);
          }
        }
      }
    } break;
    case StrategyElement::Graph_Element::PORT: {
      for (PortNode *pn : pg.getPortNodes()) {
        for (Port *pt : pn->getPorts()) {
          node c(pt->getNode());
          if (fromOp->getNodeValue(c) && test_node(expr, c, 0)) {
            for (Port *p : pt->getConnectedOutPorts()) {
              if (!fromOp->getNodeValue(p->getParentPortNode()->getCenter()))
                p->getParentPortNode()->select(true, prop_new);
            }
          }
        }
      }
    } break;
    case StrategyElement::Graph_Element::EDGE: { // if ends.first is in P,
                                                 // ends.second is the solution.
                                                 // The edge has only its source
                                                 // in P
      for (edge e : pg.getEdges()) {
        const pair<node, node> &ends = new_model->ends(e);
        if (fromOp->getNodeValue(ends.first) && !fromOp->getNodeValue(ends.second) &&
            test_edge(expr, e, 0)) {
          if (fromOp->getNodeValue(ends.first) && (!fromOp->getNodeValue(ends.second))) {
            pg[ends.second]->select(true, prop_new);
          }
        }
      }
    }
    }
  }
  }
  vect.push_back(prop_new_ptr);
}

void utree_walk::operator()(spirit::function_base const &) const {
  assert(false);
}
