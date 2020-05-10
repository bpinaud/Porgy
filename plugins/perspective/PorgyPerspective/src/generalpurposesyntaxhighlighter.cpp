/*!
 * \file generalpurposesyntaxhighlighter.cpp
 * \brief Implementation of the GeneralPurposeSyntaxHighlighter class declared
 * in the generalpurposesyntaxhighlighter.h header.
 * \author Hadrien Decoudras
 * \date 02-06-2016
 * \version 0.3
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

#include "generalpurposesyntaxhighlighter.h"
#include "highlightingrule.h"
#include "xmlsyntaxparser.h"

#include <QDebug>
#include <QMessageBox>

#include <cassert>

using namespace std;

/****************************************************************************************
 ****************************************************************************************
 *
 * Default constructor.
 *
 ****************************************************************************************/
GeneralPurposeSyntaxHighlighter::GeneralPurposeSyntaxHighlighter(const QString &syntaxFile,
                                                                 QTextDocument *parent): QSyntaxHighlighter(parent) {
  XmlSyntaxParser::parse(syntaxFile, _highlightingRules, _error);
  if (!_error.isEmpty()) {
    qDebug() << "error when parsing syntax file: " << _error;
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get highlightning rule.
 * Keywords always appear first.
 *
 ****************************************************************************************/
QStringList GeneralPurposeSyntaxHighlighter::getKeywords() {
  QStringList keywords;
  std::vector<HighlightingRule> &vec = _highlightingRules["instruction"];

  for (const auto& hr:vec) {
    QString str = hr.toString();
    XmlSyntaxParser::removeWordBoundary(str);
    keywords << str;
  }

  vec = _highlightingRules["function"];

  for (const auto& hr:vec) {
   QString str = hr.toString();
    XmlSyntaxParser::removeWordBoundary(str);
    keywords << str;
  }

  return keywords;
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get highlightning rule.
 * Keywords always appear first.
 *
 ****************************************************************************************/
QString GeneralPurposeSyntaxHighlighter::getParseError() const {
  return _error;
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Highlight if pattern is matched.
 *
 ****************************************************************************************/
void GeneralPurposeSyntaxHighlighter::highlightBlock(const QString &text) {

    for (const auto& itM :_highlightingRules) {
        for (const auto& itV: itM.second) {
            QRegExp expression(itV.getPattern());
            int index = expression.indexIn(text);

            while (index >= 0) {
                int length = expression.matchedLength();
                setFormat(index, length, itV.getFormat());
                index = expression.indexIn(text, index + length);
            }
        }
    }

  setCurrentBlockState(0);

  vector<HighlightingRule> &vec = _highlightingRules["multicomment"];
  assert(!vec.empty());
  int startIndex = 0;

  if (previousBlockState() != 1) {
    startIndex = vec.at(0).getPattern().indexIn(text);
  }

  while (startIndex >= 0) {
    int endIndex = vec.at(1).getPattern().indexIn(text, startIndex);
    int commentLength;

    if (endIndex == -1) {
      setCurrentBlockState(1);
      commentLength = text.length() - startIndex;
    } else {
      commentLength = endIndex - startIndex + vec.at(1).getPattern().matchedLength();
    }

    setFormat(startIndex, commentLength, vec.at(0).getFormat());
    startIndex = vec.at(0).getPattern().indexIn(text, startIndex + commentLength);
  }
}
