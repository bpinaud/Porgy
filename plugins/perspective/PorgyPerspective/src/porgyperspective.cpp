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
#include <tulip/DrawingTools.h>
#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainView.h>
#include <tulip/GlMainWidget.h>
#include <tulip/Graph.h>
#include <tulip/GraphHierarchiesModel.h>
#include <tulip/GraphNeedsSavingObserver.h>
#include <tulip/GraphTools.h>
#include <tulip/ImportModule.h>
#include <tulip/Interactor.h>
#include <tulip/PythonIDE.h>
#include <tulip/StringProperty.h>
#include <tulip/TlpQtTools.h>
#include <tulip/TlpTools.h>
#include <tulip/TulipFontIconEngine.h>
#include <tulip/TulipProject.h>
#include <tulip/TulipSettings.h>
#include <tulip/View.h>
#include <tulip/StableIterator.h>
#include <tulip/Workspace.h>

#include <QClipboard>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QParallelAnimationGroup>
#include <QUrl>
#include <QRegularExpression>

#include "ui_porgyperspective.h"

#include "AboutDialog.h"
#include "GlMainWidgetZoomAndPanAnimation.h"
#include "NewTrace.h"
#include "PorgyPerspectiveLogger.h"
#include "PorgyPropertyManager.h"
#include "PreferencesDialog.h"
#include "porgyperspective.h"
#include "special_paste.h"
#include "statusbarpluginprogress.h"
#include "validaterule.h"

#include <porgy/PortNodeQt.h>
#include <porgy/pluginparametersconfigurationdialog.h>
#include <porgy/pluginselectionwizard.h>

#include <portgraph/Bridge.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>
#include <portgraph/Trace.h>

using namespace tlp;
using namespace std;

static const QString porgyModelParameter("model");
static const QString GRAPHS_PATH("/graphs/");
static const QString strategies_path("/strategies/");

void porgyPerspectiveLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  static_cast<PorgyPerspective *>(Perspective::instance())->log(type, context, msg);
}

void PorgyPerspective::log(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  _ui->logWidget->log(type, context, msg);
}


PorgyPerspective::PorgyPerspective(const tlp::PluginContext *context)
    : Perspective(context), pythonIDE(nullptr), _pythonIDEDialog(nullptr), gModel(nullptr),
      _ui(nullptr), rootGraph(nullptr), rulesGraph(nullptr), modelsGraph(nullptr),
      traceGraph(nullptr), portNodeConfigurationWidget(nullptr), savingObserver(nullptr),
      _debug(false) {
  addDependency(PorgyConstants::GRAPH_VIEW_NAME, "1.0");
  addDependency(PorgyConstants::RULE_VIEW_NAME, "1.0");
  addDependency(PorgyConstants::TRACE_VIEW_NAME, "1.0");
  addDependency("TLPB Import", "1.2");
  addDependency("TLP Import", "1.0");
  addDependency(PorgyConstants::SMALL_MULTIPLE_VIEW_NAME, "1.0");
  addDependency(PorgyConstants::ANIMATION_VIEW_NAME, "1.0");
  addDependency(PorgyConstants::APPLY_STRATEGY_ALGORITHM, "1.0");
  addDependency(PorgyConstants::SIMPLECHECKAPPLY, "1.0");
  addDependency("CSV Export", "1.0");
}

void PorgyPerspective::redrawPanels(bool) {
  _ui->viewManager->_workspace->redrawPanels();
}

StatusBarPluginProgress *PorgyPerspective::_PluginProgress = new StatusBarPluginProgress();

bool PorgyPerspective::terminated() {
  if (savingObserver != nullptr && savingObserver->needsSaving()) {
    QMessageBox::StandardButton answer = QMessageBox::question(
        _mainWindow, "Save",
        "The project has been modified. Do you want to save your changes?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel | QMessageBox::Escape);
    if ((answer == QMessageBox::Yes && !fileSave()) || (answer == QMessageBox::Cancel)) {
      return false;
    }
  }
  pythonIDE->clearPythonCodeEditors();
  delete _pythonIDEDialog;
  delete gModel;
  return true;
}

class PythonIDEDialog : public QDialog {

  bool _firstShow;
  QSize _size;
  QPoint _pos;

public:
  PythonIDEDialog(QWidget *parent, Qt::WindowFlags flags)
      : QDialog(parent, flags), _firstShow(true) {}

protected:
  void showEvent(QShowEvent *e) override {
    QDialog::showEvent(e);

    if (!_firstShow) {
      move(_pos);
      resize(_size);
    }

    _firstShow = false;
  }

  void closeEvent(QCloseEvent *e) override {
    _size = size();
    _pos = pos();
    QDialog::closeEvent(e);
  }
};

void PorgyPerspective::usage(string& usage_str) const {
    usage_str += "List of OPTIONS:\n";
    usage_str += "  --Debug=on\tActivate Debug mode";
}

#define SET_TIPS(a, tt)                                                                            \
  a->setToolTip(tt);                                                                               \
  a->setStatusTip(a->toolTip())

void PorgyPerspective::start(tlp::PluginProgress *progress) {
    // configure style sheet
    QString s_sheet(R"(
  #AboutDialog #AboutTulipPageWidget, #AboutTulipPageWidget QTextEdit, #AlgorithmRunner, #AlgorithmRunner #contents, tlp--AlgorithRunnerItem, #ElementInformationWidget, #interactorConfigWidget, #interactorConfigWidgetDoc, #interactorConfigWidgetOptions, #mainWidget, #stringsListSelectionWidget, #TableViewWidget, QAbstractItemView, QCheckBox, QDialog, QHeaderView::section, QMessageBox, QRadioButton, QStackedWidget, QTabBar::tab, QTableView QTableCornerButton::section, QTextBrowser, QWizard, QWizard > * { background-color: %BG_COLOR%; }

  #AxisSlidersOptions { background-color: %BG_COLOR%; }

  #HeaderFrameData QComboBox QAbstractItemView {
  background-color: %BG_COLOR%;
  color: %FG_COLOR%;
  border: 1px solid #C9C9C9;
  }

  #parameters {
  color: black;
  font: 12px;
  }

  #parameters QHeaderView::section {
  background-color: %BG_COLOR%;
  padding-top: -1px;
  padding-left: 4px;
  padding-right: 4px;
  font: bold 12px;
  }

  #PreferencesDialog QHeaderView::section {
  height: 30px;
  font: bold 12px;
  color: %FG_COLOR%;
  border: 0px;
  }

  #PropertiesEditor QCheckBox::indicator:checked, QTableView::indicator:checked {
  image: url(:/tulip/gui/icons/eye-%FG_COLOR%.png);
  }

  #PropertiesEditor QCheckBox::indicator:indeterminate {
  image: url(:/tulip/gui/icons/eye_partially_disabled-%FG_COLOR%.png);
  }

  #PropertiesEditor QCheckBox::indicator:unchecked, QTableView::indicator:unchecked {
  image: url(:/tulip/gui/icons/eye_disabled-%FG_COLOR%.png);
  }

  #scrollArea, #scrollAreaWidgetContents {
  background-color: %BG_COLOR%;
  border: 0px;
  }

  #SearchWidget QTableView {
  border: 1px solid #C9C9C9;
  color: %FG_COLOR%;
  font: 12px;
  }

  #AboutDialog #AboutTulipPageWidget QTextEdit:enabled, #ElementInformationWidget, #ElementInformationWidget QPushButton, #PanelSelectionWizard QListView, QComboBox QAbstractItemView:enabled, QComboBox:item:enabled, QCheckBox:enabled, QGroupBox:enabled, QHeaderView::section:enabled, QLabel:enabled, QListWidget:enabled, QRadioButton:enabled, QTabBar::tab:enabled, QTableWidget:enabled, QTableView:enabled, QTextBrowser:enabled, QToolButton:enabled, QTreeView:enabled {
  color: %FG_COLOR%;
  }

  QLineEdit[clearableLineEdit] {
  border: 1px solid #808080;
  background-color: white;
  color: black;
  }

  QHeaderView::down-arrow {
  image: url(:/tulip/gui/ui/down_arrow-%FG_COLOR%.png);
  }

  QHeaderView::up-arrow {
  image: url(:/tulip/gui/ui/up_arrow-%FG_COLOR%.png);
  }

  QListView, QTableView {
  alternate-background-color: #A0A0A0;
  }

  QPlainTextEdit {
  background-color: %BG_COLOR%;
  color: %FG_COLOR%;
  selection-background-color: #C0C0C0;
  }

  QPushButton, QComboBox {
  color: black;
  }

  QPushButton, QComboBox {
  border-image: url(:/tulip/gui/ui/btn_26.png) 4;
  border-width: 4;
  padding: 0px 6px;
  font-size: 12px;
  }

  QPushButton::flat {
  border-width: 0;
  background-color: transparent;
  }

  QPushButton:hover {
  border-image: url(:/tulip/gui/ui/btn_26_hover.png) 4;
  border-width: 4;
  }

  QComboBox:hover, QToolButton:hover {
  border-image: url(:/tulip/gui/ui/btn_26_hover.png) 4;
  }

  QPushButton:disabled, QComboBox::disabled, QToolButton::disabled {
  color:gray;
  }

  QPushButton:pressed, QToolButton:pressed{
  border-image: url(:/tulip/gui/ui/btn_26_pressed.png) 4;
  }

  QPushButton::menu-indicator{
  subcontrol-origin: margin;
  subcontrol-position: center right;
  right: 4px;
  }

  QPushButton {
  outline: none;
  margin: 2
  }

  QComboBox::down-arrow {
  image: url(:/tulip/gui/ui/combobox_arrow.png);
  }

  QComboBox:drop-down {
  subcontrol-origin: padding;
  subcontrol-position: top right;
  border-left-style: none;
  border-top-right-radius: 1px;
  border-bottom-right-radius: 1px;
  }

  #bottomFrame * {
  font: bold 11px;
  }

  #bottomFrame {
  border-top: 1px solid black;
  border-bottom: 1px solid rgba(117,117,117,255);
  border-right: 1px solid rgba(117,117,117,255);
  border-left: 0px;
  background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:, y2:1,
  stop:0 rgb(75,75,75),
  stop:1 rgb(60, 60, 60));
  }

  #bottomFrame QPushButton, #bottomFrame QLabel {
  color: white;
  }

  #bottomFrame QPushButton {
  border: 0px;
  border-image: none;
  }

  #bottomFrame QPushButton:hover {
  border: 0px;
  border-image: none;
  background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:, y2:1,
  stop:0 rgb(85,85,85),
  stop:1 rgb(70, 70, 70));
  }

  #bottomFrame QPushButton:pressed, #bottomFrame .QPushButton:checked {
  border: 0px;
  border-image: none;
  background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:, y2:1,
  stop:0 rgb(105,105,105),
  stop:1 rgb(90, 90, 90));
  }

  #bottomFrame OutputPanelButton{
  border-image: url(:/tulip/graphperspective/ui/panel_button.png) 2 2 2 19;
  border-width: 2px 2px 2px 19px;
  padding-left: -17;
  padding-right: 4;
  }

  #bottomFrame OutputPanelButton:checked{
  border-image: url(:/tulip/graphperspective/ui/panel_button_checked.png) 2 2 2 19
  }

  #bottomFrame OutputPanelButton::menu-indicator{
  width:0; height:0
  }

  #bottomFrame OutputPanelButton:checked:hover{
  border-image: url(:/tulip/graphperspective/ui/panel_button_checked_hover.png) 2 2 2 19
  }

  #bottomFrame OutputPanelButton:pressed:hover{
  border-image: url(:/tulip/graphperspective/ui/panel_button_pressed.png) 2 2 2 19
  }

  #bottomFrame OutputPanelButton:hover{
  border-image: url(:/tulip/graphperspective/ui/panel_button_hover.png) 2 2 2 19
  }

  #bottomFrame QToolButton {
      border-image:none;
      border-top: 1px solid rgba(0,0,0,0);
      border-bottom: 1px solid rgba(0,0,0,0);
      border-left: 0px solid rgba(0,0,0,0);
      border-right: 0px solid rgba(0,0,0,0);
      color: white;
      font: bold 10px;
      height:20px;
      background-color: rgba(0,0,0,0);
  }

  #bottomFrame QToolButton:hover {
      border-image:none;
      background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,
      stop:0 rgba(93, 93, 93, 255),
      stop:1 rgba(150, 150, 150, 255));
      border-top: 1px solid qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
      stop: 0.0 rgba(0,0,0,0),
      stop: 0.4 rgba(170,170,170,255),
      stop: 0.5 rgba(170,170,170,255),
      stop: 1.0 rgba(0,0,0,0));
      border-bottom: 1px solid qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
      stop: 0.0 rgba(0,0,0,0),
      stop: 0.4 rgba(170,170,170,255),
      stop: 0.5 rgba(170,170,170,255),
      stop: 1.0 rgba(0,0,0,0));
  }

  QFrame[ section=\"true\" ] {
  border-top: 1px solid #D9D9D9;
  padding-top: 20px;
  }

  QLabel[groupTitle] {
  background-color: #A0A0A0;
  font-weight: bold;
  }

  QSplitter::handle {
  background-color: #C0C0C0;
  border-width:0px;
  }

  QScrollBar {
  background-color: #D0D0D0;
  }

  QScrollBar::sub-page, QScrollBar::add-page {
  background-color: #A0A0A0;
  }

  QTableWidget, QTableView {
  gridline-color: #808080;
  }

  QTabBar::tab {
  background: #A0A0A0;
  border: 1px solid #808080;
  border-top-left-radius: 2px;
  border-top-right-radius: 2px;
  font-weight: 100;
  min-width: 8ex;
  padding: 5px;
  }

  QTabBar::tab:selected {
  background: %BG_COLOR%;
  margin-bottom: -1px;
  font-weight: normal;
  }

  QTabBar::tab:!selected:hover {
  background: #D0D0D0;
  font-weight: 300;
  }

  QTabBar::tab:disabled {
  color: #808080;
  }

  QTabWidget::pane {
  background: #808080;
  padding: 1px;
  top: -1px;
  }
  )");

    if (TulipSettings::isDisplayInDarkMode())
      s_sheet.replace("%BG_COLOR%", "#323232").replace("%FG_COLOR%", "white");
    else
      s_sheet.replace("%BG_COLOR%", "white").replace("%FG_COLOR%", "black");
    _mainWindow->setStyleSheet(s_sheet);
    _ui = new Ui::PorgyPerspective();
  _ui->setupUi(_mainWindow);

  _ui->statusbar->addPermanentWidget(_PluginProgress);
  _PluginProgress->setVisible(true);
  _PluginProgress->setEnabled(false);
  _PluginProgress->reset();

  initMenus();
  buildGUI();

  _mainWindow->installEventFilter(this);

  redirectDebugOutputToQDebug();
  redirectWarningOutputToQWarning();
  redirectErrorOutputToQCritical();

  qInstallMessageHandler(porgyPerspectiveLogger);

  // Settings
  settings.setTraceViewZoomAnimationTime(1000);

  _ui->viewManager->setupUi();
  _ui->singleModeButton->setEnabled(false);
  _ui->singleModeButton->hide();
  _ui->viewManager->_workspace->setSingleModeSwitch(_ui->singleModeButton);
  _ui->viewManager->_workspace->setFocusedPanelHighlighting(true);
  _ui->splitModeButton->setEnabled(false);
  _ui->splitModeButton->hide();
  _ui->viewManager->_workspace->setSplitModeSwitch(_ui->splitModeButton);
  _ui->splitHorizontalModeButton->setEnabled(false);
  _ui->splitHorizontalModeButton->hide();
  _ui->viewManager->_workspace->setSplitHorizontalModeSwitch(_ui->splitHorizontalModeButton);
  _ui->split3ModeButton->setEnabled(false);
  _ui->split3ModeButton->hide();
  _ui->viewManager->_workspace->setSplit3ModeSwitch(_ui->split3ModeButton);
  _ui->split32ModeButton->setEnabled(false);
  _ui->split32ModeButton->hide();
  _ui->viewManager->_workspace->setSplit32ModeSwitch(_ui->split32ModeButton);
  _ui->split33ModeButton->setEnabled(false);
  _ui->split33ModeButton->hide();
  _ui->viewManager->_workspace->setSplit33ModeSwitch(_ui->split33ModeButton);
  _ui->gridModeButton->setEnabled(false);
  _ui->gridModeButton->hide();
  _ui->viewManager->_workspace->setGridModeSwitch(_ui->gridModeButton);
  _ui->sixModeButton->setEnabled(false);
  _ui->sixModeButton->hide();
  _ui->viewManager->_workspace->setSixModeSwitch(_ui->sixModeButton);
  _ui->viewManager->_workspace->setPageCountLabel(_ui->pageCountLabel);
  connect(_ui->action_Close_All, SIGNAL(triggered()), _ui->viewManager->_workspace, SLOT(closeAll()));
  //navbar
  connect(_ui->previousPageButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(previousPage()));
  connect(_ui->nextPageButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(nextPage()));
  connect(_ui->singleModeButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(switchToSingleMode()));
  connect(_ui->splitHorizontalModeButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(switchToSplitHorizontalMode()));
  connect(_ui->split33ModeButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(switchToSplit33Mode()));
  connect(_ui->split32ModeButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(switchToSplit32Mode()));
  connect(_ui->sixModeButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(switchToSixMode()));
  connect(_ui->split3ModeButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(switchToSplit3Mode()));
  connect(_ui->splitModeButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(switchToSplitMode()));
  connect(_ui->gridModeButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(switchToGridMode()));
  connect(_ui->sixModeButton, SIGNAL(clicked()), _ui->viewManager->_workspace, SLOT(switchToSixMode()));
  connect(_ui->actionExposePanels, SIGNAL(toggled(bool)), _ui->viewManager->_workspace, SLOT(expose(bool)));

  _ui->viewManager->_workspace->setExposeModeSwitch(_ui->exposeButton);

  SET_TIPS(_ui->exposeButton, _ui->actionExposePanels->toolTip());
  SET_TIPS(_ui->singleModeButton, "Switch to 1 panel mode");
  SET_TIPS(_ui->splitModeButton, "Switch to 2 panels mode");
  SET_TIPS(_ui->splitHorizontalModeButton, "Switch to 2 panels mode");
  SET_TIPS(_ui->split3ModeButton, "Switch to 2-top 1-bottom panels mode");
  SET_TIPS(_ui->split32ModeButton, "Switch to 1-left 2-right panels mode");
  SET_TIPS(_ui->split33ModeButton, "Switch to 2-left 1-right panels mode");
  SET_TIPS(_ui->gridModeButton, "Switch to 4 panels mode");
  SET_TIPS(_ui->sixModeButton, "Switch to 6 panels mode");

  SET_TIPS_WITH_CTRL_SHORTCUT(_ui->previousPageButton, "Show previous panel", "Shift+Left");
  SET_TIPS_WITH_CTRL_SHORTCUT(_ui->nextPageButton, "Show next panel", "Shift+Right");

  if (!_externalFile.isEmpty()) {
    if (QFileInfo(_externalFile).exists()) {
      fileOpen(_externalFile, progress);
    } else {
      tlp::warning() << "The file " << _externalFile.toStdString() << " does not exist" << endl;
    }
  }

  if (!_project->projectFile().isEmpty()) {
    open(progress);
  }
  pythonIDE = new PythonIDE();
  QVBoxLayout *dialogLayout = new QVBoxLayout();
  dialogLayout->addWidget(pythonIDE);
  dialogLayout->setContentsMargins(0, 0, 0, 0);
  _pythonIDEDialog = new PythonIDEDialog(nullptr, Qt::Window);
  _pythonIDEDialog->setStyleSheet(_mainWindow->styleSheet());
  _pythonIDEDialog->setWindowIcon(_mainWindow->windowIcon());
  _pythonIDEDialog->setLayout(dialogLayout);
  _pythonIDEDialog->resize(800, 600);
  _pythonIDEDialog->setWindowTitle("Tulip Python IDE");

  //check command line parameters
  if(_parameters.contains("Debug")) {
      //enable Debug mode
      debugPressed(true);
      _ui->actionDebug_mode->setChecked(true);
  }

}

void PorgyPerspective::initMenus() {
  _ui->actionNew->setIcon(TulipFontIconEngine::pixmap(string("fas-file"),32));
  _ui->actionNew_Empty_Graph_Model->setIcon(
      TulipFontIconEngine::pixmap(string("fas-file"),32));
  _ui->actionOpen->setIcon(
      TulipFontIconEngine::pixmap(string("fas-folder-open"),32));
  _ui->actionSaveMenu->setIcon(
      TulipFontIconEngine::pixmap(string("mdi-content-save"),32));
  _ui->actionSave->setIcon(TulipFontIconEngine::pixmap(string("mdi-content-save"),32));
  _ui->actionSave_as->setIcon(TulipFontIconEngine::pixmap(string("mdi-content-save"),32));
  QMenu *menusave = new QMenu();
  menusave->addAction(_ui->actionSave);
  menusave->addAction(_ui->actionSave_as);
  _ui->actionSaveMenu->setMenu(menusave);

  _ui->actionClose->setIcon(TulipFontIconEngine::pixmap(string("mdi-close-circle"), 32));
  _ui->actionUndo->setIcon(TulipFontIconEngine::pixmap(string("mdi-undo"),32));
  _ui->actionRedo->setIcon(TulipFontIconEngine::pixmap(string("mdi-redo"),32));
  _ui->actionCopy->setIcon(TulipFontIconEngine::pixmap(string("mdi-content-copy"),32));
  _ui->actionCut->setIcon(TulipFontIconEngine::pixmap(string("mdi-content-cut"),32));
  _ui->actionPaste->setIcon(TulipFontIconEngine::pixmap(string("mdi-content-paste"),32));
  _ui->actionDocumentation->setIcon(
      TulipFontIconEngine::pixmap(string("mdi-help"),32));
  _ui->action_Close_All->setIcon(
      TulipFontIconEngine::pixmap(string("mdi-window-close"),32));
  _ui->actionPreferences->setIcon(
      TulipFontIconEngine::pixmap(string("fas-wrench"),32));
  _ui->actionQuit->setIcon(
      TulipFontIconEngine::pixmap(string("mdi-window-close"),32));

  connect(_ui->actionNew, SIGNAL(triggered()), this, SLOT(fileNew()));
  connect(_ui->actionNew_Empty_Graph_Model, SIGNAL(triggered()), this,
          SLOT(newBlankDataStructure()));
  connect(_ui->actionOpen, SIGNAL(triggered()), this, SLOT(fileOpen()));
  connect(_ui->actionClose, SIGNAL(triggered()), this, SLOT(fileClose()));
  connect(_ui->actionSaveMenu, SIGNAL(triggered()), this, SLOT(fileSave()));
  connect(_ui->actionSave_as, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
  connect(_ui->actionSave, SIGNAL(triggered()), this, SLOT(fileSave()));

  connect(_ui->actionUndo, SIGNAL(triggered()), SLOT(undo()));
  connect(_ui->actionRedo, SIGNAL(triggered()), SLOT(redo()));
  connect(_ui->actionCut, SIGNAL(triggered()), SLOT(editCut()));
  connect(_ui->actionCopy, SIGNAL(triggered()), SLOT(editCopy()));
  connect(_ui->actionPaste, SIGNAL(triggered()), SLOT(editPaste()));
  connect(_ui->actionSelect_all, SIGNAL(triggered()), SLOT(editSelectAll()));
  connect(_ui->actionDeselect_all, SIGNAL(triggered()), SLOT(editDeselectAll()));
  connect(_ui->actionInvert_selection, SIGNAL(triggered()), SLOT(editReverseSelection()));
  connect(_ui->actionDel_selection, SIGNAL(triggered()), SLOT(editDelSelection()));
  connect(_ui->actionCreate_new_rule, SIGNAL(triggered()), SLOT(createNewRule()));
  connect(_ui->actionPropertyManager, SIGNAL(triggered()), SLOT(launchPropertyManager()));
  connect(_ui->actionPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
  connect(_ui->actionDebug_mode, SIGNAL(triggered(bool)), this, SLOT(debugPressed(bool)));
  connect(_ui->actionValidate_Rules_and_Graphs, SIGNAL(triggered()), this, SLOT(validate()));
  connect(_ui->actionPython_IDE, SIGNAL(triggered()), this, SLOT(showPythonIDE()));
  connect(_ui->actionTulip_Spreadsheet_View, SIGNAL(triggered()), this,
          SLOT(showSpreadSheetView()));

  connect(_ui->actionDocumentation, SIGNAL(triggered()), SLOT(showDocumentation()));
  connect(_ui->action_About, SIGNAL(triggered()), SLOT(showAbout()));
  connect(_ui->actionFull_Screen, SIGNAL(triggered(bool)), this, SLOT(showFullScreen(bool)));
  connect(_ui->actionPython_Documentation, SIGNAL(triggered()), this, SLOT(showPythonDocumentation()));

  QMenu *algomenu = _ui->menuAlgorithms->addMenu(tlp::tlpStringToQString(tlp::ALGORITHM_CATEGORY));
  QMenu *algoDebug = algomenu->addMenu("Debug");
  QMenu *layoutmenu =
      _ui->menuAlgorithms->addMenu(tlp::tlpStringToQString(tlp::LAYOUT_ALGORITHM_CATEGORY));
  QMenu *boolmenu =
      _ui->menuAlgorithms->addMenu(tlp::tlpStringToQString(tlp::BOOLEAN_ALGORITHM_CATEGORY));
  QMenu *doublemenu =
      _ui->menuAlgorithms->addMenu(tlp::tlpStringToQString(tlp::DOUBLE_ALGORITHM_CATEGORY));
  QMenu *importMenu = _ui->menuFile->addMenu(tlp::tlpStringToQString(tlp::IMPORT_CATEGORY));
  _ui->menuFile->insertMenu(_ui->actionSaveMenu, importMenu);
  //Use Plugin as the type of plugins to have both Algorithm and Import plugin
  auto list_plugins(PluginLister::availablePlugins<Plugin>());
  for (auto pName : list_plugins) {
    tlp::Plugin *plugin = PluginLister::getPluginObject(pName, nullptr);
    string className = plugin->category();
    QString name(tlpStringToQString(pName));
    if (plugin->group() == PorgyConstants::CATEGORY_NAME) {
      if (className == tlp::LAYOUT_ALGORITHM_CATEGORY) {
        QAction *item = layoutmenu->addAction(name, this, SLOT(changeMetric()));
        item->setData(name);
      } else if (className == tlp::ALGORITHM_CATEGORY) {
        QAction *item = algomenu->addAction(name, this, SLOT(applyAlgorithm()));
        item->setData(name);
      } else if (className == tlp::BOOLEAN_ALGORITHM_CATEGORY) {
        QAction *item = boolmenu->addAction(name, this, SLOT(changeMetric()));
        item->setData(name);
      } else if (className == tlp::IMPORT_CATEGORY) { // Specify Import exception
        if (name != "Rule Import") {
          QAction *item = importMenu->addAction(name, this, SLOT(ImportGraph()));
          item->setData(name);
        }
      } else if (className == tlp::DOUBLE_ALGORITHM_CATEGORY) {
        QAction *item = doublemenu->addAction(name, this, SLOT(changeMetric()));
        item->setData(name);
      }
    } else if (plugin->group() == PorgyConstants::CATEGORY_NAME_INTERNAL) {
      if (className == tlp::ALGORITHM_CATEGORY) {
        QAction *item = algoDebug->addAction(name, this, SLOT(applyAlgorithm()));
        item->setData(name);
      }
    }
    delete plugin;
  }

  // Docks menu
  foreach (QDockWidget *child, _mainWindow->findChildren<QDockWidget *>()) {
    _ui->menuDocks->addAction(child->toggleViewAction());
  }
}

void PorgyPerspective::validate() {
  RuleValidation val(rulesGraph, _mainWindow);
  val.exec();
}

void PorgyPerspective::openPreferences() {
  PreferencesDialog dlg(_mainWindow);
  dlg.readSettings();

  if (dlg.exec() == QDialog::Accepted) {
    dlg.writeSettings();

    foreach (tlp::View *v, _ui->viewManager->_workspace->panels()) {
      GlMainView *glMainView = static_cast<tlp::GlMainView *>(v);
      if (glMainView->getGlMainWidget() != nullptr) {
        glMainView->getGlMainWidget()
            ->getScene()
            ->getGlGraphComposite()
            ->getRenderingParametersPointer()
            ->setSelectionColor(TulipSettings::instance().defaultSelectionColor());
        glMainView->redraw();
      }
    }
  }
}

void PorgyPerspective::debugPressed(bool checked) {
  _ui->logDockWidget->setVisible(checked);
  _debug = checked;
}

void PorgyPerspective::buildGUI() {
  // Building element properties management widget
  //connect(_ui->viewManager, SIGNAL(showGraphRequest(tlp::Graph *)), SLOT(addPanel(tlp::Graph *)));
  connect(_ui->viewManager, SIGNAL(currentViewChanged(tlp::View *)),
          SLOT(viewActivated(tlp::View *)));
  connect(_ui->ruleWidget, SIGNAL(showRule(tlp::Graph *)), this, SLOT(showRule(tlp::Graph *)));
  connect(_ui->ruleWidget, SIGNAL(renameRule(tlp::Graph *)), this, SLOT(renameRule(tlp::Graph *)));
  connect(_ui->ruleWidget, SIGNAL(addNewRule()), this, SLOT(createNewRule()));
  connect(_ui->ruleWidget, SIGNAL(deleteRule(tlp::Graph *)), this, SLOT(deleteRule(tlp::Graph *)));
  connect(_ui->ruleWidget, SIGNAL(copyRule(tlp::Graph *)), this, SLOT(copyRule(tlp::Graph *)));
  connect(_ui->ruleWidget, SIGNAL(importRule()), this, SLOT(importRule()));
  connect(_ui->ruleWidget, SIGNAL(exportRule(tlp::Graph *)), this, SLOT(exportRule(tlp::Graph *)));
  connect(_ui->ruleWidget, SIGNAL(highlightrule(tlp::Graph *)), this,
          SLOT(highlight_rule_derivation_tree(tlp::Graph *)));

  connect(_ui->graphWidget, SIGNAL(showGraph(tlp::Graph *)), this, SLOT(showGraph(tlp::Graph *)));
  connect(_ui->graphWidget, SIGNAL(copyGraph(tlp::Graph *)), this, SLOT(copyGraph(tlp::Graph *)));

  connect(_ui->traceWidget, SIGNAL(showTrace(tlp::Graph *, bool)), this,
          SLOT(showTrace(tlp::Graph *, bool)));
  connect(_ui->traceWidget, SIGNAL(deleteTrace(tlp::Graph *)), this,
          SLOT(deleteTrace(tlp::Graph *)));

  _mainWindow->tabifyDockWidget(
      _ui->rulesDockWidget,
      _ui->graphsDockWidget); // Group graphs, rules and traces dock widget
  _mainWindow->tabifyDockWidget(_ui->rulesDockWidget, _ui->tracesDockWidget);
  _ui->graphsDockWidget->raise(); // Display graph dock widget

  _ui->pageCountLabel->setVisible(true);

  // Hide these widgets
  _ui->logDockWidget->setVisible(false);
}

PorgyPerspective::~PorgyPerspective() {
  // uninstall Qt message handler only if it is the current active perspective
  if (Perspective::instance() == this) {
    qInstallMessageHandler(nullptr);
  }
  delete _ui;
  delete savingObserver;
}

void PorgyPerspective::fileNew() {
  createPerspective(name().c_str());
}

void PorgyPerspective::newBlankDataStructure() {
  setData(PorgyTlpGraphStructure::setPorgyStructure());
}

void PorgyPerspective::open(PluginProgress *prg) {
  QStringList entries =
      _project->entryList(GRAPHS_PATH, QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  QString file = GRAPHS_PATH + entries.first() + "/graph.tlp";
  if (!_project->exists(file)) {
    file.append("b");
    if (!_project->exists(file)) {
      QMessageBox::critical(_mainWindow, "Cannot open file", "Cannot open graph file: " + file);
      return;
    }
  }
  QString absolutePath = _project->toAbsolutePath(file);
  Graph *g = fileOpen(absolutePath, prg);
  if (g != nullptr) {
    // if the file was in tlp format, force to save the graph to the tlpb format
    //        if(file.endsWith(".tlp"))
    //            savingObserver->forceToSave();
    QMap<QString, tlp::Graph *> rootIds;
    rootIds[entries.first()] = g;
    _ui->viewManager->_workspace->readProject(_project, rootIds, prg);
    _ui->strategiesManagementWidget->openAllStrategies(_project->toAbsolutePath(strategies_path));
    // reconnect signals
    foreach (View *v, _ui->viewManager->_workspace->panels()) {
      if (v->name() == PorgyConstants::TRACE_VIEW_NAME ||
          v->name() == PorgyConstants::GRAPH_VIEW_NAME) {
        connect(v, SIGNAL(applyRuleOnModel(tlp::Graph *, tlp::Graph *, tlp::Graph *)),
                SLOT(applyRuleOnModel(tlp::Graph *, tlp::Graph *, tlp::Graph *)));
        connect(v, SIGNAL(applyStrategyOnModel(tlp::Graph *, QString, tlp::Graph *)),
                SLOT(applyStrategyOnModel(tlp::Graph *, QString, tlp::Graph *)));
      }
      if ((v->name() == PorgyConstants::TRACE_VIEW_NAME) ||
          (v->name() == PorgyConstants::SMALL_MULTIPLE_VIEW_NAME)) {
        connect(v, SIGNAL(openModelView(tlp::Graph *)), this, SLOT(showGraph(tlp::Graph *)));
        connect(v, SIGNAL(newTrace(tlp::node)), this, SLOT(newTrace(tlp::node)));
      }
      if (v->name() == PorgyConstants::RULE_VIEW_NAME) {
        connect(v, SIGNAL(copy_paste_other_side(PorgyConstants::RuleSide)),
                SLOT(send_other_side(PorgyConstants::RuleSide)));
      }
      if ((v->name() == PorgyConstants::RULE_VIEW_NAME) ||
          (v->name() == PorgyConstants::GRAPH_VIEW_NAME)) {
        connect(v, SIGNAL(pastePortNodes(tlp::Coord &, tlp::Graph *, QList<PortNodeQt *>)),
                SLOT(pastePortNodes(tlp::Coord &, tlp::Graph *, QList<PortNodeQt *>)));
      }
    }
  }
}

void PorgyPerspective::highlight_rule_derivation_tree(tlp::Graph *rule) {
  Trace t(traceGraph);
  IntegerProperty *ruleIdprop(t.getTransformationRuleProperty());
  BooleanProperty *traceselection(t.getSelectionProperty());
  for (edge e : t.edges()) {
    if (static_cast<unsigned>(ruleIdprop->getEdgeValue(e)) == rule->getId()) {
      traceselection->setEdgeValue(e, true);
      const pair<node, node> &ends = t.ends(e);
      traceselection->setNodeValue(ends.first, true);
      traceselection->setNodeValue(ends.second, true);
    }
  }
}

void PorgyPerspective::fileOpen() {
  QString filename = QFileDialog::getOpenFileName(
      _mainWindow, "Open a file", QString(),
      "All supported formats(*.tlpx *.tlp *.tlp.gz *.tlpb *.tlpb.gz);;Tulip "
      "project (*.tlpx);;Tulip graph (*.tlp);;Gzipped Tulip graph(*.tlp.gz)");
  if (QFile::exists(filename)) {
    // PluginProgress* prg = progress();
    _PluginProgress->setEnabled(true);
    if (rootGraph == nullptr) {

      if (!filename.endsWith(".tlpx")) { // load tlpx project
        fileOpen(filename, _PluginProgress);
      } else {
        delete _project;
        _project = TulipProject::openProject(filename, _PluginProgress);
        open(_PluginProgress);
      }

    } else if (!filename.endsWith(".tlpx")) { // load old porgy files
      delete _project;
      _project = nullptr;
      fileOpen(filename, _PluginProgress);
    } else
      Perspective::openProjectFile(filename);
    _PluginProgress->setEnabled(false);
    _PluginProgress->reset();
  }
}

Graph *PorgyPerspective::fileOpen(const QString &filename, PluginProgress *prg) {
  Graph *graph = tlp::loadGraph(tlp::QStringToTlpString(filename), prg);
  if (graph) {
    if (setData(graph, DataSet())) {
      return graph;
    }
  }
  QMessageBox::critical(_mainWindow, "Failed to import data",
                        "Cannot import data.<br>" + tlp::tlpStringToQString(prg->getError()));
  return nullptr;
}

bool PorgyPerspective::fileSave() {

  if ((_project != nullptr) && QFile::exists(_project->projectFile())) {
    return save(_project->projectFile(),
                TulipSettings::instance().isUseTlpbFileFormat() ? ".tlpb" : ".tlp");
  } else {
    return fileSaveAs();
  }
}

bool PorgyPerspective::fileSaveAs() {
  QString outFile = QFileDialog::getSaveFileName(_mainWindow, tr("Choose the destination file"),
                                                 QString(), "Tulip project (*.tlpx)");
  if (!outFile.isEmpty()) {
    if (!outFile.endsWith(".tlpx"))
      outFile += ".tlpx";
    _project->setProjectFile(outFile);
    return save(outFile, TulipSettings::instance().isUseTlpbFileFormat() ? ".tlpb" : ".tlp");
  }
  return false;
}

void PorgyPerspective::fileClose() {
  if (_project != nullptr) {
    closeProject();
    _project = nullptr;
  }
}

/**
 * @brief PorgyPerspective::save the code of these function is extracted from
 * the graph perspective to try to have a little compatibility.
 * @param fileName
 */
bool PorgyPerspective::save(const QString &fileName, const QString &graph_extension) {
  _PluginProgress->setEnabled(true);
  _project->removeAllDir(GRAPHS_PATH); // Clear old data
  _project->mkpath(GRAPHS_PATH);       // Create a new graph structure
  QMap<Graph *, QString> rootIds;
  rootIds[rootGraph] = QString::number(0); // Save the root graph
  _ui->viewManager->_workspace->writeProject(_project, rootIds, _PluginProgress);
  QString folder = GRAPHS_PATH + "/" + QString::number(0) + "/";
  _project->mkpath(folder);
  bool ret = tlp::saveGraph(rootGraph, tlp::QStringToTlpString(_project->toAbsolutePath(
                                           folder + "graph" + graph_extension)),
                            _PluginProgress);
  if (ret) {
    _project->removeAllDir(strategies_path); // Clear old data
    _project->mkpath(strategies_path);       // Create a new strategies structure
    _ui->strategiesManagementWidget->saveAllStrategies(
        _project->toAbsolutePath(strategies_path)); // Export strategies
    ret = _project->write(fileName, _PluginProgress);
    if (ret)
      savingObserver->saved();
  }
  _PluginProgress->reset();
  _PluginProgress->setEnabled(false);

  return ret;
}

void PorgyPerspective::setEnableToolBar(const bool enable) {
  _ui->actionUndo->setEnabled(enable);
  _ui->actionRedo->setEnabled(enable);
  _ui->actionCopy->setEnabled(enable);
  _ui->actionPaste->setEnabled(enable);
  _ui->actionCut->setEnabled(enable);
  _ui->actionSaveMenu->setEnabled(enable);
  _ui->actionSave->setEnabled(enable);
  _ui->actionSave_as->setEnabled(enable);
  _ui->actionClose->setEnabled(enable);
  _ui->menuAlgorithms->setEnabled(enable);
  _ui->actionCreate_new_rule->setEnabled(enable);
  _ui->actionPropertyManager->setEnabled(enable);
  _ui->actionDel_selection->setEnabled(enable);
  _ui->actionSelect_all->setEnabled(enable);
  _ui->actionDeselect_all->setEnabled(enable);
  _ui->actionInvert_selection->setEnabled(enable);
}

bool PorgyPerspective::closeProject() {
  if (rootGraph != nullptr) {            // If there is some previous data
    if (savingObserver->needsSaving()) { // do we need to save the previous
                                         // graph
      QMessageBox::StandardButton answer = QMessageBox::question(
          _mainWindow, "Save", "The project has been modified. Do you want to save your changes?",
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel | QMessageBox::Escape);
      if (answer == QMessageBox::Yes)
        fileSave();
      if (answer == QMessageBox::Cancel) {
        return false;
      }
    }
    delete savingObserver;
    savingObserver = nullptr;
  }
  _ui->viewManager->closeProject();
  tlp::Graph *oldGraph = rootGraph; // Keep old graph to delete it later
  _mainWindow->setWindowModified(false);
  emit resetWindowTitle();

  _ui->graphWidget->setGraph(nullptr);
  _ui->graphWidget->update();

  _ui->ruleWidget->setGraph(nullptr);
  _ui->ruleWidget->update();

  _ui->traceWidget->removeListener();
  _ui->traceWidget->setGraph(nullptr);
  _ui->traceWidget->update();

  rootGraph = nullptr;
  rulesGraph = nullptr;
  modelsGraph = nullptr;
  traceGraph = nullptr;

  tlp::warning() << "Graph closed" << endl;
  setEnableToolBar(false);

  _ui->graphWidget->setEnabled(false);
  _ui->ruleWidget->setEnabled(false);
  _ui->traceWidget->setEnabled(false);
  _ui->strategiesManagementWidget->closeAll();
  _ui->strategiesManagementWidget->setEnabled(false);

  delete oldGraph; // Destruct old data.
  return true;
}

bool PorgyPerspective::setData(Graph *graph, DataSet dataSet) {
  if (rootGraph != nullptr) {            // If there is some previous data
    if (savingObserver->needsSaving()) { // do we need to save the previous
                                         // graph
      QMessageBox::StandardButton answer = QMessageBox::question(
          _mainWindow, "Save", "The project has been modified. Do you want to save your changes?",
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel | QMessageBox::Escape);
      if (answer == QMessageBox::Yes)
        fileSave();
      if (answer == QMessageBox::Cancel) {
        delete graph;
        return false;
      }
    }
    delete savingObserver;
  }
  _ui->viewManager->_workspace->closeAll();
  tlp::Graph *oldGraph = rootGraph; // Keep old graph to delete it later

  std::string errorMsg;
  if (!PorgyTlpGraphStructure::isPorgyGraph(graph, errorMsg)) {
    QMessageBox::critical(_mainWindow, "Graph is not a porgy structure",
                          tlp::tlpStringToQString(errorMsg));
    tlp::warning() << "This graph is not a porgy structure: " << errorMsg << endl;
    return false;
  }

  // The structure is valid load data
  rootGraph = graph;
  savingObserver = new GraphNeedsSavingObserver(rootGraph, _mainWindow);

  if (!errorMsg.empty()) {
    QMessageBox::warning(_mainWindow, "File corrupted/outdated", tlp::tlpStringToQString(errorMsg));
    tlp::warning() << errorMsg << endl;
    savingObserver->forceToSave();
  }

  if (!PorgyTlpGraphStructure::hasProperty(graph))
    QMessageBox::information(_mainWindow, "No property checked for matching",
                             "There is no property checked for matching. Only "
                             "graph topology will be used.<br>Have a look in "
                             "the property manager for more details");

  // Retrieve graphs.
  rulesGraph = PorgyTlpGraphStructure::getRulesRoot(graph);
  modelsGraph = PorgyTlpGraphStructure::getModelsRoot(graph);
  traceGraph = PorgyTlpGraphStructure::getTraceRoot(graph);

  // create missing properties for old graph
  if (!rulesGraph->existLocalProperty(PorgyConstants::W))
    rulesGraph->getLocalProperty<BooleanProperty>(PorgyConstants::W);
  if (!rulesGraph->existLocalProperty(PorgyConstants::M))
    rulesGraph->getLocalProperty<BooleanProperty>(PorgyConstants::M);
  if (!rulesGraph->existLocalProperty(PorgyConstants::N))
    rulesGraph->getLocalProperty<BooleanProperty>(PorgyConstants::N);
  _ui->graphWidget->setGraph(modelsGraph);
  _ui->graphWidget->update();

  _ui->ruleWidget->setGraph(rulesGraph);
  _ui->ruleWidget->update();

  _ui->traceWidget->setGraph(traceGraph);
  _ui->traceWidget->update();
  _ui->traceWidget->addListener();

  _ui->strategiesManagementWidget->setGraph(rulesGraph);
  // Restore strategies
  if (dataSet.exists("strategies")) {
    DataSet strategiesDataSet;
    dataSet.get("strategies", strategiesDataSet);
    _ui->strategiesManagementWidget->setData(strategiesDataSet);
  } else {
    // no strategy. remove data in strategiesManagementWidget
    _ui->strategiesManagementWidget->setData(DataSet());
  }

  setEnableToolBar(true);
  _ui->graphWidget->setEnabled(true);
  _ui->ruleWidget->setEnabled(true);
  _ui->traceWidget->setEnabled(true);
  _ui->strategiesManagementWidget->setEnabled(true);

  delete oldGraph; // Destruct old data.
  return true;
}

void PorgyPerspective::deleteTrace(Graph *trace) {
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(_mainWindow, tr("Delete a derivation tree"),
                                QString("Do you really want to delete \"")
                                    .append(tlpStringToQString(trace->getName()))
                                    .append("\"?"),
                                QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes) {
    if (PorgyTlpGraphStructure::getMainTrace(trace) == trace)
      QMessageBox::critical(_mainWindow, tr("Cannot delete derivation tree"),
                            tr("cannot delete the main derivation tree"));
    else {
      Graph *traceRoot = PorgyTlpGraphStructure::getTraceRoot(trace);
      traceRoot->push();
      traceRoot->delSubGraph(trace);
    }
  }
}

void PorgyPerspective::deleteRule(Graph *rule) {
  QMessageBox::StandardButton reply = QMessageBox::question(
      _mainWindow, tr("Delete a rule"), QString("Do you really want to delete \"")
                                            .append(tlpStringToQString(rule->getName()))
                                            .append("\"?"),
      QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes) {
#ifndef NDEBUG
    cerr << "deleting rule " << rule->getName() << endl;
#endif
    Graph *rules_root = PorgyTlpGraphStructure::getRulesRoot(rulesGraph);
    string rule_name = rule->getName();
    rules_root->push();
    Observable::holdObservers();
    rule->delNodes(rule->nodes(), true);
    rules_root->delAllSubGraphs(rule);
    Observable::unholdObservers();
    tlp::debug() << "Rules " << rule_name << " deleted";
  }
}

void PorgyPerspective::createNewRule() {
  bool ok;
  QString text = QInputDialog::getText(_mainWindow, tr("Create a new empty rule"), tr("Rule name:"),
                                       QLineEdit::Normal, "newrule", &ok);
  if (ok && !text.isEmpty()) {
    rulesGraph->push();
    Observable::holdObservers();
    Graph *new_rule = rulesGraph->addSubGraph(QStringToTlpString(text));
    new_rule->setAttribute<string>(PorgyConstants::LAYOUTALGORITHM, "none");
    // setting M default value to true
    BooleanProperty *Mprop = new_rule->getProperty<BooleanProperty>(PorgyConstants::M);
    Mprop->setAllNodeValue(true);
    Mprop->setAllEdgeValue(true);
    Bridge::createEmptyBridge(new_rule);
    Observable::unholdObservers();
    tlp::warning() << "New rule " << new_rule->getName() << " created" << endl;
    _ui->ruleWidget->setGraph(rulesGraph);
  } else {
    if (text.isEmpty()) {
      tlp::warning() << "Cannot create a rule with an empty name" << endl;
    }
  }
}

void PorgyPerspective::importRule() {
  /**
   * Take name, author and parameter in argument
   **/

  std::string filename;
  QString algorithm("Rule Import");
  DataSet dataSet;

  QStringList importFiles = QFileDialog::getOpenFileNames(_mainWindow, "Import rule", "",
                                                          "Porgy rule graph (*.porgy.rule)");

  if (importFiles.isEmpty())
    return;

  for (int index = 0; index < importFiles.size(); index++) {
    filename = tlp::QStringToTlpString(importFiles.at(index));

    dataSet.set("file::filename", filename);
    PluginProgress *prg = progress(NoProgressOption);
    Graph *temp_ruleGraph =
        tlp::importGraph(tlp::QStringToTlpString(algorithm), dataSet, prg, rulesGraph);
    if (temp_ruleGraph == nullptr) {
      QMessageBox::critical(_mainWindow, "Import error", "Failed to import " +
                                                                     importFiles.at(index) +
                                                                     "\nEmpty graph");
    }
    delete prg;
  }
}

void PorgyPerspective::exportRule(tlp::Graph *rule) {
  if (rule == nullptr)
    return;

  /**
   * Take name, author and parameter in argument
   **/

  QString filename;
  QString algorithm("Rule Export");
  DataSet dataSet;

  filename =
      QFileDialog::getSaveFileName(_mainWindow, "Export rule", "", "Porgy rule file(*.porgy.rule)");

  if (!filename.isEmpty()) {
    if (!filename.endsWith(".porgy.rule"))
      filename += ".porgy.rule";
  } else
    return;

  ParameterDescriptionList params =
      PluginLister::getPluginParameters(tlp::QStringToTlpString(algorithm));
  if (!params.empty()) {
    params.buildDefaultDataSet(dataSet, rule);
    bool ok = true;
    dataSet = PluginParametersConfigurationDialog::getParameters(
        _mainWindow, algorithm + " plugin parameters", params, ok, &dataSet, rule);
  }

  std::ostream *os = new std::ofstream(QStringToTlpString(filename));

  if (os->fail()) {
    QMessageBox::critical(_mainWindow, "File error", "Cannot open output file for writing: " + filename);
    delete os;
    return;
  }

  PluginProgress *prg = progress(NoProgressOption);
  prg->setTitle(tlp::QStringToTlpString(algorithm));
  bool result = tlp::exportGraph(rule, *os, tlp::QStringToTlpString(algorithm), dataSet, prg);
  delete os;

  if (!result) {
    QMessageBox::critical(_mainWindow, "Export error",
                          "Failed to export to " + filename);
  }
  delete prg;
}

void PorgyPerspective::launchPropertyManager() {
  PorgyPropertyManager property(_mainWindow, rootGraph);
  property.exec();
}

void PorgyPerspective::renameRule(tlp::Graph *rule) {

  bool ok;
  QString text =
      QInputDialog::getText(_mainWindow, tr("Rename rule"), tr("New rule name:"), QLineEdit::Normal,
                            tlp::tlpStringToQString(rule->getName()), &ok);
  if (ok && !text.isEmpty()) {
    QRegularExpression re(PorgyConstants::STRAT_MACRO_REGEXP);
    QRegularExpressionMatch match = re.match(text);
    if (!match.hasMatch())
      rule->setName(QStringToTlpString(text));
    else
      QMessageBox::information(_mainWindow, tr("Cannot rename rule"),
                               tr("A rulename cannot have two sharps (#)"));
  }
}

void PorgyPerspective::copyRule(tlp::Graph *rule) {

  bool ok;
  QString text =
      QInputDialog::getText(_mainWindow, tr("Copy rule"), tr("New rule name:"), QLineEdit::Normal,
                            tlp::tlpStringToQString(rule->getName()) + "_2", &ok);
  if (ok && !text.isEmpty()) {
    rulesGraph->push();
    Observable::holdObservers();
    Graph *new_rule = rulesGraph->addSubGraph(QStringToTlpString(text));
    tlp::copyToGraph(new_rule, rule);
    Observable::unholdObservers();
    _ui->ruleWidget->update();
  }
}

void PorgyPerspective::copyGraph(Graph * /*graph*/) {
  tlp::warning() << "TODO!!!" << endl;
  cerr << "TODO!!!!" << endl;
}

void PorgyPerspective::showRule(Graph *graph) {
  if (graph != nullptr) {
    tlp::View *view = _ui->viewManager->createView(PorgyConstants::RULE_VIEW_NAME, graph);
    if (view != nullptr) {
      connect(view, SIGNAL(copy_paste_other_side(PorgyConstants::RuleSide)),
              SLOT(send_other_side(PorgyConstants::RuleSide)));
      connect(view, SIGNAL(pastePortNodes(tlp::Coord &, tlp::Graph *, QList<PortNodeQt *>)),
              SLOT(pastePortNodes(tlp::Coord &, tlp::Graph *, QList<PortNodeQt *>)));
    }
  }
}
void PorgyPerspective::showGraph(Graph *graph) {
  if (graph != nullptr) {
    tlp::View *view = _ui->viewManager->createView(PorgyConstants::GRAPH_VIEW_NAME, graph);
    if (view != nullptr) {
      connect(view, SIGNAL(applyRuleOnModel(tlp::Graph *, tlp::Graph *, tlp::Graph *)),
              SLOT(applyRuleOnModel(tlp::Graph *, tlp::Graph *, tlp::Graph *)));
      connect(view, SIGNAL(applyStrategyOnModel(tlp::Graph *, QString, tlp::Graph *)),
              SLOT(applyStrategyOnModel(tlp::Graph *, QString, tlp::Graph *)));
      connect(view, SIGNAL(pastePortNodes(tlp::Coord &, tlp::Graph *, QList<PortNodeQt *>)),
              SLOT(pastePortNodes(tlp::Coord &, tlp::Graph *, QList<PortNodeQt *>)));
    }
  }
}
void PorgyPerspective::showTrace(Graph *graph, bool useDefaultView) {
  if (graph != nullptr) {
    DataSet pluginParameters;
    string plugin;
    Trace trace(graph);
    // User wants to show the graph with its default view.
    if (useDefaultView) {
      plugin = PorgyConstants::TRACE_VIEW_NAME;
    } else {
      // User does not know the kind of view to use, ask for it.
      plugin = PorgyConstants::TRACE_VIEW_NAME;
      if (graphIsCompatibleWithView(graph, PorgyConstants::SMALL_MULTIPLE_VIEW_NAME)) {
        list<string> plugins;
        plugins.push_back(PorgyConstants::TRACE_VIEW_NAME);
        plugins.push_back(PorgyConstants::HISTOGRAM_NAME);
        plugins.push_back(PorgyConstants::SCATTER_PLOT_NAME);
        plugins.push_back(PorgyConstants::SMALL_MULTIPLE_VIEW_NAME);
        plugins.push_back(PorgyConstants::ANIMATION_VIEW_NAME);

        PluginSelectionWizard wizard(_mainWindow);
        wizard.setPluginList(plugins);
        wizard.page(0)->setTitle(tr("Visualize ") + tlpStringToQString(graph->getName()));

        if (wizard.exec() != QDialog::Accepted) {
          return;
        }
        plugin = wizard.selectedPlugin();
      }
    }
    // If a plugin is set
    if (!plugin.empty()) {
      // Configure plugin parameters.
      // Scatter plot
      if (plugin == PorgyConstants::SCATTER_PLOT_NAME) {
        pluginParameters.set("background color", Color(255, 255, 255));
        pluginParameters.set("min Size Mapping", 20);
        pluginParameters.set("max Size Mapping", 30);
        pluginParameters.set("display graph edges", true);
        if (trace.type() == Trace::TraceType::Histogram) {
          DataSet selectedDataSetProperties;
          selectedDataSetProperties.set<string>("0", trace.depthProperty()->getName());
          vector<IntegerProperty *> portNodeProperties = trace.followedPortnodesProperties();
          ostringstream oss;
          for (unsigned int i = 0; i < portNodeProperties.size(); ++i) {
            oss.str("");
            oss << i + 1;
            selectedDataSetProperties.set<string>(oss.str(), portNodeProperties[i]->getName());
          }
          pluginParameters.set("selected graph properties", selectedDataSetProperties);
        }
      }
      View *v = nullptr;
      v = _ui->viewManager->createView(plugin, graph, pluginParameters);
      if (v->name() == PorgyConstants::TRACE_VIEW_NAME) {
        connect(v, SIGNAL(applyRuleOnModel(tlp::Graph *, tlp::Graph *, tlp::Graph *)),
                SLOT(applyRuleOnModel(tlp::Graph *, tlp::Graph *, tlp::Graph *)));
        connect(v, SIGNAL(applyStrategyOnModel(tlp::Graph *, QString, tlp::Graph *)),
                SLOT(applyStrategyOnModel(tlp::Graph *, QString, tlp::Graph *)));
        connect(v, SIGNAL(openModelView(tlp::Graph *)), this, SLOT(showGraph(tlp::Graph *)));
        connect(v, SIGNAL(newTrace(tlp::node)), this, SLOT(newTrace(tlp::node)));
      }
      if (v->name() == PorgyConstants::SMALL_MULTIPLE_VIEW_NAME) {
        connect(v, SIGNAL(openModelView(tlp::Graph *)), this, SLOT(showGraph(tlp::Graph *)));
        connect(v, SIGNAL(newTrace(tlp::node)), this, SLOT(newTrace(tlp::node)));
      }
    }
  }
}

void PorgyPerspective::newTrace(node n) {
  Graph *newTrace = NewTrace::createNewTrace(traceGraph, n, _mainWindow);
  if (newTrace != nullptr) {
    _ui->traceWidget->update();
    _ui->traceWidget->addListener();
    showTrace(newTrace, false);
  }
}

bool PorgyPerspective::graphIsCompatibleWithView(Graph *graph, const string &viewName) const {
  if (PorgyTlpGraphStructure::isModelGraph(graph))
    return viewName == PorgyConstants::GRAPH_VIEW_NAME;
  else if (PorgyTlpGraphStructure::isRuleGraph(graph))
    return viewName == PorgyConstants::RULE_VIEW_NAME;
  else if (PorgyTlpGraphStructure::isTraceGraph(graph)) {
    if ((viewName == PorgyConstants::SMALL_MULTIPLE_VIEW_NAME) ||
        (viewName == PorgyConstants::ANIMATION_VIEW_NAME) ||
        (viewName == PorgyConstants::SCATTER_PLOT_NAME)) {
      Trace trace(graph);
      // no Fail node inside a Trace
      IntegerProperty *prop = trace.getTypeProperty();
      for (node n : trace.nodes()) {
        if (prop->getNodeValue(n) != Trace::Model_Node) {
          return false;
        }
      }
      return (graph->numberOfNodes() > 1) && PorgyTlpGraphStructure::graphIsASimplePath(graph);
    }
    return viewName == PorgyConstants::TRACE_VIEW_NAME;
  }
  return false;
}

typedef std::vector<node> NodeA;
typedef std::vector<edge> EdgeA;

void GetSelection(NodeA &outNodeA, EdgeA &outEdgeA, Graph *inG, BooleanProperty *inSel) {
  assert(inSel);
  assert(inG);
  outNodeA.clear();
  outEdgeA.clear();
  // Get edges
  for (edge e : inG->edges()) {
    if (inSel->getEdgeValue(e))
      outEdgeA.push_back(e);
  }

  // Get nodes
  for (node n : inG->nodes()) {
    if (inSel->getNodeValue(n))
      outNodeA.push_back(n);
  }
}

void SetSelection(BooleanProperty *outSel, NodeA &inNodeA, EdgeA &inEdgeA, Graph *inG) {
  assert(outSel);
  assert(inG);
  (void)inG; // to remove unused param warning
  outSel->setAllNodeValue(false);
  outSel->setAllEdgeValue(false);

  // Set edges
  for (unsigned int e = 0; e < inEdgeA.size(); e++)
    outSel->setEdgeValue(inEdgeA[e], true);

  // Set nodes
  for (unsigned int n = 0; n < inNodeA.size(); n++)
    outSel->setNodeValue(inNodeA[n], true);
}

//==============================================================
void PorgyPerspective::editCut() {
  // no copy action on the trace view
  assert(currentView()->name() != PorgyConstants::TRACE_VIEW_NAME);

  Graph *graph = getCurrentGraph();

  if (!graph)
    return;
  Observable::holdObservers();

  PortGraph *pg = nullptr;
  BooleanProperty *selP = nullptr;
  if (PorgyTlpGraphStructure::isModelGraph(graph)) {
    pg = new PortGraphModel(graph);
    PortGraphModelDecorator dec(graph);
    selP = dec.getSelectionProperty();
  } else {
    pg = new PortGraphRule(graph);
    PortGraphRuleDecorator dec(graph);
    selP = dec.getSelectionProperty();
  }

  if (selP) {
    for(auto e:selP->getEdgesEqualTo(true, graph)) {
      const pair<const PortNode *, const PortNode *> ends = pg->ends(e);
      ends.first->select(true, selP);
      ends.second->select(true, selP);
    }
    delete pg;
    // Save selection
    NodeA nodeA;
    EdgeA edgeA;
    GetSelection(nodeA, edgeA, graph, selP);
    Graph *newGraph = tlp::newGraph();
    tlp::copyToGraph(newGraph, graph, selP);
    stringstream tmpss;
    DataSet dataSet;
    tlp::exportGraph(newGraph, tmpss, "TLP Export", dataSet, nullptr);
    delete newGraph;
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(tmpss.str().c_str());
    graph->push();
    // Restore selection
    SetSelection(selP, nodeA, edgeA, graph);
    tlp::removeFromGraph(graph, selP);
    redrawPanels();
  }
  Observable::unholdObservers();
}
//==============================================================
void PorgyPerspective::editCopy() {

  // no copy action on the trace view
  assert(currentView()->name() != PorgyConstants::TRACE_VIEW_NAME);

  Graph *graph = getCurrentGraph();

  if (!graph)
    return;

  Observable::holdObservers();

  PortGraph *pg = nullptr;
  BooleanProperty *selP = nullptr;
  if (PorgyTlpGraphStructure::isModelGraph(graph)) {
    pg = new PortGraphModel(graph);
    selP = PortGraphModelDecorator(graph).getSelectionProperty();
  } else {
    pg = new PortGraphRule(graph);
    selP = PortGraphRuleDecorator(graph).getSelectionProperty();
  }

  // Extend selection if necessary
  if (selP != nullptr) {
    for (edge e : pg->getEdges()) {
      if (selP->getEdgeValue(e)) {
        const pair<const PortNode *, const PortNode *> ends = pg->ends(e);
        ends.first->select(true, selP);
        ends.second->select(true, selP);
      }
    }

    Graph *newGraph = tlp::newGraph();
    tlp::copyToGraph(newGraph, graph, selP);
    stringstream tmpss;
    DataSet dataSet;
    tlp::exportGraph(newGraph, tmpss, "TLP Export", dataSet, nullptr);
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::fromUtf8(tmpss.str().c_str()));
    delete newGraph;
  }
  Observable::unholdObservers();
  delete pg;
}
//==============================================================
void PorgyPerspective::editPaste(PorgyConstants::RuleSide newside) {
  Graph *graph = getCurrentGraph();

  if (!graph)
    return;

  // Get data in the clipboard.
  QClipboard *clipboard = QApplication::clipboard();
  QString data = clipboard->text();

  // If there is data
  if (!data.isEmpty()) {

    Graph *newGraph = tlp::newGraph();
    DataSet dataSet;
    dataSet.set<string>("file::data", data.toUtf8().data());
    tlp::importGraph("TLP Import", dataSet, nullptr, newGraph);
    // If there is nodes in the imported graph.
    if (!newGraph->isEmpty()) {
      Observable::holdObservers();
      BooleanProperty *selP = graph->getProperty<BooleanProperty>("viewSelection");
      graph->push();
      PortGraphRuleDecorator dec(newGraph);
      IntegerProperty *side = dec.getSideProperty();
      // If the destination graph is a rule graph.
      if (PorgyTlpGraphStructure::isRuleGraph(graph)) {
        // If at least one node as no side treat the whole graph as a model
        // graph.
        bool sidePropertyExists = true;
        for (node n : newGraph->nodes()) {
          if (side->getNodeValue(n) == PorgyConstants::NO_SIDE) {
            sidePropertyExists = false;
            break;
          }
        }
        // If the source graph is a model graph and if the side was not
        // specified ask user.
        if (!sidePropertyExists && newside == PorgyConstants::NO_SIDE) {
          SpecialPasteDialog dialog(_mainWindow);
          if (dialog.exec()) {
            newside = dialog.getNewRuleSideName();
          } else {
            delete newGraph;
            Observable::unholdObservers();
            return;
          }
        }
        // If the side was defined overwrite existing or create new one
        if (newside != PorgyConstants::NO_SIDE) {
          side->setAllNodeValue(newside);
          side->setAllEdgeValue(newside);
        }
      } else {
        // If the destination graph is model erase all side values.
        if (graph->existProperty(PorgyConstants::RULE_SIDE)) {
          graph->delLocalProperty(PorgyConstants::RULE_SIDE);
        }
      }

      Observable::holdObservers();
      tlp::copyToGraph(graph, newGraph, nullptr, selP);
      Observable::unholdObservers();
      // Try to find the first empty space
      LayoutProperty *positions = graph->getProperty<LayoutProperty>("viewLayout");
      SizeProperty *sizes = graph->getProperty<SizeProperty>("viewSize");
      // Compute the size of the selection
      BoundingBox b = computeBoundingBox(graph, positions, sizes,
                                         graph->getProperty<DoubleProperty>("viewRotation"), selP);
      Coord shift;
      bool foundEmptySpace = false;
      int layer = 0;
      while (!foundEmptySpace) {
        // Increase the layer
        ++layer;
        // 0: north, 1: north-east, 2: east, 3:south-east, 4:south,
        // 5:south-west, 6:west, 7:north-west
        for (int direction = 0; direction < 8; ++direction) {
          // Reset shift
          shift = Coord();
          // east
          if (direction > 0 && direction < 4) {
            shift[0] = 1;
          } else if (direction > 4) {
            // west
            shift[0] = -1;
          }
          // North
          if (direction == 0 || direction == 1 || direction == 7) {
            shift[1] = -1;
          } else if (direction != 2) {
            // South
            shift[1] = 1;
          }
          shift[0] *= b.width() * layer * 1.1;
          shift[1] *= b.height() * layer * 1.1;
          BoundingBox newBox(b);
          newBox.translate(shift);

          foundEmptySpace = true;
          for (node n : graph->nodes()) {
            BoundingBox nodeBox;
            nodeBox.expand(positions->getNodeValue(n) + sizes->getNodeValue(n) / 2.f);
            nodeBox.expand(positions->getNodeValue(n) - sizes->getNodeValue(n) / 2.f);
            if (newBox.intersect(nodeBox)) {
              foundEmptySpace = false;
              break;
            }
          }
          if (foundEmptySpace) {
            for (edge e : graph->edges()) {
              const vector<Coord> &bends = positions->getEdgeValue(e);
              for (auto &c : bends) {
                if (newBox.contains(c)) {
                  foundEmptySpace = false;
                  break;
                }
              }
            }
            if (foundEmptySpace) {
              break;
            }
          }
        }
      }
      // Shift nodes center
      for(auto n:selP->getNodesEqualTo(true)) {
        positions->setNodeValue(n, positions->getNodeValue(n) + shift);
      }
      // Shift edges bends
      for(auto e:selP->getEdgesEqualTo(true)) {
        vector<Coord> bends = positions->getEdgeValue(e);
        for (auto c : bends) {
          c = c + shift;
        }
        positions->setEdgeValue(e, bends);
      }

      Observable::unholdObservers();
      redrawPanels(false);
    }
    // Delete new graph
    delete newGraph;
  }
}

//==============================================================
void PorgyPerspective::editDelSelection() {
  Graph *graph = getCurrentGraph();
  if (!graph)
    return;
  Observable::holdObservers();
  graph->push();
  BooleanProperty *selection = nullptr;
  if (PorgyTlpGraphStructure::isModelGraph(graph)) {
    selection = PortGraphModelDecorator(graph).getSelectionProperty();
  } else if (PorgyTlpGraphStructure::isRuleGraph(graph)) {
    selection = PortGraphRuleDecorator(graph).getSelectionProperty();
  } else {
    Trace t(graph);
    selection = t.getSelectionProperty();
    // check if we have a graph
    tlp::makeSelectionGraph(graph, selection);
    Graph *g = graph->addSubGraph(selection);
    for (node n : g->nodes()) {
      StringProperty *label = g->getProperty<StringProperty>("viewLabel");
      if (label->getNodeValue(n) == PorgyConstants::INITIAL_GRAPH) {
        QMessageBox::warning(_mainWindow, "Cannot delete G0", "G0 cannot be deleted.");
        graph->delSubGraph(g);
        Observable::unholdObservers();
        return;
      }
    }

    if (PorgyTlpGraphStructure::graphIsASimplePath(g) &&
        PorgyTlpGraphStructure::hasLeaf(selection, g)) {
      // delete from the whole hierarchy if we are on TraceMain, otherwise, just
      // delete the elements from the subgraph
      for (node n : g->nodes()) {
        tlp::Iterator<edge> *itEdges = new StableIterator<edge>(selection->getEdgesEqualTo(true));
        if (PorgyTlpGraphStructure::getMainTrace(g) == graph) {
          graph->delEdges(itEdges, true);
          Graph *meta = graph->getNodeMetaInfo(n);
          g->delNode(n, true);
          if (meta != nullptr) {
            Graph *r = g->getRoot()->getDescendantGraph(meta->getId())->getSuperGraph();
            r->delSubGraph(meta);
          }
        } else {
          graph->delEdges(itEdges);
          graph->delNode(n);
        }
        delete itEdges;
      }
    } else {
      QMessageBox::warning(_mainWindow, "Not a branch",
                           "Only a branch of the tree (must terminate on a "
                           "leaf and node degree equals to 1) can be "
                           "deleted.<br>");
    }
    graph->delSubGraph(g);
    Observable::unholdObservers();
    return;
  }
  tlp::Iterator<edge> *itEdges = new StableIterator<edge>(selection->getEdgesEqualTo(true));
  graph->delEdges(itEdges, true);
  delete itEdges;
  tlp::Iterator<node> *itNodes = new StableIterator<node>(selection->getNodesEqualTo(true));
  graph->delNodes(itNodes, true);
  delete itNodes;

  Observable::unholdObservers();
}
//==============================================================
void PorgyPerspective::editReverseSelection() {
  Graph *graph = getCurrentGraph();

  if (!graph)
    return;

  graph->push();
  PortGraph *pgm = nullptr;
  BooleanProperty *selectionProperty = nullptr;
  Observable::holdObservers();
  if (PorgyTlpGraphStructure::isModelGraph(graph)) {
    pgm = new PortGraphModel(graph);
    selectionProperty = PortGraphModelDecorator(graph).getSelectionProperty();
  } else if (PorgyTlpGraphStructure::isRuleGraph(graph)) {
    pgm = new PortGraphRule(graph);
    selectionProperty = PortGraphRuleDecorator(graph).getSelectionProperty();
  }
  if (pgm != nullptr) {
    for (edge e : pgm->getEdges()) {
      selectionProperty->setEdgeValue(e, !selectionProperty->getEdgeValue(e));
    }
    for (PortNode *p : pgm->getPortNodes()) {
      p->select(!selectionProperty->getNodeValue(p->getCenter()), selectionProperty);
    }
    delete pgm;
  }

  if (PorgyTlpGraphStructure::isTraceGraph(graph)) {
    Trace t(graph);
    selectionProperty = t.getSelectionProperty();
    selectionProperty->reverse(graph);
  }
  Observable::unholdObservers();
}
//==============================================================

Graph *PorgyPerspective::getCurrentGraph() {
  View *currentView = _ui->viewManager->currentView();
  return (currentView != nullptr) ? currentView->graph() : nullptr;
}

tlp::View *PorgyPerspective::currentView() {
  return _ui->viewManager->currentView();
}

//==============================================================
void PorgyPerspective::editSelectAll() {
  Graph *graph = getCurrentGraph();
  if (!graph)
    return;

  BooleanProperty *selP = nullptr;
  if (PorgyTlpGraphStructure::isModelGraph(graph)) {
    selP = PortGraphModelDecorator(graph).getSelectionProperty();
  } else if (PorgyTlpGraphStructure::isRuleGraph(graph)) {
    selP = PortGraphRuleDecorator(graph).getSelectionProperty();
  } else if (PorgyTlpGraphStructure::isTraceGraph(graph)) {
    Trace t(graph);
    selP = t.getSelectionProperty();
  }

  if (selP) {
      graph->push();
      Observable::holdObservers();
      selP->setValueToGraphNodes(true, graph);
      selP->setValueToGraphEdges(true, graph);
      Observable::unholdObservers();
  }

}
//==============================================================
void PorgyPerspective::editDeselectAll() {
  Graph *graph = getCurrentGraph();
  if (!graph)
    return;
  BooleanProperty *selP = nullptr;
  if (PorgyTlpGraphStructure::isModelGraph(graph)) {
    selP = PortGraphModelDecorator(graph).getSelectionProperty();
  } else if (PorgyTlpGraphStructure::isRuleGraph(graph)) {
    selP = PortGraphRuleDecorator(graph).getSelectionProperty();
  } else if (PorgyTlpGraphStructure::isTraceGraph(graph)) {
    Trace t(graph);
    selP = t.getSelectionProperty();
  }

  if (selP) {
      graph->push();
      Observable::holdObservers();
      selP->setValueToGraphNodes(false, graph);
      selP->setValueToGraphEdges(false, graph);
      graph->popIfNoUpdates();
      Observable::unholdObservers();
  }

}

void PorgyPerspective::viewActivated(View *view) {
  // disabled copy/cut/paste for the trace view
  if (view->name() == PorgyConstants::TRACE_VIEW_NAME) {
    _ui->actionCut->setEnabled(false);
    _ui->actionCopy->setEnabled(false);
    _ui->actionPaste->setEnabled(false);
  } else {
    _ui->actionCut->setEnabled(true);
    _ui->actionCopy->setEnabled(true);
    _ui->actionPaste->setEnabled(true);
  }
}

void PorgyPerspective::EnableInteractors(const bool enable) {
  // disable interactors
  foreach (View *v, _ui->viewManager->_workspace->panels()) {
    foreach (Interactor *i, v->interactors()) { i->action()->setEnabled(enable); }
  }
}

void PorgyPerspective::FixIfNotMainTrace(Graph *trace, Graph *mainTrace,
                                         const set<node> &toVisualize,
                                         const GraphModficationListener &listener) {
  if (trace != mainTrace) { // if not using maintrace
    ColorProperty *colorMain = mainTrace->getProperty<ColorProperty>("viewColor");
    ColorProperty *color = trace->getProperty<ColorProperty>("viewColor");
    for (const node &n : toVisualize) {
      trace->addNode(n);
      color->setNodeValue(n, colorMain->getNodeValue(n));
    }
    set<edge> edgetoVisualize(listener.getCreatedEdges());
    for (const edge &e : edgetoVisualize) {
      trace->addEdge(e);
      color->setEdgeValue(e, colorMain->getEdgeValue(e));
    }
    string err;
    Trace trace2dec(trace);
    trace2dec.redraw(err, _PluginProgress);
    zoomOnTraceElementAnimation(trace, toVisualize, sender());
  }
}

void PorgyPerspective::applyStrategyOnModel(Graph *model, QString strategy, tlp::Graph *trace) {
  assert(model != nullptr);
  _ui->traceWidget->removeListener();
  DataSet dataSet;
  dataSet.set<string>(PorgyConstants::STRATEGY, QStringToTlpString(strategy));
  dataSet.set<bool>(PorgyConstants::DEBUG, _debug);
  Observable::holdObservers();
  GraphModficationListener listener;
  Graph *mainTrace = PorgyTlpGraphStructure::getMainTrace(rootGraph);
  listener.startRecording(mainTrace);
  string err;

  EnableInteractors(false);
  bool ok = applyAlgorithm(PorgyConstants::APPLY_STRATEGY_ALGORITHM, model, dataSet, err);
  listener.stopRecording(mainTrace);
  set<node> toVisualize = listener.getCreatedNodes();
  Trace tracedec(mainTrace);
  node modelNode = tracedec.nodeForModel(model);
  if (modelNode.isValid())
    toVisualize.insert(modelNode);
  if (ok) {
    FixIfNotMainTrace(trace, mainTrace, toVisualize, listener);
    tracedec.redraw(err, _PluginProgress);
    zoomOnTraceElementAnimation(mainTrace, toVisualize, sender());
  } else {
    tlp::warning() << "Error applying strategy: " << err << endl;
  }
  Observable::unholdObservers();
  EnableInteractors(true);
  _ui->traceWidget->addListener();
}

void PorgyPerspective::zoomOnTraceElementAnimation(Graph *trace,
                                                   const std::set<tlp::node> &elements,
                                                   const QObject *object) {
  unsigned int animationTime = settings.traceViewZoomAnimationTime();
  // Graph* mainTrace = PorgyTlpGraphStructure::getMainTrace(rootGraph);
  // If we created nodes
  if (!elements.empty()) {
    BooleanProperty createdNodesProperty(trace);
    createdNodesProperty.setAllNodeValue(false);
    createdNodesProperty.setAllEdgeValue(false);
    for (set<node>::const_iterator it = elements.begin(); it != elements.end(); ++it) {
      createdNodesProperty.setNodeValue(*it, true);
    }
    vector<View *> views = _ui->viewManager->getViewsForGraph(trace);
    //      Graph* mainTrace = PorgyTlpGraphStructure::getMainTrace(rootGraph);
    if (!views.empty()) {
      // Don't animate views if user does not want it.
      if (animationTime > 0) {

        QParallelAnimationGroup *group = new QParallelAnimationGroup();
        for (View *view : views) {
          BoundingBox bbox;

          if (view == object) {
            // If the current view is the view where the elements where applied
            // zoom on modification.
            bbox = computeBoundingBox(trace, trace->getProperty<LayoutProperty>("viewLayout"),
                                      trace->getProperty<SizeProperty>("viewSize"),
                                      trace->getProperty<DoubleProperty>("viewRotation"),
                                      &createdNodesProperty);
          } else {
            // Center the view.
            bbox = computeBoundingBox(trace, trace->getProperty<LayoutProperty>("viewLayout"),
                                      trace->getProperty<SizeProperty>("viewSize"),
                                      trace->getProperty<DoubleProperty>("viewRotation"), nullptr);
          }

          if (view->name().compare(PorgyConstants::TRACE_VIEW_NAME) == 0) {
            GlMainView *mainView = static_cast<GlMainView *>(view);
            GlMainWidgetZoomAndPanAnimation *animation =
                new GlMainWidgetZoomAndPanAnimation(mainView->getGlMainWidget(), bbox, 50, group);
            animation->setDuration(animationTime);
            group->addAnimation(animation);
          }
        }
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        group->start();
        while (group->state() == QAbstractAnimation::Running) {
          QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        QApplication::restoreOverrideCursor();
        group->deleteLater();
      } else {
        for (View *view : views) {
          if (view->name().compare(PorgyConstants::TRACE_VIEW_NAME) == 0) {
            view->centerView();
          }
        }
      }
    }
  }
}

void PorgyPerspective::applyRuleOnModel(Graph *model, Graph *rule, Graph *trace) {
  assert(PorgyTlpGraphStructure::isModelGraph(model));
  assert(PorgyTlpGraphStructure::isRuleGraph(rule));
  _ui->traceWidget->removeListener();
  QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
  DataSet dataSet;
  ParameterDescriptionList params =
      PluginLister::getPluginParameters(PorgyConstants::SIMPLECHECKAPPLY);
  params.buildDefaultDataSet(dataSet, model);
  dataSet.set<std::string>(PorgyConstants::RuleName, rule->getName());
  QString title = QString("Apply rule ") + tlp::tlpStringToQString(rule->getName()) +
                  QString(" on model ") + tlp::tlpStringToQString(model->getName());
  bool ok = true;
  dataSet = PluginParametersConfigurationDialog::getParameters(_mainWindow, title, params, ok,
                                                               &dataSet, model);

  if (ok) {
    Graph *mainTrace = PorgyTlpGraphStructure::getMainTrace(rootGraph);
    Observable::holdObservers();
    GraphModficationListener listener;
    // Register modifications
    listener.startRecording(mainTrace);
    dataSet.set<std::string>(PorgyConstants::RuleName, rule->getName()); // Set last parameter
    dataSet.set<bool>(PorgyConstants::DEBUG, _debug);
    string error;
    model->push();
    EnableInteractors(false);
    model->applyAlgorithm(PorgyConstants::SIMPLECHECKAPPLY, error, &dataSet, _PluginProgress);
    model->popIfNoUpdates();
    listener.stopRecording(mainTrace);
    set<node> toVisualize = listener.getCreatedNodes();
    Trace tracedec(mainTrace);
    node modelNode = tracedec.nodeForModel(model);
    if (modelNode.isValid()) {
      toVisualize.insert(modelNode);
    }

    // Add parent node
    FixIfNotMainTrace(trace, mainTrace, toVisualize, listener);
    tracedec.redraw(error, _PluginProgress);
    zoomOnTraceElementAnimation(mainTrace, toVisualize, sender());
    Observable::unholdObservers();
  }

  QApplication::restoreOverrideCursor();
  EnableInteractors(true);
  _ui->traceWidget->addListener();
}

//**********************************************************************
bool PorgyPerspective::changeProperty(const QString &name, tlp::Graph *graph, std::string &err) {
  DataSet parameters;
  // plugin parameters dialog
  ParameterDescriptionList params =
      PluginLister::getPluginParameters(tlp::QStringToTlpString(name));
  DataSet dataSet;
  params.buildDefaultDataSet(dataSet, graph);
  QString title = QString("Lauching ") + name + " on " + tlp::tlpStringToQString(graph->getName());
  bool resultBool = true;
  parameters = PluginParametersConfigurationDialog::getParameters(_mainWindow, title, params,
                                                                  resultBool, &dataSet, graph);

  if (resultBool) {
    Observable::holdObservers();
    graph->push(); // Record current state.
    _PluginProgress->setEnabled(true);
    resultBool =
        graph->applyAlgorithm(tlp::QStringToTlpString(name), err, &parameters, _PluginProgress);
    _PluginProgress->setEnabled(false);
    _PluginProgress->reset();
    graph->popIfNoUpdates();
  }
  return resultBool;
}

//**********************************************************************
void PorgyPerspective::changeMetric() {
  QAction *action = static_cast<QAction *>(sender());
  QString algorithm = action->data().toString();
  std::string err;
  if (getCurrentGraph()&&!changeProperty(algorithm, getCurrentGraph(), err)) {
    tlp::warning() << "Error when applying metric algorithm " << algorithm.toStdString() << endl;
  }
}

void PorgyPerspective::ImportGraph() {
  QAction *action = static_cast<QAction *>(sender());
  Graph *graph = getCurrentGraph();
  if (graph != nullptr || rootGraph != nullptr) {
    QMessageBox::warning(_mainWindow, "Cannot import data",
                         "It is not possible to import data in an existing graph");
    return;
  }

  // Qt5 adds a shortcut (with '&') to the action name and does not remove it
  // when getting the text. Use data() instead to retrieve the name of the
  // action
  QString algorithm = action->data().toString();
  DataSet dataSet;
  ParameterDescriptionList params =
      PluginLister::getPluginParameters(tlp::QStringToTlpString(algorithm));
  if (!params.empty()) {
    params.buildDefaultDataSet(dataSet);
    bool ok = true;
    dataSet = PluginParametersConfigurationDialog::getParameters(
        _mainWindow, algorithm + " plugin parameters", params, ok, &dataSet);
  }
  if (dataSet.empty())
    return;
  graph = tlp::importGraph(algorithm.toStdString(), dataSet, _PluginProgress);
  if (graph != nullptr) {
    Graph *g = PorgyTlpGraphStructure::setPorgyStructure(graph);

    // apply a layout algorithm
    string errMsg;
    StringCollection algo(LAYOUT_ALGO);
    string algoname("");
    dataSet.get("Redraw algorithm", algoname);
    algo.setCurrent(algoname);
    DataSet dSet;
    dSet.set("Layout", algo);
    Graph *G0 =
        PorgyTlpGraphStructure::getModelsRoot(g)->getSubGraph(PorgyConstants::INITIAL_GRAPH);
    LayoutProperty *layout = G0->getProperty<LayoutProperty>("viewLayout");
    if (!G0->applyPropertyAlgorithm(PorgyConstants::REDRAW_GRAPH, layout, errMsg, &dSet,_PluginProgress)) {
      tlp::warning() << "Problem when applying layout algorithm: " << errMsg << endl;
    }

    setData(g);
  } else {
    QMessageBox::warning(_mainWindow, "Cannot import data",
                         "Cannot import data: " +
                             tlp::tlpStringToQString(_PluginProgress->getError()));
  }
}

//**********************************************************************
/// Apply a general algorithm
bool PorgyPerspective::applyAlgorithm() {
  QAction *action = static_cast<QAction *>(sender());
  QString algorithm = action->data().toString();
  Graph *graph = getCurrentGraph();
  if (!graph)
    return false;

  std::string errorMsg;
  DataSet dataSet;
  ParameterDescriptionList params =
      PluginLister::getPluginParameters(tlp::QStringToTlpString(algorithm));
  if (!params.empty()) {
    params.buildDefaultDataSet(dataSet, graph);
    bool ok = true;
    dataSet = PluginParametersConfigurationDialog::getParameters(
        _mainWindow, algorithm + " plugin parameters", params, ok, &dataSet, graph);
  }
  return applyAlgorithm(tlp::QStringToTlpString(algorithm), graph, dataSet, errorMsg);
}

bool PorgyPerspective::applyAlgorithm(const std::string &algorithm, tlp::Graph *graph,
                                      tlp::DataSet &parameters, std::string &errorMsg) {
  Observable::holdObservers();
  graph->push();

  _PluginProgress->setEnabled(true);
  bool ok = true;
  if (!graph->applyAlgorithm(algorithm, errorMsg, &parameters, _PluginProgress)) {
    QMessageBox::warning(_mainWindow, "Cannot execute algorithm",
                         "Cannot run the plugin: " + tlp::tlpStringToQString(errorMsg));
    ok = false;
  }
  graph->popIfNoUpdates();
  _PluginProgress->setEnabled(false);
  _PluginProgress->reset();
  Observable::unholdObservers();
  return ok;
}

void PorgyPerspective::send_other_side(const PorgyConstants::RuleSide side) {
  View *view = currentView();
  if (view != nullptr) {
    assert(view->name() == PorgyConstants::RULE_VIEW_NAME);
    Observable::holdObservers();
    editCopy();
    editPaste(side);
    Observable::unholdObservers();
  }
}

void PorgyPerspective::pastePortNodes(Coord &center, Graph *graph, QList<PortNodeQt *> portNodes) {
  graph->push();
  Observable::holdObservers();
  foreach (const PortNodeQt *pn, portNodes) { pn->addToTlpGraphUsingPosition(graph, center); }
  Observable::unholdObservers();
}

void PorgyPerspective::showDocumentation() {
  QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromUtf8(tlp::TulipShareDir.c_str()) +
                                                "doc/porgy/html/index.html"));
}

void PorgyPerspective::showAbout() {
  AboutDialog diag(_mainWindow);
  diag.exec();
}

void PorgyPerspective::addPanel(Graph *graph) {
  if (graph != nullptr) {
    if (PorgyTlpGraphStructure::isModelGraph(graph)) {
      showGraph(graph);
    } else if (PorgyTlpGraphStructure::isRuleGraph(graph)) {
      showRule(graph);
    } else if (PorgyTlpGraphStructure::isTraceGraph(graph)) {
      showTrace(graph);
    }
  } /*else{

   }*/
}

void PorgyPerspective::showSpreadSheetView(Graph *graph) {
  QMessageBox::StandardButton button =
      QMessageBox::warning(_mainWindow, "Spreadsheet view: warning",
                           "This Tulip view gives access to the low-level Tulip datastructure. "
                           "There is a high risk of breaking your project, unexpected crash and "
                           "data loss. Do you really want to continue?",
                           QMessageBox::Yes | QMessageBox::No);
  if (button == QMessageBox::Yes) {
    if (graph == nullptr)
      graph = rootGraph;
    _ui->viewManager->createView("Spreadsheet view", graph);
  }
}

void PorgyPerspective::showPythonDocumentation() {
  QDesktopServices::openUrl(QUrl::fromLocalFile(tlpStringToQString(tlp::TulipShareDir) +
                                                "../doc/tulip/tulip-python/html/index.html"));
}

void PorgyPerspective::showPythonIDE(Graph *g) {
  QMessageBox::StandardButton button = QMessageBox::warning(
      _mainWindow, "Python IDE: warning",
      "The Python IDE gives access to the low-level Tulip datastructure. There "
      "is a high risk of breaking your project, unexpected crash and data "
      "loss. Do you really want to continue?",
      QMessageBox::Yes | QMessageBox::No);
  if (button == QMessageBox::Yes) {
    if (g == nullptr)
      g = rootGraph;
    gModel = new GraphHierarchiesModel(this);
    gModel->graphImported(g);
    gModel->setCurrentGraph(g);
    pythonIDE->setGraphsModel(gModel);
    pythonIDE->projectNeedsPythonIDE(_project);

    _pythonIDEDialog->show();
    _pythonIDEDialog->raise();
  }
}

void PorgyPerspective::undo() {
  if (rootGraph->canPop()) {
    Observable::holdObservers();
    rootGraph->pop();
    Observable::unholdObservers();
    foreach (View *v, _ui->viewManager->_workspace->panels()) { v->undoCallback(); }
  }
}

void PorgyPerspective::redo() {
  if (rootGraph->canUnpop()) {
    Observable::holdObservers();
    rootGraph->unpop();
    Observable::unholdObservers();

    foreach (auto *v, _ui->viewManager->_workspace->panels()) { v->undoCallback(); }
  }
}

PLUGIN(PorgyPerspective)
