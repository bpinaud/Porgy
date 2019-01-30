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
#ifndef ABSTRACTGRAPHMORPHING_H
#define ABSTRACTGRAPHMORPHING_H

#include <tulip/Animation.h>

#include <porgy/GraphVisualProperties.h>

namespace tlp {
class Graph;
}

class VisualPropertiesMorphing;

/**
  * @brief Abstract class to help to perform graph morphing. Implements
  *displayedGraph and switchDisplayedGraph to make it work.
  **/
class AbstractGraphMorphing : public tlp::Animation {
public:
  AbstractGraphMorphing(tlp::Graph *fromGraph, const GraphVisualProperties &fromVisualProperties,
                        GraphVisualProperties &fromOutVisualProperties, tlp::Graph *toGraph,
                        const GraphVisualProperties &toVisualProperties,
                        GraphVisualProperties &toOutVisualProperties, int frameNumber,
                        VisualPropertiesFlags propertiesToAnimate =
                            VisualPropertiesFlags(ElementColor | ElementSize | ElementLayout),
                        QObject *parent = nullptr);
  ~AbstractGraphMorphing() override;
  void frameChanged(int frame) override;

protected:
  /**
    * @brief Returns the graph displayed by the widget.
    **/
  virtual tlp::Graph *displayedGraph() = 0;
  /**
    * @brief Update the displayed graph and visual properties.
    **/
  virtual void switchDisplayedGraph(tlp::Graph *graphToDisplay,
                                    GraphVisualProperties &overloadedProperties) = 0;

private:
  void setFrameCount(int frameCount) override;

  template <typename PropertyType, typename NodeType, typename EdgeType>
  void computeMiddleValueForProperty(tlp::Graph *fromGraph, PropertyType *from, tlp::Graph *toGraph,
                                     PropertyType *to, PropertyType *outFrom,
                                     PropertyType *outTo) const;
  template <typename T>
  T computeMiddleValue(const T &from, const T &to) const;

  void computeDisparitionApparition(tlp::Graph *fromGraph, tlp::Graph *toGraph,
                                    GraphVisualProperties &fromProperties,
                                    GraphVisualProperties &toProperties);
  void hideElements(tlp::Graph *fromGraph, tlp::Graph *toGraph,
                    GraphVisualProperties &visualProperties);

  void initInternalAnimations(const GraphVisualProperties &from, const GraphVisualProperties &to);
  int computeSwitchFrame() const;
  int computeFirstAnimationFrameNumber() const;
  int computeSecondAnimationFrameNumber() const;
  tlp::Graph *fromGraph;
  tlp::Graph *toGraph;

  VisualPropertiesFlags animationProperties;
  int switchFrame;
  VisualPropertiesMorphing *firstAnimation;
  GraphVisualProperties firstAnimationOutVisualProperties;
  VisualPropertiesMorphing *secondAnimation;
  GraphVisualProperties secondAnimationOutVisualProperties;
};
#include "AbstractGraphMorphing.cxx"

#endif // ABSTRACTGRAPHMORPHING_H
