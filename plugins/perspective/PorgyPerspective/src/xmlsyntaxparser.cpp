/*!
 * \file xmlsyntaxparser.cpp
 * \brief Implementation of the XmlSyntaxParser class declared in the
 * porgyxmlsyntaxparser.h header.
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

#include "xmlsyntaxparser.h"
#include "highlightingrule.h"

#include <QFile>
#include <QString>
#include <QXmlStreamReader>

#include <tulip/TlpQtTools.h>

bool XmlSyntaxParser::parse(const QString &f, std::unordered_map<std::string, std::vector<HighlightingRule>> &rules, QString &error) {

    QFile fi(f);

    if (!fi.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = "Cannot open syntax file";
        return false;
    }

    QXmlStreamReader reader(&fi);
    QXmlStreamAttributes attributes;
    QTextCharFormat cFormat;

    while (!reader.atEnd()) {
      if (reader.readNextStartElement()) {
        if (reader.hasError()) {
          error = reader.errorString();
          return false;
        }
        if (reader.name().toString() == QString("PorgyStrategyLanguage")) {
          if (reader.attributes().value("version").toString() != "0.1") {
            error = "Not a valid Porgy Strategy syntax file version 0.1";
            return false;
          }
        }
        else if (reader.name().toString() == "instructions") {
          attributes = reader.attributes();
          cFormat.setForeground(
              XmlSyntaxParser::intToRgb(attributes.value("color").toString().toInt()));
          cFormat.setFontWeight(XmlSyntaxParser::isBold(attributes.value("style").toString()));
          cFormat.setFontItalic(XmlSyntaxParser::isItalic(attributes.value("style").toString()));
        } else if (reader.name().toString() == "functions") {
          attributes = reader.attributes();
          cFormat.setForeground(
              XmlSyntaxParser::intToRgb(attributes.value("color").toString().toInt()));
          cFormat.setFontWeight(XmlSyntaxParser::isBold(attributes.value("style").toString()));
          cFormat.setFontItalic(XmlSyntaxParser::isItalic(attributes.value("style").toString()));
        } else if (reader.name().toString() == "operators") {
          attributes = reader.attributes();
          cFormat.setForeground(
              XmlSyntaxParser::intToRgb(attributes.value("color").toString().toInt()));
          cFormat.setFontWeight(XmlSyntaxParser::isBold(attributes.value("style").toString()));
          cFormat.setFontItalic(XmlSyntaxParser::isItalic(attributes.value("style").toString()));
        } else if (reader.name().toString() == "symbols") {
          attributes = reader.attributes();
          cFormat.setForeground(
              XmlSyntaxParser::intToRgb(attributes.value("color").toString().toInt()));
          cFormat.setFontWeight(XmlSyntaxParser::isBold(attributes.value("style").toString()));
          cFormat.setFontItalic(XmlSyntaxParser::isItalic(attributes.value("style").toString()));
        } else if (reader.name().toString() == "singlecomments") {
          attributes = reader.attributes();
          cFormat.setForeground(
              XmlSyntaxParser::intToRgb(attributes.value("color").toString().toInt()));
          cFormat.setFontWeight(XmlSyntaxParser::isBold(attributes.value("style").toString()));
          cFormat.setFontItalic(XmlSyntaxParser::isItalic(attributes.value("style").toString()));
        } else if (reader.name().toString() == "multicomments") {
          attributes = reader.attributes();
          cFormat.setForeground(
              XmlSyntaxParser::intToRgb(attributes.value("color").toString().toInt()));
          cFormat.setFontWeight(XmlSyntaxParser::isBold(attributes.value("style").toString()));
          cFormat.setFontItalic(XmlSyntaxParser::isItalic(attributes.value("style").toString()));
        } else if (reader.name().toString() == "quotations") {
          attributes = reader.attributes();
          cFormat.setForeground(
              XmlSyntaxParser::intToRgb(attributes.value("color").toString().toInt()));
          cFormat.setFontWeight(XmlSyntaxParser::isBold(attributes.value("style").toString()));
          cFormat.setFontItalic(XmlSyntaxParser::isItalic(attributes.value("style").toString()));
        }

        else if (reader.name().toString() == "function") {
          QRegularExpression rExp(reader.attributes().value("value").toString());
          rules["function"].push_back(HighlightingRule(rExp, cFormat));
        } else {
          QRegularExpression rExp(reader.readElementText());
          rules[tlp::QStringToTlpString(reader.name().toString())].push_back(HighlightingRule(rExp, cFormat));
        }
      }
    }
    return reader.hasError();
}
