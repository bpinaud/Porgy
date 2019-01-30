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
#include "qtimagetooltip.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QStylePainter>
#include <QToolTip>
#include <QVBoxLayout>

#include <porgy/graphsnapshotmanager.h>

#include <tulip/Graph.h>
#include <tulip/TlpQtTools.h>

QtImageToolTip::QtImageToolTip() {}

void QtImageToolTip::showWidget(const QPoint &pos, QWidget *widget, QWidget *w, const QRect &rect) {

  if (QTipImageLabel::instance &&
      QTipImageLabel::instance->isVisible()) { // a tip does already exist
    if (widget == nullptr) {                   // empty text means hide current tip
      QTipImageLabel::instance->hideTip();
      return;
    } else if (!QTipImageLabel::instance->fadingOut) {
      // If the tip has changed, reuse the one
      // that is showing (removes flickering)
      QPoint localPos = pos;
      if (w)
        localPos = w->mapFromGlobal(pos);
      if (QTipImageLabel::instance->tipChanged(localPos, widget, w)) {
        QTipImageLabel::instance->reuseTip(widget);
        QTipImageLabel::instance->setTipRect(w, rect);
        QTipImageLabel::instance->placeTip(pos, w);
      }
      return;
    }
  }

  if (widget != nullptr) { // no tip can be reused, create new tip:
#ifndef Q_WS_WIN
    new QTipImageLabel(widget, w); // sets QTipLabel::instance to itself
#else
    // On windows, we can't use the widget as parent otherwise the window will
    // be
    // raised when the tooltip will be shown
    new QTipImageLabel(widget,
                       QApplication::desktop()->screen(QTipImageLabel::getTipScreen(pos, w)));
#endif
    QTipImageLabel::instance->setTipRect(w, rect);
    QTipImageLabel::instance->placeTip(pos, w);
    QTipImageLabel::instance->setObjectName(QLatin1String("qtooltip_imagelabel"));
    QTipImageLabel::instance->show();
  }
}

void QtImageToolTip::showGraph(const QPoint &pos, tlp::Graph *graph, const QSize &size,
                               const tlp::Color &background,
                               const tlp::GlGraphRenderingParameters &parameters, QWidget *w,
                               const QRect &rect) {
  QPixmap pixmap = GraphSnapshotManager::takeSingleSnapshot(graph, size, background, parameters);
  QWidget *widget = new QWidget(w);
  widget->setLayout(new QVBoxLayout(widget));
  QLabel *nameLabel = new QLabel(widget);
  nameLabel->setObjectName("graphNameLabel");
  nameLabel->setText(tlp::tlpStringToQString(graph->getName()));
  nameLabel->setAlignment(Qt::AlignCenter);
  QLabel *pictureLabel = new QLabel(widget);
  pictureLabel->setObjectName("graphPictureLabel");
  // Remove alpha channel on the image
  pictureLabel->setPixmap(
      QPixmap::fromImage(pixmap.toImage().convertToFormat(QImage::Format_RGB32)));
  widget->layout()->addWidget(nameLabel);
  widget->layout()->addWidget(pictureLabel);

  showWidget(pos, widget, w, rect);
}

void QtImageToolTip::showImage(const QPoint &pos, const QImage &img, QWidget *w,
                               const QRect &rect) {
  QPixmap pixmap = QPixmap::fromImage(img);
  QLabel *label = new QLabel();
  label->setPixmap(pixmap);

  showWidget(pos, label, w, rect);
}

QTipImageLabel *QTipImageLabel::instance = nullptr;

QTipImageLabel::QTipImageLabel(QWidget *todisplay, QWidget *w)
#ifndef QT_NO_S_STYLESHEET
    : QWidget(w, Qt::ToolTip | Qt::BypassGraphicsProxyWidget), styleSheetParent(nullptr),
      widget(nullptr), _todisplay(nullptr)
#else
    : QWidget(w, Qt::ToolTip | Qt::BypassGraphicsProxyWidget), widget(0), _todisplay(nullptr)
#endif
{
  delete instance;
  instance = this;
  setForegroundRole(QPalette::ToolTipText);
  setBackgroundRole(QPalette::ToolTipBase);
  setPalette(QToolTip::palette());
  ensurePolished();
  //    setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0,
  //    this));
  int margin = 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, nullptr, this);
  setLayout(new QVBoxLayout(this));
  layout()->setContentsMargins(margin, margin, margin, margin);
  //    setFrameStyle(QFrame::NoFrame);
  //    setAlignment(Qt::AlignLeft);
  //    setIndent(1);
  qApp->installEventFilter(this);
  setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, nullptr, this) / 255.0);
  setMouseTracking(true);
  fadingOut = false;
  reuseTip(todisplay);
}

void QTipImageLabel::restartExpireTimer() {
  int time = 10000 + 40 /** qMax(0, text().length()-100)*/;
  expireTimer.start(time, this);
  hideTimer.stop();
}

void QTipImageLabel::reuseTip(QWidget *todisplay) {
#ifndef QT_NO_STYLE_STYLESHEET
  if (styleSheetParent) {
    disconnect(styleSheetParent, SIGNAL(destroyed()), QTipImageLabel::instance,
               SLOT(styleSheetParentDestroyed()));
    styleSheetParent = nullptr;
  }
#endif
  if (_todisplay != nullptr) {
    layout()->removeWidget(_todisplay);
    _todisplay->deleteLater();
    _todisplay = nullptr;
  }
  if (layout()) {
    _todisplay = todisplay;
    layout()->addWidget(todisplay);
  }
  QSize extra(1, 0);
  resize(sizeHint() + extra);
  restartExpireTimer();
}

void QTipImageLabel::paintEvent(QPaintEvent *ev) {
  QStylePainter p(this);
  QStyleOptionFrame opt;
  opt.init(this);
  p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
  p.end();

  QWidget::paintEvent(ev);
}

void QTipImageLabel::resizeEvent(QResizeEvent *e) {
  QStyleHintReturnMask frameMask;
  QStyleOption option;
  option.init(this);
  if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
    setMask(frameMask.region);

  QWidget::resizeEvent(e);
}

void QTipImageLabel::mouseMoveEvent(QMouseEvent *e) {
  if (rect.isNull())
    return;
  QPoint pos = e->globalPos();
  if (widget)
    pos = widget->mapFromGlobal(pos);
  if (!rect.contains(pos))
    hideTip();
  QWidget::mouseMoveEvent(e);
}

QTipImageLabel::~QTipImageLabel() {
  instance = nullptr;
}

void QTipImageLabel::hideTip() {
  if (!hideTimer.isActive())
    hideTimer.start(300, this);
}

void QTipImageLabel::hideTipImmediately() {
  close(); // to trigger QEvent::Close which stops the animation
  deleteLater();
}

void QTipImageLabel::setTipRect(QWidget *w, const QRect &r) {
  if (!rect.isNull() && !w)
    qWarning("QToolTip::setTipRect: Cannot pass nullptr widget if rect is set");
  else {
    widget = w;
    rect = r;
  }
}

void QTipImageLabel::timerEvent(QTimerEvent *e) {
  if (e->timerId() == hideTimer.timerId() || e->timerId() == expireTimer.timerId()) {
    hideTimer.stop();
    expireTimer.stop();
#if defined(Q_WS_MAC) && !defined(QT_NO_EFFECTS)
    /*        if (QApplication::isEffectEnabled(Qt::UI_FadeTooltip)){
        // Fade out tip on mac (makes it invisible).
        // The tip will not be deleted until a new tip is shown.

        // DRSWAT - Cocoa
        macWindowFade(qt_mac_window_for(this));
        QTipImageLabel::instance->fadingOut = true; // will never be false
    again.
    }
    else*/
    hideTipImmediately();
#else
    hideTipImmediately();
#endif
  }
}

bool QTipImageLabel::eventFilter(QObject *o, QEvent *e) {
  switch (e->type()) {
#ifdef Q_WS_MAC
  case QEvent::KeyPress:
  case QEvent::KeyRelease: {
    int key = static_cast<QKeyEvent *>(e)->key();
    Qt::KeyboardModifiers mody = static_cast<QKeyEvent *>(e)->modifiers();
    if (!(mody & Qt::KeyboardModifierMask) && key != Qt::Key_Shift && key != Qt::Key_Control &&
        key != Qt::Key_Alt && key != Qt::Key_Meta)
      hideTip();
    break;
  }
#endif
  case QEvent::Leave:
    hideTip();
    break;
  case QEvent::WindowActivate:
  case QEvent::WindowDeactivate:
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseButtonDblClick:
  case QEvent::FocusIn:
  case QEvent::FocusOut:
  case QEvent::Wheel:
    hideTipImmediately();
    break;

  case QEvent::MouseMove:
    if (o == widget && !rect.isNull() && !rect.contains(static_cast<QMouseEvent *>(e)->pos()))
      hideTip();
  default:
    break;
  }
  return false;
}

int QTipImageLabel::getTipScreen(const QPoint &pos, QWidget *w) {
  if (QApplication::desktop()->isVirtualDesktop())
    return QApplication::desktop()->screenNumber(pos);
  else
    return QApplication::desktop()->screenNumber(w);
}

/**
  * Compute the position of the tip in function of its size and its appearance.
  **/
void QTipImageLabel::placeTip(const QPoint &pos, QWidget *w) {
#ifndef QT_NO_STYLE_STYLESHEET
  if (testAttribute(Qt::WA_StyleSheet)) {
    // the stylesheet need to know the real parent
    QTipImageLabel::instance->setProperty("_q_stylesheet_parent", qVariantFromValue(w));
    // we force the style to be the QStyleSheetStyle, and force to clear the
    // cache as well.
    QTipImageLabel::instance->setStyleSheet(QLatin1String("/* */"));

    // Set up for cleaning up this later...
    QTipImageLabel::instance->styleSheetParent = w;
    if (w) {
      connect(w, SIGNAL(destroyed()), QTipImageLabel::instance, SLOT(styleSheetParentDestroyed()));
    }
  }
#endif // QT_NO_STYLE_STYLESHEET

#ifdef Q_WS_MAC
  // When in full screen mode, there is no Dock nor Menu so we can use
  // the whole screen for displaying the tooltip. However when not in
  // full screen mode we need to save space for the dock, so we use
  // availableGeometry instead.
  // extern bool qt_mac_app_fullscreen; //qapplication_mac.mm
  QRect screen;
  // if(qt_mac_app_fullscreen)
  //    screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));
  // else
  screen = QApplication::desktop()->availableGeometry(getTipScreen(pos, w));
#else
  QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));
#endif

  QPoint p = pos;
  p += QPoint(2,
#ifdef Q_WS_WIN
              21
#else
              16
#endif
              );
  if (p.x() + this->width() > screen.x() + screen.width())
    p.rx() -= 4 + this->width();
  if (p.y() + this->height() > screen.y() + screen.height())
    p.ry() -= 24 + this->height();
  if (p.y() < screen.y())
    p.setY(screen.y());
  if (p.x() + this->width() > screen.x() + screen.width())
    p.setX(screen.x() + screen.width() - this->width());
  if (p.x() < screen.x())
    p.setX(screen.x());
  if (p.y() + this->height() > screen.y() + screen.height())
    p.setY(screen.y() + screen.height() - this->height());
  this->move(p);
}

bool QTipImageLabel::tipChanged(const QPoint &pos, QWidget *todisplay, QObject *o) {
  if (QTipImageLabel::instance->_todisplay != todisplay)
    return true;

  if (o != widget)
    return true;

  if (!rect.isNull())
    return !rect.contains(pos);
  else
    return false;
}
