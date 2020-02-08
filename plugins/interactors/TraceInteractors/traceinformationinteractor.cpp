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
#include "traceinformationinteractor.h"
#include "../../StandardInteractorPriority.h"
#include "TraceInformationConfigWidget.h"
#include "traceinformationgraphcomponent.h"

#include <portgraph/PorgyConstants.h>

#include <porgy/MetaNodeZoomInteractorComponent.h>
#include <porgy/elementinformationinteractorcomponent.h>

#include <tulip/GlMainWidget.h>
#include <tulip/MouseInteractors.h>

using namespace tlp;
using namespace std;

TraceInformationInteractor::TraceInformationInteractor(const PluginContext *)
    : NodeLinkDiagramComponentInteractor(":/tulip/gui/icons/i_select.png", "Get information",StandardInteractorPriority::GetInformation),
      traceInformation(nullptr), configWidget(nullptr) {
}

TraceInformationInteractor::~TraceInformationInteractor() {
  delete configWidget;
}

bool TraceInformationInteractor::isCompatible(const string &viewName) const {
  return viewName == PorgyConstants::TRACE_VIEW_NAME;
}

QWidget *TraceInformationInteractor::configurationWidget() const {
  return configWidget;
}

void TraceInformationInteractor::construct() {
  configWidget = new TraceInformationConfigWidget();
  traceInformation = new TraceInformationGraphComponent(this);
  push_back(new MouseNKeysNavigator(false));
  push_back(new MetaNodeZoomInteractorComponent);
  push_back(new ElementInformationInteractorComponent());
  push_back(traceInformation);

}

void TraceInformationInteractor::install(QObject *target) {
  if (target != nullptr) {
    NodeLinkDiagramComponentInteractor::install(target);
    traceInformation->setGlMainWidget(qobject_cast<tlp::GlMainWidget *>(target));
  } else { // Install(nullptr) is called by the uninstall function
    traceInformation->restoreOriginalVisualProperties();
  }
}

PLUGIN(TraceInformationInteractor)
