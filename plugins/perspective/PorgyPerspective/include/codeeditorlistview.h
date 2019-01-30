/*!
 * \file codeeditorlistview.h
 * \brief Header containing the declaration of the CodeEditorListView widget.
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

#ifndef CODEEDITORLISTVIEW_H
#define CODEEDITORLISTVIEW_H

#include <QListView>

/*!
 * \class CodeEditorListView
 * \brief The CodeEditorListView class allows to display source codes which are
 * currently edited.
 *        This class has been derived from QListView in order to implement
 * specific zoom and
 *        drag'n'drop controls. The CodeEditorListView object is designed to be
 * used with the
 *        SourceCodeModel object, a custom Model associated to this view
 * component through the use
 *        of the setModel method already defined in the parent classes.
 *
 *        This widget is controlled by the CodeEditorView object.
 */
class CodeEditorListView : public QListView {
  Q_OBJECT

public:
  /*!
   * \brief Default constructor.
   * \param parent : Parent widget.
   * \param model : Object used for Model/View binding.
   */
  CodeEditorListView(QWidget *parent = nullptr);

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  /*!
   * \brief Call the base class wheel event if 'ctrl' key is not pressed.
   *        This function allow to zoom-in and zoom-out if the 'ctrl' key
   * modifier
   *        is pressed.
   * \param event : Mouse wheel event.
   */
  void wheelEvent(QWheelEvent *event) override;

  /*!
   * \brief Envent triggered when the user drag an item of the list view widget.
   * \param event : Drag event.
   */
  void dragEnterEvent(QDragEnterEvent *event) override;

  /*!
   * \brief Event triggered when the user drop source code files on the list
   * view widget.
   *        The files dropped from the outside of the software are processed by
   * the Model
   *        and displayed in the View.
   * \param event : Drop event.
   */
  void dropEvent(QDropEvent *event) override;

  /*!
   * \brief Event triggered when the user move an item over the list view
   * widget.
   * \param event
   */
  void dragMoveEvent(QDragMoveEvent *event) override;

signals:
  /*!
   * \brief Signal used to trigger the zoomIn slot.
   */
  void wheelZoomIn(int);

  /*!
   * \brief Signal used to trigger the zoomOut slot.
   */
  void wheelZoomOut(int);

signals:
  /*!
   * \brief Signal triggered when one or more source code files are dropped from
   * the
   *        outside of the software.
   */
  void requestDropLoadSourceFiles(const QStringList &);
  void deleteStrat(QModelIndexList);
  void exportStrat();

public slots:
  /*!
   * \brief Zoom-in function of the QTextEdit object.
   *        This slot is intended to be used by a parent object. It allows to
   *        magnify the font of the list view.
   */
  void zoomIn(int s = 1);

  /*!
   * \brief Zoom-out function of the QTextEdit object.
   *        This slot is intended to be used by a parent object. It allows to
   *        reduce the font size of the list view.
   */
  void zoomOut(int s = 1);

private slots:
  void deleteCurrentStrat();
};

#endif // CODEEDITORLISTVIEW_H
