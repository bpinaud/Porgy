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
#include <QVBoxLayout>

#include <tulip/GraphHierarchiesModel.h>
#include <tulip/PluginLister.h>
#include <tulip/View.h>
#include <tulip/Workspace.h>

using namespace tlp;

ViewManager::ViewManager(QWidget *parent)
    : QWidget(parent), _layout(new QVBoxLayout(this)),
      _model(new GraphHierarchiesModel(this)),_currentView(nullptr) {}

ViewManager::~ViewManager() {

}

tlp::View *ViewManager::createView(const std::string &viewName, tlp::Graph *graph,
                                   const tlp::DataSet &parameters) {

  tlp::View *view = tlp::PluginLister::getPluginObject<tlp::View>(viewName, nullptr);
  if (view != nullptr) {
    view->setupUi();
    view->setGraph(graph);
    view->setState(parameters);
    if (!_model->indexOf(view->graph()).isValid())
      _model->addGraph(view->graph());
    _workspace->addPanel(view);
    _workspace->setActivePanel(view);
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

std::vector<tlp::View *> ViewManager::getViewsForGraph(tlp::Graph *graph) const {
  std::vector<tlp::View *> result;
  for (tlp::View *view : _workspace->panels()) {
    if (view->graph() == graph) {
      result.push_back(view);
    }
  }
  return result;
}

void ViewManager::closeProject() {
  _workspace->closeAll();
  _currentView = nullptr;
}

void ViewManager::setupUi() {
  _workspace = new tlp::Workspace(parentWidget());
  _layout->addWidget(_workspace);

  _workspace->setModel(_model);
  _workspace->setAutoCenterPanelsOnDraw(false);

//  connect(_workspace, SIGNAL(addPanelRequest(tlp::Graph *)), SIGNAL(showGraphRequest(tlp::Graph *)));
  connect(_workspace, SIGNAL(panelFocused(tlp::View *)), SIGNAL(currentViewActivated(tlp::View *)));
 // connect(_workspace, SIGNAL(showGraphRequest(tlp::Graph *)), SIGNAL(showGraphRequest(tlp::Graph *)));
 // connect(_workspace, SIGNAL(currentViewActivated(tlp::View *)), SLOT(currentViewChangedInternal(tlp::View *)));
}
