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
#include <porgy/pluginparametersconfigurationdialog.h>

#include "ui_pluginparametersconfigurationdialog.h"

#include <tulip/ParameterListModel.h>
#include <tulip/TulipItemDelegate.h>
#include <tulip/TulipMetaTypes.h>

using namespace tlp;

PluginParametersConfigurationDialog::PluginParametersConfigurationDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::PluginParametersConfigurationDialog), _defalutValues(nullptr),
      _graph(nullptr), _parametersModel(nullptr) {
  ui->setupUi(this);
  ui->tableView->setItemDelegate(new tlp::TulipItemDelegate(ui->tableView));
  setStyleSheet(parent->styleSheet());
}

PluginParametersConfigurationDialog::~PluginParametersConfigurationDialog() {
  delete ui;
  delete _defalutValues;
}

tlp::DataSet PluginParametersConfigurationDialog::values() const {
  if (_parametersModel != nullptr) {
    return _parametersModel->parametersValues();
  } else {
    return tlp::DataSet();
  }
}
void PluginParametersConfigurationDialog::setDefaultValues(const tlp::DataSet &values) {
  tlp::DataSet *oldParam = _defalutValues;
  _defalutValues = new tlp::DataSet(values);
  if (_parametersModel != nullptr) {
    _parametersModel->setParametersValues(*_defalutValues);
  }

  delete oldParam;
}

void PluginParametersConfigurationDialog::setParametersList(
    const tlp::ParameterDescriptionList &parametersList) {
  _parameters = parametersList;
  updateModel();
}

void PluginParametersConfigurationDialog::setGraph(tlp::Graph *graph) {
  _graph = graph;
  updateModel();
}

void PluginParametersConfigurationDialog::updateModel() {
  tlp::ParameterListModel *oldModel = _parametersModel;

  _parametersModel = new tlp::ParameterListModel(_parameters, _graph, ui->tableView);
  if (_defalutValues != nullptr) {
    _parametersModel->setParametersValues(*_defalutValues);
  }
  ui->tableView->setModel(_parametersModel);
  ui->tableView->resizeColumnsToContents();

  if (oldModel != nullptr) {
    oldModel->deleteLater();
  }
}

DataSet PluginParametersConfigurationDialog::getParameters(
    QWidget *parent, const QString &title, const tlp::ParameterDescriptionList &parameters,
    bool &ok, tlp::DataSet *defaultValues, tlp::Graph *graph) {
  PluginParametersConfigurationDialog dialog(parent);
  dialog.setWindowTitle(title);
  dialog.setParametersList(parameters);
  dialog.setGraph(graph);
  bool deleteDataSet = false;
  if (defaultValues == nullptr) {
    defaultValues = new DataSet();
    parameters.buildDefaultDataSet(*defaultValues, graph);
  }
  dialog.setDefaultValues(*defaultValues);
  if (dialog.exec() == QDialog::Accepted) {
    ok = true;
    if (deleteDataSet) {
      delete defaultValues;
    }
    return dialog.values();
  } else {
    ok = false;
    if (deleteDataSet) {
      delete defaultValues;
    }
    return DataSet();
  }
}
