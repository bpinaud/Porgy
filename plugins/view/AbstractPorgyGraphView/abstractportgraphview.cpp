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
#include "abstractportgraphview.h"

#include <tulip/GlMainWidget.h>
#include <tulip/Interactor.h>
#include <tulip/TlpQtTools.h>

#include <portgraph/PortGraphModel.h>
#include <portgraph/PortNode.h>

#include <porgy/pluginparametersconfigurationdialog.h>

#include <QGraphicsView>
#include <QMessageBox>

using namespace tlp;
using namespace std;

AbstractPortGraphView::AbstractPortGraphView():AbstractPorgyGraphView(true) {}

AbstractPortGraphView::~AbstractPortGraphView() {}

void AbstractPortGraphView::setupWidget() {
  AbstractPorgyGraphView::setupWidget();
  setOverviewVisible(false);
}

void AbstractPortGraphView::redrawGraph(Graph *g) {
  if (g == nullptr)
    g = graph();
  PortGraphDecorator *dec = buildPortGraphDecorator(g);
  list<string> plugins = dec->layoutPluginsList();
  if (plugins.empty()) {
    tlp::warning() << "No layout plugin for the graph " << graph()->getName() << endl;
    return;
  } else {
    string pluginName;
    DataSet parameters;
    bool ok = false;
    if (plugins.size() == 1) {
      pluginName = plugins.front();
      parameters = PluginParametersConfigurationDialog::getParameters(
          graphicsView()->window(), "Configure draw plugin",
          tlp::PluginLister::instance()->getPluginParameters(pluginName), ok, nullptr, g);
    } else {
      assert(false);
    }

    Observable::holdObservers();
    if (!pluginName.empty() && ok) { // If the plugin is valid and the user
                                     // doesn't click on cancel button.
      string errMsg;
      graph()->getRoot()->push(false);
      if (dec->redraw(pluginName, parameters, errMsg)) {
        Observable::unholdObservers();
        dec->popIfNoUpdates();
        centerView();
      } else {
        QMessageBox::critical(this->getGlMainWidget()->parentWidget(), "Cannot redraw graph",
                              trUtf8("Error when drawing graph ") +
                                  tlp::tlpStringToQString(graph()->getName()) + " : " +
                                  tlp::tlpStringToQString(errMsg));
        dec->popIfNoUpdates();
        Observable::unholdObservers();
      }
    }
  }
  delete dec;
}

void AbstractPortGraphView::invertSelection(SelectedEntity &selectedentity) {
  Graph *g = graph();
  PortGraphDecorator *dec = buildPortGraphDecorator(g);
  PortGraph *pg = buildPortGraph(g);
  BooleanProperty *elementSelected = dec->getSelectionProperty();
  // selection add/remove graph item
  node n(selectedentity.getNode());
  if (n.isValid()) {
    (*pg)[n]->select(!elementSelected->getNodeValue(n), elementSelected);
  } else {
    edge e(selectedentity.getEdge());
    elementSelected->setEdgeValue(e, !elementSelected->getEdgeValue(e));
  }
  delete dec;
  delete pg;
}

void AbstractPortGraphView::graphChanged(Graph *g) {
  AbstractPorgyGraphView::graphChanged(g);
  if (checkInteractors())
    emit(interactorsChanged());
}

void AbstractPortGraphView::setInteractors(const QList<tlp::Interactor *> &l) {
  AbstractPorgyGraphView::setInteractors(l);
  checkInteractors();
}
