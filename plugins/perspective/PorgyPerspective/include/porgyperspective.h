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
#ifndef PORGYPERSPECTIVE_H
#define PORGYPERSPECTIVE_H

#include <tulip/Coord.h>
#include <tulip/Perspective.h>

#include <porgy/porgysettings.h>

#include <portgraph/PorgyConstants.h>
#include <portgraph/PorgyRelease.h>

#include "graphmodificationlistener.h"

namespace tlp {
class Graph;
class GraphNeedsSavingObserver;
class View;
class PythonIDE;
class GraphHierarchiesModel;
}

class PortNodeConfigurationWidget;
class PortNodeQt;
class StatusBarPluginProgress;
class QDialog;

namespace Ui {
class PorgyPerspective;
}

class PorgyPerspective : public tlp::Perspective {
  Q_OBJECT

  enum ViewManagerImplementation { MDI_AREA_IMPL, WORKSPACE_IMPL };

  static StatusBarPluginProgress *_PluginProgress;

  tlp::PythonIDE *pythonIDE;
  QDialog *_pythonIDEDialog;
  tlp::GraphHierarchiesModel *gModel;

  /**
   * @brief Build all the GUI of the controllers, ie widgets, menu ect...
   **/
  void buildGUI();
  void usage(std::string& usage_str) const override;
  /**
   * @brief Build the menu bar of the Porgy controller.
   **/
  void initMenus();
  void setEnableToolBar(const bool enable);
  /**
   * @brief Reset all the internal data.
   **/
  void initData();

  void zoomOnTraceElementAnimation(tlp::Graph *g, const std::set<tlp::node> &elements,
                                   const QObject *object = nullptr);

  bool setData(tlp::Graph *graph, tlp::DataSet dataSet = tlp::DataSet());

  bool closeProject();

  /**
   * @brief save saves the current state in the given file.
   * @param fileName
   */
  bool save(const QString &fileName, const QString &graph_extension);

  tlp::Graph *fileOpen(const QString &file, tlp::PluginProgress *prg);

  /**
   * @brief getCurrentGraph Returns the graph of the current view or nullptr if
   * there is no view.
   * @return
   */
  tlp::Graph *getCurrentGraph();

  tlp::View *currentView();

  bool applyAlgorithm(const std::string &algorithm, tlp::Graph *graph, tlp::DataSet &parameters,
                      std::string &errorMsg);
  bool changeProperty(const QString &name, tlp::Graph *graph, std::string &err);

  /**
   * @brief graphIsCompatibleWithView checks if the view with the given name is
   * compatible with the graph.
   * @param graph The graph to test.
   * @param viewName The name of the view.
   * @return true if the view and the graph are compatible.
   */
  bool graphIsCompatibleWithView(tlp::Graph *graph, const std::string &viewName) const;
  void open(tlp::PluginProgress *prg);

  void EnableInteractors(const bool enable);

  Ui::PorgyPerspective *_ui;

  // Graph
  tlp::Graph *rootGraph;
  tlp::Graph *rulesGraph;
  tlp::Graph *modelsGraph;
  tlp::Graph *traceGraph;

  PortNodeConfigurationWidget *portNodeConfigurationWidget;
  tlp::GraphNeedsSavingObserver *savingObserver;

  PorgySettings settings;
  bool _debug;

  void FixIfNotMainTrace(tlp::Graph *trace, tlp::Graph *mainTrace,
                         const std::set<tlp::node> &toVisualize,
                         const GraphModficationListener &listener);

public:
  PLUGININFORMATION("Porgy", "Porgy Team", "8/11/2010", "Porgy", PORGYVERSION,
                    PorgyConstants::CATEGORY_NAME)
  PorgyPerspective(const tlp::PluginContext *context);
  ~PorgyPerspective() override;

  void redrawPanels(bool center = false) override;
  void start(tlp::PluginProgress *progress) override;
  bool terminated() override;
  void getData(tlp::Graph **graph, tlp::DataSet *data);

  void log(QtMsgType, const QMessageLogContext &, const QString &);

private slots:

  /**
   * @brief viewActivated is called when a view is activated in the view
   * manager.
   * @param view
   */
  void viewActivated(tlp::View *view);
  void fileNew();
  void newBlankDataStructure();
  void debugPressed(bool checked);
  void highlight_rule_derivation_tree(tlp::Graph *rule);
  void openPreferences();
  void newTrace(tlp::node n);
  void showPythonDocumentation();
  /**
       * @brief fileOpen open a file and load data in the perspective.
       */
  void fileOpen();

  /**
       * @brief fileSave saves the modifications in the current edited file or
    *in a new file if there is the first time the data are saved.
       **/
  bool fileSave();
  /**
       * @brief fileSaveAs saves the modifications in a new file.
       */
  bool fileSaveAs();
  void validate();
  /**
       * @brief fileClose proposes to save the modifications and close the
   * current project.
       */
  void fileClose();

  void applyRuleOnModel(tlp::Graph *model, tlp::Graph *rule, tlp::Graph *trace);
  void applyStrategyOnModel(tlp::Graph *model, QString strategy, tlp::Graph *);
  void createNewRule();
  void launchPropertyManager();

  void showGraph(tlp::Graph *graph);
  void showRule(tlp::Graph *graph);
  void showTrace(tlp::Graph *graph, bool useDefaultView = false);
  void showPythonIDE(tlp::Graph *graph = nullptr);
  void showSpreadSheetView(tlp::Graph *graph = nullptr);

  void editDelSelection();
  void editReverseSelection();
  void editSelectAll();
  void editDeselectAll();
  void editCut();
  void ImportGraph();
  void editCopy();
  void editPaste(PorgyConstants::RuleSide side = PorgyConstants::NO_SIDE);
  void send_other_side(const PorgyConstants::RuleSide);

  void pastePortNodes(tlp::Coord &center, tlp::Graph *, QList<PortNodeQt *> portNodes);
  // rules handling
  void renameRule(tlp::Graph *rule);
  void deleteRule(tlp::Graph *rule);
  void copyRule(tlp::Graph *);
  void importRule();
  void exportRule(tlp::Graph *rule);
  void copyGraph(tlp::Graph *);
  void changeMetric();
  bool applyAlgorithm();

  /**
   * @brief deleteTrace delete the trace represented by the given graph.
   * @param traceGraph the trace graph to delete
   */
  void deleteTrace(tlp::Graph *traceGraph);

  /**
   * @brief showDocumentation display the documentation dock widget.
   */
  void showDocumentation();
  void showAbout();
  void toggleViewManagerImplementation();

  void addPanel(tlp::Graph *graph);

  /**
   * @brief undo undo the last modifications.
   */
  void undo();

  /**
   * @brief redo redo last undo modifications.
   */
  void redo();
};

#endif // PORGYPERSPECTIVE_H
