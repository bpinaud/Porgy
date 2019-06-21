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
#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <string>

#include <tulip/DataSet.h>

#include <QList>
#include <QMap>
#include <QWidget>

namespace tlp {
class Graph;
class View;
class GraphHierarchiesModel;
class TulipProject;
class PluginProgress;
class Workspace;
}

class QHBoxLayout;
class QMdiArea;

class ViewManagerImplementation : public QObject {
  Q_OBJECT
public:
  ViewManagerImplementation(QObject *parent = nullptr) : QObject(parent) {}

  virtual ~ViewManagerImplementation() override {}
  virtual void setupUi(QWidget *parent) = 0;
  virtual void showView(tlp::View *view) = 0;
  virtual void drawAllViews() = 0;
  virtual QList<tlp::View *> panels() const = 0;
  virtual void saveToProject(tlp::TulipProject *project, QMap<tlp::Graph *, QString> rootIds,
                             tlp::PluginProgress *) = 0;
  virtual void restoreFromProject(tlp::TulipProject *project, QMap<QString, tlp::Graph *> rootIds,
                                  tlp::PluginProgress *) = 0;
  virtual void closeView(tlp::View *) = 0;
  virtual void closeAll() = 0;
signals:
  void showGraphRequest(tlp::Graph *);
  void currentViewActivated(tlp::View *);
};

/**
 * @brief The ViewManager class is the interface between the perspective and the
 * different kind of workspace
 */
class ViewManager : public QWidget {
  Q_OBJECT

  QHBoxLayout *_horizontalLayout;
  ViewManagerImplementation *_implementation;
  QWidget *_parentWidget;
  tlp::View *_currentView;

public:
  ViewManager(QWidget *parent = nullptr);
  ~ViewManager() override;

  void setImplementation(ViewManagerImplementation *implementation);
  /**
   * @brief createView create a new view
   * @param viewName
   * @param graph
   * @param parameters
   */
  tlp::View *createView(const std::string &viewName, tlp::Graph *graph,
                        const tlp::DataSet &parameters = tlp::DataSet());

  /**
   * @brief currentView return the currently activated view or nullptr if there
   * is no view.
   * @return
   */
  tlp::View *currentView() const;

  /**
   * @brief drawAllView
   */
  void drawAllViews();

  std::vector<tlp::View *> getViewsForGraph(tlp::Graph *) const;
  /**
   * @brief views return all the views.
   * @return
   */
  QList<tlp::View *> views() const;

  void closeView(tlp::View *view);

  void saveToProject(tlp::TulipProject *project, QMap<tlp::Graph *, QString> rootIds,
                     tlp::PluginProgress *);
  void restoreFromProject(tlp::TulipProject *project, QMap<QString, tlp::Graph *> rootIds,
                          tlp::PluginProgress *);

public slots:
  /**
   * @brief closeAllView close all the opened view
   */
  void closeAllView();
  /**
   * @brief closeProject close all the opened views and nullify the current view
   */
  void closeProject();

signals:
  void showGraphRequest(tlp::Graph *);
  void currentViewChanged(tlp::View *);

private slots:
  void currentViewChangedInternal(tlp::View *);
};

class MDIAreaImplementation : public ViewManagerImplementation {

  QMdiArea *_MdiArea;
  QHBoxLayout *_layout;

public:
  MDIAreaImplementation();
  ~MDIAreaImplementation() override;
  void setupUi(QWidget *parent) override;
  void showView(tlp::View *view) override;
  void drawAllViews() override;
  QList<tlp::View *> panels() const override {
    return QList<tlp::View *>();
  }
  void saveToProject(tlp::TulipProject *, QMap<tlp::Graph *, QString>,
                     tlp::PluginProgress *) override {}

  void restoreFromProject(tlp::TulipProject *, QMap<QString, tlp::Graph *>,
                          tlp::PluginProgress *) override {}
  void closeView(tlp::View *) override {}
  void closeAll() override {}
};

class WorkspacePanelImplementation : public ViewManagerImplementation {

  tlp::Workspace *_workspace;
  QHBoxLayout *_layout;
  tlp::GraphHierarchiesModel *_model;

public:
  WorkspacePanelImplementation();
  ~WorkspacePanelImplementation() override;
  void setupUi(QWidget *parent) override;
  void showView(tlp::View *view) override;
  void drawAllViews() override;
  QList<tlp::View *> panels() const override;
  void saveToProject(tlp::TulipProject *project, QMap<tlp::Graph *, QString> rootIds,
                     tlp::PluginProgress *) override;
  void restoreFromProject(tlp::TulipProject *project, QMap<QString, tlp::Graph *> rootIds,
                          tlp::PluginProgress *) override;
  void closeView(tlp::View *) override;
  void closeAll() override;
};

#endif // VIEWMANAGER_H
