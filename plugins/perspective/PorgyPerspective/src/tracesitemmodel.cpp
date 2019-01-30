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
#include "tracesitemmodel.h"

#include <portgraph/PorgyConstants.h>

#include <tulip/Perspective.h>
#include <tulip/TlpQtTools.h>

#include <QMainWindow>
#include <QMessageBox>

using namespace tlp;
using namespace std;

TracesItemModel::TracesItemModel(QObject *parent) : SubgraphsHierachyTreeModel(parent) {
#ifdef NDEBUG
  setColumns(Columns(NameColumn));
#else
  setColumns(Columns(IdColumn | NameColumn | NodesNumberColumn));
#endif
}

// Do not allow to rename TraceMain. This name is mandatory for things to work.
bool TracesItemModel::setGraphName(Graph *graph, const QVariant &data) const {
  if (graph->getName() == PorgyConstants::TRACEMAIN) {
    QMessageBox::information(Perspective::instance()->mainWindow(), "Cannot rename graph",
                             "Cannnot rename main trace graph. The name is "
                             "mandatory for Porgy to work.");
    return false;
  }
  graph->setName(tlp::QStringToTlpString(data.toString()));
  return true;
}

// look of add_subgraph and del_subgraph only.
void TracesItemModel::treatGraphEvent(const GraphEvent &graphEvent) {
  Graph *graph = graphEvent.getGraph();
  if (graph == _rootGraph) { // The root graph is updated.
    switch (graphEvent.getType()) {
    case GraphEvent::TLP_AFTER_ADD_SUBGRAPH:
      _rootToAdd.insert(
          const_cast<Graph *>(graphEvent.getSubGraph())); // Mark the graph for addition treat the
                                                          // added graph in the treatEvents function
      break;
    case GraphEvent::TLP_BEFORE_DEL_SUBGRAPH: {
      tlp::Graph *graph = const_cast<Graph *>(graphEvent.getSubGraph());
      if (!_rootToAdd.erase(graph)) { // The graph was not already added but
                                      // marked for addition
        // We need to supress the graph immediatly as it is valid
        removeRootGraph(graph); // Remove the graph from the hierarchy
        _toUpdate.erase(graph); // Supress all the possible modifications stored
      }
    } break;
    default:
      break;
    }

  } else {
    // A subgraph is updated
    switch (graphEvent.getType()) {
    case GraphEvent::TLP_AFTER_SET_ATTRIBUTE:
      // If the attribute updated is not the same
      if (_columnsFlags.testFlag(NameColumn) && graphEvent.getAttributeName() != string("name")) {
        return;
      }
    case GraphEvent::TLP_BEFORE_DEL_SUBGRAPH:
      _toUpdate.erase(const_cast<Graph *>(graphEvent.getSubGraph()));
      AbstractGraphHierachyItemModel::treatGraphEvent(graphEvent);
      break;
    default:
      AbstractGraphHierachyItemModel::treatGraphEvent(graphEvent);
      break;
    }
  }
}
