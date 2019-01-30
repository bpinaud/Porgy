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
#ifndef PORGYSMALLMULTIPLESTRACEVIEW_H
#define PORGYSMALLMULTIPLESTRACEVIEW_H

#include "abstracttraceview.h"
#include "graphstatemanager.h"

#include <tulip/GlGraphRenderingParameters.h>
#include <tulip/View.h>

#include <QAbstractAnimation>

namespace tlp {
class Graph;
class GlMainWidget;
class GlScene;
}

class PorgyAnimationViewQuickAccessBar;
class PorgyAnimationConfigurationWidget;

/**
  * @brief View to visualize trace graph evolution with animation and small
  *multiples. This view can only display Trace graphs with only one single path.
  *If a node in the trace graph have more than one child the view don't display
  *the graph.
  **/
class PorgyAnimationTraceView : public AbstractTraceView {
  Q_OBJECT

  friend class GlSceneMorphing;

  PorgyAnimationViewQuickAccessBar *_bar;

  // Configuration widgets
  PorgyAnimationConfigurationWidget *_configurationWidget; // The generic configuration widget.

  unsigned _currentState;                // The index of the current state in the GraphStateManager
  QAbstractAnimation *_currentAnimation; // The current animation.
  bool _goNextStep;                      // Flag to control if at the end of an animation we had to
                                         // go to the next step.
  GraphStateManager _states;             // The states
  GraphVisualProperties _visualProperties; // Contain the overloaded visual
                                           // properties to perform animations
                                           // without having to modify original
                                           // graph properties.

public:
  PLUGININFORMATION(PorgyConstants::ANIMATION_VIEW_NAME, "Bruno Pinaud", "17/01/2017", "", "1.0",
                    PorgyConstants::CATEGORY_NAME)
  PorgyAnimationTraceView(const tlp::PluginContext *);
  ~PorgyAnimationTraceView() override;

  QList<QWidget *> configurationWidgets() const override;

  tlp::DataSet state() const override;
  void setState(const tlp::DataSet &) override;
  tlp::QuickAccessBar *getQuickAccessBarImpl() override;
  void setupWidget() override;

protected slots:
  void graphChanged(tlp::Graph *) override;

private:
  tlp::GlGraphRenderingParameters renderingParameters() const;

  //    void fillContextMenu(QMenu* menu,const QPointF &);

  /**
   * @brief setAnimationData updates the data displayed by the animation item.
   * This function is used to switch the graph during the animation process.
   * @param graph
   * @param properties
   */
  void setAnimationData(tlp::Graph *graph, GraphVisualProperties &properties);

  /**
   * @brief removeRedrawTriggers add the camera, the graph and all the graph
   * properties as redraw triggers.
   * @param scene
   */
  void addRedrawTriggers(tlp::GlScene *scene);

private slots:
  // Animation control functions.
  /**
   * @brief start Begin to animate the states from the current state to the
   * final state or until the user click on pause or stop
   */
  void start();
  /**
   * @brief pause Pause the current animation. Click on play to launch it again.
   */
  void pause();
  /**
   * @brief stop Stop the animation
   */
  void stop();
  /**
   * @brief seekBackward go one step backwark from the current state if possible
   */
  void seekBackward();
  /**
   * @brief seekForward go one step forward from the current state if possible
   */
  void seekForward();
  /**
   * @brief seekFirst go to the first state
   */
  void seekFirst();
  /**
   * @brief seekFirst go to the last state
   */
  void seekLast();

  /**
    * @brief Handle the animation states.
    **/
  void currentAnimationStateChanged(QAbstractAnimation::State newState,
                                    QAbstractAnimation::State oldState);

  /**
    * @brief Force to regenerate intermediary states
    **/
  void updateIntermediateStates();

private:
  /**
    * @brief Animate from the current state to the new state.
    **/
  void animate(unsigned int to);
  /**
    * @brief Build a QabstractAnimation between the current state and the to
    *state. If an animation is already running stop them. The current state is
    *built from the displayed graph and visual properties.
    * @param to The final state
    **/
  QAbstractAnimation *buildMorphingBetweenStates(unsigned int to);
};

#endif // PORGYSMALLMULTIPLESTRACEVIEW_H
