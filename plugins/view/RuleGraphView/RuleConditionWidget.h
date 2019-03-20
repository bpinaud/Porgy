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
#ifndef RULECONDITIONWIDGET_H
#define RULECONDITIONWIDGET_H

#include <QWidget>

namespace Ui {
    class RuleCondition;
}

namespace tlp {
class Graph;
}

class RuleConditionWidget: public QWidget
{
    Q_OBJECT

    Ui::RuleCondition *_ui;
    tlp::Graph *_g;

public:
    RuleConditionWidget(QWidget *parent=nullptr);
    ~RuleConditionWidget() override;

    void setRuleCondition(tlp::Graph *g);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void tryCondition();
};

#endif // RULECONDITIONWIDGET_H
