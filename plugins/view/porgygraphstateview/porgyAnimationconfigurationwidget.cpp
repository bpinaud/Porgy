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
#include "porgyAnimationconfigurationwidget.h"

#include "ui_porgyAnimationconfigurationwidget.h"

PorgyAnimationConfigurationWidget::PorgyAnimationConfigurationWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::PorgyAnimationConfigurationWidget) {
  ui->setupUi(this);
}

PorgyAnimationConfigurationWidget::~PorgyAnimationConfigurationWidget() {
  delete ui;
}

unsigned int PorgyAnimationConfigurationWidget::animationDuration() const {
  return ui->animationDurationSpinBox->value();
}

void PorgyAnimationConfigurationWidget::setAnimationDuration(unsigned duration) {
  ui->animationDurationSpinBox->setValue(duration);
}
