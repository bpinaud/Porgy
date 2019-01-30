#include "AnimationViewQuickAccessBar.h"
#include "intermediatestates.h"
#include "rulepreviewwidget.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsSimpleTextItem>
#include <QPushButton>

#include <tulip/GlGraphComposite.h>
#include <tulip/GlOffscreenRenderer.h>
#include <tulip/Graph.h>
#include <tulip/TlpQtTools.h>

using namespace tlp;
using namespace std;

PorgyAnimationViewQuickAccessBar::PorgyAnimationViewQuickAccessBar(QGraphicsItem *graphicsItem,
                                                                   QWidget *parent)
    : IntermediateStatesQuickAccessBar(graphicsItem, parent),
      stateName(new QPushButton("State name", this)),
      rulePreview(new QPushButton("Rule preview", this)),
      play_pause(new QPushButton(QIcon(":/traceAnimationView/icons/32/start.png"), "", this)),
      stopButton(new QPushButton(QIcon(":/traceAnimationView/icons/32/stop.png"), "", this)),
      seek_backward(
          new QPushButton(QIcon(":/traceAnimationView/icons/32/seek-backward.png"), "", this)),
      seek_forward(
          new QPushButton(QIcon(":/traceAnimationView/icons/32/seek-forward.png"), "", this)),
      skip_backward(
          new QPushButton(QIcon(":/traceAnimationView/icons/32/skip-backward.png"), "", this)),
      skip_forward(
          new QPushButton(QIcon(":/traceAnimationView/icons/32/skip-forward.png"), "", this)),
      label(new QGraphicsSimpleTextItem(graphicsItem)),
      rulePreviewItem(new QGraphicsProxyWidget(graphicsItem)),
      rulePreviewWidget(new RulePreviewWidget()), _mode(PlayModes(STOP)) {

  QFont f(label->font());
  f.setBold(true);
  f.setPixelSize(20);
  label->setFont(f);
  label->setVisible(false);
  label->setPos(280, -20);

  rulePreviewItem->setWidget(rulePreviewWidget);
  rulePreviewItem->setVisible(false);
  rulePreviewItem->resize(200, 200);
  rulePreviewItem->setPos(0, -250);

  stopButton->setEnabled(false);
  skip_backward->setEnabled(false);
  seek_backward->setEnabled(false);

  connect(stateName, SIGNAL(clicked()), this, SLOT(stateNameSlot()));
  connect(rulePreview, SIGNAL(clicked()), this, SLOT(rulePreviewClicked()));
  connect(play_pause, SIGNAL(clicked()), this, SLOT(playClicked()));
  connect(stopButton, SIGNAL(clicked()), this, SLOT(stopClicked()));
  connect(seek_backward, SIGNAL(clicked()), this, SIGNAL(seekBackward()));
  connect(seek_forward, SIGNAL(clicked()), this, SIGNAL(seekForward()));
  connect(skip_backward, SIGNAL(clicked()), this, SLOT(seekFirst()));
  connect(skip_forward, SIGNAL(clicked()), this, SLOT(seekLast()));

  addSeparator();
  addButtonAtEnd(rulePreview);
  addButtonAtEnd(stateName);
  addSeparator();
  QVector<QAbstractButton *> vect;
  vect << skip_backward << seek_backward << stopButton << play_pause << seek_forward
       << skip_forward;
  addButtonsAtEnd(vect);
  connect(this, SIGNAL(settingsChanged()), this, SLOT(updateSettings()));
}

void PorgyAnimationViewQuickAccessBar::rulePreviewClicked() {
  rulePreviewItem->setVisible(!rulePreviewItem->isVisible());
}

bool PorgyAnimationViewQuickAccessBar::rulePreviewVisible() const {
  return rulePreviewItem->isVisible();
}

void PorgyAnimationViewQuickAccessBar::setRule(Graph *rule) {
  if (rule == nullptr) {
    rulePreviewWidget->setRuleName("No Rule");
    rulePreviewWidget->clearImg();
    rulePreviewItem->resize(200, 200);
    rulePreviewItem->setPos(0, -250);
  } else if (rule->getName() == rulePreviewWidget->getRuleName())
    return;
  else {
    rulePreviewWidget->setRuleName(tlpStringToQString(rule->getName()));
    GlOffscreenRenderer *renderer = GlOffscreenRenderer::getInstance();
    renderer->setSceneBackgroundColor(Color(255, 255, 255));
    renderer->setViewPortSize(rulePreviewItem->size().width(), rulePreviewItem->size().height());
    renderer->clearScene();
    GlGraphComposite *composite = new GlGraphComposite(rule);
    GlGraphRenderingParameters *param(composite->getRenderingParametersPointer());
    GlGraphRenderingParameters *sm_param(
        scene()->getGlGraphComposite()->getRenderingParametersPointer());
    param->setEdgeColorInterpolate(false);
    param->setAntialiasing(true);
    param->setEdgeSizeInterpolate(true);
    param->setViewNodeLabel(sm_param->isViewNodeLabel());
    param->setLabelScaled(sm_param->isLabelScaled());
    // Overwrite default properties with new one.
    renderer->addGraphCompositeToScene(composite);
#ifdef __APPLE__
    renderer->renderScene(true, false);
#else
    renderer->renderScene(true, true);
#endif
    QImage img(renderer->getImage());
    rulePreviewWidget->setRuleImg(img);
  }
}

void PorgyAnimationViewQuickAccessBar::updateSettings() {
  QColor backgroundColor(colorToQColor(scene()->getBackgroundColor()));
  QColor fontColor(255 ^ backgroundColor.red(), 255 ^ backgroundColor.green(),
                   255 ^ backgroundColor.blue());
  label->setBrush(QBrush(fontColor));
}

PorgyAnimationViewQuickAccessBar::~PorgyAnimationViewQuickAccessBar() {
  reset();
  delete rulePreviewWidget;
}

void PorgyAnimationViewQuickAccessBar::stateNameSlot() {
  label->setVisible(!label->isVisible());
}

void PorgyAnimationViewQuickAccessBar::setStateName(const QString &name) {
  label->setText(name);
}

void PorgyAnimationViewQuickAccessBar::playClicked() {
  if (_mode.testFlag(STOP) || _mode.testFlag(PAUSE)) {
    play_pause->setIcon(QIcon(":/traceAnimationView/icons/32/pause.png"));
    _mode = PlayModes(PLAY);
    stopButton->setEnabled(true);
    skip_backward->setEnabled(true);
    seek_backward->setEnabled(true);
    skip_forward->setEnabled(true);
    seek_forward->setEnabled(true);
    emit(play());
  } else if (_mode.testFlag(PLAY)) {
    _mode = PlayModes(PAUSE);
    play_pause->setIcon(QIcon(":/traceAnimationView/icons/32/start.png"));
    emit(pause());
  }
}

void PorgyAnimationViewQuickAccessBar::endAnim(const bool last) {
  _mode = PlayModes(STOP);
  play_pause->setIcon(QIcon(":/traceAnimationView/icons/32/start.png"));
  stopButton->setEnabled(false);
  if (last) {
    skip_forward->setEnabled(false);
    seek_forward->setEnabled(false);
  }
  setIntermediateStatesEnabled(true);
}

void PorgyAnimationViewQuickAccessBar::stopClicked() {
  endAnim(false);
  emit(stop());
}

void PorgyAnimationViewQuickAccessBar::seekFirst() {
  skip_backward->setEnabled(false);
  seek_backward->setEnabled(false);
  skip_forward->setEnabled(true);
  seek_forward->setEnabled(true);
  setRule(nullptr);
  emit(skipBackward());
}

void PorgyAnimationViewQuickAccessBar::seekLast() {
  skip_backward->setEnabled(true);
  seek_backward->setEnabled(true);
  skip_forward->setEnabled(false);
  seek_forward->setEnabled(false);
  emit(skipForward());
}
