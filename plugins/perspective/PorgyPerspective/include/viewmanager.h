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
#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <tulip/DataSet.h>

#include <QWidget>
#include "porgyperspective.h"

namespace tlp {
class Graph;
class View;
class GraphHierarchiesModel;
class Workspace;
}

class QVBoxLayout;
class QLabel;


/**
 * @brief The ViewManager class is the interface between the perspective and the
 * different kind of workspace
 */
class ViewManager : public QWidget {
  Q_OBJECT

  friend PorgyPerspective;

  tlp::Workspace *_workspace;
  QVBoxLayout *_layout;
  tlp::GraphHierarchiesModel *_model;

  tlp::View *_currentView;

public:
  ViewManager(QWidget *parent = nullptr);
  ~ViewManager() override;
  void setupUi();
  /**
   * @brief createView create a new view
   * @param viewName
   * @param graph
   * @param parameters
   */
  tlp::View *createView(const std::string &viewName, tlp::Graph *graph,
                        const tlp::DataSet &parameters = tlp::DataSet());

  /**
   * @brief currentView return the currently activated view or nullptr if there
   * is no view.
   * @return
   */
  tlp::View *currentView() const;

  std::vector<tlp::View *> getViewsForGraph(tlp::Graph *) const;

public slots:
  /**
   * @brief closeProject close all the opened views and nullify the current view
   */
  void closeProject();

signals:
  //void showGraphRequest(tlp::Graph *);
  void currentViewChanged(tlp::View *);
  void currentViewActivated(tlp::View *);

private slots:
  void currentViewChangedInternal(tlp::View *);
};

#endif // VIEWMANAGER_H
