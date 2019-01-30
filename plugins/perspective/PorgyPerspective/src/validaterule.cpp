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
#include "validaterule.h"
#include "ui_rulevalidation.h"

#include <tulip/Graph.h>
#include <tulip/SimpleTest.h>
#include <tulip/TlpQtTools.h>

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Port.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>

using namespace tlp;
using namespace std;

RuleValidation::RuleValidation(Graph *rulesRoot, QWidget *parent)
    : QDialog(parent), _ui(new Ui::ValidateRule), _rules(rulesRoot) {
  _ui->setupUi(this);
  connect(_ui->buttonBox, SIGNAL(clicked(QAbstractButton *)), this,
          SLOT(validate(QAbstractButton *)));
}

RuleValidation::~RuleValidation() {
  delete _ui;
}

void RuleValidation::validate(QAbstractButton *b) {
  if (_ui->buttonBox->buttonRole(b) == QDialogButtonBox::ApplyRole) {
    if (!_ui->G0RadioButton->isChecked()) {
      content += "<strong>Finding edge loops/multiple edges</strong><br>";
      _ui->resultbrowser->setHtml(content);
      // edge loops and multiple edges
      for(Graph* g:_rules->subGraphs()) {
        vector<edge> multiple, loops;
        if (SimpleTest::simpleTest(g, &multiple, &loops)) {
          content += "Rule <em>" + tlp::tlpStringToQString(g->getName()) +
                     "</em> does not have any edge loop nor multiple edges.<br>";
          _ui->resultbrowser->setHtml(content);
          continue;
        }
        if (!loops.empty()) {
          size_t num(loops.size());
          content += "Rule <em>" + tlp::tlpStringToQString(g->getName()) + "</em> has <strong>" +
                     QString::number(num) + "</strong> edge loops";
          if (num > 0) {
            content += ": ";
            for (edge e : loops) {
              content += QString::number(e.id) + " ";
            }
          }
          content += ".<br>";
        }
        if (!multiple.empty()) {
          size_t num(multiple.size());
          content += "Rule <em>" + tlp::tlpStringToQString(g->getName()) + "</em> has <strong>" +
                     QString::number(num) + "</strong> multiple edges";
          if (num > 0) {
            content += ": ";
            for (edge e : multiple) {
              content += QString::number(e.id) + " ";
            }
          }
          content += ".<br>";
        }
        _ui->resultbrowser->setHtml(content);
      }
      // display arity value
      content += "<br/><strong>Arity property -- Remember that the value of "
                 "the arity property of each port needs to be considered as "
                 "the minimum required degree for any corresponding port in "
                 "the graph to rewrite</strong><br/>";
      for(Graph* g:_rules->subGraphs()) {
        PortGraphRule pgr(g);
        content += "Rule <em>" + tlp::tlpStringToQString(g->getName()) + "</em><br/>";
        for (PortNode *pn : pgr.getMember(PorgyConstants::SIDE_LEFT)) {
          for (Port *p : pn->getPorts()) {
            content += "Port " + QString::number(p->getNode().id) + " has arity set to " +
                       QString::number(p->getArity()) + "<br/>";
          }
        }
        content += "<br/>";
        _ui->resultbrowser->setHtml(content);
      }
    } else {
      content += "<strong>G0 validation -- Finding edge loops/multiple "
                 "edges</strong><br>";
      Graph *g = PorgyTlpGraphStructure::findModel(_rules, "G0");
      vector<edge> multiple, loops;
      if (SimpleTest::simpleTest(g, &multiple, &loops)) {
        content += "G0 does not have any edge loop nor multiple edges.<br>";
      }
      if (!loops.empty()) {
        size_t num(loops.size());
        content += "G0 has <strong>" + QString::number(num) + "</strong> edge loops";
        if (num > 0) {
          content += ": ";
          for (edge e : loops) {
            content += QString::number(e.id) + " ";
          }
        }
        content += ".<br>";
      }
      if (!multiple.empty()) {
        size_t num(multiple.size());
        content += "G0 has <strong>" + QString::number(num) + "</strong> multiple edges";
        if (num > 0) {
          content += ": ";
          for (edge e : multiple) {
            content += QString::number(e.id) + " ";
          }
        }
        content += ".<br>";
      }
      _ui->resultbrowser->setHtml(content);
    }
  } else if (_ui->buttonBox->buttonRole(b) == QDialogButtonBox::ResetRole) {
    content = "";
    _ui->resultbrowser->setHtml("<strong>Choose \"Rules\" or \"Graph G0\" and "
                                "press \"Apply\" to start "
                                "validation.</strong>");
  }
}
