/*!
 * \file xmlsyntaxparser.h
 * \brief Header containing the declaration of the XmlSyntaxParser object.
 * \author Hadrien Decoudras
 * \date 02-06-2016
 * \version 0.4
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

#ifndef XMLSYNTAXPARSER_H
#define XMLSYNTAXPARSER_H

#include <QBrush>
#include <QFont>
#include <QXmlStreamReader>
#include <unordered_map>
#include <vector>
#include <string>

#include "highlightingrule.h"

/*!
 * \class XmlSyntaxParser
 * \brief Class used to retrieve a Scripting Language syntax data from an XML
 * file.
 *        The organisation of the XML file is detailed by the XML Schema
 * Definition
 *        porgy-script.xsd. A graphical representation (porgy-script_schema.svg)
 * and an
 *        XML sample (script-sample.xml) are also available under the 'syntax'
 * directory.
 * \warning The rule defined in the porgy-script.xsd file must be followed in
 * order to not
 *          break the parsing process. Use a validator to check the validity of
 * an XML
 *          syntax file (OxygenXML).
 *          If you modify the structure of an XML syntax file, report the
 * changes in the
 *          associated XSD file (porgy-sript.xsd).
 */
struct XmlSyntaxParser {

  static bool parse(const QString &f, std::unordered_map<std::string, std::vector<HighlightingRule>> &rules, QString &error);

  /*!
   * \brief Converts an RGB color into a QBrush.
   * \param color : RGB color.
   * \return A QBrush corresponding to the integer passed in parameter.
   */
  inline static QBrush intToRgb(int color) {
    return QBrush(QColor((color >> 0x10) & 0xFF, (color >> 0x08) & 0xFF, color & 0xFF));
  }

  /*!
   * \brief Indicates that a keyword must be rendered as bold.
   * \param style : Style attribute content.
   * \return QFont::Bold if the style attribute content containts the 'bold'
   * word;
   *         QFont::Normal otherwise.
   *
   */
  inline static int isBold(const QString &style) {
    if (style.contains("bold"))
      return QFont::Bold;

    return QFont::Normal;
  }

  /*!
   * \brief Indicates that a keyword must be rendered as italic
   * \param style : Style attribute content.
   * \return True if the style attribute content containts the 'italic' word;
   *         false otherwise.
   */
  inline static bool isItalic(const QString &style) {
    return (style.contains("italic"));
  }

  /*!
   * \brief Detects if a regular expression has boudaries.
   *        The metacharacter '\b' is an anchor like the caret and the dollar
   * sign.
   *        It matches at a position that is called a "word boundary". This
   * match
   *        is zero-length. There are three different positions that qualify as
   * word boundaries:
   *
   *        Before the first character in the string, if the first character is
   * a word character.
   *        After the last character in the string, if the last character is a
   * word character.
   *        Between two characters in the string, where one is a word character
   * and the other
   *        is not a word character.
   *        Simply put: '\b' allows you to perform a "whole words only" search
   * using a regular expression
   *        in the form of \bword\b. A "word character" is a character that can
   * be used to form words.
   *        All characters that are not "word characters" are "non-word
   * characters".
   * \param word : String which has to be tested.
   * \return True if the parameter is a word; False otherwise.
   * \see http://www.regular-expressions.info/wordboundaries.html
   */
  inline static bool hasWordBoundary(const QString &word) {
    return (word.contains("\\b"));
  }

  /*!
   * \brief Allows to remove the word boundaries of a regular expression.
   * \param word : String which needs to be modified.
   * \return True if the parameter if the operation succeeded (the parameter is
   * a word);
   *         False otherwise.
   */
  inline static bool removeWordBoundary(QString &word) {
    if (XmlSyntaxParser::hasWordBoundary(word)) {
      word = word.mid(2, word.size() - 4);
      return true;
    }

    return false;
  }
};

#endif // XMLSYNTAXPARSER_H
