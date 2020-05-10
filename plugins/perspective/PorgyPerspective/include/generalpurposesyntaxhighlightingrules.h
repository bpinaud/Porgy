/*!
 * \file generalpurposesyntaxhighlightingrules.h
 * \brief Header containing the declaration of the
 * GeneralPurposeSyntaxHighlightingRules class.
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

#ifndef GENERALPURPOSESYNTAXHIGHLIGHTINGRULES_H
#define GENERALPURPOSESYNTAXHIGHLIGHTINGRULES_H

#include <unordered_map>
#include <vector>
#include <string>

class HighlightingRule;

/*!
 * \class GeneralPurposeSyntaxHighlightingRules
 * \brief The GeneralPurposeSyntaxHighlightingRules class allows to gather all
 * the
 *        highlighting rules of a scripting language.
 *
 *        The getRules and some of the overloaded square brackets operators
 * functions
 *        return a copy of a vector of HighlightingRule. This has been designed
 * in such
 *        a way because the HighlightingRule containers do not contain a lot of
 * rules.
 *        The use of pointers or references must be considered if they contain a
 * consequent
 *        amount of data, or if these functions are repeatedly called.
 */
class GeneralPurposeSyntaxHighlightingRules {
public:
  /*!
   * \brief Default constructor.
   */
  GeneralPurposeSyntaxHighlightingRules();

  /*!
   * \brief Default destructor.
   */
  ~GeneralPurposeSyntaxHighlightingRules();

  /*!
   * \brief Adds an highlighting rule.
   * \param key : Type of highlighted keyword.
   * \param rule : Highlighting rule.
   */
  void appendRule(const std::string &key, HighlightingRule *rule);

  /*!
   * \brief Get an iterator positioned on the first member of the highlighting
   * rules container.
   * \return an iterator positioned on the first member of the highlighting
   * rules container.
   */
  std::unordered_map<std::string, std::vector<HighlightingRule *>>::const_iterator begin() const;

  /*!
   * \brief Get an iterator positioned on the end (end + 1) member of the
   * highlighting rules container.
   * \return An iterator positioned on the end (end + 1) member of the
   * highlighting rules container.
   */
  std::unordered_map<std::string, std::vector<HighlightingRule *>>::const_iterator end() const;

  /*!
   * \brief Get the size of the highlighting rule container.
   * \return The size of the highlighting rule container.
   */
  std::size_t size() const;

  /*!
   * \brief Get the size of a set of highlighting rules for a given type of
   * rule.
   * \param key : Type of rule.
   * \return The size of a set of highlighting rules for a given type of rule.
   */
  std::size_t size(const std::string &key) const;

  /*!
   * \brief Get a copy of a set of rules for a given type of rule.
   * \param key : Type of rule.
   * \return A copy of a set of rules for a given type of rule.
   */
  std::vector<HighlightingRule *> getRules(const std::string &key);

  /*!
   * \brief Get a copy of a set of rules for a given type of rule.
   * \param key : Type of rule.
   * \return A copy of a set of rules for a given type of rule.
   */
  std::vector<HighlightingRule *> operator[](const std::string &key) const;

  /*!
   * \brief Set an highlighting rule of a set of rules for a given type of rule.
   * \param key : Type of rule.
   * \return A reference to a set of rules for a given type of rule.
   */
  std::vector<HighlightingRule *> &operator[](const std::string &key);

private:
  std::unordered_map<std::string, std::vector<HighlightingRule *>>
      _highlightingRules; /*!< Set of highlightning rules. */
};

#endif // GENERALPURPOSESYNTAXHIGHLIGHTINGRULES_H
