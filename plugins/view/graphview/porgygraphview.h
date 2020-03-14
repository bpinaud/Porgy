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
#ifndef PORGYGRAPHVIEW_H
#define PORGYGRAPHVIEW_H

#include "abstractportgraphview.h"

#include <tulip/Coord.h>
#include <tulip/GlScene.h>

#include <QAction>

#include <portgraph/PorgyConstants.h>

class GraphViewConfigWidget;

namespace tlp {
class BooleanProperty;
class ColorProperty;
}
class PortNodeQt;
/**
 * @brief The PorgyGraphView class is dedicated to display PortGraphModel
 * objects.
 */
class PorgyGraphView : public AbstractPortGraphView {
  Q_OBJECT

  tlp::ColorProperty *elementsColor;
  tlp::ColorProperty *overloadedElementsColor;

  std::vector<tlp::BooleanProperty *> availableSelectionProperties();

  tlp::SelectedEntity selectedEntity;

public:
  PLUGININFORMATION(PorgyConstants::GRAPH_VIEW_NAME, "Jonathan Dubois", "04/01/2011", "", "1.0",
                    PorgyConstants::CATEGORY_NAME)
  PorgyGraphView(const tlp::PluginContext *);
  ~PorgyGraphView() override;

  bool eventFilter(QObject *object, QEvent *event) override;

signals:
  void applyRuleOnModel(tlp::Graph *model, tlp::Graph *rule, tlp::Graph *);
  void applyStrategyOnModel(tlp::Graph *model, QString strategy, tlp::Graph *);
  void pastePortNodes(tlp::Coord &center, tlp::Graph *model, QList<PortNodeQt *> portNodes);

private slots:

  void invertSelection();

  void copySelectionInANewProperty();
  void copySelectionToProperty();
  void copyPropertyToSelection();

protected:
//  void resetGraphicsScene() override;
//  bool rebuildSceneOnShowEvent() override {
//      return true;
//  }
  void setupWidget() override;
  tlp::QuickAccessBar *getQuickAccessBarImpl() override;
  void fillContextMenu(QMenu *contextMenu, const QPointF &position) override;
  PortGraph *buildPortGraph(tlp::Graph *graph) const override;
  PortGraphDecorator *buildPortGraphDecorator(tlp::Graph *graph) const override;
  void initGlParameters(tlp::GlGraphRenderingParameters &parameters) const override;
  bool checkInteractors() override;
};

#endif // PORGYMODELGRAPHVIEW_H
