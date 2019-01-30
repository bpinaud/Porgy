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
#ifndef PORGYSMALLMULTIPLESCONFIGURATIONWIDGET_H
#define PORGYSMALLMULTIPLESCONFIGURATIONWIDGET_H

#include <QWidget>

namespace Ui {
class PorgySmallMultiplesConfigurationWidget;
}

class PorgySMConfigurationWidget : public QWidget {
  Q_OBJECT

public:
  explicit PorgySMConfigurationWidget(QWidget *parent = nullptr);
  ~PorgySMConfigurationWidget() override;

  void setLineNumber(const unsigned num);
  unsigned int lineNumber() const;
  void setSpacing(const double &space);
  double spacing() const;

signals:
  void spaceChanged();
  void smallMultiplesLayoutUpdated();

private:
  Ui::PorgySmallMultiplesConfigurationWidget *ui;
};

#endif // PORGYSMALLMULTIPLESCONFIGURATIONWIDGET_H
