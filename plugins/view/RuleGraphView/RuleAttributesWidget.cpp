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
#include "RuleAttributesWidget.h"

#include "ui_RuleAttributesWidget.h"

#include <QStringList>

#include <portgraph/PorgyConstants.h>

#include <tulip/TlpQtTools.h>

using namespace tlp;
using namespace std;

RuleAttributesWidget::RuleAttributesWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::RuleAttributes), _g(nullptr) {
  _ui->setupUi(this);
  QString t(LAYOUT_ALGO);
  QStringList tmp = t.split(";");
  _ui->LayoutAlgoCombo->addItems(tmp);

  connect(_ui->LayoutAlgoCombo, SIGNAL(currentIndexChanged(const QString &)), this,
          SLOT(setLayoutAlgorithmAttribute(const QString &)));
  connect(_ui->EdgeOrientationCheckbox, SIGNAL(toggled(bool)), this,
          SLOT(setEdgeOrientationAttribute(bool)));
}

RuleAttributesWidget::~RuleAttributesWidget() {
  delete _ui;
}

void RuleAttributesWidget::setLayoutAlgorithmAttribute(const QString &text) {
  _g->setAttribute(PorgyConstants::LAYOUTALGORITHM, tlp::QStringToTlpString(text));
}

void RuleAttributesWidget::setEdgeOrientationAttribute(bool state) {
  _g->setAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED, state);
  emit SignalViewArrows(state);
}

void RuleAttributesWidget::setRuleAttributes(Graph *g) {
  string algo;
  _g = g;
  g->getAttribute(PorgyConstants::LAYOUTALGORITHM, algo);
  int index = _ui->LayoutAlgoCombo->findText(tlpStringToQString(algo));
  if (index != -1)
    _ui->LayoutAlgoCombo->setCurrentIndex(index);

  bool state = false;
  g->getAttribute(PorgyConstants::EDGE_ORIENTATION_ENABLED, state);
  _ui->EdgeOrientationCheckbox->setChecked(state);
}
