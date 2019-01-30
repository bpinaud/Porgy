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

#include "TraceInformationConfigWidget.h"
#include "ui_configWidget.h"

TraceInformationConfigWidget::TraceInformationConfigWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::InformationInteractorConfig) {
  ui->setupUi(this);
}

TraceInformationConfigWidget::~TraceInformationConfigWidget() {
  delete ui;
}

bool TraceInformationConfigWidget::ShowInstance() const {
  return ui->showInstance->isChecked();
}

bool TraceInformationConfigWidget::ShowPProperty() const {
  return ui->showPProperty->isChecked();
}

bool TraceInformationConfigWidget::ShowBanProperty() const {
  return ui->showBanProperty->isChecked();
}
