#include "intermediatestates.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QPushButton>

#include <tulip/GlGraphRenderingParameters.h>

using namespace std;

IntermediateStatesQuickAccessBar::IntermediateStatesQuickAccessBar(QGraphicsItem *graphicsItem,
                                                                   QWidget *parent)
    : QuickAccessBarImpl(graphicsItem,
                         QuickAccessBarImpl::QuickAccessButtons(
                             QuickAccessBarImpl::SCREENSHOT | QuickAccessBarImpl::BACKGROUNDCOLOR |
                             QuickAccessBarImpl::SHOWLABELS | QuickAccessBarImpl::LABELSSCALED |
                             QuickAccessBarImpl::SHOWEDGES),
                         parent),
      checkLHS(new QCheckBox("Show LHS", this)), checkRHS(new QCheckBox("Show RHS", this)),
      checkP(new QCheckBox("Show P", this)), group(new QButtonGroup(this)),thumblabel(new QPushButton("Show/Hide Thumbnail labels", this)) {


  group->setExclusive(false);
  group->addButton(checkLHS, ViewLHS);
  group->addButton(checkRHS, ViewRHS);
  group->addButton(checkP, ViewLHSP);
  connect(group, SIGNAL(buttonClicked(int)), this, SIGNAL(intermediateStatesChanged()));

  checkLHS->setChecked(true);
  checkRHS->setChecked(true);

  thumblabel->setToolTip("Show/Hide labels in thumbnails");
  thumblabel->setCheckable(true);
  thumblabel->setChecked(false);
  thumblabel->setFlat(true);
  thumblabel->setIconSize(QSize(20, 20));
  thumblabel->setMaximumSize(26, 26);
  thumblabel->setIcon(QIcon(":/Porgy/views/SMView/labels_disabled.png"));
  connect(thumblabel, SIGNAL(toggled(bool)), this, SLOT(showThumbLabels(bool)));

  QVector<QAbstractButton *> vect;
  addButtonAtEnd(thumblabel);
  addSeparator();
  vect << checkLHS << checkRHS << checkP;
  addButtonsAtEnd(vect);
}

bool IntermediateStatesQuickAccessBar::isShowThumbLabels() const {
    return thumblabel->isChecked();
}

void IntermediateStatesQuickAccessBar::showThumbLabels(bool t) {
    thumblabel->setIcon(QIcon(t ? ":/Porgy/views/SMView/labels_enabled.png"
                                    : ":/Porgy/views/SMView/labels_disabled.png"));
    emit toggleThumbLabels(t);
}

PorgyIntermediaryStateFlags IntermediateStatesQuickAccessBar::intermediateStates() const {
  PorgyIntermediaryStateFlags flags;
  if (checkP->isChecked()) {
    flags |= ViewLHSP;
  }
  if (checkLHS->isChecked()) {
    flags |= ViewLHS;
  }
  if (checkRHS->isChecked()) {
    flags |= ViewRHS;
  }
  return flags;
}

void IntermediateStatesQuickAccessBar::setIntermediateStatesEnabled(const bool f) {
  checkRHS->setEnabled(f);
  checkLHS->setEnabled(f);
  checkP->setEnabled(f);
}

void IntermediateStatesQuickAccessBar::setintermediateStates(
    const PorgyIntermediaryStateFlags &flags) {
  checkRHS->setChecked(flags.testFlag(ViewRHS));
  checkLHS->setChecked(flags.testFlag(ViewLHS));
  checkP->setChecked(flags.testFlag(ViewLHSP));
}
