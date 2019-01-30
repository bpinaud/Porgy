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
#ifndef ABSTRACTGRAPHVIEW_H
#define ABSTRACTGRAPHVIEW_H

#include <tulip/GlGraphRenderingParameters.h>
#include <tulip/GlMainView.h>

namespace tlp {
class GlMainWidget;
class GlOverviewGraphicsItem;
}
/**
 * @brief The AbstractPorgyGraphView class is a base class for porgy
 * graph/rule/trace view. It's an extended version of the GlMainView.
 *
 *  Perform all the graph display routine for subclasses like graph display,
 * listening of graph modifications and some advanced actions like animated
 * center view.
 */
class AbstractPorgyGraphView : public tlp::GlMainView {
  Q_OBJECT

public:
  AbstractPorgyGraphView(const bool activateToolip);
  ~AbstractPorgyGraphView() override;

  /**
   * @brief state return the state of the glMainWidget
   * @return
   */
  tlp::DataSet state() const override;
  /**
   * @brief setState reinitialize a previous glMainWidget state.
   * @param parameters
   */
  void setState(const tlp::DataSet &parameters) override;

protected:
  void initScene(tlp::Graph *graph, const tlp::DataSet dataSet);

  /**
   * @brief graphChanged update the visual parameters when the graph changed.
   */
  void graphChanged(tlp::Graph *) override;

  /**
   * @brief glParameters initialize the gl parameters for the view.
   * @return
   */
  virtual void initGlParameters(tlp::GlGraphRenderingParameters &) const {}

  /**
   * @brief initObservers registers all the observers for this view. By default
   * registers all the visual properties.
   * This function is called at the end of the graphChanged function
   */
  virtual void initObservers();
  /**
   * @brief clearObservers unregisters all the observers for this view. By
   * default unregisters all the visual properties.
   * This function is called at the beginning of the graphChanged function
   */
  virtual void clearObservers();

protected slots:
  void centerView(bool graphChanged = false) override;
};

#endif // ABSTRACTGRAPHVIEW_H
