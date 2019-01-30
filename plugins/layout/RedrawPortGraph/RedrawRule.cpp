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

#include "LayoutUtils.h"

#include <portgraph/Bridge.h>
#include <portgraph/BridgePort.h>
#include <portgraph/PorgyConstants.h>
#include <portgraph/PortGraphRule.h>
#include <portgraph/PortNode.h>

#include <tulip/BooleanProperty.h>
#include <tulip/ColorProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/LayoutProperty.h>
#include <tulip/PropertyAlgorithm.h>
#include <tulip/SizeProperty.h>

using namespace std;
using namespace tlp;

namespace {
const char *paramHelp[] = {
    // node size
    "This parameter defines the property used for node's sizes.",

    // node color
    "This parameter defines the property used for node/edge's color.",

    // border color
    "This parameter defines the property used for node/edge border color."};
}

class RedrawRule : public LayoutAlgorithm {

  typedef pair<node, int> pairnodeint;
  typedef pair<node, map<node, int>> pairnodemapnodeint;

  PortGraphRuleDecorator *_dec;

  Graph *createAbstractGraph(PortGraphRule &p) const {
    Graph *sub = graph->addSubGraph();
    IntegerProperty *side = _dec->getSideProperty();
    // add portnode centers
    for (PortNode *pn : p.getPortNodes()) {
      sub->addNode(pn->getCenter());
    }
    sub->addNode(p.getBridge()->getCenter());
    // add an edge if centers are connected via their respective ports
    // copy required because edges may be added
    vector<edge> v(graph->edges());
    for (edge e : v) {
      const pair<node, node> &ends = graph->ends(e);
      if ((_dec->getSide(e) != PorgyConstants::SIDE_BRIDGE_OPP) &&
          (!PortNodeBase::isCenter(ends.second, graph)) &&
          (!PortNodeBase::isCenter(ends.first, graph))) {
        const PortNodeBase *source, *target;
        if (Bridge::isBridge(ends.first, graph))
          source = p.getBridge();
        else
          source = p[ends.first];
        if (Bridge::isBridge(ends.second, graph))
          target = p.getBridge();
        else
          target = p[ends.second];
        if (source != target) {
          edge e1 = sub->addEdge(source->getCenter(), target->getCenter());
          // copie de rule_side
          side->setEdgeValue(e1, side->getEdgeValue(e));
        }
      }
    }
    return sub;
  }

  /*translation des centres vers leur place. Inutile de s'occuper
   * des ports.
   */
  void Translate(PortGraphRule &p, SizeProperty *size) const {
    const double translation_offset = 5;
    double extremeL = HUGE_VAL, extremeR = -HUGE_VAL;
    node n;
    for (PortNode *pn : p.getPortNodes()) {
      n = pn->getCenter();
      double x = result->getNodeValue(n).getX();
      double width = size->getNodeValue(n).getW();

      if (_dec->getSide(n) == PorgyConstants::SIDE_LEFT) {
        extremeR = max(extremeR, x + width / 2);
      } else if (_dec->getSide(n) == PorgyConstants::SIDE_RIGHT) {
        extremeL = min(extremeL, x - width / 2);
      }
    }

    // calcul du dcalage
    double offsetLeftSide = -translation_offset - extremeR;
    double offsetRightSide = translation_offset - extremeL;

    // translation des sommets
    for (PortNode *pn : p.getPortNodes()) {
      n = pn->getCenter();
      Coord posn = result->getNodeValue(n);
      if (_dec->getSide(n) == PorgyConstants::SIDE_LEFT)
        posn.setX(posn.getX() + offsetLeftSide);
      else if (_dec->getSide(n) == PorgyConstants::SIDE_RIGHT)
        posn.setX(posn.getX() + offsetRightSide);
      result->setNodeValue(n, posn);
    }
  }

  // Once the right hand side is drawn, we can use this function to draw the
  // left hand side by symetry
  void drawLeftHandSide(PortGraphRule &p) const {
    BooleanProperty init(graph); // tmp property
    float extremeLeft = 0., minY = 0., maxY = 0.;
    float currentX = extremeLeft;
    float currentY = minY;

    init.setAllNodeValue(false);
    // Map that takes a left node in param and returns a map that contains the
    // number of edges in common with each right node
    map<node, map<node, int>> edgeCounter;
    for (BridgePort *bp : p.getBridge()->getBridgePorts()) {
      node left = p[bp->getLeft()]->getCenter();
      node right = p[bp->getRight()]->getCenter();
      ++edgeCounter[left][right];
    }

    // search for the most connected node in the right hand side of the rule
    for (const auto &mp : edgeCounter) { // left node
      int maxEdges = -1;
      node rightNode;
      for (const auto &pni : mp.second) { // right node
        rightNode = pni.first;
        if ((pni.second >= maxEdges) && (init.getNodeValue(rightNode) == false)) {
          maxEdges = pni.second;
        }
      }

      assert((maxEdges > -1) || (init.getNodeValue(rightNode) == true));
      if (!init.getNodeValue(rightNode)) {
        Coord rightCoord = result->getNodeValue(rightNode);
        rightCoord.setX(-rightCoord[0]);
        result->setNodeValue(mp.first, rightCoord);

        maxY = max(maxY, rightCoord.getY());
        minY = min(minY, rightCoord.getY());
        extremeLeft = max(extremeLeft, result->getNodeValue(mp.first).getX());

        init.setNodeValue(mp.first, true);
        init.setNodeValue(rightNode, true);
      }
    }
    for (PortNode *pn : p.getMember(PorgyConstants::SIDE_LEFT)) {
      node left = pn->getCenter();
      node right;
      for (PortNode *pn2 : p.getMember(PorgyConstants::SIDE_RIGHT)) {
        right = pn2->getCenter();
        if (pn->getName() == pn2->getName() && init.getNodeValue(right) == false) {
          init.setNodeValue(left, true);
          init.setNodeValue(right, true);
          Coord rightCoord = result->getNodeValue(right);
          rightCoord.setX(-rightCoord[0]);
          result->setNodeValue(left, rightCoord);

          maxY = max(maxY, result->getNodeValue(left).getY());
          minY = min(minY, result->getNodeValue(left).getY());
          extremeLeft = max(extremeLeft,
                            result->getNodeValue(left).getX()); // + leftDeg* PortNode::space + 2);
          break;
        }
      }
    }

    if (maxY - minY == 0) {
      maxY = 10;
      minY = -maxY;
    }
    currentX = extremeLeft;
    currentY = minY;

    for (PortNode *pn : p.getMember(PorgyConstants::SIDE_LEFT)) {
      node left = pn->getCenter();
      if (init.getNodeValue(left) == true)
        continue;
      result->setNodeValue(left, Coord(currentX, currentY, 0));
      currentY += 2 * PorgyConstants::SPACE;
      extremeLeft = max(extremeLeft, result->getNodeValue(left).getX());
      if (currentY >= maxY) {
        currentY = minY;
        currentX = extremeLeft + 1;
      }
    }
  }

  bool drawSide(Graph *g) const {
    string errMsg;

    DoubleProperty elength(g);
    elength.setAllEdgeValue(5);
    dataSet->set("Edge Length Property", &elength);
    dataSet->set("Unit edge length", 1);
    if (!g->applyPropertyAlgorithm("FM^3 (OGDF)", result, errMsg, dataSet,pluginProgress)) {
      cerr << __PRETTY_FUNCTION__ << ": " << __LINE__ << "=> Pb avec FM^3 : " << errMsg << endl;
      return false;
    }

    // fast overlap removal
    dataSet->set("layout", result);
    dataSet->set<double>("x border", 3.0f);
    dataSet->set<double>("y border", 3.0f);
    if (!g->applyPropertyAlgorithm("Connected Component Packing", result, errMsg, dataSet,pluginProgress)) {
      cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << "Pb avec Fast Overlap Removal: " << errMsg
           << endl;
      return false;
    }
    dataSet->set("layout", result);
    dataSet->set<double>("x border", 3.0f);
    dataSet->set<double>("y border", 3.0f);
    if (!g->applyPropertyAlgorithm("Fast Overlap Removal", result, errMsg, dataSet,pluginProgress)) {
      cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << "Pb avec Fast Overlap Removal: " << errMsg
           << endl;
      return false;
    }
    result->perfectAspectRatio(g);
    return true;
  }

  void drawSameSizes(PortGraphRule &) const;

public:
  PLUGININFORMATION(PorgyConstants::REDRAW_RULE, "Bruno Pinaud", "14/03/12", "Comments", "1.0",
                    PorgyConstants::CATEGORY_NAME)

  RedrawRule(const tlp::PluginContext *context) : LayoutAlgorithm(context), _dec(nullptr) {
    addDependency("FM^3 (OGDF)", "1.2");
    addDependency("Connected Component Packing", "1.0");
    addDependency("Fast Overlap Removal", "1.3");

    addInOutParameter<SizeProperty>("node size", paramHelp[0], "viewSize");
    addInOutParameter<ColorProperty>("Color", paramHelp[1], "viewColor");
    addInOutParameter<ColorProperty>("Bordercolor", paramHelp[2], "viewBorderColor");
  }

  ~RedrawRule() override {
    delete _dec;
  }

  bool run() override {
    PortGraphRule p(graph);
    _dec = new PortGraphRuleDecorator(graph);
    Graph *sub = createAbstractGraph(p);

    // split graph in LHS, RHS and bridge
    string errMsg;
    tuple<Graph *, Graph *, Graph *> lhs_rhs(
        PortGraphRule::extractLHSRHS(sub, pluginProgress, errMsg));

    // start by drawing RHS
    if (get<1>(lhs_rhs) != nullptr) {
      drawSide(get<1>(lhs_rhs));
      // draw LHS if it exists (possible when editing a rule)
      if (get<0>(lhs_rhs) != nullptr)
        drawLeftHandSide(p);
    }
    // no RHS. Do a layout of LHS
    else if (get<0>(lhs_rhs) != nullptr) {
      drawSide(get<0>(lhs_rhs));
    } else // nothing to do
      return true;

    PortGraphRule::cleanLHSRHS(sub);
    removeAbstractGraph(sub);

    SizeProperty *nodeSize = graph->getProperty<SizeProperty>("viewSize");
    ColorProperty *viewColor = graph->getProperty<ColorProperty>("viewColor");
    ColorProperty *bordercolor = graph->getProperty<ColorProperty>("viewBorderColor");

    if (dataSet != nullptr) {
      dataSet->get("node size", nodeSize);
      dataSet->get("Color", viewColor);
      dataSet->get("Bordercolor", bordercolor);
    }

    Translate(p, nodeSize);

    //    PortNode *pn;
    //    forEach(pn, p.getPortNodes()) {
    //        if(!Bridge::isBridge(pn->getCenter(), graph)) {
    //            pn->draw(nodeSize, viewColor, result, bordercolor);
    //        }
    //    }
    // temp code
    p.getBridge()->update();

    p.getBridge()->draw(nodeSize, viewColor, result, bordercolor);
    p.getBridge()->setPositions(result);
    p.drawSameSizes(nodeSize, result);

    return true;
  }

  bool check(string &errMsg) override {
    if (!PorgyTlpGraphStructure::isRuleGraph(graph)) {
      errMsg = "Graph is not a rule";
      return false;
    }
    return true;
  }
};

PLUGIN(RedrawRule)
