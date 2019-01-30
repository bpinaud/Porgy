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
#ifndef QTIMAGETOOLTIP_H
#define QTIMAGETOOLTIP_H

#include <QBasicTimer>
#include <QLabel>
#include <QVariant>

#include <tulip/Color.h>
#include <tulip/GlGraphRenderingParameters.h>

namespace tlp {
class Graph;
}

class QtImageToolTip {
public:
  static void showImage(const QPoint &pos, const QImage &img, QWidget *w, const QRect &rect);
  static void showGraph(const QPoint &pos, tlp::Graph *graph, const QSize &size,
                        const tlp::Color &background,
                        const tlp::GlGraphRenderingParameters &parameters, QWidget *w = nullptr,
                        const QRect &rect = QRect());
  static void showWidget(const QPoint &pos, QWidget *widget, QWidget *w, const QRect &rect);

private:
  QtImageToolTip();
};

class QTipImageLabel : public QWidget {
  Q_OBJECT
public:
  QTipImageLabel(QWidget *todisplay, QWidget *w);
  ~QTipImageLabel() override;
  static QTipImageLabel *instance;

  bool eventFilter(QObject *, QEvent *) override;

  QBasicTimer hideTimer, expireTimer;

  bool fadingOut;

  void reuseTip(QWidget *todisplay);
  void hideTip();
  void hideTipImmediately();
  void setTipRect(QWidget *w, const QRect &r);
  void restartExpireTimer();
  bool tipChanged(const QPoint &pos, QWidget *todisplay, QObject *o);
  void placeTip(const QPoint &pos, QWidget *w);

  static int getTipScreen(const QPoint &pos, QWidget *w);

protected:
  void timerEvent(QTimerEvent *e) override;
  void paintEvent(QPaintEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  void resizeEvent(QResizeEvent *e) override;

#ifndef QT_NO_STYLE_STYLESHEET
public slots:
  /** \internal
    Cleanup the _q_stylesheet_parent propery.
   */
  void styleSheetParentDestroyed() {
    setProperty("_q_stylesheet_parent", QVariant());
    styleSheetParent = nullptr;
  }

private:
  QWidget *styleSheetParent;
#endif

private:
  QWidget *widget;
  QWidget *_todisplay;
  QRect rect;
};

#endif // QTIMAGETOOLTIP_H
