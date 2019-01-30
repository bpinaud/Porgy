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
#include "rulesgraphitemmodel.h"

using namespace tlp;
using namespace std;

RulesGraphItemModel::RulesGraphItemModel(QObject *parent) : SubgraphsHierachyTreeModel(parent) {
#ifdef NDEBUG
  setColumns(Columns(NameColumn));
#else
  setColumns(Columns(IdColumn | NameColumn));
#endif
}

void RulesGraphItemModel::treatGraphEvent(const GraphEvent &gEvt) {
  Graph *graph = gEvt.getGraph();
  if (graph == _rootGraph) { // The root graph "Rules" is updated.
    SubgraphsHierachyTreeModel::treatGraphEvent(gEvt);
  }
  // for rules, it is not needed to observe the subgraphs of each rule. They are
  // only used inside some plugins
}
