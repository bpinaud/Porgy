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
#ifndef METAVALUECALCULATOR_H
#define METAVALUECALCULATOR_H

#include <tulip/ColorProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/StringProperty.h>

typedef tlp::AbstractProperty<tlp::IntegerType, tlp::IntegerType, tlp::NumericProperty>
    AbstractIntegerProperty;
//**********************************************************************
// define some specific MetaValueCalculator classes for edges
// IntegerProperty
class IntegerPropertyCalculator : public AbstractIntegerProperty::MetaValueCalculator {
public:
  void computeMetaValue(AbstractIntegerProperty *, tlp::node, tlp::Graph *, tlp::Graph *) override {
  }

  void computeMetaValue(AbstractIntegerProperty *intP, tlp::edge mE, tlp::Iterator<tlp::edge> *itE,
                        tlp::Graph *) override {
    // meta edge value is the value of the first underlying edge
    intP->setEdgeValue(mE, intP->getEdgeValue(itE->next()));
  }
};

// viewColor
class viewColorPropertyCalculator : public tlp::AbstractColorProperty::MetaValueCalculator {
public:
  void computeMetaValue(tlp::AbstractColorProperty *, tlp::node, tlp::Graph *,
                        tlp::Graph *) override {}

  void computeMetaValue(tlp::AbstractColorProperty *colorP, tlp::edge mE,
                        tlp::Iterator<tlp::edge> *itE, tlp::Graph *) override {
    // meta edge value is the value of the first underlying edge
    colorP->setEdgeValue(mE, colorP->getEdgeValue(itE->next()));
  }
};

// StringProperty
class StringPropertyCalculator : public tlp::AbstractStringProperty::MetaValueCalculator {
public:
  void computeMetaValue(tlp::AbstractStringProperty *, tlp::node, tlp::Graph *,
                        tlp::Graph *) override {}

  void computeMetaValue(tlp::AbstractStringProperty *stringP, tlp::edge mE,
                        tlp::Iterator<tlp::edge> *itE, tlp::Graph *) override {
    // meta edge value is the value of the first underlying edge
    stringP->setEdgeValue(mE, stringP->getEdgeValue(itE->next()));
  }
};
#endif // METAVALUECALCULATOR_H
