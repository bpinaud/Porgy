/*!
 * \file highlightingrule.cpp
 * \brief Implementation of the HighlightingRule class defined in the
 * highlightingrule.h header.
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

#include "highlightingrule.h"

/****************************************************************************************
 ****************************************************************************************
 *
 * Default constructor.
 *
 ****************************************************************************************/
HighlightingRule::HighlightingRule(const QRegularExpression &pattern, const QTextCharFormat &format)
    : _pattern(pattern), _format(format) {}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get pattern.
 *
 ****************************************************************************************/
QRegularExpression HighlightingRule::getPattern() const {
  return _pattern;
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get format.
 *
 ****************************************************************************************/
QTextCharFormat HighlightingRule::getFormat() const {
  return _format;
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Set pattern.
 *
 ****************************************************************************************/
void HighlightingRule::setPattern(const QRegularExpression &pattern) {
  _pattern = pattern;
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Set format.
 *
 ****************************************************************************************/
void HighlightingRule::setFormat(const QTextCharFormat &format) {
  _format = format;
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Pattern to string.
 *
 ****************************************************************************************/
QString HighlightingRule::toString() const {
  return _pattern.pattern();
}
