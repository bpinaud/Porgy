#ifndef UTREEWALK_H
#define UTREEWALK_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_utree.hpp>

#include <portgraph/PortGraphModel.h>
#include <portgraph/PortNode.h>

#include "RunStrategyElement.h"

namespace tlp {
class Graph;
class BooleanProperty;
class DataSet;
}

typedef std::shared_ptr<tlp::BooleanProperty> booleanptr;

// execute the utree structure to compute the booleanproperty
class utree_walk {

  std::vector<booleanptr> &vect;
  std::string &_p, _ban;
  tlp::Graph *new_model;

  //    void select_elts(tlp::BooleanProperty* prop_new, const
  //    StrategyElement::ngbprop_ds_vect& expr, const
  //    StrategyElement::position_keywords &pos_keyword,
  //                     const StrategyElement::Graph_Element& elem,
  //                     PortGraphModel &pg, booleanptr fromOp) const;
  bool test_node(const StrategyElement::ngbprop_ds_vect &expr, tlp::node c, unsigned id) const;
  bool test_edge(const StrategyElement::ngbprop_ds_vect &expr, tlp::edge c, unsigned id) const;

public:
  typedef void result_type;
  utree_walk(std::vector<booleanptr> &vect, std::string &p, std::string &ban,
             tlp::Graph *new_model);

  void operator()(boost::spirit::utree::invalid_type) const;
  void operator()(boost::spirit::utree::nil_type) const;
  template <typename T>
  void operator()(T) const {
    assert(false);
  }
  void operator()(bool) const;
  // used for crtban, crtpos, crtgraph, emptyset
  void operator()(boost::spirit::binary_range_type const &b) const;
  // one(F) and all(F)
  void operator()(boost::spirit::utf8_string_range_type const &type) const;
  void operator()(boost::spirit::utf8_symbol_range_type const &str) const;
  template <typename Iterator>
  void operator()(boost::iterator_range<Iterator> const &range) const {
    typedef typename boost::iterator_range<Iterator>::const_iterator iterator;
    for (iterator i = range.begin(); i != range.end(); ++i) {
      boost::spirit::utree::visit(*i, *this);
    }
  }

  void operator()(boost::spirit::any_ptr const &a) const;
  void operator()(boost::spirit::function_base const &) const;
};

#endif // UTREEWALK_H
