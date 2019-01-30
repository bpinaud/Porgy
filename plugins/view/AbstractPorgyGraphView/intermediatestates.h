#ifndef PORGYINTERQUICKACCESSBAR_H
#define PORGYINTERQUICKACCESSBAR_H

#include <tulip/QuickAccessBar.h>

#include "graphstatemanager.h"

class QCheckBox;
class QButtonGroup;

class IntermediateStatesQuickAccessBar : public tlp::QuickAccessBarImpl {
  Q_OBJECT

  QCheckBox *checkLHS;
  QCheckBox *checkRHS;
  QCheckBox *checkP;
  QButtonGroup *group;
  QPushButton *thumblabel;

public:
  IntermediateStatesQuickAccessBar(QGraphicsItem *graphicsItem, QWidget *parent = nullptr);
  PorgyIntermediaryStateFlags intermediateStates() const;
  void setintermediateStates(const PorgyIntermediaryStateFlags &flags);
  void setIntermediateStatesEnabled(bool);
  bool isShowThumbLabels() const;

signals:
  void intermediateStatesChanged();
  void toggleThumbLabels(bool);

public slots:
  void showThumbLabels(bool);

};

#endif
