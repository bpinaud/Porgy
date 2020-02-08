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
#include <porgy/pluginselectionwizard.h>

#include "ui_pluginselectionwizard.h"

#include <tulip/PluginModel.h>

using namespace tlp;

PluginSelectionWizard::PluginSelectionWizard(QWidget *parent)
    : QWizard(parent), ui(new Ui::PluginSelectionWizard), _model(nullptr) {
  ui->setupUi(this);
  connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), SLOT(doubleClicked(QModelIndex)));
}

PluginSelectionWizard::~PluginSelectionWizard() {
  delete ui;
}

std::list<std::string> PluginSelectionWizard::pluginList() const {
  return _model->plugins().toStdList();
}
void PluginSelectionWizard::setPluginList(const std::list<std::string> &plugins) {
  SimplePluginListModel *oldModel = _model;
  _model = new SimplePluginListModel(plugins, this);
  ui->treeView->setModel(_model);
  delete oldModel;
}

std::string PluginSelectionWizard::selectedPlugin() const {
  QModelIndexList indexes = ui->treeView->selectionModel()->selectedRows();
  return indexes.empty() ? std::string() : _model->pluginName(indexes.front());
}

void PluginSelectionWizard::doubleClicked(const QModelIndex &) {
  accept(); // Close the wizard
}
