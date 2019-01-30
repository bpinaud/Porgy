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
#ifndef NEWTRACE_H
#define NEWTRACE_H

#include <QDialog>

#include <tulip/Node.h>

namespace Ui {
class NewTrace;
}

namespace tlp {
class Graph;
class BooleanProperty;
}

class QWidget;

class NewTrace : public QDialog {
  Q_OBJECT
  NewTrace(QWidget *parent, tlp::Graph *graph, tlp::BooleanProperty *sel);
  tlp::Graph *buildNewTrace(bool createNewGraph, tlp::BooleanProperty *prop);
  void fillPortNodesForGraph(tlp::Graph *g, QStringList &list);

  void setActualTraceName();

  tlp::Graph *graph;
  Ui::NewTrace *ui;
  tlp::BooleanProperty *sel;

  void fillPortNodesList();
  void fillPropertyCombobox();
  void fillPropertyRatioCombobox();

public:
  ~NewTrace() override;
  static tlp::Graph *createNewTrace(tlp::BooleanProperty *sel, tlp::Graph *parentGraph,
                                    QWidget *parent);
  static tlp::Graph *createNewTrace(tlp::Graph *tracemain, tlp::node root, QWidget *parent);
  static tlp::Graph *computeOnTrace(tlp::BooleanProperty *sel, tlp::Graph *parentGraph,
                                    QWidget *parent);

private slots:
  void property_ratio_radio();
  void property_histogram_radio();
  void histogram_radio();
  void standard_radio();

protected slots:
  void accept() override;

signals:
  void addNewTraceView();
  void addGraph();
};

#endif
