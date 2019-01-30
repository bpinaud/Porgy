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
#include <QInputDialog>
#include <QMouseEvent>

#include <tulip/Camera.h>
#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>

#include "MouseAddPortNode.h"
#include "addPortNode.h"
#include "addPortNodeConfigurationWidget.h"

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortGraphRule.h>

using namespace tlp;
using namespace std;

MouseAddPortNode::MouseAddPortNode(AddPortNode *interactor) : interactor(interactor) {}

MouseAddPortNode::MouseAddPortNode(const MouseAddPortNode &mouseaddportnode)
    : GLInteractorComponent(), interactor(mouseaddportnode.interactor) {}

bool MouseAddPortNode::eventFilter(QObject *widget, QEvent *e) {
  QMouseEvent *qMouseEv = static_cast<QMouseEvent *>(e);
  GlMainWidget *glw = static_cast<GlMainWidget *>(widget);
  SelectedEntity selectedEntity;
  if (qMouseEv != nullptr) {
    if (e->type() == QEvent::MouseMove) {
      if (glw->pickNodesEdges(qMouseEv->x(), qMouseEv->y(), selectedEntity, nullptr, true, false) &&
          selectedEntity.getEntityType() == SelectedEntity::NODE_SELECTED) {
        glw->setCursor(Qt::ForbiddenCursor);
      } else {
        glw->setCursor(Qt::ArrowCursor);
      }

      return false;
    }

    if ((e->type() == QEvent::MouseButtonPress) && (qMouseEv->button() == Qt::LeftButton)) {
      if (glw->pickNodesEdges(qMouseEv->x(), qMouseEv->y(), selectedEntity, nullptr, true, false) &&
          selectedEntity.getEntityType() == SelectedEntity::NODE_SELECTED) {
        return true;
      }
      AddPortNodeConfigurationWidget *config =
          static_cast<AddPortNodeConfigurationWidget *>(interactor->configurationWidget());
      if (config->isEnabled() && config->IsPortNodeSelected()) {
        Graph *_graph = glw->getScene()->getGlGraphComposite()->getInputData()->getGraph();
        unsigned int numberOfNodesToAdd = 1;
        if (qMouseEv->modifiers().testFlag(Qt::ControlModifier)) {
          bool ok = false;
          numberOfNodesToAdd =
              QInputDialog::getInt(glw, tr("Insert portnodes"),
                                   tr("Number of portnodes to insert:"), 1, 1, 2147483647, 1, &ok);
          // User cancel return
          if (!ok) {
            return false;
          }
        }

        PortNodeQt *pn = config->getSelectedPortNode();
        if (pn == nullptr)
          return true;

        _graph->push(); // allow to undo
        Observable::holdObservers();
        Coord posCenter(static_cast<double>(glw->width()) - static_cast<double>(qMouseEv->x()),
                        static_cast<double>(qMouseEv->y()), 0);
        posCenter =
            glw->getScene()->getGraphCamera().viewportTo3DWorld(glw->screenToViewport(posCenter));

        unsigned int elementsByRow = ceil(sqrt(numberOfNodesToAdd));

        // Need to compute the node size
        Size defaultSize(0, 0, 0);
        Coord startPos = posCenter;
        SizeProperty *nodeSize = _graph->getProperty<SizeProperty>("viewSize");
        LayoutProperty *layout = _graph->getProperty<LayoutProperty>("viewLayout");
        ColorProperty *color = _graph->getProperty<ColorProperty>("viewColor");
        ColorProperty *bordercolor = _graph->getProperty<ColorProperty>("viewBorderColor");

        for (unsigned int i = 0; i < numberOfNodesToAdd; ++i) {
          node newNode = pn->addToTlpGraphUsingPosition(_graph, posCenter);

          if (defaultSize[0] == 0 && defaultSize[1] == 0 && defaultSize[2] == 0) {
            defaultSize = nodeSize->getNodeValue(newNode);
          }

          int column = i % elementsByRow;
          int row = floor(static_cast<float>(i) / static_cast<float>(elementsByRow));
          Coord c = startPos + Coord(column * defaultSize[0], row * defaultSize[1]);
          layout->setNodeValue(newNode, c);

          if (PorgyTlpGraphStructure::isRuleGraph(_graph)) {
            PortGraphRule pg(_graph);
            pg[newNode]->draw(nodeSize, color, layout, bordercolor);
          } else {
            PortGraphModel pg(_graph);
            pg[newNode]->draw(nodeSize, color, layout, bordercolor);
          }
        }

        Observable::unholdObservers();
        return true;
      }
    }
  }
  return false;
}
