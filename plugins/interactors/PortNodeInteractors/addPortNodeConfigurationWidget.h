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

#ifndef ADD_PN_CONFIG_W_H_
#define ADD_PN_CONFIG_W_H_

#include <QMap>
#include <QWidget>

#include <porgy/PortNodeQtModel.h>

#include <tulip/Observable.h>

namespace Ui {
class AddPortNodeConfigurationWidget;
}

class PortNodeQt;
class QTableWidgetItem;
class PortNodeQtModel;
class AddPortNodeConfigurationWidget : public QWidget, public tlp::Observable {
  Q_OBJECT

  Ui::AddPortNodeConfigurationWidget *ui;

  QMap<tlp::Graph *, PortNodeQtModel *> _models;
  tlp::Graph *_currentGraph;
  PortNodeQtModel *_currentModel;

public:
  AddPortNodeConfigurationWidget(QWidget *parent = nullptr);
  ~AddPortNodeConfigurationWidget() override;

  void loadPortNodesFromGraph(tlp::Graph *graph);
  tlp::Graph *currentGraph() const {
    return _currentGraph;
  }

  /**
   * @brief isCurrentGraph checks if the list of port nodes displayed correspond
   * to the graph given in parameters.
   *  If it correspond no need to reload the list. This function compare the
   * root graph of the given graph to the current graph and returns true if they
   * are the same.
   * @return true if the list of port nodes diplayed to the given graph.
   */
  bool isCurrentGraph(tlp::Graph *);

  bool IsPortNodeSelected() const;
  PortNodeQt *getSelectedPortNode() const;
  void addPortNode(unsigned size, PortNodeQt *pn);

protected:
  void treatEvents(const std::vector<tlp::Event> &events) override;

private slots:
  void editPortNode(const QModelIndex &index);
  void createNewPortNode();
};

#endif
