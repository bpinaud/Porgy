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
#include <porgy/PortNodeQtModel.h>
#include <porgy/porgymimedata.h>

#include <portgraph/PortGraphModel.h>
#include <portgraph/PortGraphRule.h>

using namespace tlp;

PortNodeQtModel::PortNodeQtModel(QObject *parent)
    : QAbstractListModel(parent), _rules_root(nullptr), _models_root(nullptr),
      _needToReloadData(false) {}

PortNodeQtModel::~PortNodeQtModel() {
  clear();
}

void PortNodeQtModel::clear() {
  for (auto &v : _portNodesList)
    delete v;
}

void PortNodeQtModel::initPortNodesList(tlp::Graph *graph) {
  // Stop listening old graph
  if (_rules_root != nullptr) {
    _rules_root->removeListener(this);
    _rules_root->removeObserver(this);
  }
  if (_models_root != nullptr) {
    _models_root->removeListener(this);
    _models_root->removeObserver(this);
  }
  // Clear data
  beginResetModel();
  clear();
  endResetModel();

  _rules_root = PorgyTlpGraphStructure::getRulesRoot(graph);
  _models_root = PorgyTlpGraphStructure::getModelsRoot(graph);
  if (_rules_root != nullptr && _models_root != nullptr) {
    updateData();
    _rules_root->addListener(this);
    _rules_root->addObserver(this);

    _models_root->addListener(this);
    _models_root->addObserver(this);
  }
}

void PortNodeQtModel::addPortNode(const PortNodeQt &prtNode) {
  // Check if the port node already exist in the list.
  bool add = true;
  for (PortNodeQt *port : _portNodesList) {
    // Check if the port node is not already in the list.
    if (prtNode == port) {
      // The port was found no need to add it again
      add = false;
      // Treat the next port
      break;
    }
  }
  if (add) {
    // The port was not found add it in the list
    addPortNode(new PortNodeQt(prtNode));
  }
}

void PortNodeQtModel::addPortNode(PortNodeQt *portNode) {
  beginInsertRows(QModelIndex(), _portNodesList.size(), _portNodesList.size());
  _portNodesList.push_back(portNode);
  endInsertRows();
}

int PortNodeQtModel::rowCount(const QModelIndex &) const {
  return _portNodesList.size();
}
QVariant PortNodeQtModel::data(const QModelIndex &index, int role) const {
  if (hasIndex(index.row(), index.column())) {
    switch (role) {
    case Qt::DisplayRole:
      return QVariant(_portNodesList[index.row()]->getName());
      break;
    case Qt::BackgroundRole:
      return QVariant(_portNodesList[index.row()]->getColor());
      break;
    }
  }
  return QVariant();
}
Qt::ItemFlags PortNodeQtModel::flags(const QModelIndex &) const {
  return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

PortNodeQt *PortNodeQtModel::portNodeFromIndex(const QModelIndex &index) const {
  if (!hasIndex(index.row(), index.column())) {
    return nullptr;
  }
  return _portNodesList[index.row()];
}

QStringList PortNodeQtModel::mimeTypes() const {
  QStringList list;
  list << PorgyMimeData::portNodeQtPtrListMimeTypes;
  return list;
}

QMimeData *PortNodeQtModel::mimeData(const QModelIndexList &indexes) const {
  PorgyMimeData *data = nullptr;
  if (!indexes.empty()) {
    data = new PorgyMimeData();
    data->setPortNodeQtPtrList(portNodesFromIndexes(indexes));
  }
  return data;
}

QList<PortNodeQt *> PortNodeQtModel::portNodesFromIndexes(const QModelIndexList &indexes) const {
  QList<PortNodeQt *> portNodes;
  foreach (const QModelIndex &p, indexes) { portNodes << portNodeFromIndex(p); }
  return portNodes;
}

void PortNodeQtModel::treatEvent(const tlp::Event &event) {

  if (event.type() == tlp::Event::TLP_DELETE) {
    // One of a graph is deleted avoid segfault
    if (_rules_root != nullptr) {
      _rules_root->removeListener(this);
      _rules_root->removeObserver(this);
    }
    _rules_root = nullptr;
    if (_models_root != nullptr) {
      _models_root->removeListener(this);
      _models_root->removeObserver(this);
    }
    _models_root = nullptr;

    // Clear data
    beginResetModel();
    clear();
    endResetModel();

  } else if (event.type() == tlp::Event::TLP_MODIFICATION) {
    const tlp::GraphEvent *gEvt = dynamic_cast<const tlp::GraphEvent *>(&event);
    if (gEvt != nullptr) {
      switch (gEvt->getType()) {
      case tlp::GraphEvent::TLP_ADD_NODE:
      case tlp::GraphEvent::TLP_ADD_NODES:
        _needToReloadData = true;
        break;

      default:
        break;
      }
    }
  }
}

void PortNodeQtModel::treatEvents(const std::vector<tlp::Event> &) {
  if (_needToReloadData) {
    updateData();
    _needToReloadData = false;
  }
}

void PortNodeQtModel::updateData() {

  // Search for new elements
  for (Graph *sub : _models_root->subGraphs()) {
    loadOneGraph<PortGraphModel>(PortGraphModel(sub));
  }
  for (Graph *sub : _rules_root->subGraphs()) {
    loadOneGraph<PortGraphRule>(PortGraphRule(sub));
  }
}
