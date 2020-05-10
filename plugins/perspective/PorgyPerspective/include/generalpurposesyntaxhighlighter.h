/*!
 * \file porgysyntaxhighlighter.h
 * \brief Header containing the declaration of the
 * GeneralPurposeSyntaxHighlighter object.
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

#ifndef GENERALPURPOSESYNTAXHIGHLIGHTER_H
#define GENERALPURPOSESYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QStringList>
#include <QString>

#include "highlightingrule.h"

#include <vector>
#include <unordered_map>
#include <string>

/*!
 * \class GeneralPurposeSyntaxHighlighter
 * \brief Class used to highlighlight keywords of a Scripting Language.
 */
class GeneralPurposeSyntaxHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  /*!
   * \brief Default constructor.
   *        Implements the syntax highlighter.
   * \param syntaxFile : Path to a Scripting Language syntax file.
   * \param parent : Parent document.
   */
  GeneralPurposeSyntaxHighlighter(const QString &syntaxFile, QTextDocument *parent);

  /*!
   * \brief Return the Scripting Language keywords.
   * \return The porgy scripting language keywords.
   */
  QStringList getKeywords();

  /*!
   * \brief Get a potential parse error.
   * \return A potential parse error.
   */
  QString getParseError() const;

protected:
  /*!
   * \brief Highlight a keyword.
   * \param text : Input string.
   */
  void highlightBlock(const QString &text) override;

private:
  std::unordered_map<std::string, std::vector<HighlightingRule>> _highlightingRules;
  QString _error;
};

#endif // GENERALPURPOSESYNTAXHIGHLIGHTER_H
