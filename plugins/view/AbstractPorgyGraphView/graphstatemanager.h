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
#ifndef GRAPHSTATEMANAGER_H
#define GRAPHSTATEMANAGER_H

#include <vector>

#include <tulip/Node.h>
#include <tulip/Size.h>

#include <porgy/GraphVisualProperties.h>

#include <QAbstractListModel>

namespace tlp {
class Graph;
class BooleanProperty;
}

struct GraphState {
  GraphState(tlp::Graph *graph, const GraphVisualProperties &properties,
             const std::string &stateName = "", tlp::Graph *rule = nullptr);
  GraphState();
  tlp::Graph *graph;
  GraphVisualProperties properties;
  std::string name;
  tlp::Graph *rule;
};

enum PorgyIntermediaryState {
  NoIntermediaryState = 0x0,
  ViewLHSP = 0x1,
  ViewLHS = 0x4,
  ViewRHS = 0x8,
  AllStates = 0xF
};
Q_DECLARE_FLAGS(PorgyIntermediaryStateFlags, PorgyIntermediaryState)
Q_DECLARE_OPERATORS_FOR_FLAGS(PorgyIntermediaryStateFlags)

/**
* @brief Generate visual states for Porgy trace tools. Use this class to
*highlight some interresting properties like P, left members or right members.
* Don't modify original properties.
**/
class GraphStateManager : public QAbstractListModel {
  Q_OBJECT
public:
  GraphStateManager(tlp::Graph *traceRoot = nullptr,
                    const std::vector<tlp::node> &models = std::vector<tlp::node>(),
                    QObject *parent = nullptr);
  ~GraphStateManager() override;

  /**
   * @brief getGraph returns the graph
   * @return
   */
  tlp::Graph *getGraph() const {
    return traceRoot;
  }

  /**
   * @brief setData update the internal state using the root graph and using
   * only the nodes given in parameters. If there is no nodes given take all the
   * node.
   * @param graph
   * @param models
   */

  void setGraphData(tlp::Graph *graph, const std::vector<tlp::node> &models = std::vector<tlp::node>());

  /**
    * @brief rowCount return the number of element call the stateNumber function
    * @param parent
    * @return
    */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  /**
    * @brief regenerate state list.
    **/
  void updateStateList(PorgyIntermediaryStateFlags flags = PorgyIntermediaryStateFlags(ViewLHS |
                                                                                       ViewRHS));
  unsigned int stateNumber() const {
    return stateList.size();
  }

  /**
   * @brief getState return the state for the given number
   * @param stateIndex the index of the state
   * @return
   */
  GraphState &getState(unsigned int stateIndex);

private:
  GraphState buildSimpleGraphState(tlp::node model) const;
  GraphState buildStateForP(tlp::node fromModel, tlp::node toModel, tlp::node modelToSearchP);
  GraphState buildStateForL(tlp::node fromModel, tlp::node toModel);
  GraphState buildStateForR(tlp::node fromModel, tlp::node toModel);

  void clearStateList();
  void clearCreatedProperties();

  template <typename PROPTYPE, typename NODETYPE, typename EDGETYPE>
  void setValueForElements(tlp::Graph *graph, tlp::BooleanProperty *elements,
                           const NODETYPE &nodeValue, const EDGETYPE &edgeValue,
                           PROPTYPE *propertyToUpdate, bool treatNodes, bool treatEdges) const;
  void increaseSizeForElements(tlp::Graph *graph, tlp::BooleanProperty *elements,
                               const tlp::Size &toSet,
                               tlp::SizeProperty *sizePropertyToUpdate) const;
  tlp::Graph *traceRoot;
  std::vector<tlp::node> models;

  std::vector<GraphState> stateList;
  std::vector<tlp::PropertyInterface *> createdProperties;
};

Q_DECLARE_METATYPE(GraphStateManager *)

#include "graphstatemanager.cxx"

#endif // GRAPHSTATEMANAGER_H
