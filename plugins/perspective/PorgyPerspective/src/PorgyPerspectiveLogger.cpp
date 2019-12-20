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

#include "PorgyPerspectiveLogger.h"
#include "ui_PorgyPerspectiveLogger.h"

#include <tulip/TulipFontIconEngine.h>

using namespace tlp;

PorgyPerspectiveLogger::PorgyPerspectiveLogger(QWidget *parent)
    : QWidget(parent, Qt::Window), _logSeverity(QtDebugMsg), _logCount(0),
      _ui(new Ui::PorgyPerspectiveLogger) {
  _ui->setupUi(this);
  _ui->clearButton->setIcon(TulipFontIconEngine::icon("fa-trash"));
  connect(_ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));
}

PorgyPerspectiveLogger::~PorgyPerspectiveLogger() {
  delete _ui;
}

static QIcon iconForType(QtMsgType type) {

  QIcon px;

  switch (type) {
  case QtDebugMsg:
  case QtInfoMsg:
    px = TulipFontIconEngine::icon("fa-info-circle");
    break;

  case QtWarningMsg:
    px = TulipFontIconEngine::icon("fa-exclamation");
    break;

  case QtCriticalMsg:
  case QtFatalMsg:
    px = TulipFontIconEngine::icon("fa-minus-circle");
    break;
  }

  return px;
}

unsigned int PorgyPerspectiveLogger::count() const {
  return _logCount;
}

void PorgyPerspectiveLogger::log(QtMsgType type, const QMessageLogContext &, const QString &msg) {
  //  if (type == QtFatalMsg) {
  //    std::cerr << msg.toUtf8().data() << std::endl;
  //    abort();
  //  }

  if (type > _logSeverity)
    _logSeverity = type;

  _logCount++;
  _ui->listWidget->addItem(new QListWidgetItem(iconForType(type), msg));
}

void PorgyPerspectiveLogger::clear() {
  _ui->listWidget->clear();
  _logCount = 0;
  _logSeverity = QtDebugMsg;
  emit cleared();
}
