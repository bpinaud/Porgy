#include "tracequickaccessbar.h"

#include <QPushButton>

#include <tulip/GlGraphRenderingParameters.h>
#include <tulip/GlMainView.h>

using namespace tlp;

TraceQuickAccessBar::TraceQuickAccessBar(QWidget *parent)
    : QuickAccessBarImpl(
          nullptr, QuickAccessBarImpl::QuickAccessButtons(
                       QuickAccessBarImpl::SCREENSHOT | QuickAccessBarImpl::BACKGROUNDCOLOR |
                       QuickAccessBarImpl::SHOWLABELS | QuickAccessBarImpl::SIZEINTERPOLATION |
                       QuickAccessBarImpl::LABELSSCALED | QuickAccessBarImpl::SHOWEDGES),
          parent),
      _displayMetaNode(
          new QPushButton(QIcon(":/porgyicons/TraceView/labels_disabled.png"), "", this)),
      _metanodelabel(
          new QPushButton(QIcon(":/porgyicons/TraceView/labels_disabled.png"), "", this)),
      _arrows(new QPushButton(QIcon(":/porgyicons/TraceView/edges_arrow_enabled.png"), "", this)) {

  _displayMetaNode->setToolTip("Render metanodes");
  _displayMetaNode->setCheckable(true);
  _displayMetaNode->setChecked(true);
  _displayMetaNode->setFlat(true);
  _displayMetaNode->setIconSize(QSize(20, 20));
  _displayMetaNode->setMaximumSize(26, 26);
  _metanodelabel->setToolTip("Show/Hide metanode labels");
  _metanodelabel->setCheckable(true);
  _metanodelabel->setFlat(true);
  _metanodelabel->setIconSize(QSize(20, 20));
  _metanodelabel->setMaximumSize(26, 26);
  _arrows->setToolTip("Show/Hide edge arrows");
  _arrows->setCheckable(true);
  _arrows->setChecked(true);
  _arrows->setFlat(true);
  _arrows->setIconSize(QSize(20, 20));
  _arrows->setMaximumSize(26, 26);

  connect(_metanodelabel, SIGNAL(toggled(bool)), this, SLOT(togglemetanodelabel(bool)));
  connect(_arrows, SIGNAL(toggled(bool)), this, SLOT(showArrows(bool)));
  connect(_displayMetaNode, SIGNAL(toggled(bool)), this, SLOT(renderMetaNodes(bool)));
  QVector<QAbstractButton *> buttons;
  buttons << _metanodelabel << _arrows << _displayMetaNode;
  addButtonsAtEnd(buttons);
}

void TraceQuickAccessBar::renderMetaNodes(bool t) {
  renderingParameters()->setDisplayMetaNodes(t);
  _mainView->emitDrawNeededSignal();
  emit settingsChanged();
}

void TraceQuickAccessBar::togglemetanodelabel(bool t) {
  renderingParameters()->setViewMetaLabel(t);
  _metanodelabel->setIcon(QIcon(t ? ":/porgyicons/TraceView/labels_enabled.png"
                                  : ":/porgyicons/TraceView/labels_disabled.png"));
  _mainView->emitDrawNeededSignal();
  emit settingsChanged();
}

void TraceQuickAccessBar::showArrows(bool t) {
  renderingParameters()->setViewArrow(t);
  _metanodelabel->setIcon(QIcon(t ? ":/porgyicons/TraceView/edges_arrow_enabled.png"
                                  : ":/porgyicons/TraceView/edges_arrow_disabled.png"));
  _mainView->emitDrawNeededSignal();
  emit settingsChanged();
}
