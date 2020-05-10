/*!
 * \file generalpurposesyntaxhighlightingrules.cpp
 * \brief Implementation of the GeneralPurposeSyntaxHighlightingRules class
 * defined in the generalpurposesyntaxhighlightingrules.h header.
 * \author Hadrien Decoudras
 * \date 04-07-2016
 * \version 0.1
 */

/**
 *
 * This file is part of Porgy (http://tulip.labri.fr/TulipDrupal/?q=porgy)
 *
 * from LaBRI, University of Bordeaux, Inria and King's College London
 *
 * Porgy is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Porgy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this Porgy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "generalpurposesyntaxhighlightingrules.h"
#include "highlightingrule.h"

using namespace std;

/****************************************************************************************
 ****************************************************************************************
 *
 * Default constructor.
 *
 ****************************************************************************************/
GeneralPurposeSyntaxHighlightingRules::GeneralPurposeSyntaxHighlightingRules() {}

/****************************************************************************************
 ****************************************************************************************
 *
 * Default destructor.
 *
 ****************************************************************************************/
GeneralPurposeSyntaxHighlightingRules::~GeneralPurposeSyntaxHighlightingRules() {
  for (auto &elt : _highlightingRules) {
    for (auto highlightr : elt.second)
      delete highlightr;
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Add rule.
 *
 ****************************************************************************************/
void GeneralPurposeSyntaxHighlightingRules::appendRule(const std::string &key,
                                                       HighlightingRule *rule) {
  _highlightingRules[key].push_back(rule);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get map begin.
 *
 ****************************************************************************************/
std::unordered_map<std::string, std::vector<HighlightingRule *>>::const_iterator
GeneralPurposeSyntaxHighlightingRules::begin() const {
  return _highlightingRules.cbegin();
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get map end.
 *
 ****************************************************************************************/
std::unordered_map<std::string, std::vector<HighlightingRule *>>::const_iterator
GeneralPurposeSyntaxHighlightingRules::end() const {
  return _highlightingRules.cend();
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get map size.
 *
 ****************************************************************************************/
std::size_t GeneralPurposeSyntaxHighlightingRules::size() const {
  return _highlightingRules.size();
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get vector size.
 *
 ****************************************************************************************/
std::size_t GeneralPurposeSyntaxHighlightingRules::size(const std::string &key) const {
  if (_highlightingRules.find(key) == _highlightingRules.end()) {
    return -1;
  }

  return _highlightingRules.at(key).size();
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get a vector of rules.
 *
 ****************************************************************************************/
std::vector<HighlightingRule *>
GeneralPurposeSyntaxHighlightingRules::getRules(const std::string &key) {
  if (_highlightingRules.find(key) == _highlightingRules.end()) {
    return std::vector<HighlightingRule *>();
  }

  return _highlightingRules.at(key);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get a vector of rules.
 *
 ****************************************************************************************/
std::vector<HighlightingRule *> GeneralPurposeSyntaxHighlightingRules::
operator[](const std::string &key) const {
  if (_highlightingRules.find(key) == _highlightingRules.end()) {
    return std::vector<HighlightingRule *>();
  }

  return _highlightingRules.at(key);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Set a rule.
 *
 ****************************************************************************************/
std::vector<HighlightingRule *> &GeneralPurposeSyntaxHighlightingRules::
operator[](const std::string &key) {
  return _highlightingRules[key];
}

