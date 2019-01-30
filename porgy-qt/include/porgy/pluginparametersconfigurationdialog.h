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
#ifndef PLUGINPARAMETERSCONFIGURATIONDIALOG_H
#define PLUGINPARAMETERSCONFIGURATIONDIALOG_H

#include <QDialog>

#include <tulip/WithParameter.h>

#include <portgraph/porgyconf.h>

namespace Ui {
class PluginParametersConfigurationDialog;
}

namespace tlp {
class Graph;
class ParameterListModel;
}

class PORGY_SCOPE PluginParametersConfigurationDialog : public QDialog {
  Q_OBJECT

  Ui::PluginParametersConfigurationDialog *ui;
  tlp::DataSet *_defalutValues;
  tlp::ParameterDescriptionList _parameters;
  tlp::Graph *_graph;
  tlp::ParameterListModel *_parametersModel;

public:
  explicit PluginParametersConfigurationDialog(QWidget *parent = nullptr);
  ~PluginParametersConfigurationDialog() override;

  tlp::DataSet values() const;
  void setDefaultValues(const tlp::DataSet &values);

  tlp::ParameterDescriptionList parametersList() const {
    return _parameters;
  }
  void setParametersList(const tlp::ParameterDescriptionList &);

  tlp::Graph *graph() const {
    return _graph;
  }
  void setGraph(tlp::Graph *);

  static tlp::DataSet getParameters(QWidget *parent, const QString &title,
                                    const tlp::ParameterDescriptionList &parameters, bool &ok,
                                    tlp::DataSet *defaultValues = nullptr,
                                    tlp::Graph *graph = nullptr);

private:
  void updateModel();
};

#endif // PLUGINPARAMETERSCONFIGURATIONDIALOG_H
