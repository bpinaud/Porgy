/*!
 * \file codeeditorlistview.cpp
 * \brief Implementation of the CodeEditorListView class declared in the
 * codeeditorlistview.h header.
 * \author Hadrien Decoudras
 * \date 19-06-2016
 * \version 0.1
 */

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

#include "codeeditorlistview.h"

#include <tulip/TulipFontIconEngine.h>

#include <QContextMenuEvent>
#include <QFile>
#include <QMenu>
#include <QMimeData> //required by Qt5
#include <QUrl>

using namespace tlp;

/****************************************************************************************
 ****************************************************************************************
 *
 * Default constructor.
 *
 ****************************************************************************************/
CodeEditorListView::CodeEditorListView(QWidget *parent) : QListView(parent) {}

void CodeEditorListView::contextMenuEvent(QContextMenuEvent *event) {
  QMenu menu;
  menu.setStyleSheet("QMenu::item:disabled {color: white; background-color: "
                     "qlineargradient(spread:pad, x1:0, y1:0, x2:, y2:1, "
                     "stop:0 rgb(75,75,75), stop:1 rgb(60, 60, 60))}");
  menu.addAction(TulipFontIconEngine::icon("fa-times-circle-o"),
                 "Delete strategy", this, SLOT(deleteCurrentStrat()));
  menu.addAction(TulipFontIconEngine::icon("fa-save"),
                 "Export strategy", this, SIGNAL(exportStrat()));
  menu.move(event->globalPos());
  menu.exec();
}

void CodeEditorListView::deleteCurrentStrat() {
  emit deleteStrat(selectedIndexes());
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Zoom-in/Zoom-out and scroll with the mouse wheel.
 *
 ****************************************************************************************/
void CodeEditorListView::wheelEvent(QWheelEvent *event) {
  if (event->delta() > 0 && event->modifiers() == Qt::ControlModifier) {
    emit wheelZoomIn(1);
    zoomIn(1);
  } else if (event->modifiers() == Qt::ControlModifier) {
    emit wheelZoomOut(1);
    zoomOut(1);
  } else {
    // Prend toujours le contrôle (prioritaire) sur les deux évenements
    // ci-dessus.
    // D'où la désactivation lors du zoom.
    QListView::wheelEvent(event);
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Accept everything
 *
 ****************************************************************************************/
void CodeEditorListView::dragEnterEvent(QDragEnterEvent *event) {
  event->acceptProposedAction();
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Drop.
 *
 ****************************************************************************************/
void CodeEditorListView::dropEvent(QDropEvent *event) {
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    QStringList list;

    for (int i = 0; i < urls.size(); ++i) {
      list << urls.at(i).toString().mid(7);
    }

    emit requestDropLoadSourceFiles(list);
  }
}

void CodeEditorListView::dragMoveEvent(QDragMoveEvent *event) {
  event->acceptProposedAction();
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Zoom-in.
 *
 ****************************************************************************************/
void CodeEditorListView::zoomIn(int s) {
  QFont f = font();
  const int newSize = f.pointSize() + s;

  if (newSize <= 0)
    return;

  f.setPointSize(newSize);
  setFont(f);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * zoom-out.
 *
 ****************************************************************************************/
void CodeEditorListView::zoomOut(int s) {
  zoomIn(-s);
}
