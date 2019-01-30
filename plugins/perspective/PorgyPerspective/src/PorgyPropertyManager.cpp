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

#include "PorgyPropertyManager.h"
#include "ui_PorgyPropertyManager.h"

#include <portgraph/PorgyTlpGraphStructure.h>

#include <tulip/BooleanProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/PropertyCreationDialog.h>

#include <QMessageBox>

using namespace tlp;
using namespace std;

PropertyModel::PropertyModel(Graph *graph, QObject *parent)
    : QAbstractTableModel(parent), graph(graph) {
  beginResetModel();
  PorgyTlpGraphStructure::getMatchingPropertyNames(graph, matchingprop);
  PorgyTlpGraphStructure::getPropertiesToDisplay(graph, showprop);
  for(auto tmp:graph->getRoot()->getProperties()) {
    // check if property name does not end with
    // PorgyConstants::MATCHING_PROPERTIES_SUFFIX
    if ((tmp.length() <= PorgyConstants::MATCHING_PROPERTIES_SUFFIX.length()) ||
        tmp.compare(tmp.length() - PorgyConstants::MATCHING_PROPERTIES_SUFFIX.length() - 1,
                    PorgyConstants::MATCHING_PROPERTIES_SUFFIX.length(),
                    PorgyConstants::MATCHING_PROPERTIES_SUFFIX) != 0) {
      // if the property name contains less character than
      // PorgyConstants::MATCHING_PROPERTIES_SUFFIX we can add it directly

      if (PorgyConstants::PROPERTY_MANAGER_FORBIDDEN.find(tmp) ==
          PorgyConstants::PROPERTY_MANAGER_FORBIDDEN.end()) {
        _propertyTable.push_back(tmp);
      }
    }
  }
  sort();
  endResetModel();
}

QVariant PropertyModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Vertical) {
      return QVariant(QString::fromUtf8(_propertyTable[section].c_str()));
    } else {
      if (section == 0)
        return "Test for matching";
      else
        return "Show info";
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void PropertyModel::sort(int, Qt::SortOrder) {
  emit layoutAboutToBeChanged();
  std::sort(_propertyTable.begin(), _propertyTable.end());
  emit layoutChanged();
}

QVariant PropertyModel::data(const QModelIndex &index, int role) const {
  if (role == Qt::CheckStateRole && index.column() == 0) {
    if (matchingprop.find(_propertyTable[index.row()]) != matchingprop.end())
      return QVariant(Qt::Checked);
    else
      return QVariant(Qt::Unchecked);
  }
  if (role == Qt::CheckStateRole && index.column() == 1) {
    if (showprop.find(_propertyTable[index.row()]) != showprop.end())
      return QVariant(Qt::Checked);
    else
      return QVariant(Qt::Unchecked);
  }
  if (role == Qt::DisplayRole)
    return "";
  return QVariant();
}

bool PropertyModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (hasIndex(index.row(), index.column())) {
    if (role == Qt::CheckStateRole) {
      if (index.column() == 0) {
        if (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked) {
          matchingprop.insert(_propertyTable[index.row()]);
          showprop.insert(_propertyTable[index.row()]);
          emit dataChanged(index, createIndex(index.row(), index.column() + 1));
        } else {
          matchingprop.erase(_propertyTable[index.row()]);
          emit dataChanged(index, index);
        }

        return true;
      }
      if (index.column() == 1) {
        if (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked)
          showprop.insert(_propertyTable[index.row()]);
        else
          showprop.erase(_propertyTable[index.row()]);
        emit dataChanged(index, index);
        return true;
      }
    }
  }
  return false;
}

Qt::ItemFlags PropertyModel::flags(const QModelIndex &) const {
  return Qt::ItemFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void PropertyModel::addProp(const string &name) {
  beginResetModel();
  _propertyTable.push_back(name);
  sort();
  endResetModel();
}

void PropertyModel::save() const {
  PorgyTlpGraphStructure::setMatchingPropertyNames(graph, matchingprop);
  PorgyTlpGraphStructure::setPropertiesToDisplay(graph, showprop);
}

bool PropertyModel::delProp(QModelIndexList list_idx) {
  bool ret = true;
  beginResetModel();
  foreach (const QModelIndex &idx, list_idx) {
    string name = _propertyTable[idx.row()];
    if (name.compare(0, 4, "view") != 0) {
      _propertyTable.erase(_propertyTable.begin() + idx.row());
      graph->getRoot()->delLocalProperty(name);
      Graph *rules_root(PorgyTlpGraphStructure::getRulesRoot(graph));
      if (rules_root->existLocalProperty(name + PorgyConstants::MATCHING_PROPERTIES_SUFFIX)) {
        rules_root->delLocalProperty(name + PorgyConstants::MATCHING_PROPERTIES_SUFFIX);
        rules_root->delLocalProperty(name + PorgyConstants::MATCHING_PROPERTIES_COMPARISON_SUFFIX);
      }
    } else
      ret = false;
  }
  endResetModel();
  return ret;
}

int PropertyModel::columnCount(const QModelIndex &) const {
  return 2;
}

int PropertyModel::rowCount(const QModelIndex &) const {
  return _propertyTable.size();
}

PorgyPropertyManager::PorgyPropertyManager(QWidget *parent, Graph *g)
    : QDialog(parent), graph(g), ui(new Ui::PorgyPropertyManager) {
  ui->setupUi(this);
  connect(ui->createPropertyButton, SIGNAL(clicked()), SLOT(create()));
  connect(ui->deletePropertyButton, SIGNAL(clicked()), SLOT(remove()));
  ui->propertyTable->setModel(new PropertyModel(g, ui->propertyTable));
  ui->propertyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->propertyTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void PorgyPropertyManager::accept() {
  // save changes to the graph
  static_cast<PropertyModel *>(ui->propertyTable->model())->save();
  QDialog::accept();
}

PorgyPropertyManager::~PorgyPropertyManager() {
  delete ui;
}

void PorgyPropertyManager::create() {
  // Use the Tulip creation dialog to generate the property.
  tlp::PropertyInterface *newProperty =
      PropertyCreationDialog::createNewProperty(graph->getRoot(), this, "string");
  if (newProperty != nullptr) {
    static_cast<PropertyModel *>(ui->propertyTable->model())->addProp(newProperty->getName());
  }
}

void PorgyPropertyManager::remove() {
  // Allows any suppression except for standard Tulip "view*" properties
  QModelIndexList selection = ui->propertyTable->selectionModel()->selectedIndexes();
  if (!static_cast<PropertyModel *>(ui->propertyTable->model())->delProp(selection)) {
    QMessageBox::warning(this, "Cannot remove Tulip properties",
                         "Reserved Tulip visual properties (starting by view*) "
                         "cannot be deleted");
  }
}
