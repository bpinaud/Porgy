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
#include "addPortNode.h"
#include "InteractorPortGraphAddEdgeComponent.h"
#include "MouseAddPortNode.h"
#include "addPortNodeConfigurationWidget.h"

#include "../../StandardInteractorPriority.h"

#include <tulip/MouseInteractors.h>
#include <tulip/View.h>

#include <QMessageBox>

using namespace tlp;
using namespace std;

bool AddPortNode::isCompatible(const string &viewName) const {
  return ((viewName == PorgyConstants::GRAPH_VIEW_NAME) ||
          (viewName == PorgyConstants::RULE_VIEW_NAME));
}

AddPortNode::AddPortNode(const tlp::PluginContext *)
    : NodeLinkDiagramComponentInteractor(":/addPortNode.png",
                                         "Add a portnode/an edge between two portnodes",StandardInteractorPriority::AddNodesOrEdges),
      _configurationWidget(nullptr) {
}

AddPortNode::~AddPortNode() {
  delete _configurationWidget;
}

QCursor AddPortNode::cursor() const {
  return QCursor(Qt::PointingHandCursor);
}

void AddPortNode::construct() {
  _configurationWidget = new AddPortNodeConfigurationWidget();
  push_front(new MousePanNZoomNavigator);
  push_front(new PortGraphAddEdgeComponent);
  push_front(new MouseAddPortNode(this));
  if ((view() != nullptr) && (_configurationWidget->isEnabled())) {
    _configurationWidget->loadPortNodesFromGraph(view()->graph());
  }
}
QWidget *AddPortNode::configurationWidget() const {
  if (_configurationWidget->isEnabled()) {
    if (!_configurationWidget->isCurrentGraph(view()->graph())) {
      _configurationWidget->loadPortNodesFromGraph(view()->graph());
    }
  }
  return _configurationWidget;
}

PLUGIN(AddPortNode)
