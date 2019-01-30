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
#include "viewmanager.h"

#include <QGraphicsView>
#include <QHBoxLayout>
#include <QMdiArea>
#include <QMdiSubWindow>

#include <tulip/GraphHierarchiesModel.h>
#include <tulip/PluginLister.h>
#include <tulip/View.h>
#include <tulip/Workspace.h>

using namespace tlp;

ViewManager::ViewManager(QWidget *parent)
    : QWidget(parent), _horizontalLayout(new QHBoxLayout(this)), _implementation(nullptr),
      _parentWidget(nullptr), _currentView(nullptr) {}

ViewManager::~ViewManager() {}

void ViewManager::setImplementation(ViewManagerImplementation *implementation) {
  ViewManagerImplementation *oldImpl = _implementation;
  QWidget *oldParentWidget = _parentWidget;
  if (_parentWidget != nullptr) {
    _parentWidget->setVisible(false);
    layout()->removeWidget(_parentWidget);
  }

  _implementation = implementation;
  _parentWidget = nullptr;
  if (_implementation != nullptr) {
    _parentWidget = new QWidget(this);
    _horizontalLayout->addWidget(_parentWidget);
    _implementation->setupUi(_parentWidget);
    _parentWidget->setVisible(true);
    connect(_implementation, SIGNAL(showGraphRequest(tlp::Graph *)),
            SIGNAL(showGraphRequest(tlp::Graph *)));
    connect(_implementation, SIGNAL(currentViewActivated(tlp::View *)),
            SLOT(currentViewChangedInternal(tlp::View *)));
  }

  delete oldImpl;
  delete oldParentWidget;
}

tlp::View *ViewManager::createView(const std::string &viewName, tlp::Graph *graph,
                                   const tlp::DataSet &parameters) {

  tlp::View *view = tlp::PluginLister::instance()->getPluginObject<tlp::View>(viewName, nullptr);
  if (view != nullptr) {
    view->setupUi();
    view->setGraph(graph);
    view->setState(parameters);
    _implementation->showView(view);
  } else {
    tlp::warning() << "Cannot create the view: \"" << viewName << "\"" << std::endl;
  }
  return view;
}
tlp::View *ViewManager::currentView() const {
  return _currentView;
}

void ViewManager::currentViewChangedInternal(tlp::View *view) {
  _currentView = view;
  emit currentViewChanged(_currentView);
}

void ViewManager::drawAllViews() {
  _implementation->drawAllViews();
}

std::vector<tlp::View *> ViewManager::getViewsForGraph(tlp::Graph *graph) const {
  std::vector<tlp::View *> result;
  for (tlp::View *view : _implementation->panels()) {
    if (view->graph() == graph) {
      result.push_back(view);
    }
  }
  return result;
}

QList<tlp::View *> ViewManager::views() const {
  return _implementation->panels();
}

void ViewManager::saveToProject(tlp::TulipProject *project, QMap<tlp::Graph *, QString> rootIds,
                                tlp::PluginProgress *progress) {
  _implementation->saveToProject(project, rootIds, progress);
}

void ViewManager::restoreFromProject(tlp::TulipProject *project, QMap<QString, Graph *> rootIds,
                                     tlp::PluginProgress *progress) {
  _implementation->restoreFromProject(project, rootIds, progress);
}

void ViewManager::closeView(tlp::View *view) {
  _implementation->closeView(view);
}

void ViewManager::closeAllView() {
  _implementation->closeAll();
}

void ViewManager::closeProject() {
  closeAllView();
  _currentView = nullptr;
}

MDIAreaImplementation::MDIAreaImplementation() : _MdiArea(nullptr) {}

MDIAreaImplementation::~MDIAreaImplementation() {}

void MDIAreaImplementation::setupUi(QWidget *parent) {
  _MdiArea = new QMdiArea(parent);
  _layout = new QHBoxLayout(parent);
  _layout->addWidget(_MdiArea);
}

void MDIAreaImplementation::showView(tlp::View *view) {
  QMdiSubWindow *subWindow1 = new QMdiSubWindow(_MdiArea, Qt::WindowFlags(Qt::WA_NativeWindow));
  view->graphicsView()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  view->graphicsView()->setCacheMode(QGraphicsView::CacheBackground);
  subWindow1->setWidget(view->graphicsView());
  subWindow1->setAttribute(Qt::WA_DeleteOnClose);
  subWindow1->setOption(QMdiSubWindow::RubberBandResize, true);
  subWindow1->setOption(QMdiSubWindow::RubberBandMove, true);
  _MdiArea->addSubWindow(subWindow1);
  subWindow1->setVisible(true);
}

void MDIAreaImplementation::drawAllViews() {}

WorkspacePanelImplementation::WorkspacePanelImplementation()
    : _workspace(nullptr), _layout(nullptr), _model(new tlp::GraphHierarchiesModel(this)) {}

WorkspacePanelImplementation::~WorkspacePanelImplementation() {
  delete _workspace;
  // delete _model; //not necessary. _model has a parent
}

void WorkspacePanelImplementation::setupUi(QWidget *parent) {
  _workspace = new tlp::Workspace(parent);
  _layout = new QHBoxLayout(parent);
  _layout->addWidget(_workspace);

  _workspace->setModel(_model);
  _workspace->setAutoCenterPanelsOnDraw(false);

  connect(_workspace, SIGNAL(addPanelRequest(tlp::Graph *)),
          SIGNAL(showGraphRequest(tlp::Graph *)));
  connect(_workspace, SIGNAL(panelFocused(tlp::View *)), SIGNAL(currentViewActivated(tlp::View *)));
}

void WorkspacePanelImplementation::showView(tlp::View *view) {
  if (!_model->indexOf(view->graph()).isValid())
    _model->addGraph(view->graph());
  _workspace->addPanel(view);
}

void WorkspacePanelImplementation::drawAllViews() {
  _workspace->redrawPanels();
}

QList<tlp::View *> WorkspacePanelImplementation::panels() const {
  return _workspace->panels();
}

void WorkspacePanelImplementation::saveToProject(tlp::TulipProject *p,
                                                 QMap<Graph *, QString> rootIds,
                                                 tlp::PluginProgress *pp) {
  _workspace->writeProject(p, rootIds, pp);
}

void WorkspacePanelImplementation::restoreFromProject(tlp::TulipProject *p,
                                                      QMap<QString, Graph *> rootIds,
                                                      tlp::PluginProgress *pp) {
  _workspace->readProject(p, rootIds, pp);
}

void WorkspacePanelImplementation::closeView(tlp::View *view) {
  _workspace->delView(view);
}

void WorkspacePanelImplementation::closeAll() {
  _workspace->closeAll();
}
