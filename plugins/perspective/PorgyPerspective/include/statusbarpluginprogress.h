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
#ifndef STATUSBARPLUGINPROGRESS_H
#define STATUSBARPLUGINPROGRESS_H

#include <QWidget>
#include <tulip/SimplePluginProgress.h>

#include <QTime>
namespace Ui {
class StatusBarPluginProgress;
}

/**
 * @brief The StatusBarPluginProgress class is a simple plugin progress returned
 * by the porgy perspective when the progress function is called.
 * This widget is designed to be set in the progress bar
 */
class StatusBarPluginProgress : public QWidget, public tlp::SimplePluginProgress {
  Q_OBJECT
  Ui::StatusBarPluginProgress *_ui;
  QTime _lastUpdate;
  void checkLastUpdate();

public:
  explicit StatusBarPluginProgress(QWidget *parent = nullptr);
  ~StatusBarPluginProgress() override;
  void setComment(const std::string &mgs) override;
  void setError(const std::string &error) override;
  void progress_handler(int step, int max_step) override;
  bool isPreviewMode() const override {
    return false;
  }
  void reset();
private slots:
  void cancelClicked();
  void stopClicked();
};

#endif // STATUSBARPLUGINPROGRESS_H
