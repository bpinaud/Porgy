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

#ifndef PORGYPERSPECTIVELOGGER_H
#define PORGYPERSPECTIVELOGGER_H

#include <QDebug>
#include <QPixmap>
#include <QWidget>

namespace Ui {
class PorgyPerspectiveLogger;
}

class PorgyPerspectiveLogger : public QWidget {
  Q_OBJECT

  QtMsgType _logSeverity;
  unsigned int _logCount;

  Ui::PorgyPerspectiveLogger *_ui;

public:
  PorgyPerspectiveLogger(QWidget *parent = nullptr);
  ~PorgyPerspectiveLogger() override;
  // QPixmap icon();
  unsigned int count() const;
  void log(QtMsgType, const QMessageLogContext &, const QString &);

public slots:
  void clear();

signals:
  void cleared();
};

#endif
