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
#include "RuleAlgorithmWidget.h"

#include "ui_RuleAlgorithmWidget.h"

#include <QEvent>
#include <QFont>
#include <QStringList>
#include <QWheelEvent>

#include <portgraph/PorgyConstants.h>
#include <portgraph/PortGraphRule.h>

#include <tulip/BooleanProperty.h>
#include <tulip/Graph.h>
#include <tulip/IntegerProperty.h>
#include <tulip/TlpQtTools.h>

using namespace tlp;
using namespace std;

RuleAlgorithmWidget::RuleAlgorithmWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::RuleAlgorithm), _g(nullptr) {
  _ui->setupUi(this);
  connect(_ui->parseButton, SIGNAL(clicked()), SLOT(tryAlgorithm()));
  _ui->instructionsText->installEventFilter(this);
}

RuleAlgorithmWidget::~RuleAlgorithmWidget() {
  delete _ui;
}

bool RuleAlgorithmWidget::eventFilter(QObject *, QEvent *event) {
  if (event->type() == QEvent::Wheel) {
    QWheelEvent *ev = static_cast<QWheelEvent *>(event);
    if (ev->modifiers() &
#if defined(__APPLE__)
        Qt::AltModifier
#else
        Qt::ControlModifier
#endif
        ) {
      int numDegrees = ev->delta() / 8;
      int numSteps = numDegrees / 15;
      QFont f = _ui->instructionsText->font();
      f.setPointSize(f.pointSize() + numSteps);
      _ui->instructionsText->setFont(f);
      event->accept();
      return true;
    }
  }
  return false;
}

void RuleAlgorithmWidget::tryAlgorithm() {
  string text = QStringToTlpString(_ui->instructionsText->toPlainText());
  string err("");
  if (text.empty()) {
    _ui->parsingResult->setText(tlpStringToQString(""));
    _g->setAttribute(PorgyConstants::RULE_ALGORITHM, text);
    return;
  }

  std::string lhsSelectionProperty("RuleAlgorithmWidget_lhs_selection");
  std::string rhsSelectionProperty("RuleAlgorithmWidget_rhs_selection");
  std::string mappingProperty("RuleAlgorithmWidget_both_mapping");
  tlp::BooleanProperty *left_side =
      _g->getLocalProperty<tlp::BooleanProperty>(lhsSelectionProperty);
  tlp::BooleanProperty *right_side =
      _g->getLocalProperty<tlp::BooleanProperty>(rhsSelectionProperty);
  PortGraphRuleDecorator dec(_g);
  tlp::IntegerProperty *side_property = dec.getSideProperty();
  tlp::IntegerProperty *mapping_property =
      _g->getLocalProperty<tlp::IntegerProperty>(mappingProperty);
  for (node n : _g->nodes()) {
    if (side_property->getNodeValue(n) == PorgyConstants::SIDE_LEFT) {
      left_side->setNodeValue(n, true);
    } else if (side_property->getNodeValue(n) == PorgyConstants::SIDE_RIGHT) {
      right_side->setNodeValue(n, true);
    }
    mapping_property->setNodeValue(n, n.id);
  }
  for (edge e : _g->edges()) {
    if (side_property->getEdgeValue(e) == PorgyConstants::SIDE_LEFT) {
      left_side->setEdgeValue(e, true);
    } else if (side_property->getEdgeValue(e) == PorgyConstants::SIDE_RIGHT) {
      right_side->setEdgeValue(e, true);
    }
    mapping_property->setEdgeValue(e, e.id);
  }

  tlp::DataSet dataSet;
  dataSet.set<string>("Instructions", text);
  dataSet.set<string>("Mode", "parse");
  dataSet.set<tlp::Graph *>("Lhs", _g); // use decoy properties for the mapping
  dataSet.set<string>("LhsMappingProperty", mappingProperty);
  dataSet.set<string>("LhsSelectionProperty", lhsSelectionProperty);
  dataSet.set<string>("RhsMappingProperty", mappingProperty);
  dataSet.set<string>("RhsSelectionProperty", rhsSelectionProperty);

  if (!_g->applyAlgorithm(PorgyConstants::APPLY_RULE_ALGORITHM, err, &dataSet)) {
    _ui->parsingResult->setText(tlpStringToQString("Parsing Failed"));
    _ui->parsingResult->setStyleSheet("QLabel { color : red; }");

  } else {
    _ui->parsingResult->setText(tlpStringToQString("Parsing Succeeded"));
    _ui->parsingResult->setStyleSheet("QLabel { color : green; }");
  }
  _g->setAttribute(PorgyConstants::RULE_ALGORITHM, text);
  _g->delLocalProperty(lhsSelectionProperty);
  _g->delLocalProperty(rhsSelectionProperty);
  _g->delLocalProperty(mappingProperty);
}

void RuleAlgorithmWidget::setRuleAlgorithm(Graph *g) {
  _g = g;
  string text("");
  if (!_g->existAttribute(PorgyConstants::RULE_ALGORITHM)) {
    _g->setAttribute(PorgyConstants::RULE_ALGORITHM, text);
  } else {
    _g->getAttribute(PorgyConstants::RULE_ALGORITHM, text);
  }
  _ui->instructionsText->setPlainText(tlpStringToQString(text));
  tryAlgorithm();
}
