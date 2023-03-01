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
#ifndef ABSTRACTPORTGRAPHVIEW_H
#define ABSTRACTPORTGRAPHVIEW_H

#include "AbstractPorgyGraphView.h"

#include <portgraph/PortGraph.h>

#include <tulip/GlGraphRenderingParameters.h>
#include <tulip/Graph.h>

class PortGraph;

namespace tlp {
struct SelectedEntity;
}

/**
 * @brief The AbstractPortGraphView class is an abstract class to handle view
 * displaying port graphs. Contains the code common to Graph and Rule view.
 */
class AbstractPortGraphView : public AbstractPorgyGraphView {
  Q_OBJECT

public:
  AbstractPortGraphView();
  ~AbstractPortGraphView() override;

protected:
  void setInteractors(const std::list<tlp::Interactor *> &) override;
  void graphChanged(tlp::Graph *) override;
  void setupWidget() override;
  virtual bool checkInteractors() = 0;
  /**
   * @brief buildPortGraph constructs the PortGraph object corresponding to the
   * kind of port graph the view contains.
   * @param graph
   * @return
   */
  virtual PortGraph *buildPortGraph(tlp::Graph *graph) const = 0;
  virtual PortGraphDecorator *buildPortGraphDecorator(tlp::Graph *graph) const = 0;

protected slots:
  void redrawGraph(tlp::Graph *g = nullptr);
  void invertSelection(tlp::SelectedEntity &selectedentity);
};

#endif // ABSTRACTPORTGRAPHVIEW_H
