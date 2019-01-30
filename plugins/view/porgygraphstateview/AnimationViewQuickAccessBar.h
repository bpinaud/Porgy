#ifndef PORGYQUICKACCESSBAR_H
#define PORGYQUICKACCESSBAR_H

#include "intermediatestates.h"

class QPushButton;
class QGraphicsSimpleTextItem;
class QGraphicsProxyWidget;
class QGraphicsItem;

class RulePreviewWidget;

namespace tlp {
class Graph;
}

class PorgyAnimationViewQuickAccessBar : public IntermediateStatesQuickAccessBar {
  Q_OBJECT

  QPushButton *stateName;
  QPushButton *rulePreview;
  QPushButton *play_pause;
  QPushButton *stopButton;
  QPushButton *seek_backward;
  QPushButton *seek_forward;
  QPushButton *skip_backward;
  QPushButton *skip_forward;
  QGraphicsSimpleTextItem *label;
  QGraphicsProxyWidget *rulePreviewItem;
  RulePreviewWidget *rulePreviewWidget;

public:
  PorgyAnimationViewQuickAccessBar(QGraphicsItem *graphicsItem, QWidget *parent = nullptr);
  ~PorgyAnimationViewQuickAccessBar() override;

  enum PlayMode { STOP = 0x1, PLAY = 0x2, PAUSE = 0x4 };
  Q_DECLARE_FLAGS(PlayModes, PlayMode)

  void endAnim(const bool last);
  void setStateName(const QString &name);
  bool rulePreviewVisible() const;
  void setRule(tlp::Graph *rule);

private slots:
  void playClicked();
  void stopClicked();
  void seekFirst();
  void seekLast();
  void stateNameSlot();
  void updateSettings();
  void rulePreviewClicked();

private:
  PlayModes _mode;

signals:
  void play();
  void pause();
  void stop();
  void seekBackward();
  void seekForward();
  void skipBackward();
  void skipForward();
};
Q_DECLARE_OPERATORS_FOR_FLAGS(PorgyAnimationViewQuickAccessBar::PlayModes)

#endif // PORGYQUICKACCESSBAR_H
