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
#include "addPortNodeConfigurationWidget.h"
#include "createPortNode.h"
#include "ui_AddPortNodeConfigurationWidget.h"

#include <porgy/PortNodeQt.h>
#include <porgy/PortQt.h>

#include <tulip/Perspective.h>

#include <QMainWindow>

using namespace tlp;

AddPortNodeConfigurationWidget::AddPortNodeConfigurationWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::AddPortNodeConfigurationWidget), _currentGraph(nullptr),
      _currentModel(nullptr) {
  ui->setupUi(this);
  connect(ui->createPnButton, SIGNAL(clicked()), SLOT(createNewPortNode()));
  connect(ui->portNodeListView, SIGNAL(doubleClicked(const QModelIndex &)),
          SLOT(editPortNode(const QModelIndex &)));
}

AddPortNodeConfigurationWidget::~AddPortNodeConfigurationWidget() {
  delete ui;
}

bool AddPortNodeConfigurationWidget::IsPortNodeSelected() const {
  assert(_currentModel != nullptr);
  return ui->portNodeListView->selectionModel()->hasSelection();
}

bool AddPortNodeConfigurationWidget::isCurrentGraph(tlp::Graph *graph) {
  return graph != nullptr ? graph->getRoot() == _currentGraph : graph == _currentGraph;
}

void AddPortNodeConfigurationWidget::loadPortNodesFromGraph(tlp::Graph *graph) {
  if (graph != nullptr) {
    // Use the root graph as key
    graph = graph->getRoot();
  }
  // If we display the same hierarchy don't update the list.
  if (graph != _currentGraph) {
    _currentGraph = graph;
    auto it = _models.find(graph);
    // Search if there is a model for this root
    if (it == _models.end()) {
      // No model create new one.
      _currentModel = new PortNodeQtModel(this);
      _models[graph] = _currentModel;
      // Listen graph to capt its destruction
      graph->addObserver(this);
      // Init port node list.
      _currentModel->initPortNodesList(graph);
    } else {
      // Reload previous model
      _currentModel = it.value();
    }
    // Change the view model.
    ui->portNodeListView->setModel(_currentModel);
  }
}

PortNodeQt *AddPortNodeConfigurationWidget::getSelectedPortNode() const {
  assert(_currentModel != nullptr);
  QModelIndexList selectedItems = ui->portNodeListView->selectionModel()->selectedIndexes();
  return selectedItems.isEmpty() ? nullptr
                                 : _currentModel->portNodeFromIndex(selectedItems.front());
}

void AddPortNodeConfigurationWidget::createNewPortNode() {
  assert(_currentModel != nullptr);
  CreatePortNode dialog(Perspective::instance()->mainWindow());
  if (dialog.exec()) {
    _currentModel->addPortNode(
        PortNodeQt(dialog.getName(), dialog.getColor(), dialog.getPortQts()));
  }
}

void AddPortNodeConfigurationWidget::editPortNode(const QModelIndex &index) {
  assert(_currentModel != nullptr);
  PortNodeQt *portNode = _currentModel->portNodeFromIndex(index);
  if (portNode != nullptr) {
    CreatePortNode dialog(Perspective::instance()->mainWindow(), *portNode);
    if (dialog.exec()) {
      portNode->setName(dialog.getName());
      portNode->setColor(dialog.getColor());
      portNode->replacePorts(dialog.getPortQts());
    }
  }
}

void AddPortNodeConfigurationWidget::treatEvents(const std::vector<tlp::Event> &events) {
  for (auto it : events) {
    if (it.type() == tlp::Event::TLP_DELETE) {
      // A graph is destructed
      tlp::Graph *rootGraph = dynamic_cast<tlp::Graph *>(it.sender());

      // The current graph is destructed remove the model from the list
      if (_currentGraph == rootGraph) {
        ui->portNodeListView->setModel(nullptr);
      }
      // Erase the model corresponding to the graph.
      if (rootGraph) {
        auto itModel = _models.find(rootGraph);
        if (itModel != _models.end()) {
          itModel.value()->deleteLater();
          _models.erase(itModel);
        }
      }
    }
  }
}
