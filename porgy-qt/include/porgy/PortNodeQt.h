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

#ifndef PORTNODEQT_H
#define PORTNODEQT_H

#include <QColor>

#include <portgraph/PorgyConstants.h>
#include <portgraph/porgyconf.h>
#include <porgy/PortQt.h>

#include <tulip/Coord.h>

class PortNode;
//class PortQt;

namespace tlp {
class Graph;
struct node;
}

class PORGY_SCOPE PortNodeQt {
  QString name;
  QColor color;
  std::vector<PortQt> ports;

public:
  PortNodeQt(const QString &, const QColor &, const std::vector<PortQt> &ports);
  PortNodeQt(const PortNode *);
  PortNodeQt(const PortNodeQt &);
  virtual ~PortNodeQt();

  inline QString getName() const {
    return name;
  }
  inline QColor getColor() const {
    return color;
  }
  inline void setName(const QString &name) {
    this->name = name;
  }
  inline void setColor(const QColor &newC) {
    color = newC;
  }

  inline const std::vector<PortQt> &getPorts() const {
    return ports;
  }

  void replacePorts(const std::vector<PortQt> &ql);

  /**
   * @brief addToTlpGraph add the current port node to the graph. Detect it's
   * side using it's position to the center.
   * @param g
   * @param isRule
   * @param posCenter
   * @return
   */
  tlp::node addToTlpGraphUsingPosition(tlp::Graph *g, const tlp::Coord &posCenter) const;

  /**
   * @brief similarTo compare the given port node to the current port node Qt.
   * Compare the name and the structure of port (number and name).
   * @return true if the pport node are equivalent.
   */
  bool operator==(const PortNodeQt *p) const;

private:
  tlp::node addToTlpGraph(tlp::Graph *g, const bool isRule, const tlp::Coord &posCenter,
                          const PorgyConstants::RuleSide side) const;
};

#endif
