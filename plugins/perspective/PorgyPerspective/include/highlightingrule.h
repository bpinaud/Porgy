/*!
 * \file highlightingrule.h
 * \brief Header containing the declaration of the HighlightingRule class.
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

#ifndef HIGHLIGHTINGRULE_H
#define HIGHLIGHTINGRULE_H

#include <QTextCharFormat>
#include <QRegularExpression>


/*!
 * \class HighlightingRule
 * \brief Class used to define an highlighting rule for a particular keyword or
 * expression.
 */
class HighlightingRule {
public:
  /*!
   * \brief Default constructor.
   */
  HighlightingRule() = delete;

  /*!
   * \brief Constructor which allows to populate the pattern of a keyword and
   * its format.
   * \param pattern : Regular expression representing a keyword.
   * \param format : Style of the keyword.
   */
  HighlightingRule(const QRegularExpression &pattern, const QTextCharFormat &format);

  /*!
   * \brief Get the pattern of a keyword.
   * \return A regular expression object representing the pattern of a keyword.
   */
  QRegularExpression getPattern() const;

  /*!
   * \brief Get the format of a keyword.
   * \return The format of a keyword.
   */
  QTextCharFormat getFormat() const;

  /*!
   * \brief Set the pattern of a keyword.
   * \param pattern : Pattern of a keyword.
   */
  void setPattern(const QRegularExpression &pattern);

  /*!
   * \brief Set the format of a keyword.
   * \param format : Format of a keyword.
   */
  void setFormat(const QTextCharFormat &format);

  /*!
   * \brief Get the pattern of a keyword under the form of a string.
   * \return The pattern of a keyword under the form of a string
   */
  QString toString() const;

private:
  QRegularExpression _pattern;        /*!< Regular expression representing a keyword. */
  QTextCharFormat _format; /*!< Format of a keyword (style). */
};

#endif // HIGHLIGHTINGRULE_H
