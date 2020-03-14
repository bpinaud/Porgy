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
#ifndef PORGYRULEGRAPHVIEW_H
#define PORGYRULEGRAPHVIEW_H

#include "abstractportgraphview.h"

#include <tulip/Coord.h>
#include <tulip/GlScene.h>

#include <portgraph/PorgyConstants.h>

class RuleViewConfigWidget;
class PortNodeQt;
class RuleAttributesWidget;
class RuleAlgorithmWidget;
class RuleConditionWidget;
class PorgyQuickAccessBar;

class QGraphicsSceneDragDropEvent;
class QDragMoveEvent;

class PorgyRuleGraphView : public AbstractPortGraphView {
  Q_OBJECT

  tlp::SelectedEntity selectedEntity;
  RuleAttributesWidget *_RuleAttributes;
  RuleAlgorithmWidget *_RuleAlgorithm;
  RuleConditionWidget *_RuleCondition;
  PorgyQuickAccessBar *_bar;

public:
  PLUGININFORMATION(PorgyConstants::RULE_VIEW_NAME, "B. Pinaud, J. Dubois", "04/01/2011", "", "1.0",
                    PorgyConstants::CATEGORY_NAME)

  PorgyRuleGraphView(const tlp::PluginContext *);
  ~PorgyRuleGraphView() override;
  tlp::DataSet state() const override;
  bool eventFilter(QObject *object, QEvent *event) override;
  void graphChanged(tlp::Graph *) override;
  bool _stateEdgeOrientation;
//  void resetGraphicsScene() override;
//  bool rebuildSceneOnShowEvent() override {
//      return true;
//  }

protected:
  void setupWidget() override;
  void fillContextMenu(QMenu *menu, const QPointF &) override;
  PortGraph *buildPortGraph(tlp::Graph *graph) const override;
  PortGraphDecorator *buildPortGraphDecorator(tlp::Graph *graph) const override;
  void initGlParameters(tlp::GlGraphRenderingParameters &parameters) const override;
  QList<QWidget *> configurationWidgets() const override;
  bool checkInteractors() override;
  tlp::QuickAccessBar *getQuickAccessBarImpl() override;

private slots:
  void pasteSelectionToLHS();
  void pasteSelectionToRHS();
  void selectRHS();
  void selectLHS();
  void invertSelection();
  void ViewArrows(bool state);
  void redrawRuleGraph();
  void filterGraphLayout(tlp::BooleanProperty*);

private:
  void selectSide(PorgyConstants::RuleSide);
  bool treatDragMoveEvent(QDragMoveEvent *evt);
  bool treatGraphicsDragMoveEvent(QGraphicsSceneDragDropEvent *evt);

signals:
  void copy_paste_other_side(const PorgyConstants::RuleSide side);
  void pastePortNodes(tlp::Coord &center, tlp::Graph *model, QList<PortNodeQt *> portNodes);
};

#endif // PORGYRULEGRAPHVIEW_H
