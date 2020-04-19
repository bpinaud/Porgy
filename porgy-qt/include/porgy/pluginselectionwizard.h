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
#ifndef PLUGINSELECTIONWIZARD_H
#define PLUGINSELECTIONWIZARD_H

#include <QWizard>

#include <portgraph/porgyconf.h>

namespace Ui {
class PluginSelectionWizard;
}

namespace tlp {
class SimplePluginListModel;
}

class QModelIndex;

/**
 * @brief The PluginSelectionWizard class is a simple dialog to choose a plugin.
 */
class PORGY_SCOPE PluginSelectionWizard : public QWizard {
  Q_OBJECT

  Ui::PluginSelectionWizard *ui;
  tlp::SimplePluginListModel *_model;

public:
  explicit PluginSelectionWizard(QWidget *parent = nullptr);
  ~PluginSelectionWizard() override;

  void setPluginList(const std::list<std::string> &plugins);

  std::string selectedPlugin() const;

private:
  void updateModel();
private slots:
  void doubleClicked(const QModelIndex &);
};

#endif // PLUGINSELECTIONWIZARD_H
