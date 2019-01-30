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
#ifndef PORTQT_H
#define PORTQT_H

#include <QColor>
#include <QString>

#include <tulip/TulipViewSettings.h>

#include <portgraph/PorgyConstants.h>
#include <portgraph/porgyconf.h>

class PORGY_SCOPE PortQt {
  QString name;
  QColor color;
  tlp::NodeShape::NodeShapes shape;
  unsigned arity;

public:
  PortQt(const QString &name = "",
         const tlp::NodeShape::NodeShapes shape = PorgyConstants::PORT_DEFAULT_SHAPE,
         const QColor &color = QColor());
  inline const QString getName() const {
    return name;
  }
  inline tlp::NodeShape::NodeShapes getShape() const {
    return shape;
  }
  inline void setName(const QString &s) {
    name = s;
  }
  inline void setShape(const tlp::NodeShape::NodeShapes &s) {
    shape = s;
  }
  inline void setArity(const unsigned a) {
    arity = a;
  }
  inline unsigned &setArity() {
    return arity;
  }
  inline unsigned getArity() const {
    return arity;
  }
  inline const QColor getColor() const {
    return color;
  }
  inline void setColor(const QColor &c) {
    color = c;
  }

  /**
   * @brief operator== compares the given port to this object. Actually just
   * check if the names are equals.
   * @param port the port to compare with.
   * @return true is the ports have the same parameters else return false.
   */
  inline bool operator==(const PortQt &p) const {
    return p.name == name;
  }
};

#endif
