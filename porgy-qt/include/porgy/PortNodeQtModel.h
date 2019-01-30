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
#ifndef PORTNODEQTMODEL_H
#define PORTNODEQTMODEL_H

#include <QAbstractListModel>

#include <porgy/PortNodeQt.h>

#include <tulip/Observable.h>

#include <portgraph/Bridge.h>
#include <portgraph/PortNode.h>
#include <portgraph/porgyconf.h>

namespace tlp {
class Graph;
}

/**
  * @brief Simple list model for PortNodes to use with QListView.
  **/
class PORGY_SCOPE PortNodeQtModel : public QAbstractListModel, public tlp::Observable {
  /**
   * @brief _portNodesList is the list of port nodes available
   */
  std::vector<PortNodeQt *> _portNodesList;
  /**
   * @brief _rules_root The rule root graph. Listen this graph for node creation
   * to automatically update the Portnode list.
   */
  tlp::Graph *_rules_root;
  /**
   * @brief _rules_root The models root graph. Listen this graph for node
   * creation to automatically update the Portnode list.
   */
  tlp::Graph *_models_root;

  /**
   * @brief _needToReloadData marked true if data have changed and need to be
   * updated.
   */
  bool _needToReloadData;

public:
  PortNodeQtModel(QObject *parent = nullptr);
  ~PortNodeQtModel() override;

  void initPortNodesList(tlp::Graph *graph);

  void addPortNode(const PortNodeQt &prtNode);

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  PortNodeQt *portNodeFromIndex(const QModelIndex &index) const;
  QList<PortNodeQt *> portNodesFromIndexes(const QModelIndexList &indexes) const;

  QStringList mimeTypes() const override;
  QMimeData *mimeData(const QModelIndexList &indexes) const override;

protected:
  void treatEvent(const tlp::Event &) override;
  void treatEvents(const std::vector<tlp::Event> &events) override;

private:
  void updateData();
  void clear();

  void addPortNode(PortNodeQt *portNode);

  template <typename TYPE>
  void loadOneGraph(const TYPE &pg) {
    for (PortNode *pn : pg.getPortNodes()) {
      if (!Bridge::isBridge(pn->getCenter(), pg.getGraph())) {
        PortNodeQt *pnqt = new PortNodeQt(pn);
        // Check if the port node already exist in the list.
        bool add = true;
        for (PortNodeQt *port : _portNodesList) {

          // Check if the port node is not already in the list.
          if (*port == pnqt) {
            // The port was found no need to add it again
            add = false;
            // Treat the next port
            break;
          }
        }
        if (add) {
          // The port was not found add it in the list
          addPortNode(pnqt);
        } else {
          delete pnqt;
        }
      }
    }
  }
};

#endif // PORTNODEQTMODEL_H
