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
#include "statusbarpluginprogress.h"
#include "ui_statusbarpluginprogress.h"

#include <QStyle>

StatusBarPluginProgress::StatusBarPluginProgress(QWidget *parent)
    : QWidget(parent), _ui(new Ui::StatusBarPluginProgress) {
  _ui->setupUi(this);
  _ui->cancelPushButton->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
  _ui->stopPushButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop));
  connect(_ui->cancelPushButton, SIGNAL(clicked()), SLOT(cancelClicked()));
  connect(_ui->stopPushButton, SIGNAL(clicked()), SLOT(stopClicked()));
}

StatusBarPluginProgress::~StatusBarPluginProgress() {
  delete _ui;
}

void StatusBarPluginProgress::setComment(const std::string &mgs) {
  _ui->label->setText("<b>" + QString::fromUtf8(mgs.c_str()) + "</b>");
  checkLastUpdate();
}

void StatusBarPluginProgress::setError(const std::string &error) {
  SimplePluginProgress::setError(error);
}

void StatusBarPluginProgress::progress_handler(int step, int max_step) {
  _ui->progressBar->setValue(step);
  _ui->progressBar->setMaximum(max_step);
  checkLastUpdate();
}

void StatusBarPluginProgress::checkLastUpdate() {
  if (_lastUpdate.msecsTo(QTime::currentTime()) > 1000) {
    QApplication::processEvents(QEventLoop::AllEvents, 1000);
    _lastUpdate = QTime::currentTime();
  }
}
void StatusBarPluginProgress::cancelClicked() {
  cancel();
}

void StatusBarPluginProgress::stopClicked() {
  stop();
}

void StatusBarPluginProgress::reset() {
  _ui->progressBar->setValue(0);
  _ui->label->setText("");
  checkLastUpdate();
}
