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

#include <QMainWindow>
#include <QMessageBox>

#include <porgy/PortNodeQt.h>
#include <porgy/PortQt.h>

#include <portgraph/PorgyConstants.h>
#include <portgraph/PorgyTlpGraphStructure.h>

#include "addPortNodeConfigurationWidget.h"
#include "createPort.h"
#include "createPortNode.h"
#include "ui_DialogPortNode.h"

#include <tulip/TlpQtTools.h>

using namespace std;
using namespace tlp;

CreatePortNode::CreatePortNode(QWidget *parent) : QDialog(parent), ui(new Ui::DialogPortNode) {
  init(tlp::colorToQColor(PorgyConstants::DEFAULT_COLOR));
}

CreatePortNode::CreatePortNode(QWidget *parent, const PortNodeQt &pN)
    : QDialog(parent), ui(new Ui::DialogPortNode) {
  init(pN.getColor());
  ui->lineEditName->setText(pN.getName());
  for (const PortQt &p : pN.getPorts()) {
    QTableWidgetItem *nameItem = new QTableWidgetItem();
    int n = ui->PortList->rowCount();
    ui->PortList->insertRow(n);
    nameItem->setText(p.getName());
    ui->PortList->setItem(n, 0, nameItem);
    ports[nameItem] = p;
  }
}

CreatePortNode::~CreatePortNode() {
  delete ui;
}

vector<PortQt> CreatePortNode::getPortQts() const {
  vector<PortQt> v;
  v.reserve(ports.size());
  for (const auto &p : ports) {
    v.push_back(p.second);
  }
  return v;
}

void CreatePortNode::init(const QColor &color) {
  ui->setupUi(this);
  ui->pushButtonColor->setDialogParent(this);
  ui->pushButtonColor->setColor(color);
  connect(ui->pushButtonAdd, SIGNAL(clicked()), this, SLOT(showPortDialog()));
  connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(removePort()));
  connect(ui->pushButtonModif, SIGNAL(clicked()), this, SLOT(showPortDialogModify()));
  connect(ui->PortList, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this,
          SLOT(showPortDialogModify(QTableWidgetItem *)));
  connect(ui->PortList, SIGNAL(itemSelectionChanged()), this, SLOT(enableButtons()));
}

void CreatePortNode::done(int r) {
  if (QDialog::Accepted == r) { // ok was pressed
    if (ports.empty()) {
      QMessageBox::warning(parentWidget(), "Cannot create Portnode",
                           "It is not possible to create a portnode with no "
                           "port. Add at least one.");
      return;
    }
  }

  QDialog::done(r);
}

void CreatePortNode::showPortDialog() {
  CreatePort dialog(this);
  if (dialog.exec()) {
    PortQt port(dialog.getName(), dialog.getGlyph(), dialog.getColor());
    QTableWidgetItem *nameItem = new QTableWidgetItem();
    int n = ui->PortList->rowCount();
    ui->PortList->insertRow(n);
    ui->PortList->setItem(n, 0, nameItem);
    nameItem->setText(dialog.getName());
    ports[nameItem] = port;
  }
}

void CreatePortNode::showPortDialogModify(QTableWidgetItem *item) {
  QTableWidgetItem *nameItem = ui->PortList->item(ui->PortList->currentRow(), 0);
  PortQt &port = ports[item == nullptr ? nameItem : item];
  CreatePort dialog(this, port);
  if (dialog.exec()) {
    port.setName(dialog.getName());
    port.setShape(dialog.getGlyph());
    port.setColor(dialog.getColor());
    nameItem->setText(port.getName());
  }
}

void CreatePortNode::removePort() {
  ports.erase(ui->PortList->item(ui->PortList->currentRow(), 0));
  ui->PortList->removeRow(ui->PortList->currentRow());
}

void CreatePortNode::enableButtons() {
  if (ui->PortList->selectedItems().isEmpty()) {
    ui->pushButtonDelete->setEnabled(false);
    ui->pushButtonModif->setEnabled(false);
  } else {
    ui->pushButtonDelete->setEnabled(true);
    ui->pushButtonModif->setEnabled(true);
  }
}

QString CreatePortNode::getName() const {
  return ui->lineEditName->text();
}

QColor CreatePortNode::getColor() const {
  return ui->pushButtonColor->color();
}
