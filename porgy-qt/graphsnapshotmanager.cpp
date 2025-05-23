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
#include <porgy/graphsnapshotmanager.h>

#include <tulip/Color.h>
#include <tulip/GlGraphComposite.h>
#include <tulip/GlOffscreenRenderer.h>
#include <tulip/Graph.h>
#include <tulip/PropertyInterface.h>
#include <tulip/TlpQtTools.h>

#include <QBuffer>

using namespace std;
using namespace tlp;

GraphSnapshotManager::GraphSnapshotManager(QObject *parent)
    : QObject(parent), _snapshotSize(QSize(128, 128)), _backgroundColor(Color(255, 255, 255)) {}

QString GraphSnapshotManager::snapshot2base64html(Graph* graph, const GlGraphRenderingParameters& parameters){
    QImage img = takeSingleSnapshot(graph, QSize(512, 512), Color(255, 255, 255), parameters);
    QByteArray data;
    QBuffer buffer(&data);
    img.save(&buffer, "PNG", 100);
    return QString("<img src='data:image/png;base64, %0'>").arg(QString(data.toBase64()));
}

QImage GraphSnapshotManager::takeSingleSnapshot(Graph *graph, const QSize &size,
                                         const tlp::Color &backgroundColor,
                                         const tlp::GlGraphRenderingParameters &parameters) {

  // Create graph preview texture
  GlOffscreenRenderer *offscreenRenderer = tlp::GlOffscreenRenderer::getInstance();
  offscreenRenderer->setSceneBackgroundColor(backgroundColor);
  offscreenRenderer->setViewPortSize(size.width(), size.height());
  // Empty the scene
  offscreenRenderer->clearScene();
  tlp::GlGraphComposite *composite = new tlp::GlGraphComposite(graph);
  composite->setRenderingParameters(parameters);

  // Overwrite default properties with new one.
  offscreenRenderer->addGraphCompositeToScene(composite);
#ifdef __APPLE__
  offscreenRenderer->renderScene(true, false);
#else
  offscreenRenderer->renderScene(true, true);
#endif
  QImage img(offscreenRenderer->getImage());
  offscreenRenderer->clearScene();
  delete composite;
  return img;
  //return QPixmap::fromImage(img);
}

void GraphSnapshotManager::observe(Graph *graph) {
  // Listen graph structure modification
  graph->addListener(this);
  graph->addObserver(this);
  // get visual properties names
  set<string> visualProperties = generateVisualPropertiesNames(graph);
  for (PropertyInterface *property : graph->getObjectProperties()) {
    // It s a visual property listen it
    if (visualProperties.find(property->getName()) != visualProperties.end()) {
      property->addListener(this);
      property->addObserver(this);
      _oberverList[property].insert(graph);
    }
  }
}

void GraphSnapshotManager::stopObserving(Graph *graph) {
  graph->removeListener(this);
  graph->removeObserver(this);
  set<PropertyInterface *> toErase;
  for (auto it = _oberverList.begin(); it != _oberverList.end(); ++it) {
    for (auto itG = it->second.begin(); itG != it->second.end(); ++itG) {
      if (*itG == graph) {
        it->second.erase(itG);
        if (it->second.empty()) {
          it->first->removeListener(this);
          it->first->removeObserver(this);
          toErase.insert(it->first);
        }
        break;
      }
    }
  }
  for (auto it : toErase) {
    _oberverList.erase(it);
  }
}

QPixmap GraphSnapshotManager::takeSnapshot(Graph *graph) {

  map<Graph *, QPixmap>::iterator it = _previews.find(graph);
  if (it == _previews.end()) {
    observe(graph);
    // Generate preview
    QPixmap img = QPixmap::fromImage(takeSingleSnapshot(graph, _snapshotSize, _backgroundColor, _renderingParameters));
    _previews[graph] = img;
    return img;

  } else {
    return it->second;
  }
}

void GraphSnapshotManager::clear() {
  for (map<Graph *, QPixmap>::iterator it = _previews.begin(); it != _previews.end(); ++it) {
    it->first->removeListener(this);
  }
  _previews.clear();

  for (map<PropertyInterface *, set<Graph *>>::iterator it = _oberverList.begin();
       it != _oberverList.end(); ++it) {
    it->first->removeListener(this);
  }
  _oberverList.clear();

  emit allSnapshotsChanged();
}

void GraphSnapshotManager::removeSnapshot(Graph *graph) {
  map<Graph *, QPixmap>::iterator it = _previews.find(graph);
  if (it != _previews.end()) {
    _previews.erase(it);
    stopObserving(graph);
  }
}

void GraphSnapshotManager::setPreviewSize(const QSize &newSize) {
  _snapshotSize = newSize;
  clear();
}

void GraphSnapshotManager::setBackgroundColor(const Color &color) {
  _backgroundColor = color;
  clear();
}

void GraphSnapshotManager::setRenderingParameters(
    const GlGraphRenderingParameters &renderingParameters) {
  _renderingParameters = renderingParameters;
  clear();
}

void GraphSnapshotManager::treatEvents(const vector<Event> &) {
  for (set<Graph *>::iterator it = _toUpdate.begin(); it != _toUpdate.end(); ++it) {
    emit snapshotChanged(*it);
  }
  _toUpdate.clear();
}

void GraphSnapshotManager::treatEvent(const Event &event) {
  if (event.type() == Event::TLP_DELETE) {
    Graph *graph = dynamic_cast<Graph *>(event.sender());
    // Remove graph observer
    if (graph != nullptr) {
      // Erase preview
      _previews.erase(graph);
      stopObserving(graph);
    }

  } else {
    const PropertyEvent *propertyEvent = dynamic_cast<const PropertyEvent *>(&event);
    if (propertyEvent != nullptr) {
      PropertyInterface *property = propertyEvent->getProperty();
      // If the property is observed
      if (_oberverList.find(property) != _oberverList.end()) {
        set<Graph *> toUpdate;
        const set<Graph *> &graphs = _oberverList[property];
        // If set all nodes or edges update all the graph list
        if (propertyEvent->getType() == PropertyEvent::TLP_AFTER_SET_ALL_NODE_VALUE ||
            propertyEvent->getType() == PropertyEvent::TLP_AFTER_SET_ALL_EDGE_VALUE) {
          toUpdate = graphs;
        } else {
          // Find the graphs that have been affected by modifications.
          for (set<Graph *>::const_iterator it = graphs.begin(); it != graphs.end(); ++it) {
            Graph *g = *it;
            switch (propertyEvent->getType()) {
            case PropertyEvent::TLP_AFTER_SET_NODE_VALUE: {
              if (g->isElement(propertyEvent->getNode())) {
                toUpdate.insert(g);
              }
            } break;
            case PropertyEvent::TLP_AFTER_SET_EDGE_VALUE: {
              if (g->isElement(propertyEvent->getEdge())) {
                toUpdate.insert(g);
              }
            } break;
            default:
              break;
            }
          }
        }

        for (set<Graph *>::iterator it = toUpdate.begin(); it != toUpdate.end(); ++it) {
          // Remove preview
          _previews.erase(*it);
          _toUpdate.insert(*it);
        }
      }

    } else {
      const GraphEvent *gEv = dynamic_cast<const GraphEvent *>(&event);
      if (gEv != nullptr) {
        Graph *g = gEv->getGraph();
        switch (gEv->getType()) {
        case GraphEvent::TLP_ADD_NODE:
        case GraphEvent::TLP_ADD_NODES:
        case GraphEvent::TLP_ADD_EDGE:
        case GraphEvent::TLP_ADD_EDGES:
        case GraphEvent::TLP_BEFORE_DEL_NODE:
        case GraphEvent::TLP_BEFORE_DEL_EDGE: {
          // Remove preview
          _previews.erase(g);
          _toUpdate.insert(g);
        } break;
        case GraphEvent::TLP_ADD_INHERITED_PROPERTY:
        case GraphEvent::TLP_ADD_LOCAL_PROPERTY: {
          set<string> visualPropertiesNames = generateVisualPropertiesNames(g);
          // If the property is a visual property
          if (visualPropertiesNames.find(gEv->getPropertyName()) != visualPropertiesNames.end()) {
            // Remove preview
            _previews.erase(g);
            _toUpdate.insert(g);
          }
        } break;
        case GraphEvent::TLP_BEFORE_DEL_INHERITED_PROPERTY:
        case GraphEvent::TLP_BEFORE_DEL_LOCAL_PROPERTY: {
          // Remove properties from observer list
          PropertyInterface *property = g->getProperty(gEv->getPropertyName());
          map<PropertyInterface *, set<Graph *>>::iterator it = _oberverList.find(property);
          // If the property is observed
          if (it != _oberverList.end()) {
            // Get graph to update
            set<Graph *> toUpdate = it->second;
            // Erase the property list
            _oberverList.erase(it);
            // Update each graph in the list
            for (set<Graph *>::iterator it = toUpdate.begin(); it != toUpdate.end(); ++it) {
              _previews.erase(*it);
              _toUpdate.insert(*it);
            }
          }
        } break;
        default:
          break;
        }
      }
    }
  }
}

set<string> GraphSnapshotManager::generateVisualPropertiesNames(Graph *graph) const {
  GlGraphRenderingParameters params;
  GlGraphInputData glGraphInputData(graph, &params);
  std::set<tlp::PropertyInterface *> properties = glGraphInputData.properties();
  set<string> visualProperties;
  for (std::set<tlp::PropertyInterface *>::iterator it = properties.begin(); it != properties.end();
       ++it) {
    visualProperties.insert((*it)->getName());
  }
  return visualProperties;
}
