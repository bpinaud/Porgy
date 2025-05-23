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
#ifndef ADD_PN_H_
#define ADD_PN_H_

#include <tulip/NodeLinkDiagramComponentInteractor.h>

#include <portgraph/PorgyConstants.h>

class AddPortNodeConfigurationWidget;

class AddPortNode : public tlp::NodeLinkDiagramComponentInteractor {

  AddPortNodeConfigurationWidget *_configurationWidget;

public:
  PLUGININFORMATION("AddPortNode", "Nicolas Moreaud", "19/07/2010", "Add portnode interactor",
                    "1.0", PorgyConstants::CATEGORY_NAME)
  AddPortNode(const tlp::PluginContext *);
  ~AddPortNode() override;
  QCursor cursor() const  override;
  void construct() override;
  QWidget *configurationOptionsWidget() const override;
  bool isCompatible(const std::string &) const override;
};

#endif
