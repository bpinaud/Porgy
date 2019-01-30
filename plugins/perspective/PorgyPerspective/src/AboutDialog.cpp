#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include <boost/version.hpp>

#include <tulip/TlpQtTools.h>
#include <tulip/TulipRelease.h>
#include <tulip/tulipconf.h>

#include <portgraph/PorgyRelease.h>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AboutDialog) {
  ui->setupUi(this);
  QString title("Porgy ");
  title += PORGYVERSION;
  ui->PorgyLabel->setText("<html><head/><body><p align=\"center\"><span "
                          "style=\" font-size:24pt; font-weight:600;\">" +
                          title + "</span></p></body></html>");
  ui->TulipVersionLabel->setText(
      QString("<span style=\"font-size:12pt;\"> " + QString(TULIP_VERSION) + "</span>"));
  ui->TulipRunningVersionLabel->setText(
      QString("<span style=\"font-size:12pt;\">(currently running ") +
      tlp::tlpStringToQString(tlp::getTulipVersion()) + ")</span>");
  ui->QtVersionLabel->setText(
      QString("<span style=\"font-size:12pt;\"> " + QString(QT_VERSION_STR) + "</span>"));
  ui->QtRunningVersionLabel->setText(
      QString("<span style=\"font-size:12pt;\">(currently running ") + qVersion() + ")</span>");
  ui->BoostVersionLabel->setText(
      QString("<span style=\"font-size:12pt;\"> " + QString(BOOST_LIB_VERSION) + "</span>"));
}

AboutDialog::~AboutDialog() {
  delete ui;
}
