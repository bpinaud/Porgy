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
#ifndef PORGYSMALLMULTIPLESTRACEVIEW_H
#define PORGYSMALLMULTIPLESTRACEVIEW_H

#include "abstracttraceview.h"
#include "graphstatemanager.h"

#include <portgraph/PorgyConstants.h>

#include <tulip/GlMainWidget.h>

class IntermediateStatesQuickAccessBar;
class PorgySMConfigurationWidget;

namespace tlp {
class GlScene;
class QuickAccessBar;
}

class PorgySMView : public AbstractTraceView {
  Q_OBJECT

  PorgySMConfigurationWidget *_configurationWidget;
  IntermediateStatesQuickAccessBar *_bar;

  tlp::Graph *_smallMultipleGraph;    // The graph to handle small multiples elements
  std::vector<std::string> _textures; // The texture createed for the smallmultiples
  GraphStateManager _states;          // The states

  enum SmallMultipleUpdateType {
    Update_Name = 0x1,
    Update_texture = 0x2, // Update the preview of the element use it after a graph changed
    Update_Layout = 0x4,  // Update the layout of the element. Call after
                          // changing the spacing or the number of row
    Reset = 0xF           // Reset all elements update the number of nodes and regenerate
                          // all the visual properties. Call after updating states list
  };
  Q_DECLARE_FLAGS(SmallMultipleUpdateTypeFlags, SmallMultipleUpdateType)

public:
  PLUGININFORMATION(PorgyConstants::SMALL_MULTIPLE_VIEW_NAME, "Jonathan Dubois", "17/01/2011", "",
                    "1.0", PorgyConstants::CATEGORY_NAME)
  PorgySMView(const tlp::PluginContext *);
  ~PorgySMView() override;
  std::list<QWidget *> configurationWidgets() const override;

  tlp::DataSet state() const override;
  tlp::QuickAccessBar *getQuickAccessBarImpl() override;
  void setState(const tlp::DataSet &) override;

  void setupWidget() override;
  void treatEvent(const tlp::Event &ev) override;

protected slots:
  void graphChanged(tlp::Graph *) override;

private:
  void fillContextMenu(QMenu *contextMenu, const QPointF &) override;

  std::string getTextureNameForId(int stateId);
  std::string generateTexture(unsigned int stateId);
  void clearTextures();

private slots:
  /**
        * @brief Force to redraw the layout of the small multiples.
        **/
  void updateSmallMultipleLayout();
  void updateTexture();

  /**
        * @brief Force to regenerate intermediary states
        **/
  void updateIntermediateStates();

  /**
       * @brief updateSmallMultiples updates the small multiple graph and
   * texture with the content of the _state attribute.
       */
  void updateSmallMultiples(SmallMultipleUpdateTypeFlags updateType);
};

#endif // PORGYSMALLMULTIPLESTRACEVIEW_H
