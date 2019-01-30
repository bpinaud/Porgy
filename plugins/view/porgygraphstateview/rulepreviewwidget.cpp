#include "rulepreviewwidget.h"

#include "ui_rulepreviewwidget.h"

#include <tulip/TlpQtTools.h>

RulePreviewWidget::RulePreviewWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::RulePreviewWidget) {
  _ui->setupUi(this);
}

RulePreviewWidget::~RulePreviewWidget() {
  delete _ui;
}

void RulePreviewWidget::setRuleName(const QString &name) {
  _ui->ruleName->setText(name);
}

const std::string RulePreviewWidget::getRuleName() const {
  return tlp::QStringToTlpString(_ui->ruleName->text());
}

void RulePreviewWidget::setRuleImg(const QImage &img) {
  QPixmap pixmap(QPixmap::fromImage(img));
  _ui->ruleImg->setPixmap(pixmap);
}

void RulePreviewWidget::clearImg() {
  _ui->ruleImg->setPixmap(QPixmap());
}
