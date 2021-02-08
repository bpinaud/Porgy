/*!
 * \file codeeditorplaintextview.h
 * \brief Header containing the declaration of the CodeEditorPlainTextView
 * widget.
 * \author Hadrien Decoudras
 * \date 21-05-2016
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

#ifndef CODEEDITORPLAINTEXTVIEW_H
#define CODEEDITORPLAINTEXTVIEW_H

#include <QPlainTextEdit>

class GeneralPurposeSyntaxHighlighter;
class QAbstractListModel;
class QCompleter;

class StrategyManager;

/*!
 * \class CodeEditorPlainTextView
 * \brief Class representing a code editing area.
 *        The CodeEditorPlainTextView class is based on a QPlainTextEdit widget.
 *        This implementation simply adds line numbering and line highlighting
 *        features.
 *
 *        This object is used to display source codes of the SourceCodeModel
 *        object. A source code is displayed when the user click on an item of
 * the
 *        CodeEditorListView widget.
 *
 *        Syntax Highlighting and auto-completion are disabled if no syntax file
 * is
 *        specified; or if no corresponding file is found; or if a parsing
 * failure
 *         of the syntax file happens.
 *
 *        This widget is controlled by the CodeEditorView object.
 *
 *        This class is based on a sample available on the Qt official website.
 *
 * \see http://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html.
 */
class CodeEditorPlainTextView : public QPlainTextEdit {
  Q_OBJECT

  StrategyManager* _stratModel;

  /*!
   * \class LineNumberArea
   * \brief Class used to display the line numbering area.
   */
  class LineNumberArea : public QWidget {
    CodeEditorPlainTextView *_codeEditor;

  public:
    /*!
     * \brief Default constructor.
     * \param editor : Parent widget.
     */
    LineNumberArea(CodeEditorPlainTextView *parent);

    /*!
     * \brief Recommended size for the widget.
     * \return The recommanded size for the widget.
     */
    QSize sizeHint() const override;

  protected:
    /*!
     * \brief Paint event override allowing to render the code editing area.
     * \param event : Paint event.
     */
    void paintEvent(QPaintEvent *event) override;
  };

public:
  /*!
   * \brief Default constructor.
   * \param parent : Parent widget.
   * \param zoom : Default zoom.
   *               Setting this parameter with a negative value results in a
   * zoom-out effect.
   *               It also sets the default zoom value. The opposite process
   * applies for
   *               positive values.
   * \param syntaxFile : Syntax file used for syntax highlightning.
   */
  CodeEditorPlainTextView(QWidget *parent = nullptr);
  void setSyntaxFile(const QString &syntaxFile);

  ~CodeEditorPlainTextView() override;

  void setModel(StrategyManager *model);

  /*!
   * \brief Render the line numbering area.
   * \param event : Paint event.
   */
  void lineNumberAreaPaintEvent(QPaintEvent *event);

  /*!
   * \brief Calculate the updated witdh value of the line numbering area.
   *        The width of this area may vary over time, depending of the number
   * of line.
   *        It is dynamically adapted to the number of digits used to represent
   * a line number.
   * \return The new width of the line number area.
   */
  int lineNumberAreaWidth();

protected:
  /*!
   * \brief Call the base class resize event and update the line numbering area.
   * \param event : Resize event.
   */
  void resizeEvent(QResizeEvent *event) override;

  /*!
   * \brief Call the base class wheel event if 'ctrl' key is not pressed.
   *        This function allow to zoom-in and zoom-out if the 'ctrl' key
   * modifier
   *        is pressed.
   * \param event : Mouse wheel event.
   */
  void wheelEvent(QWheelEvent *event) override;

  /*!
   * \brief Display the autocompletion popup if two or more consecutive letters
   *        match the model defined by the QCompleter object of the class.
   * \param e : Keyboard event.
   */
  void keyPressEvent(QKeyEvent *e) override;

  /*!
   * \brief Receive keyboard focus events.
   *        Updates the widget, depending on the focus policy.
   * \param e : Focus event.
   */
  void focusInEvent(QFocusEvent *e) override;

  void dragEnterEvent(QDragEnterEvent *e) override;
  void dropEvent(QDropEvent *e) override;
  void dragMoveEvent(QDragMoveEvent *e) override;

signals:
  /*!
   * \brief Signal used to trigger the zoomIn slot.
   */
  void wheelZoomIn(int);

  /*!
   * \brief Signal used to trigger the zoomOut slot.
   */
  void wheelZoomOut(int);

private slots:
  /*!
   * \brief Update the width of the line numbering area.
   *        This slot is triggered when the blockCountChanged signal is emited.
   */
  void updateLineNumberAreaWidth(int);

  /*!
   * \brief Highlight the currently edited line.
   *        This slot is triggered when the cursorPositionChanged signal is
   * emited.
   */
  void highlightCurrentLine();

  /*!
   * \brief Update the line numbering area.
   *        This slot is triggered when the updateRequest signal is emited.
   */
  void updateLineNumberArea(const QRect &, int);

  void contextMenuEvent(QContextMenuEvent *e) override;
  void addMacro();

public slots:


  /*!
   * \brief Autocomplete a keyword.
   */
  void insertCompletion(const QString &);

private:
  GeneralPurposeSyntaxHighlighter *_syntaxHighlighter; /*!< Syntax higlighlightning object. */

  QAbstractListModel *_completerListModel;
  QCompleter *_syntaxCompleter;    /*!< Syntax autocompletion object. */
  LineNumberArea *_lineNumberArea; /*!< Line numbering area. */

  /*!
   * \brief Return the text under the cursor.
   * \return The text under the cursor.
   */
  QString _textUnderCursor() const;
};

#endif // CODEEDITORPLAINTEXTVIEW_H
