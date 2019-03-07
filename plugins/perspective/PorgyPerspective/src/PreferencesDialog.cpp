/**
 *
 * This file is part of Tulip (www.tulip-software.org)
 *
 * Authors: David Auber and the Tulip development Team
 * from LaBRI, University of Bordeaux
 *
 * Tulip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Tulip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 */

#include "PreferencesDialog.h"

#include "ui_PreferencesDialog.h"

#include <tulip/TulipItemDelegate.h>
#include <tulip/TulipMetaTypes.h>
#include <tulip/TulipSettings.h>

#include <QMessageBox>

using namespace tlp;

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::PreferencesDialog) {
  _ui->setupUi(this);
  _ui->graphDefaultsTable->setItemDelegate(new tlp::TulipItemDelegate(_ui->graphDefaultsTable));
  connect(_ui->graphDefaultsTable, SIGNAL(cellChanged(int, int)), this, SLOT(cellChanged(int, int)));

  // disable edition for title items (in column 0)
  for (int i = 0; i < _ui->graphDefaultsTable->rowCount(); ++i)
    _ui->graphDefaultsTable->item(i, 0)->setFlags(Qt::ItemIsEnabled);
  _ui->graphDefaultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

PreferencesDialog::~PreferencesDialog() {
  delete _ui;
}

void PreferencesDialog::usetlpbformat(int state) {
  if (state == Qt::Checked) {
    QMessageBox::warning(this, "Use tlpb file format",
                         "Be careful: using the tlpb file format means faster "
                         "Tulip project loading/saving but you will lose "
                         "compatibility with previous versions of Tulip.");
  }
}

void PreferencesDialog::writeSettings() {

  QAbstractItemModel *model = _ui->graphDefaultsTable->model();

  TulipSettings::instance().setDefaultSelectionColor(
      model->data(model->index(0, 1)).value<tlp::Color>());
  TulipSettings::instance().setDefaultLabelColor(
      model->data(model->index(1, 1)).value<tlp::Color>());
  TulipSettings::instance().setUseTlpbFileFormat(_ui->usetlpbformat->isChecked());
}

void PreferencesDialog::readSettings() {

  QAbstractItemModel *model = _ui->graphDefaultsTable->model();

  model->setData(model->index(0, 1), QVariant::fromValue<tlp::Color>(
                                         TulipSettings::instance().defaultSelectionColor()));
  model->setData(model->index(0, 2), QVariant::fromValue<tlp::Color>(
                                         TulipSettings::instance().defaultSelectionColor()));
  model->setData(model->index(1, 1),
                 QVariant::fromValue<tlp::Color>(TulipSettings::instance().defaultLabelColor()));
  model->setData(model->index(1, 2),
                 QVariant::fromValue<tlp::Color>(TulipSettings::instance().defaultLabelColor()));
  // edges selection color is not editable
  //_ui->graphDefaultsTable->item(3,2)->setFlags(Qt::ItemIsSelectable |
  // Qt::ItemIsEnabled);
}

void PreferencesDialog::cellChanged(int row, int column) {
  // force selection color to be the same for nodes & edges
  QAbstractItemModel *model = _ui->graphDefaultsTable->model();
  model->setData(model->index(row, column == 1 ? 2 : 1), model->data(model->index(row, column)));

  if (TulipSettings::instance().isUseTlpbFileFormat()) {
    _ui->usetlpbformat->setChecked(true);
  } else
    connect(_ui->usetlpbformat, SIGNAL(stateChanged(int)), this, SLOT(usetlpbformat(int)));
}
