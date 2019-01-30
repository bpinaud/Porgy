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
#ifndef _PORT_H
#define _PORT_H

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortBase.h>

class PORTGRAPH_SCOPE Port : public PortBase {

public:
  Port(const tlp::node, const PortNode *);

  // conversion from Port to tlp::node
  explicit operator const tlp::node() const {
    return getNode();
  }

  std::vector<Port *> getConnectedPorts() const;
  std::vector<Port *> getConnectedInPorts() const;
  std::vector<Port *> getConnectedOutPorts() const;

  Port *getOneConnectedPort() const;
  unsigned int getArity() const;

  /**
   * @brief check if two ports are equivalent
   * @return
   */
  static bool check(const Port *rule_port, const Port *modele_port, bool exact,
                    const PorgyTlpGraphStructure::matchpropvector &v);
  unsigned int deg() const;
  unsigned int indeg() const;
  unsigned int outdeg() const;
};

#endif
