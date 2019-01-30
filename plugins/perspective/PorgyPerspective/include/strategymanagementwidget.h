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
/*!
 * \file codeeditorview.h
 * \brief Header containing the declaration of the CodeEditorView widget.
 * \author Hadrien Decoudras
 * \date 22-05-2016
 * \version 0.1
 */
#ifndef STRATEGYMANAGEMENTWIDGET_H
#define STRATEGYMANAGEMENTWIDGET_H

#include <QToolBar>
#include <QWidget>

#include "strategymanager.h"

class QMenu;

namespace Ui {
class StrategyManagementWidget;
}

/*!
 * \class CodeEditorView
 * \brief Class representing a code editor widget.
 *        The code editor is composed by a toolbar attached to customs QListView
 * and QPlainTextEdit widgets.
 *
 *        This object acts both like the Controller of the Code Editor widget
 * and like a View component.
 *        As such it gathers all the View components (CodeEditorListView and
 * CodeEditorPlainTextView) and
 *        is responsible of their management (Allocation, lifetime,
 * manipulation...).
 *
 *        In order to work properly, the Code Editor widget must use a Model
 * where all the edited source
 *        codes will be stored. The SourceCodeModel object performs this
 * function.
 *
 *        The CodeEditorView object manage the View objects by calling the data
 * from the Model and vice-versa.
 *
 *        Syntax highlighting and auto-completion features are disabled if no
 * syntax file is specified;
 *        or if no corresponding file is found; or if a parsing failure of the
 * syntax file happens.
 *        This syntax file is provided under the form of a XML file
 * (porgystrategylang.stx) which is parsed by
 *        the XmlSyntaxParser object. The syntax file come along with an XSD
 * file used for its description and
 *        its validation under the syntax directory (porgy-script.xsd).
 *
 *        The CodeEditorView widget has been designed to be used with the Porgy
 * Tulip's plugin. As such, it embeds
 *        the Porgy Scripting language, but it implements the basic
 * functionalities of a scripting language editor
 *        and it can be rapidly modified to suit others needs.
 *        In case of an adaptation, two components must be treated carefully:
 * the syntax highlighter and the auto-completer.
 *        As a result, the PodStandardXmlSyntaxParser, the XmlSyntaxParser, the
 * GeneralPurposeSyntaxHighlighter must be
 *        modified.
 */

class StrategyToolbar : public QToolBar {
  void initQAction(QAction *action, const QString &tooltip, const char *icon,
                   const bool status = true);

public:
  StrategyToolbar(QWidget *parent);

  QAction *_actionNew;
  QAction *_actionOpen;
  QAction *_actionSave;
  QAction *_actionUndo;
  QAction *_actionRedo;
  QAction *_toolButtonCopy;
  QAction *_toolButtonCut;
  QAction *_toolButtonPaste;
  QAction *_toolButtonZoomIn;
  QAction *_toolButtonZoomOut;
  QAction *_toolButtonClear;
  QAction *_toolButtonDelete;

  QMenu *_menuNew;  /*!< New menu. */
  QMenu *_menuOpen; /*!< Open menu. */
  QMenu *_menuSave; /*!< Save menu. */
};

class StrategyManagementWidget : public QWidget {
  Q_OBJECT

  Ui::StrategyManagementWidget *ui;
  QAction *_actionDuplicate; /*!< Duplicate action. */

  StrategyManager _sourceCodeModel; /*!< Model shared pointer. */

  /*!
   * \brief Connect the components of the GUI.
   */
  void _initConnections();
  void toolbarEnabled(const bool status);
  void doSave(const QString &fileName, const QString &strategy);

public:
  StrategyManagementWidget(QWidget *parent = nullptr);
  ~StrategyManagementWidget() override;
  void setGraph(tlp::Graph *g);

  /**
    * @brief Restore previously saved data.
    **/
  void setData(const tlp::DataSet &);

signals:
  void executeStrategy(QString strategyCode);
  void debugPressed(bool checked = false);

public:
  int currentStrategy() const;
  void closeAll();

  /*!
   * \brief Opens all the source code of the given directory.
   * \param dir : Path of a directory.
   */
  void openAllStrategies(const QString &dir);

  /*!
   * \brief Saves all sources in a given directory.
   * \param dir : Path of a directory.
   */
  void saveAllStrategies(const QString &dir);

private slots:
  /*!
   * \brief Add a source code.
   *        This function affects both the View and the Model.
   * \param name : Source code name.
   * \param code : Source code.
   */
  void addNewStrategy(const QString &name = "New strategy", const QString &code = "");

  /**
    * @brief Try to create a strategy from the elements selected in the root
    *trace graph. The name of the new strategy will be the
    **/
  void addStrategyFromSelectedElementsInTraceRoot();
  void updateCurrentStrategyCode(const QModelIndex &index);

  /*!
   * \brief Activates the paste button.
   */
  void deleteStrat(QModelIndexList m);

  /*!
   * \brief Import a source code file in the code editor widget.
   */
  void open();

  /*!
   * \brief Opens files on drop.
   * \param fileNames : Files dropped.
   */
  void openDrop(const QStringList &fileNames);

  /*!
   * \brief Opens all files from a directory.
   */
  void openAllStrategies();

  /*!
   * \brief Export the code of a strategy.
   */
  void save();

  /*!
   * \brief Saves all sources.
   */
  void saveAllStrategies();

  /*!
   * \brief Clear the code area.
   */
  void clear();

  /*!
   * \brief Duplicate the currently selected source code.
   *        This function affects both the View and the Model.
   */
  void duplicateSourceCode();

  /*!
   * \brief Slot triggered when the user select an other item in the QListView.
   *        The Model is updated with the previous source code selected and the
   *        View is updated with the current selected source code.
   * \param current : The currently selected QListView item.
   * \param previous : The previously selected QListView item.
   */
  void selectedSourceCodeChanged(const QModelIndex &current, const QModelIndex &previous);

  /*!
   * \brief Remove the specified source code file from the model.
   *        This function also affects the View.
   */
  void deleteSourceCode();
};

#endif
