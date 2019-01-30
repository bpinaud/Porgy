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
#ifndef GRAPHSNAPSHOTMANAGER_H
#define GRAPHSNAPSHOTMANAGER_H

#include <QObject>
#include <QPixmap>

#include <tulip/Color.h>
#include <tulip/GlGraphRenderingParameters.h>
#include <tulip/Observable.h>
#include <tulip/PropertyInterface.h>

#include <map>

#include <portgraph/porgyconf.h>

namespace tlp {
class Graph;
}

class PORGY_SCOPE GraphSnapshotManager : public QObject, tlp::Observable {
  Q_OBJECT

  /** Graph observation variables */
  mutable std::map<tlp::Graph *, QPixmap> _previews;
  mutable std::map<tlp::PropertyInterface *, std::set<tlp::Graph *>> _oberverList;
  std::set<tlp::Graph *> _toUpdate;

  /** Snapshots visual parameters */
  QSize _snapshotSize;
  tlp::Color _backgroundColor;
  tlp::GlGraphRenderingParameters _renderingParameters;

public:
  GraphSnapshotManager(QObject *parent = nullptr);

  /**
    * @brief Take a snapshot of a graph.
    **/
  static QPixmap
  takeSingleSnapshot(tlp::Graph *graph, const QSize &size,
                     const tlp::Color &backgroundColor = tlp::Color(255, 255, 255),
                     const tlp::GlGraphRenderingParameters & = tlp::GlGraphRenderingParameters());

  /**
    * @brief Return the snapshot of a graph. If the snapshot was already
    *computed return the last snapshot taken. The snapshot is valid until a
    *graph visual parameter will be updated or if the graph structure change.
    * To stop listening graph modification use the removeSnapshot function
    * If the graph snapshot changed the snapshotChanged signal is send.
    **/
  QPixmap takeSnapshot(tlp::Graph *graph);
  void removeSnapshot(tlp::Graph *graph);

  void clear();

  QSize previewSize() const {
    return _snapshotSize;
  }
  void setPreviewSize(const QSize &newSize);

  void setRenderingParameters(const tlp::GlGraphRenderingParameters &renderingParameters);
  tlp::GlGraphRenderingParameters renderingParameters() const {
    return _renderingParameters;
  }

  tlp::Color backgroundColor() const {
    return _backgroundColor;
  }
  void setBackgroundColor(const tlp::Color &color);

  void treatEvents(const std::vector<tlp::Event> &events) override;
  void treatEvent(const tlp::Event &) override;

private:
  void observe(tlp::Graph *);
  void stopObserving(tlp::Graph *);

  std::set<std::string> generateVisualPropertiesNames(tlp::Graph *graph) const;

signals:
  void snapshotChanged(tlp::Graph *);
  void allSnapshotsChanged();
};

#endif // GRAPHSNAPSHOTMANAGER_H
