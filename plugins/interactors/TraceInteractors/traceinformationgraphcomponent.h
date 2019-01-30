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
#ifndef TRACEINFORMATIONGRAPHCOMPONENT_H
#define TRACEINFORMATIONGRAPHCOMPONENT_H

#include <tulip/Edge.h>
#include <tulip/GLInteractor.h>
#include <tulip/Observable.h>

#include <porgy/GraphVisualProperties.h>

#include <map>

namespace tlp {
class GlMainView;
class Graph;
class BooleanProperty;
class Color;
class GlMetaNodeRenderer;
}

class TraceInformationMetanodeRenderer;
class TraceInformationInteractor;

class TraceInformationGraphComponent : public tlp::GLInteractorComponent, public tlp::Observable {
  Q_OBJECT
public:
  TraceInformationGraphComponent(TraceInformationInteractor *interactor);
  TraceInformationGraphComponent(const TraceInformationGraphComponent &trace);
  bool eventFilter(QObject *, QEvent *event) override;
  void setGlMainWidget(tlp::GlMainWidget *widget);

  void restoreOriginalVisualProperties();

private:
  void updateColors(tlp::edge newEdge, tlp::edge oldEdge);
  std::list<std::string> strategyCode(const tlp::node to) const;
  void clearOverloadedProperties();

  tlp::GlMainWidget *_glMainWidget;
  tlp::GlMetaNodeRenderer *_originalRenderer;
  TraceInformationMetanodeRenderer *_interactorComponentRenderer;
  tlp::edge _currentTransformation;
  TraceInformationInteractor *parentInteractor;

  GraphVisualProperties origModelVisualProperties, destModelVisualProperties;
};

#endif // TRACEINFORMATIONGRAPHCOMPONENT_H
