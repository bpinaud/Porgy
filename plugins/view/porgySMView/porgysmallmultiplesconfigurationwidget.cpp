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
#include "porgysmallmultiplesconfigurationwidget.h"

#include "ui_porgysmallmultiplesconfigurationwidget.h"

PorgySMConfigurationWidget::PorgySMConfigurationWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::PorgySmallMultiplesConfigurationWidget) {
  ui->setupUi(this);

  connect(ui->space, SIGNAL(valueChanged(double)), this, SIGNAL(spaceChanged()));
  connect(ui->lineNumberSpinBox, SIGNAL(valueChanged(int)), this,
          SIGNAL(smallMultiplesLayoutUpdated()));
}

PorgySMConfigurationWidget::~PorgySMConfigurationWidget() {
  delete ui;
}

double PorgySMConfigurationWidget::spacing() const {
  return ui->space->value();
}

void PorgySMConfigurationWidget::setSpacing(const double &space) {
  ui->space->setValue(space);
}

unsigned int PorgySMConfigurationWidget::lineNumber() const {
  return ui->lineNumberSpinBox->value();
}

void PorgySMConfigurationWidget::setLineNumber(const unsigned num) {
  ui->lineNumberSpinBox->setValue(num);
}
