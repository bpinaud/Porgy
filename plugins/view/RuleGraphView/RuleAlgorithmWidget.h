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
#ifndef RULEALGORITHMWIDGET_H
#define RULEALGORITHMWIDGET_H

#include <QWidget>

namespace Ui {
class RuleAlgorithm;
}

namespace tlp {
class Graph;
}

class RuleAlgorithmWidget : public QWidget {
  Q_OBJECT

  Ui::RuleAlgorithm *_ui;
  tlp::Graph *_g;

public:
  RuleAlgorithmWidget(QWidget *parent = nullptr);
  ~RuleAlgorithmWidget() override;

  void setRuleAlgorithm(tlp::Graph *g);

protected:
  bool eventFilter(QObject *object, QEvent *event) override;

private slots:
  void tryAlgorithm();
};

#endif // RULEALGORITHMWIDGET_H
