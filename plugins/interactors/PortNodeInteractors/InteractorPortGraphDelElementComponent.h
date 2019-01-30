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
#ifndef INTERACTORPORTGRAPHDELELEMENTCOMPONENT_H
#define INTERACTORPORTGRAPHDELELEMENTCOMPONENT_H

#include <tulip/MouseInteractors.h>
#include <tulip/Observable.h>

#include <vector>

class PortGraphDelElementComponent : public tlp::MouseElementDeleter, public tlp::Observable {
  std::vector<tlp::node> nodetodelete;
  bool started;

  void delPortNode(tlp::node, tlp::Graph *);
  void delEdge(tlp::Graph *, const tlp::edge);
  void treatEvent(const tlp::Event &message) override;
  void treatEvents(const std::vector<tlp::Event> &) override;
  void delElement(tlp::Graph *graph, tlp::SelectedEntity &selectedEntity) override;

public:
  PortGraphDelElementComponent();
  void viewChanged(tlp::View *view) override;
  bool eventFilter(QObject *, QEvent *) override;
};

#endif
