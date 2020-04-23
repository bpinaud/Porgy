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
 * \file codeeditorview.cpp
 * \brief Implementation of the CodeEditorView class declared in the
 * codeeditorview.h header.
 * \author Hadrien Decoudras
 * \date 22-05-2016
 * \version 0.2
 */

#include "ui_strategymanagementwidget.h"

#include "codeeditorplaintextview.h"
#include "strategymanagementwidget.h"

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Trace.h>

#include <tulip/BooleanProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/TlpQtTools.h>
#include <tulip/TulipFontIconEngine.h>

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
#include <QDesktopServices>

using namespace tlp;
using namespace std;

int StrategyManagementWidget::currentStrategy() const {
  QModelIndexList rows = ui->listView->selectionModel()->selectedRows();

  assert(rows.size() < 2);
  if (rows.empty()) {
    return -1;
  } else {
    return rows.front().row();
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Open strategy.
 *
 ****************************************************************************************/
void StrategyManagementWidget::open() {
  QStringList fNames =
      QFileDialog::getOpenFileNames(this, "Open a strategy code file", "", "Strategy File (*.sty)");
  if (!fNames.isEmpty())
    openDrop(fNames);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Drop open.
 *
 ****************************************************************************************/
void StrategyManagementWidget::openDrop(const QStringList &fileNames) {
  foreach (const QString &name, fileNames) {
    QFile f(name);

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QMessageBox::critical(this, "Unable to open file", f.errorString());
      return;
    }
    QString content(f.readAll());

    unsigned index = _sourceCodeModel.indexOfStrategy(name);

    if (index == UINT_MAX) {
      StrategyManager::StrategyErrors error;
      QFileInfo fInfo(f);
      index = _sourceCodeModel.addStrategy(fInfo.baseName(), error, content);
    } else {
      QMessageBox::StandardButton overwrite =
          QMessageBox::question(this, "Overwrite existing strategy?",
                                "A strategy with the same name already exists.\n Are you sure you "
                                "want to overwrite it?",
                                QMessageBox::Yes | QMessageBox::No);
      if (overwrite == QMessageBox::Yes) {
        _sourceCodeModel.setStrategyCode(index, content);
      }
    }

    f.close();
    ui->listView->selectionModel()->setCurrentIndex(_sourceCodeModel.index(index),
                                                    QItemSelectionModel::ClearAndSelect);
  }

  if (!_sourceCodeModel.empty()) {
    toolbarEnabled(true);
  }
}

void StrategyManagementWidget::toolbarEnabled(const bool status) {
  ui->listView->setEnabled(status);
  ui->plainTextEdit->setEnabled(status);
  _actionDuplicate->setEnabled(status);
  ui->toolbar->_toolButtonPaste->setEnabled(status);
  ui->toolbar->_actionSave->setEnabled(status);
  ui->toolbar->_toolButtonClear->setEnabled(status);
  ui->toolbar->_toolButtonDelete->setEnabled(status);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Open all files.
 *
 ****************************************************************************************/
void StrategyManagementWidget::openAllStrategies() {
  QString dName = QFileDialog::getExistingDirectory(this, "Open all strategy code");
  QDir directory(dName);

  QStringList list;

  if (directory.exists()) {
    foreach (const QString &file,
             directory.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name)) {
      if (file.contains(".sty")) {
        list << directory.absoluteFilePath(file);
      }
    }

    if (!list.empty()) {
      openDrop(list);
    }
  }
}

void StrategyManagementWidget::doSave(const QString &fileName, const QString &strategy) {
  if (!fileName.isEmpty()) {
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
      QMessageBox::critical(this, "Unable to open file", f.errorString());
      return;
    }
    QTextStream out(&f);
    out << strategy;
    f.close();
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Save strategy.
 *
 ****************************************************************************************/
void StrategyManagementWidget::save() {
  QString fileName =
      QFileDialog::getSaveFileName(this, "Save current strategy", "", "Strategy File (*.sty)");
  if (!fileName.isEmpty()) {
    if (!fileName.endsWith(".sty"))
      fileName += ".sty";
    doSave(fileName, ui->plainTextEdit->toPlainText().toUtf8());
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Save all sources.
 *
 ****************************************************************************************/
void StrategyManagementWidget::saveAllStrategies() {
  QString dName = QFileDialog::getExistingDirectory(this, "Save all strategies");

  if (!dName.isEmpty()) {
    for (int i = 0; i < _sourceCodeModel.count(); ++i) {
      if (i == ui->listView->selectionModel()->currentIndex().row()) {
        _sourceCodeModel.setStrategyName(
            i, ui->listView->selectionModel()->currentIndex().data().toString());
        _sourceCodeModel.setStrategyCode(i, ui->plainTextEdit->document()->toPlainText());
      }

      QString savePath = dName + "/" + _sourceCodeModel.getStrategyName(i) + ".sty";
      doSave(savePath, _sourceCodeModel.getStrategy(i));
    }
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Clear code area.
 *
 ****************************************************************************************/
void StrategyManagementWidget::clear() {
  if (!ui->plainTextEdit->toPlainText().isEmpty()) {
    QMessageBox::StandardButton res =
        QMessageBox::question(this, "Clear strategy?", "The strategy will be deleted.<br>This "
                                                       "operation cannot be undone. Do you really "
                                                       "want to continue?",
                              QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::Yes) {
      ui->plainTextEdit->clear();
    }
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Add Source code.
 *
 ****************************************************************************************/
void StrategyManagementWidget::addNewStrategy(const QString &name, const QString &code) {
  QString newName(name);
  unsigned cpt = 0;
  // be sure to have a unique name
  while (_sourceCodeModel.indexOfStrategy(newName) != UINT_MAX) {
    ++cpt;
    newName = name + QString("_") + QString::number(cpt);
  }

  StrategyManager::StrategyErrors error;
  unsigned int index = _sourceCodeModel.addStrategy(newName, error, code);
  ui->listView->selectionModel()->setCurrentIndex(_sourceCodeModel.index(index),
                                                  QItemSelectionModel::ClearAndSelect);

  if (_sourceCodeModel.count() == 1) {
    toolbarEnabled(true);
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Open all source codes.
 *
 ****************************************************************************************/
void StrategyManagementWidget::openAllStrategies(const QString &dir) {
  QDir directory(dir);
  QStringList list;

  if (directory.exists()) {
    foreach (const QString &file,
             directory.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name))
      list << directory.absoluteFilePath(file);

    if (!list.empty())
      openDrop(list);
  }
}

void StrategyManagementWidget::closeAll() {
  unsigned nb_strat(_sourceCodeModel.count());
  for (unsigned i = 0; i < nb_strat; ++i) {
    _sourceCodeModel.removeStrategy(0);
  }
  toolbarEnabled(false);
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Save all source codes.
 *
 ****************************************************************************************/
void StrategyManagementWidget::saveAllStrategies(const QString &dir) {
  QDir directory(dir);

  if (directory.exists()) {
    for (int i = 0; i < _sourceCodeModel.count(); ++i) {
      if (i == ui->listView->selectionModel()->currentIndex().row()) {
        _sourceCodeModel.setStrategyName(
            i, ui->listView->selectionModel()->currentIndex().data().toString());
        _sourceCodeModel.setStrategyCode(i, ui->plainTextEdit->document()->toPlainText());
      }

      QString savePath = dir + "/" + _sourceCodeModel.getStrategyName(i) + ".sty";
      QFile f(savePath);

      if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::information(this, "Unable to open file", f.errorString());
        continue;
      }

      QTextStream o(&f);
      o << _sourceCodeModel.getStrategy(i);

      f.close();
    }
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Duplicate source code.
 *
 ****************************************************************************************/
void StrategyManagementWidget::duplicateSourceCode() {
  addNewStrategy(
      _sourceCodeModel.getStrategyName(ui->listView->selectionModel()->currentIndex().row()),
      ui->plainTextEdit->document()->toPlainText());
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Update model with written source code.
 *
 ****************************************************************************************/
void StrategyManagementWidget::selectedSourceCodeChanged(const QModelIndex &current,
                                                         const QModelIndex &previous) {
  if (previous.row() > -1 && _sourceCodeModel.count() > previous.row()) {
    _sourceCodeModel.setStrategyCode(previous.row(), ui->plainTextEdit->document()->toPlainText());
  }
  if (current.row() > -1)
    ui->plainTextEdit->setPlainText(_sourceCodeModel.getStrategy(current.row()));
  else if (current.row() == -1)
    ui->plainTextEdit->clear();
}

void StrategyManagementWidget::deleteSourceCode() {
  QMessageBox::StandardButton res = QMessageBox::question(
      this, "Delete strategy?", "Do you really want to delete the active strategy?",
      QMessageBox::Yes | QMessageBox::No);

  if (res == QMessageBox::Yes) {
    deleteStrat(QModelIndexList() << ui->listView->selectionModel()->currentIndex());
  }
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Tool bar initialization.
 *
 ****************************************************************************************/
void StrategyToolbar::initQAction(QAction *action, const QString &tooltip, const char *icon,
                                  const bool status) {
  action->setToolTip(tooltip);
  action->setIcon(TulipFontIconEngine::icon(icon));
  action->setEnabled(status);
  addAction(action);
}

StrategyToolbar::StrategyToolbar(QWidget *parent)
    : QToolBar(parent), _actionNew(new QAction(this)), _actionOpen(new QAction(this)),
      _actionSave(new QAction(this)), _actionUndo(new QAction(this)),
      _actionRedo(new QAction(this)), _toolButtonCopy(new QAction(this)),
      _toolButtonCut(new QAction(this)), _toolButtonPaste(new QAction(this)),
      _toolButtonZoomIn(new QAction(this)), _toolButtonZoomOut(new QAction(this)),
      _toolButtonClear(new QAction(this)), _toolButtonDelete(new QAction(this)), _toolButtonHelp(new QAction(this)),
      _menuNew(new QMenu(this)), _menuOpen(new QMenu(this)), _menuSave(new QMenu(this)) {

  initQAction(_actionNew, "New Strategy...", "fa-file-o");
  _actionNew->setMenu(_menuNew);
  initQAction(_actionOpen, "Open an existing strategy...", "fa-upload");
  _actionOpen->setMenu(_menuOpen);
  initQAction(_actionSave, "Export strategies...", "fa-download", false);
  _actionSave->setMenu(_menuSave);
  addSeparator();
  initQAction(_actionUndo, "Undo", "fa-undo", false);
  initQAction(_actionRedo, "Redo", "fa-redo", false);
  addSeparator();
  initQAction(_toolButtonCopy, "Copy", "fa-copy", false);
  initQAction(_toolButtonCut, "Cut", "fa-cut", false);
  initQAction(_toolButtonPaste, "Paste", "fa-paste", false);
  addSeparator();
  initQAction(_toolButtonZoomIn, "Zoom-in", "fa-search-plus");
  initQAction(_toolButtonZoomOut, "Zoom-out", "fa-search-minus");
  addSeparator();
  initQAction(_toolButtonClear, "Clear the code of the current strategy","fa-eraser", false);
  initQAction(_toolButtonDelete, "Delete current strategy", "fa-times-circle-o",
              false);
  initQAction(_toolButtonHelp, "Strategy language documentation", "md-help-circle-outline",
              false);
}

void StrategyManagementWidget::addStrategyFromSelectedElementsInTraceRoot() {
  Observable::holdObservers();

  Graph *traceRoot = PorgyTlpGraphStructure::getMainTrace(_sourceCodeModel.graph());
  BooleanProperty *currentSelection = traceRoot->getProperty<BooleanProperty>("viewSelection");

  // check if we have a graph, if not extend the selection
  for(auto e:currentSelection->getEdgesEqualTo(true)) {
    const pair<node, node> &ends = traceRoot->ends(e);
    if (!currentSelection->getNodeValue(ends.first)) {
      currentSelection->setNodeValue(ends.first, true);
      tlp::debug() << "node " << to_string(ends.first) << " added to the selection" << endl;
    }
    if (!currentSelection->getNodeValue(ends.second)) {
      currentSelection->setNodeValue(ends.second, true);
      tlp::debug() << "node " << to_string(ends.second) << " added to the selection" << endl;
    }
  }

  node firstNode, lastNode;
  Trace trace(traceRoot);
  string strategy(trace.computeStrategyText(currentSelection, firstNode, lastNode));

  if (strategy.empty()) {
    QMessageBox::critical(this, QString("Error when computing strategy"),
                          QString("An error occured when computing the stratgy"));
  } else {
    StringProperty *label = traceRoot->getProperty<StringProperty>("viewLabel");
    addNewStrategy(QString("Code from ") + tlpStringToQString(label->getNodeValue(firstNode)) +
                       QString(" to ") + tlpStringToQString(label->getNodeValue(lastNode)),
                   tlpStringToQString(strategy));
  }

  Observable::unholdObservers();
}

void StrategyManagementWidget::updateCurrentStrategyCode(const QModelIndex &index) {
  _sourceCodeModel.setStrategyCode(index.row(), ui->plainTextEdit->document()->toPlainText());
}

/****************************************************************************************
 ****************************************************************************************
 *
 * Connections setup.
 *
 ****************************************************************************************/
void StrategyManagementWidget::_initConnections() {
  connect(ui->plainTextEdit, SIGNAL(undoAvailable(bool)), ui->toolbar->_actionUndo,
          SLOT(setEnabled(bool)));
  connect(ui->plainTextEdit, SIGNAL(redoAvailable(bool)), ui->toolbar->_actionRedo,
          SLOT(setEnabled(bool)));
  connect(ui->plainTextEdit, SIGNAL(copyAvailable(bool)), ui->toolbar->_toolButtonCopy,
          SLOT(setEnabled(bool)));
  connect(ui->plainTextEdit, SIGNAL(copyAvailable(bool)), ui->toolbar->_toolButtonCut,
          SLOT(setEnabled(bool)));

  connect(ui->toolbar->_actionUndo, SIGNAL(triggered()), ui->plainTextEdit, SLOT(undo()));
  connect(ui->toolbar->_actionRedo, SIGNAL(triggered()), ui->plainTextEdit, SLOT(redo()));

  connect(ui->toolbar->_toolButtonCopy, SIGNAL(triggered()), ui->plainTextEdit, SLOT(copy()));
  connect(ui->toolbar->_toolButtonCut, SIGNAL(triggered()), ui->plainTextEdit, SLOT(cut()));
  connect(ui->toolbar->_toolButtonPaste, SIGNAL(triggered()), ui->plainTextEdit, SLOT(paste()));

  connect(ui->toolbar->_toolButtonZoomIn, SIGNAL(triggered()), ui->listView, SLOT(zoomIn()));
  connect(ui->toolbar->_toolButtonZoomOut, SIGNAL(triggered()), ui->listView, SLOT(zoomOut()));
  connect(ui->toolbar->_toolButtonZoomIn, SIGNAL(triggered()), ui->plainTextEdit, SLOT(zoomIn()));
  connect(ui->toolbar->_toolButtonZoomOut, SIGNAL(triggered()), ui->plainTextEdit, SLOT(zoomOut()));

  connect(ui->plainTextEdit, SIGNAL(wheelZoomIn(int)), ui->listView, SLOT(zoomIn(int)));
  connect(ui->plainTextEdit, SIGNAL(wheelZoomOut(int)), ui->listView, SLOT(zoomOut(int)));
  connect(ui->listView, SIGNAL(wheelZoomIn(int)), ui->plainTextEdit, SLOT(zoomIn(int)));
  connect(ui->listView, SIGNAL(wheelZoomOut(int)), ui->plainTextEdit, SLOT(zoomOut(int)));

  connect(ui->toolbar->_actionNew, SIGNAL(triggered()), this, SLOT(addNewStrategy()));
  connect(ui->toolbar->_actionOpen, SIGNAL(triggered()), this, SLOT(open()));
  connect(ui->toolbar->_actionSave, SIGNAL(triggered()), this, SLOT(save()));

  ui->toolbar->_menuNew->addAction(TulipFontIconEngine::icon("fa-file-o"),
                                   "New strategy", this, SLOT(addNewStrategy()),
                                   QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_N));
  _actionDuplicate = ui->toolbar->_menuNew->addAction("Duplicate selected strategy", this,
                                                      SLOT(duplicateSourceCode()),
                                                      QKeySequence(Qt::CTRL + Qt::Key_D));
  _actionDuplicate->setEnabled(false);
  ui->toolbar->_toolButtonHelp->setEnabled(true);
  ui->toolbar->_menuNew->addAction("New strategy from derivation tree", this,
                                   SLOT(addStrategyFromSelectedElementsInTraceRoot()),
                                   QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_T));
  ui->toolbar->_menuOpen->addAction(
      TulipFontIconEngine::icon("fa-folder-open-o"), "Open a strategy",
      this, SLOT(open()), QKeySequence(Qt::CTRL + Qt::Key_O));
  ui->toolbar->_menuOpen->addAction("Open all strategies", this, SLOT(openAllStrategies()),
                                    QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_O));
  ui->toolbar->_menuSave->addAction(TulipFontIconEngine::icon("fa-save"),
                                    "Export selected strategy", this, SLOT(save()),
                                    QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_S));
  ui->toolbar->_menuSave->addAction("Export all strategies", this, SLOT(saveAllStrategies()),
                                    QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_A));

  if (ui->listView->selectionModel()) {
    connect(ui->toolbar->_toolButtonClear, SIGNAL(triggered()), this, SLOT(clear()));
    connect(ui->toolbar->_toolButtonDelete, SIGNAL(triggered()), this, SLOT(deleteSourceCode()));
    connect(ui->toolbar->_toolButtonHelp, SIGNAL(triggered()), this, SLOT(showStratDocumentation()));
    connect(ui->listView, SIGNAL(requestDropLoadSourceFiles(const QStringList &)), this,
            SLOT(openDrop(const QStringList &)));
    connect(ui->listView->selectionModel(),
            SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(selectedSourceCodeChanged(const QModelIndex &, const QModelIndex &)));
    connect(ui->listView, SIGNAL(pressed(const QModelIndex &)), this,
            SLOT(updateCurrentStrategyCode(const QModelIndex &)));
    connect(ui->listView, SIGNAL(deleteStrat(QModelIndexList)), this,
            SLOT(deleteStrat(QModelIndexList)));
    connect(ui->listView, SIGNAL(exportStrat()), this, SLOT(save()));
  }
}

void StrategyManagementWidget::showStratDocumentation() {
  QDesktopServices::openUrl(QUrl("https://hal.inria.fr/hal-01566525"));
}

void StrategyManagementWidget::deleteStrat(QModelIndexList m) {
  foreach (const QModelIndex &index, m) {
    _sourceCodeModel.removeStrategy(index.row());

    if (_sourceCodeModel.empty()) {
      toolbarEnabled(false);
    }
  }
}

StrategyManagementWidget::StrategyManagementWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::StrategyManagementWidget) {
  ui->setupUi(this);
  ui->listView->setModel(&_sourceCodeModel);
  ui->plainTextEdit->setModel(&_sourceCodeModel);
  ui->listView->setMinimumWidth(ui->listView->width() / 10.0f);

  ui->plainTextEdit->setSyntaxFile(":/strategymanagementwidget/syntax/porgystrategylang.xml");
  ui->plainTextEdit->setMinimumWidth(ui->plainTextEdit->width() / 10.0f);
  ui->plainTextEdit->setEnabled(false);

  QList<int> widgetsWidth = ui->splitter->sizes();
  widgetsWidth[0] = ui->listView->width() - (ui->listView->width() / 2.0f);
  widgetsWidth[1] = ui->listView->width() + (ui->listView->width() / 2.0f);
  ui->splitter->setSizes(widgetsWidth);
  _initConnections();
}

StrategyManagementWidget::~StrategyManagementWidget() {
  delete ui;
}

void StrategyManagementWidget::setGraph(Graph *g) {
  _sourceCodeModel.setGraph(g);
}

void StrategyManagementWidget::setData(const DataSet &data) {
  _sourceCodeModel.setData(data);
  if (!_sourceCodeModel.empty()) {
    ui->listView->selectionModel()->select(_sourceCodeModel.index(0),
                                           QItemSelectionModel::ClearAndSelect);
    ui->plainTextEdit->setPlainText(_sourceCodeModel.getStrategy(0));
  } else {
    ui->listView->selectionModel()->clearSelection();
  }
}
