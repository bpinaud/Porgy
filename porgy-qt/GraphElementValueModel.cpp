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
#include <porgy/GraphElementValueModel.h>
#include <porgy/PropertyValueToQVariantManager.h>

#include <portgraph/PorgyConstants.h>

#include <tulip/PropertyInterface.h>
#include <tulip/StringCollection.h>

#include <utility>
#include <vector>

using namespace std;
using namespace tlp;

namespace {
bool lessThan(PropertyInterface *p1, PropertyInterface *p2) {
  string p1l = p1->getName();
  string p2l = p2->getName();
  std::transform(p1l.begin(), p1l.end(), p1l.begin(), ::tolower);
  std::transform(p2l.begin(), p2l.end(), p2l.begin(), ::tolower);
  return p1l < p2l;
}

}

GraphElementValueModel::GraphElementValueModel(
    tlp::Graph *graph, tlp::ElementType eltType, unsigned int eltId,
    const std::unordered_set<std::string> &propertiesToDisplay, QObject *parent)
    : QAbstractTableModel(parent), _propertyDataConverter(new PropertyValueToQVariantManager()),
      _graph(graph), _displayType(eltType), _eltId(eltId) {
  initPropertiesList(propertiesToDisplay);
}

GraphElementValueModel::GraphElementValueModel(tlp::Graph *graph, tlp::ElementType eltType,
                                               unsigned int eltId, QObject *parent)
    : QAbstractTableModel(parent), _propertyDataConverter(new PropertyValueToQVariantManager()),
      _graph(graph), _displayType(eltType), _eltId(eltId) {
  unordered_set<string> propertiesToDisplay;
  for (const string &propertyName : graph->getProperties()) {
    propertiesToDisplay.insert(propertyName);
  }
  initPropertiesList(propertiesToDisplay);
}

GraphElementValueModel::~GraphElementValueModel() {
  delete _propertyDataConverter;
}

void GraphElementValueModel::initPropertiesList(const unordered_set<string> &propertiesToDisplay) {
  beginResetModel();
  _propertyTable.clear();
  _propertyTable.reserve(propertiesToDisplay.size());
  if (_graph != nullptr) {
    for (const std::string &st : propertiesToDisplay) {
      if (_graph->existProperty(st)) {
        PropertyInterface *property = _graph->getProperty(st);
        _propertyTable.push_back(property);
      }
    }
  }
  endResetModel();
}

QVariant GraphElementValueModel::data(const QModelIndex &index, int role) const {
  if (hasIndex(index.row(), index.column())) {
    PropertyInterface *p = _propertyTable[index.row()];
    return _displayType == NODE ? _propertyDataConverter->nodeData(node(_eltId), p, role)
                                : _propertyDataConverter->edgeData(edge(_eltId), p, role);
  }
  return QVariant();
}

bool GraphElementValueModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role != Qt::EditRole) {
    return QAbstractTableModel::setData(index, value, role);
  }
  if (hasIndex(index.row(), index.column())) {
    PropertyInterface *p = _propertyTable[index.row()];
    bool result = _displayType == NODE
                      ? _propertyDataConverter->setNodeData(value, node(_eltId), p, role)
                      : _propertyDataConverter->setEdgeData(value, edge(_eltId), p, role);
    if (result) {
      emit dataChanged(index, index);
      return true;
    }
  }
  return false;
}

Qt::ItemFlags GraphElementValueModel::flags(const QModelIndex &index) const {
  if (hasIndex(index.row(), index.column())) {
    PropertyInterface *p = _propertyTable[index.row()];
    return _displayType == NODE ? _propertyDataConverter->nodeFlags(node(_eltId), p)
                                : _propertyDataConverter->edgeFlags(edge(_eltId), p);
  }
  return Qt::ItemFlags();
}

int GraphElementValueModel::columnCount(const QModelIndex &) const {
  return 1;
}

int GraphElementValueModel::rowCount(const QModelIndex &) const {
  return _propertyTable.size();
}

QVariant GraphElementValueModel::headerData(int section, Qt::Orientation orientation,
                                            int role) const {
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Vertical) {
      return QVariant(QString::fromUtf8(_propertyTable[section]->getName().c_str()));
    } else {
      return QVariant((_displayType == NODE ? tr("Node ") : tr("Edge ")) + QString::number(_eltId));
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void GraphElementValueModel::sort(int, Qt::SortOrder order) {
    emit layoutAboutToBeChanged();
    if(order == Qt::AscendingOrder)
        std::stable_sort(_propertyTable.begin(), _propertyTable.end(), lessThan);
    else {
        std::stable_sort(_propertyTable.rbegin(), _propertyTable.rend(), lessThan);
    }
//    if(order == Qt::AscendingOrder)
//        std::reverse(_propertyTable.begin(), _propertyTable.end());
    emit layoutChanged();
}
