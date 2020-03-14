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
#ifndef PORGYTRACEGRAPHVIEW_H
#define PORGYTRACEGRAPHVIEW_H

#include "abstracttraceview.h"

#include <portgraph/PorgyConstants.h>

#include <tulip/GlScene.h>

namespace tlp {
class Camera;
class GlRect;
class GlLayer;
}

class QGraphicsSceneDragDropEvent;
class QGraphicsRectItem;

class PorgyTraceGraphView : public AbstractTraceView {
  Q_OBJECT

  tlp::node dragNode;

  void updateEntities();
  /**
    * @brief Check if there is a meta node under the mouse cursor.
    * @return The node id if there is a valid meta node under the cursor.
    **/
  tlp::node pickMetaNodeUderCursor(const QPoint &cursorPosition) const;

  /**
   * @brief checkMetanodeUnderDragEvent Check if a metanode is under the
   * pointer, update the dragNode var and the highlight entity.
   * @param event
   * @return
   */
  bool checkMetanodeUnderDragEvent(QGraphicsSceneDragDropEvent *event);

  bool checkDragEventValidity(QGraphicsSceneDragDropEvent *event);
  tlp::QuickAccessBar *getQuickAccessBarImpl() override;
  QGraphicsRectItem *_dragOverItem;

public:
  PLUGININFORMATION(PorgyConstants::TRACE_VIEW_NAME, "B. Pinaud, J. Dubois", "04/01/2011", "",
                    "1.0", PorgyConstants::CATEGORY_NAME)
  PorgyTraceGraphView(const tlp::PluginContext *);
  bool eventFilter(QObject *object, QEvent *event) override;
  QList<QWidget *> configurationWidgets() const override;

signals:
  void applyRuleOnModel(tlp::Graph *model, tlp::Graph *rule, tlp::Graph *trace);
  void applyStrategyOnModel(tlp::Graph *model, QString strategy, tlp::Graph *);

protected slots:
  void redrawTraceGraph();
  void invertSelection();
  void applyColorMapping();
  void fillContextMenu(QMenu *contextMenu, const QPointF &position) override;

protected:
  void setupWidget() override;
//  void resetGraphicsScene() override;
//  bool rebuildSceneOnShowEvent() override {
//      return true;
//  }
  void initGlParameters(tlp::GlGraphRenderingParameters &parameters) const override;
  void graphChanged(tlp::Graph *) override;
};

#endif // PORGYTRACEGRAPHVIEW_H
