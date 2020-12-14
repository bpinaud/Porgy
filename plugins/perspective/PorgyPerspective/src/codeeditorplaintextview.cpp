/*!
 * \file codeeditorplaintextview.cpp
 * \brief Implementation of the CodeEditorPlainTextView class declared in the
 * codeeditorplaintextview.h header.
 * \author Hadrien Decoudras
 * \date 22-05-2016
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

#include "codeeditorplaintextview.h"
#include "generalpurposesyntaxhighlighter.h"
#include "strategymanager.h"
#include "xmlsyntaxparser.h"

#include <QAbstractItemView> //required by Qt5
#include <QAction>
#include <QCompleter>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QStringListModel>

#include <tulip/TlpQtTools.h>
#include <tulip/TulipMimes.h>
#include <tulip/TulipSettings.h>

#include <portgraph/PorgyTlpGraphStructure.h>

using namespace tlp;
using namespace std;

/****************************************************************************************
 ****************************************************************************************
 *
 * Constructor.
 *
 ****************************************************************************************/
CodeEditorPlainTextView::CodeEditorPlainTextView(QWidget *parent)
    : QPlainTextEdit(parent), _syntaxHighlighter(nullptr), _syntaxCompleter(nullptr),
      _lineNumberArea(new LineNumberArea(this)) {
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();

  QFontMetrics fMetrics(font());
#if (QT_VERSION>=QT_VERSION_CHECK(5,11,0))
  setTabStopDistance(4 * fMetrics.horizontalAdvance(' '));
#else
  setTabStopWidth(4 * fMetrics.width(' '));
#endif
}

void CodeEditorPlainTextView::setSyntaxFile(const QString &syntaxFile) {
  QFileInfo f(syntaxFile);

  if (f.exists()) {
    _syntaxHighlighter = new GeneralPurposeSyntaxHighlighter(syntaxFile, document());
    QString err = _syntaxHighlighter->getParseError();

    if (err.isEmpty()) {
      QStringList keywords = _syntaxHighlighter->getKeywords();

      _syntaxCompleter = new QCompleter(this);
      _completerListModel = new QStringListModel(keywords, _syntaxCompleter);
      connect(_syntaxCompleter, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
      _syntaxCompleter->setModel(_completerListModel);
      _syntaxCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);

      _syntaxCompleter->setCompletionMode(QCompleter::PopupCompletion);
      _syntaxCompleter->setWrapAround(false);
      _syntaxCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    } else {
      QMessageBox::warning(this, "I/O Error",
                           "Failed to parse " + syntaxFile +
                               "! Auto-completion and Syntax highlighting are disabled!");
    }
  } else
    QMessageBox::warning(this, "I/O Error", "No syntax file detected! "
                                            "Auto-completion and Syntax "
                                            "highlighting are disabled!");
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Destructor.
 *
 ****************************************************************************************/
CodeEditorPlainTextView::~CodeEditorPlainTextView() {
  delete _completerListModel;
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Line numbering area rendering.
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(_lineNumberArea);
  if(!TulipSettings::isDisplayInDarkMode())
    painter.fillRect(event->rect(), Qt::lightGray);
  else
    painter.fillRect(event->rect(), Qt::black);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + static_cast<int>(blockBoundingRect(block).height());

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      if(!TulipSettings::isDisplayInDarkMode())
        painter.setPen(Qt::black);
      else
        painter.setPen(Qt::white);
      painter.setFont(font());

      painter.drawText(0, top, _lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight,
                       number);
    }

    block = block.next();
    top = bottom;
    bottom = top + static_cast<int>(blockBoundingRect(block).height());
    ++blockNumber;
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * New width of the line numbering area.
 *
 ****************************************************************************************/
int CodeEditorPlainTextView::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());

  while (max >= 10) {
    max /= 10;
    ++digits;
  }

#if(QT_VERSION >= QT_VERSION_CHECK(5,11,0))
    int space = 3 + this->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
#else
    int space = 3 + this->fontMetrics().width(QLatin1Char('9')) * digits;
#endif
  return space;
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Resize if main container is resized.
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::resizeEvent(QResizeEvent *event) {
  QPlainTextEdit::resizeEvent(event);

  QRect cr = contentsRect();
  _lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Zoom-in/Zoom-out and scroll with the mouse wheel.
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::wheelEvent(QWheelEvent *event) {
    if (event->angleDelta().y() > 0 && event->modifiers() == Qt::ControlModifier) {
    emit wheelZoomIn(1);
    zoomIn(1);
  } else if (event->modifiers() == Qt::ControlModifier) {
    emit wheelZoomOut(1);
    zoomOut(1);
  } else {
    // normal Qt behavior
    QPlainTextEdit::wheelEvent(event);
  }
}

void CodeEditorPlainTextView::setModel(StrategyManager *model) {
  _stratModel = model;
}

void CodeEditorPlainTextView::contextMenuEvent(QContextMenuEvent *e) {
  QMenu *m = createStandardContextMenu();
  m->addSeparator();
  QMenu *macro = m->addMenu("Add an existing strategy as a macro");
  for (int i = 0; i < _stratModel->count(); ++i) {
    QAction *act = macro->addAction(_stratModel->getStrategyName(i), this, SLOT(addMacro()));
    act->setData(_stratModel->getStrategyName(i));
  }
  m->exec(e->globalPos());
}

void CodeEditorPlainTextView::addMacro() {
  QAction *action = static_cast<QAction *>(sender());
  QString stratName = action->data().toString();
  insertPlainText("#" + stratName + "#");
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Autocompletion.
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::keyPressEvent(QKeyEvent *e) {
  if (_syntaxCompleter && _syntaxCompleter->popup()->isVisible()) {
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab: {
      e->ignore();
      return;
    }

    default: { break; }
    }
  }

  bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);

  if (!_syntaxCompleter || !isShortcut) {
    QPlainTextEdit::keyPressEvent(e);
  }

  const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);

  if (!_syntaxCompleter || (ctrlOrShift && e->text().isEmpty())) {
    return;
  }

  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");

  bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;

  QString completionPrefix = _textUnderCursor();

  if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < 2 ||
                      eow.contains(e->text().right(1)))) {
    _syntaxCompleter->popup()->hide();
    return;
  }

  if (completionPrefix != _syntaxCompleter->completionPrefix()) {
    _syntaxCompleter->setCompletionPrefix(completionPrefix);
    _syntaxCompleter->popup()->setCurrentIndex(_syntaxCompleter->completionModel()->index(0, 0));
  }

  QRect cRect = cursorRect();
  cRect.setWidth(_syntaxCompleter->popup()->sizeHintForColumn(0) +
                 _syntaxCompleter->popup()->verticalScrollBar()->sizeHint().width());
  _syntaxCompleter->complete(cRect);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * jknnkn.
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::focusInEvent(QFocusEvent *e) {
  if (_syntaxCompleter) {
    _syntaxCompleter->setWidget(this);
  }

  QPlainTextEdit::focusInEvent(e);
}

void CodeEditorPlainTextView::dragEnterEvent(QDragEnterEvent *dragEnterEvent) {
  const QMimeData *mimeData = dragEnterEvent->mimeData();
  if (mimeData->hasFormat(GRAPH_MIME_TYPE)) {
    const GraphMimeType *gMimeType = static_cast<const GraphMimeType *>(mimeData);
    Graph *graphPointer = gMimeType->graph();

    // Not a rule graph ignore the drag enter event.
    if (!PorgyTlpGraphStructure::isRuleGraph(graphPointer)) {
      dragEnterEvent->setDropAction(Qt::IgnoreAction);
      dragEnterEvent->accept();
      return;
    }

    dragEnterEvent->acceptProposedAction();
  }
}

void CodeEditorPlainTextView::dropEvent(QDropEvent *dropEvent) {

  const QMimeData *mimeData = dropEvent->mimeData();
  QString codeToAppend;

  if (mimeData->hasFormat(GRAPH_MIME_TYPE)) {
    const GraphMimeType *gMimeType = static_cast<const GraphMimeType *>(mimeData);
    Graph *graphPointer = gMimeType->graph();
    codeToAppend.append(tlpStringToQString(graphPointer->getName()));
  }
  // If we have code to append.
  if (!codeToAppend.isEmpty()) {
    codeToAppend = "one(" + codeToAppend + ")";
    QString currentText = document()->toPlainText();

    if (!currentText.isEmpty()) {
      int currentChar = currentText.size() - 1;

      // Search the last non space char.
      while (currentText[currentChar] == QChar(' ') && currentChar > 0) {
        --currentChar;
      }

      // If the last char is not a ; add it before inserting rule name.
      if (currentChar != 0 && currentText[currentChar] != QChar(';') &&
          currentText[currentChar] != QChar('(') && currentText[currentChar] != QChar(',')) {
        currentText.append(QChar(';'));
      }
    }

    currentText.append(codeToAppend);
    document()->setPlainText(currentText);
  }
}

void CodeEditorPlainTextView::dragMoveEvent(QDragMoveEvent *e) {
  e->acceptProposedAction();
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Update the QPlainTextEdit viewport margins in order to make room for the
 * line numbering area.
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::updateLineNumberAreaWidth(int) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Highlight the edited line.
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::highlightCurrentLine() {
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;
    QColor lineColor(QColor(Qt::gray).lighter(140));
    if(TulipSettings::isDisplayInDarkMode())
        lineColor = QColor(255-lineColor.red(), 255-lineColor.green(), 255-lineColor.blue());
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
  }

  setExtraSelections(extraSelections);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Resize the line numbering area..
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::updateLineNumberArea(const QRect &rect, int n) {
  if (n > 0)
    _lineNumberArea->scroll(0, n);
  else
    _lineNumberArea->update(0, rect.y(), _lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Zoom-in.
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::zoomIn(const int range) {
  QFont f(font());
  const int newSize = f.pointSize() + range;

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
void CodeEditorPlainTextView::zoomOut(const int s) {
  zoomIn(-s);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Complete.
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::insertCompletion(const QString &text) {
  if (_syntaxCompleter->widget() != this)
    return;

  QTextCursor tCursor(textCursor());
  int extra = text.length() - _syntaxCompleter->completionPrefix().length();
  tCursor.movePosition(QTextCursor::Left);
  tCursor.movePosition(QTextCursor::EndOfWord);
  tCursor.insertText(text.right(extra));
  setTextCursor(tCursor);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Get text under cursor.
 *
 ****************************************************************************************/
QString CodeEditorPlainTextView::_textUnderCursor() const {
  QTextCursor tCursor(textCursor());
  tCursor.select(QTextCursor::WordUnderCursor);

  return tCursor.selectedText();
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Constructor.
 *
 ****************************************************************************************/
CodeEditorPlainTextView::LineNumberArea::LineNumberArea(CodeEditorPlainTextView *parent)
    : QWidget(parent), _codeEditor(parent) {}

/****************************************************************************************
 ****************************************************************************************
 *
 * Preferred size.
 *
 ****************************************************************************************/
QSize CodeEditorPlainTextView::LineNumberArea::sizeHint() const {
  return QSize(_codeEditor->lineNumberAreaWidth(), 0);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Paint line number area..
 *
 ****************************************************************************************/
void CodeEditorPlainTextView::LineNumberArea::paintEvent(QPaintEvent *event) {
  _codeEditor->lineNumberAreaPaintEvent(event);
}
