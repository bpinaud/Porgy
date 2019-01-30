#ifndef UTREEPRINT_H
#define UTREEPRINT_H

#include <iostream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_utree.hpp>

class utree_print {
  std::ostream &out;

public:
  typedef void result_type;
  explicit utree_print(std::ostream &out);

  void operator()(boost::spirit::utree::invalid_type) const;
  void operator()(boost::spirit::utree::nil_type) const;
  template <typename T>
  void operator()(T val) const {
    out << val << ' ';
  }
  void operator()(bool b) const;

  // used for crtban, crtpos, crtgraph, emptyset
  void operator()(boost::spirit::binary_range_type const &b) const;
  void operator()(boost::spirit::utf8_string_range_type const &str) const;
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

std::ostream &operator<<(std::ostream &out, boost::spirit::utree const &x);

#endif // UTREEPRINT_H
