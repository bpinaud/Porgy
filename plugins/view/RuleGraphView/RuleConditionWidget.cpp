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
#include "RuleConditionWidget.h"

#include "ui_RuleConditionWidget.h"

#include <QStringList>
#include <QWheelEvent>
#include <QEvent>
#include <QFont>

#include <portgraph/PorgyConstants.h>
#include <portgraph/PortGraphRule.h>

#include <tulip/TlpQtTools.h>
#include <tulip/Graph.h>
#include <tulip/BooleanProperty.h>
#include <tulip/IntegerProperty.h>

using namespace tlp;
using namespace std;

RuleConditionWidget::RuleConditionWidget(QWidget *parent):QWidget(parent), _ui(new Ui::RuleCondition), _g(nullptr)
{
    _ui->setupUi(this);
    connect(_ui->parseButton, SIGNAL(clicked()), SLOT(tryCondition()));
    _ui->conditionsText->installEventFilter(this);
}

RuleConditionWidget::~RuleConditionWidget() {
    delete _ui;
}

bool RuleConditionWidget::eventFilter(QObject *, QEvent *event) {
    if(event->type()==QEvent::Wheel) {
        QWheelEvent *ev = static_cast<QWheelEvent*>(event);
        if (ev->modifiers() &
#if defined(__APPLE__)
            Qt::AltModifier
#else
            Qt::ControlModifier
#endif
        ) {
            int numDegrees = ev->delta()/8;
            int numSteps = numDegrees / 15;
            QFont f = _ui->conditionsText->font();
            f.setPointSize(f.pointSize()+numSteps);
            _ui->conditionsText->setFont(f);
            event->accept();
            return true;
        }
    }
    return false;
}

void RuleConditionWidget::tryCondition() {
    string text = QStringToTlpString(_ui->conditionsText->toPlainText());
    string err("");
    if (text == "") {
        _ui->conditionParsingResult->setText(tlpStringToQString(""));
        _g->setAttribute(PorgyConstants::RULE_CONDITION, text);
        return;
    }
    
    std::string lhsSelectionProperty("RuleConditionWidget_lhs_selection");
    std::string mappingProperty("RuleConditionWidget_lhs_mapping");
    tlp::BooleanProperty *left_side = _g->getLocalProperty<tlp::BooleanProperty>(lhsSelectionProperty);
    PortGraphRuleDecorator dec(_g);
    tlp::IntegerProperty *side_property = dec.getSideProperty();
    tlp::IntegerProperty *mapping_property = _g->getLocalProperty<tlp::IntegerProperty>(mappingProperty);
    for(node n:_g->nodes()) {
        if (side_property->getNodeValue(n) == PorgyConstants::SIDE_LEFT){
            left_side->setNodeValue(n, true);
        }
        mapping_property->setNodeValue(n, n.id);
    }
    for(edge e:_g->edges()) {
        if (side_property->getEdgeValue(e) == PorgyConstants::SIDE_LEFT){
            left_side->setEdgeValue(e, true);
        }
        mapping_property->setEdgeValue(e, e.id);
    }

    tlp::DataSet dataSet;
    dataSet.set<string>("Conditions", text);
    dataSet.set<string>("Mode", "parse");
    dataSet.set<PortGraph*>("Model", nullptr);
    dataSet.set<string>("LhsMappingProperty", mappingProperty);

    if (!_g->applyAlgorithm(PorgyConstants::APPLY_RULE_CONDITION, err, &dataSet))
    {
        _ui->conditionParsingResult->setText(tlpStringToQString("Condition parsing failed! " + err));
        _ui->conditionParsingResult->setStyleSheet("QLabel { color : red; }");
    
    } else {
        _ui->conditionParsingResult->setText(tlpStringToQString("Condition parsing succeeded. " + err));
        _ui->conditionParsingResult->setStyleSheet("QLabel { color : green; }");
    }
    
    _g->setAttribute(PorgyConstants::RULE_CONDITION, text);
    _g->delLocalProperty(mappingProperty);
}

void RuleConditionWidget::setRuleCondition(Graph *g) {
    _g = g;
    string text("");
    if (!_g->existAttribute(PorgyConstants::RULE_CONDITION))
    {
        _g->setAttribute(PorgyConstants::RULE_CONDITION, text);
    } else {
        _g->getAttribute(PorgyConstants::RULE_CONDITION, text);
    }
    _ui->conditionsText->setPlainText(tlpStringToQString(text));
    tryCondition();
}
