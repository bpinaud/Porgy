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
#include "graphmodificationlistener.h"

#include <tulip/AbstractProperty.h>
#include <tulip/Graph.h>

using namespace tlp;
using namespace std;

GraphModficationListener::GraphModficationListener() {}

GraphModficationListener::~GraphModficationListener() {}

void GraphModficationListener::startRecording(Graph *graph) {
  addedProperties.clear();
  updatedProperties.clear();
  removedProperties.clear();
  updatedNodes.clear();
  createdNodes.clear();
  createdEdges.clear();
  deletedNodes.clear();
  deletedEdges.clear();
  graph->addListener(this);
  for (const string& propertyName:graph->getProperties()) {
    graph->getProperty(propertyName)->addListener(this);
  }
}

void GraphModficationListener::stopRecording(Graph *graph) {
  graph->removeListener(this);
  for(auto propertyName:graph->getProperties()) {
    graph->getProperty(propertyName)->removeListener(this);
  }
}

void GraphModficationListener::addLocalProperty(Graph *graph, const string &propertyName) {
  addedProperties.push_back(propertyName);
  graph->getProperty(propertyName)->addListener(this);
}

void GraphModficationListener::afterSetNodeValue(PropertyInterface *property, const node n) {
  if (find(addedProperties.begin(), addedProperties.end(), property->getName()) ==
      addedProperties.end())
    updatedProperties.insert(property->getName());

  updatedNodes.insert(n);
}
void GraphModficationListener::afterSetEdgeValue(PropertyInterface *property, const edge) {
  if (find(addedProperties.begin(), addedProperties.end(), property->getName()) ==
      addedProperties.end())
    updatedProperties.insert(property->getName());
}
void GraphModficationListener::afterSetAllNodeValue(PropertyInterface *property) {
  if (find(addedProperties.begin(), addedProperties.end(), property->getName()) ==
      addedProperties.end())
    updatedProperties.insert(property->getName());
}
void GraphModficationListener::afterSetAllEdgeValue(PropertyInterface *property) {
  if (find(addedProperties.begin(), addedProperties.end(), property->getName()) ==
      addedProperties.end())
    updatedProperties.insert(property->getName());
}
void GraphModficationListener::destroy(PropertyInterface *property) {
  if (find(addedProperties.begin(), addedProperties.end(), property->getName()) ==
      addedProperties.end())
    addedProperties.erase(
        find(addedProperties.begin(), addedProperties.end(), property->getName()));
  if (updatedProperties.find(property->getName()) == updatedProperties.end())
    updatedProperties.erase(property->getName());
  removedProperties.insert(property->getName());
}

void GraphModficationListener::addNode(Graph *, const node n) {
  createdNodes.insert(n);
  deletedNodes.erase(n);
}

void GraphModficationListener::addNodes(Graph *, const std::vector<tlp::node> &nodes) {
  createdNodes.insert(nodes.begin(), nodes.end());
  for (const auto &it : nodes) {
    deletedNodes.erase(it);
  }
}

void GraphModficationListener::delNode(Graph *, const node n) {
  deletedNodes.insert(n);
  createdNodes.erase(n);
}

void GraphModficationListener::addEdge(Graph *, const edge e) {
  createdEdges.insert(e);
  deletedEdges.erase(e);
}

void GraphModficationListener::addEdges(Graph *, const std::vector<tlp::edge> &edges) {
  createdEdges.insert(edges.begin(), edges.end());
  for (const auto &it : edges) {
    deletedEdges.erase(it);
  }
}
void GraphModficationListener::delEdge(Graph *, const edge e) {
  deletedEdges.insert(e);
  createdEdges.erase(e);
}

void GraphModficationListener::treatEvent(const tlp::Event &e) {
  const tlp::GraphEvent *gEvt = dynamic_cast<const tlp::GraphEvent *>(&e);
  if (gEvt != nullptr) {
    switch (gEvt->getType()) {
    case tlp::GraphEvent::TLP_ADD_NODE:
      addNode(gEvt->getGraph(), gEvt->getNode());
      break;
    case tlp::GraphEvent::TLP_ADD_NODES:
      addNodes(gEvt->getGraph(), gEvt->getNodes());
      break;
    case tlp::GraphEvent::TLP_DEL_NODE:
      delNode(gEvt->getGraph(), gEvt->getNode());
      break;
    case tlp::GraphEvent::TLP_ADD_EDGE:
      addEdge(gEvt->getGraph(), gEvt->getEdge());
      break;
    case tlp::GraphEvent::TLP_ADD_EDGES:
      addEdges(gEvt->getGraph(), gEvt->getEdges());
      break;
    case tlp::GraphEvent::TLP_DEL_EDGE:
      delEdge(gEvt->getGraph(), gEvt->getEdge());
      break;
    case tlp::GraphEvent::TLP_ADD_LOCAL_PROPERTY:
      addLocalProperty(gEvt->getGraph(), gEvt->getPropertyName());
      break;
    case tlp::GraphEvent::TLP_BEFORE_DEL_LOCAL_PROPERTY:
      destroy(gEvt->getGraph()->getProperty(gEvt->getPropertyName()));
      break;
    default:
      break;
    }
  } else {
    const tlp::PropertyEvent *pEvt = dynamic_cast<const tlp::PropertyEvent *>(&e);
    if (pEvt != nullptr) {
      switch (pEvt->getType()) {
      case tlp::PropertyEvent::TLP_AFTER_SET_NODE_VALUE:
        afterSetNodeValue(pEvt->getProperty(), pEvt->getNode());
        break;
      case tlp::PropertyEvent::TLP_AFTER_SET_ALL_NODE_VALUE:
        afterSetAllNodeValue(pEvt->getProperty());
        break;
      case tlp::PropertyEvent::TLP_AFTER_SET_EDGE_VALUE:
        afterSetEdgeValue(pEvt->getProperty(), pEvt->getEdge());
        break;
      case tlp::PropertyEvent::TLP_AFTER_SET_ALL_EDGE_VALUE:
        afterSetAllEdgeValue(pEvt->getProperty());
        break;
      default:
        break;
      }
    }
  }
}
