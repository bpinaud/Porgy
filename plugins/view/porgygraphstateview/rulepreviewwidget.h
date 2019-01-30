#ifndef RULEPREVIEWWIDGET_H
#define RULEPREVIEWWIDGET_H

#include <QWidget>

namespace Ui {
class RulePreviewWidget;
}

class RulePreviewWidget : public QWidget {
  Q_OBJECT

  Ui::RulePreviewWidget *_ui;

public:
  explicit RulePreviewWidget(QWidget *parent = nullptr);
  ~RulePreviewWidget() override;
  void setRuleName(const QString &name);
  const std::string getRuleName() const;
  void setRuleImg(const QImage &img);
  void clearImg();

signals:

public slots:
};

#endif // RULEPREVIEWWIDGET_H
