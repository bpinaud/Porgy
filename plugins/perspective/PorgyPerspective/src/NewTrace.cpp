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
#include "NewTrace.h"
#include "ui_NewTrace.h"

#include <portgraph/Bridge.h>
#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/PortGraphModel.h>
#include <portgraph/PortNode.h>
#include <portgraph/Trace.h>

#include <tulip/BooleanProperty.h>
#include <tulip/ColorProperty.h>
#include <tulip/ConnectedTest.h>
#include <tulip/DoubleProperty.h>
#include <tulip/Graph.h>
#include <tulip/IntegerProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/TlpQtTools.h>

#include <QMessageBox>

using namespace tlp;
using namespace std;

Graph *NewTrace::createNewTrace(Graph *tracemain, node root, QWidget *parent) {
  BooleanProperty sel(tracemain);
  sel.setNodeValue(root, true);
  return createNewTrace(&sel, tracemain, parent);
}

Graph *NewTrace::createNewTrace(BooleanProperty *sel, tlp::Graph *parentGraph, QWidget *parent) {
  NewTrace trace(parent, parentGraph, sel);
  Graph *graph = nullptr;
  if (trace.exec()) {
    Observable::holdObservers();
    graph = trace.buildNewTrace(true, sel);
    Observable::unholdObservers();
  }
  return graph;
}

Graph *NewTrace::computeOnTrace(BooleanProperty *sel, tlp::Graph *parentGraph, QWidget *parent) {
  NewTrace trace(parent, parentGraph, sel);
  trace.setActualTraceName();
  Graph *graph = nullptr;
  if (trace.exec()) {
    Observable::holdObservers();
    graph = trace.buildNewTrace(false, sel);
    Observable::unholdObservers();
  }
  return graph;
}

NewTrace::NewTrace(QWidget *parent, Graph *g, BooleanProperty *sel)
    : QDialog(parent), graph(g), ui(new Ui::NewTrace), sel(sel) {
  ui->setupUi(this);
  connect(ui->ratioPropertyButton, SIGNAL(pressed()), this, SLOT(property_ratio_radio()));
  connect(ui->PropertyHistogramTree, SIGNAL(pressed()), this, SLOT(property_histogram_radio()));
  connect(ui->HistogramTree, SIGNAL(pressed()), this, SLOT(histogram_radio()));
  connect(ui->StandardTree, SIGNAL(pressed()), this, SLOT(standard_radio()));

  fillPortNodesList();
  fillPropertyCombobox();
  fillPropertyRatioCombobox();
}

NewTrace::~NewTrace() {
  delete ui;
}

void NewTrace::property_ratio_radio() {
  ui->portSelectionlistWidget->setEnabled(false);
  ui->typeElementComboBox->setEnabled(false);
  ui->propertySelectionComboBox->setEnabled(false);
  ui->propertyRelationComboBox->setEnabled(false);
  ui->propertyValueLineEdit->setEnabled(false);
  ui->ratioPropertyCurrentComboBox->setEnabled(true);
  ui->ratioPropertyAntecedentComboBox->setEnabled(true);
  ui->ratioPropertyAntecedentLabel->setEnabled(true);
}

void NewTrace::property_histogram_radio() {
  ui->portSelectionlistWidget->setEnabled(false);
  ui->typeElementComboBox->setEnabled(true);
  ui->propertySelectionComboBox->setEnabled(true);
  ui->propertyRelationComboBox->setEnabled(true);
  ui->propertyValueLineEdit->setEnabled(true);
  ui->ratioPropertyCurrentComboBox->setEnabled(false);
  ui->ratioPropertyAntecedentComboBox->setEnabled(false);
  ui->ratioPropertyAntecedentLabel->setEnabled(false);
}

void NewTrace::histogram_radio() {
  ui->portSelectionlistWidget->setEnabled(true);
  ui->typeElementComboBox->setEnabled(false);
  ui->propertySelectionComboBox->setEnabled(false);
  ui->propertyRelationComboBox->setEnabled(false);
  ui->propertyValueLineEdit->setEnabled(false);
  ui->ratioPropertyCurrentComboBox->setEnabled(false);
  ui->ratioPropertyAntecedentComboBox->setEnabled(false);
  ui->ratioPropertyAntecedentLabel->setEnabled(false);
}

void NewTrace::standard_radio() {
  ui->portSelectionlistWidget->setEnabled(false);
  ui->typeElementComboBox->setEnabled(false);
  ui->propertySelectionComboBox->setEnabled(false);
  ui->propertyRelationComboBox->setEnabled(false);
  ui->propertyValueLineEdit->setEnabled(false);
  ui->ratioPropertyCurrentComboBox->setEnabled(false);
  ui->ratioPropertyAntecedentComboBox->setEnabled(false);
  ui->ratioPropertyAntecedentLabel->setEnabled(false);
}

Graph *NewTrace::buildNewTrace(bool createNewGraph, BooleanProperty *prop) {
  Graph *newTraceGraph = graph;
  if (createNewGraph) {
    newTraceGraph = PorgyTlpGraphStructure::getTraceRoot(graph)->addSubGraph(
        prop, tlp::QStringToTlpString(ui->graphname->text()));
    PorgyTlpGraphStructure::fixTraceGraph(newTraceGraph);
  }
  if (ui->HistogramTree->isChecked()) {
    Trace trace(newTraceGraph);
    Observable::holdObservers();
    list<string> selected;
    foreach (QListWidgetItem *item, ui->portSelectionlistWidget->selectedItems()) {
      selected.push_back(tlp::QStringToTlpString(item->text()));
    }
    trace.followPortNode(selected);
    newTraceGraph->getLocalProperty<ColorProperty>("viewColor")->setAllNodeValue(Color(0, 0, 0));
    newTraceGraph->getLocalProperty<IntegerProperty>("viewShape")
        ->setAllNodeValue(NodeShape::Circle);
    Observable::unholdObservers();
  } else if (ui->PropertyHistogramTree->isChecked()) {
    Trace trace(newTraceGraph);
    Observable::holdObservers();
    trace.followPropertyValue(tlp::QStringToTlpString(ui->graphname->text()),
                              QStringToTlpString(ui->typeElementComboBox->currentText()),
                              QStringToTlpString(ui->propertySelectionComboBox->currentText()),
                              QStringToTlpString(ui->propertyRelationComboBox->currentText()),
                              QStringToTlpString(ui->propertyValueLineEdit->text()));
    newTraceGraph->getLocalProperty<ColorProperty>("viewColor")->setAllNodeValue(Color(0, 0, 0));
    newTraceGraph->getLocalProperty<IntegerProperty>("viewShape")
        ->setAllNodeValue(NodeShape::Circle);
    Observable::unholdObservers();
  } else if (ui->ratioPropertyButton->isChecked()) {
    Trace trace(newTraceGraph);
    Observable::holdObservers();
    trace.computePropertyRatioValue(
        QStringToTlpString(ui->ratioPropertyAntecedentComboBox->currentText()),
        QStringToTlpString(ui->ratioPropertyCurrentComboBox->currentText()));
    Observable::unholdObservers();
  }
  return newTraceGraph;
}

void NewTrace::accept() {
  Graph *TraceMain = PorgyTlpGraphStructure::getMainTrace(graph);
  Graph *g = tlp::newGraph();
  Observable::holdObservers();
  copyToGraph(g, TraceMain, sel);
  // Check number of nodes
  if (g->isEmpty()) {
    QMessageBox::critical(this, "Create a new derivation tree", "Cannot create an empty graph.");
    Observable::unholdObservers();
    delete g;
    QDialog::reject();
    return;
  }
  // Check connectivity
  if (!ConnectedTest::isConnected(g)) {
    QMessageBox::critical(this, "Create a new derivation tree",
                          "Error during the creation of the subgraph. Is it connected?");
    Observable::unholdObservers();
    delete g;
    QDialog::reject();
    return;
  }

  // Check if we have a path
  if (!ui->StandardTree->isChecked() && !PorgyTlpGraphStructure::graphIsASimplePath(g)) {
    QMessageBox::critical(this, "Create a new derivation tree",
                          "Error during the creation of the subgraph. It is not a path.");
    Observable::unholdObservers();
    delete g;
    QDialog::reject();
    return;
  }
  Observable::unholdObservers();
  delete g;
  QDialog::accept();
}

void NewTrace::fillPortNodesList() {
  QStringList portNodes;
  // We are only interested in the nodes found in the rules
  // Search in rules
  fillPortNodesForGraph(PorgyTlpGraphStructure::getRulesRoot(graph), portNodes);
  ui->portSelectionlistWidget->clear();
  portNodes.sort();
  ui->portSelectionlistWidget->addItems(portNodes);
}

void NewTrace::fillPortNodesForGraph(tlp::Graph *g, QStringList &list) {
  for(Graph* sub:g->subGraphs()) {
    PortGraphModel portGraph(sub);
    for (PortNode *pn : portGraph.getPortNodes()) {
      QString name = tlp::tlpStringToQString(pn->getName());
      if (!list.contains(name) && !Bridge::isBridge(pn->getCenter(), portGraph.getGraph()))
        list << name;
    }
  }
}

void NewTrace::fillPropertyCombobox() {
  QStringList nodesNames;
  tlp::Graph *g = PorgyTlpGraphStructure::getRulesRoot(graph);

  // Use the rules default properties to avoid P, N, M, New...
  for(auto propertyName:g->getProperties()) {
    tlp::PropertyInterface *property = g->getProperty(propertyName);
    if (property->getTypename() == IntegerProperty::propertyTypename ||
        property->getTypename() == DoubleProperty::propertyTypename ||
        property->getTypename() == StringProperty::propertyTypename) {
      // Only available for double, integer and string properties
      nodesNames << tlp::tlpStringToQString(propertyName);
    }
  }

  ui->propertySelectionComboBox->clear();
  nodesNames.sort();
  ui->propertySelectionComboBox->addItems(nodesNames);
}

void NewTrace::fillPropertyRatioCombobox() {
  QStringList nodesNames;
  // Use the rules default properties to avoid P, N, M, New...
  for(auto propertyName:graph->getProperties()) {
    tlp::PropertyInterface *property = graph->getProperty(propertyName);
    if (property->getTypename() == IntegerProperty::propertyTypename ||
        property->getTypename() == DoubleProperty::propertyTypename) {
      // Only available for double and integer properties
      nodesNames << tlp::tlpStringToQString(propertyName);
    }
  }

  ui->ratioPropertyCurrentComboBox->clear();
  ui->ratioPropertyAntecedentComboBox->clear();
  nodesNames.sort();
  ui->ratioPropertyCurrentComboBox->addItems(nodesNames);
  ui->ratioPropertyAntecedentComboBox->addItems(nodesNames);
}

void NewTrace::setActualTraceName() {
  this->setWindowTitle(tlp::tlpStringToQString("Update existing derivation tree"));
  ui->graphname->setText(tlp::tlpStringToQString(graph->getName()));
  ui->graphname->setEnabled(false);
}
