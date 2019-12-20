#include "porgyRuleViewQuickAccessBar.h"

#include <QCheckBox>
#include <QMessageBox>

#include <tulip/BooleanProperty.h>

#include <portgraph/Bridge.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphRule.h>

using namespace tlp;
using namespace std;

PorgyQuickAccessBar::PorgyQuickAccessBar(Graph *rule, QWidget *parent)
    : QuickAccessBarImpl(
          nullptr, QuickAccessBarImpl::QuickAccessButtons(
                       QuickAccessBarImpl::SCREENSHOT | QuickAccessBarImpl::BACKGROUNDCOLOR |
                       QuickAccessBarImpl::SHOWLABELS | QuickAccessBarImpl::SIZEINTERPOLATION |
                       QuickAccessBarImpl::LABELSSCALED | QuickAccessBarImpl::SHOWEDGES),
          parent),
      rule_graph(rule), checkRHS(new QCheckBox("RHS", this)), checkLHS(new QCheckBox("LHS", this)),
      checkArrowNodeEdges(new QCheckBox("Arrow node edges", this)), sidep(new BooleanProperty(rule)) {

  checkRHS->setChecked(true);
  checkLHS->setChecked(true);
  checkArrowNodeEdges->setChecked(true);
  QVector<QAbstractButton *> vect;
  vect << checkLHS << checkRHS << checkArrowNodeEdges;
  addButtonsAtEnd(vect);

  connect(checkLHS, SIGNAL(toggled(bool)), this, SLOT(LHS(bool)));
  connect(checkRHS, SIGNAL(toggled(bool)), this, SLOT(RHS(bool)));
  connect(checkArrowNodeEdges, SIGNAL(toggled(bool)), this, SLOT(arrowNodeEdges(bool)));
}

PorgyQuickAccessBar::~PorgyQuickAccessBar() {
    delete sidep;
}

void PorgyQuickAccessBar::setGraph(Graph *g) {
  rule_graph = g;
}

bool PorgyQuickAccessBar::LHSVisible() const {
  return flags.testFlag(LHSFLAG);
}

bool PorgyQuickAccessBar::RHSVisible() const {
  return flags.testFlag(RHSFLAG);
}

bool PorgyQuickAccessBar::allVisible() const {
  return flags.testFlag(LHSFLAG) && flags.testFlag(RHSFLAG) && flags.testFlag(ARROW_EDGES_FLAG);
}

void PorgyQuickAccessBar::reset() {
  QuickAccessBarImpl::reset();
  sidep->setAllNodeValue(true);
  sidep->setAllEdgeValue(true);
  checkLHS->setChecked(true);
  checkRHS->setChecked(true);
  checkArrowNodeEdges->setChecked(true);
  flags = RHSFLAG | LHSFLAG | ARROW_EDGES_FLAG;
}

void PorgyQuickAccessBar::computeGraphToDisplay() {
  PortGraphRuleDecorator rule(rule_graph);
  IntegerProperty *sideprop = rule.getSideProperty();
  // all flags. Display the rule
  if (flags == SideToDisplayFlags(LHSFLAG | RHSFLAG | ARROW_EDGES_FLAG)) {
    emit new_graph_to_display(nullptr);
    return;
  }
  sidep->setAllNodeValue(true);
  sidep->setAllEdgeValue(true);
  // Bridge always visible
  PortGraphRule pgr(rule_graph);
  Bridge *br = pgr.getBridge();
  sidep->setNodeValue(br->getCenter(), false);
  for (node n : rule.nodes()) {
    int side = sideprop->getNodeValue(n);
    if (flags.testFlag(LHSFLAG) && (side == PorgyConstants::RuleSide::SIDE_LEFT)) {
      sidep->setNodeValue(n, false);
    }
    if (flags.testFlag(RHSFLAG) && (side == PorgyConstants::RuleSide::SIDE_RIGHT)) {
      sidep->setNodeValue(n, false);
    }
  }
  for (edge e : rule.edges()) {
    int side = sideprop->getEdgeValue(e);
    if (flags.testFlag(LHSFLAG) && (side == PorgyConstants::RuleSide::SIDE_LEFT)) {
      sidep->setEdgeValue(e, false);
    }
    if (flags.testFlag(RHSFLAG) && (side == PorgyConstants::RuleSide::SIDE_RIGHT)) {
      sidep->setEdgeValue(e, false);
    }
    if (flags.testFlag(ARROW_EDGES_FLAG) && ((side == PorgyConstants::RuleSide::SIDE_BRIDGE) ||
                                             (side == PorgyConstants::RuleSide::SIDE_BRIDGE_PORT) ||
                                             (side == PorgyConstants::RuleSide::SIDE_BRIDGE_OPP))) {
      sidep->setEdgeValue(e, false);
    }
  }

  emit new_graph_to_display(sidep);
}

void PorgyQuickAccessBar::LHS(bool state) {
  if (state == false && !checkRHS->isChecked()) {
    QMessageBox::warning(parentWidget(), "Cannot display an empty graph",
                         "At least one of RHS/LHS has to be displayed.");
    checkLHS->setChecked(true);
    return;
  }
  checkArrowNodeEdges->setEnabled(checkRHS->isChecked() & state);
  flags = SideToDisplayFlags();
  if (state) {
    flags |= LHSFLAG;
    if (checkRHS->isChecked()) {
      flags |= RHSFLAG;
      if (checkArrowNodeEdges->isChecked())
        flags |= ARROW_EDGES_FLAG;
    }
  } else
    flags |= RHSFLAG;
  computeGraphToDisplay();
}

void PorgyQuickAccessBar::RHS(bool state) {
  if (state == false && !checkLHS->isChecked()) {
    QMessageBox::warning(parentWidget(), "Cannot display an empty graph",
                         "At least one of RHS/LHS has to be displayed.");
    checkRHS->setChecked(true);
    return;
  }
  checkArrowNodeEdges->setEnabled(checkLHS->isChecked() & state);
  flags = SideToDisplayFlags();
  if (state) {
    flags |= RHSFLAG;
    if (checkLHS->isChecked()) {
      flags |= LHSFLAG;
      if (checkArrowNodeEdges->isChecked())
        flags |= ARROW_EDGES_FLAG;
    }
  } else
    flags |= LHSFLAG;
  computeGraphToDisplay();
}

void PorgyQuickAccessBar::arrowNodeEdges(bool state) {
  flags = LHSFLAG | RHSFLAG;
  if (state)
    flags |= ARROW_EDGES_FLAG;
  computeGraphToDisplay();
}
