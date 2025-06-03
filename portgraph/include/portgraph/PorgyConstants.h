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
#ifndef PORGYCONSTANTS_H
#define PORGYCONSTANTS_H

#include <string>
#include <unordered_set>

#include <tulip/Color.h>
#include <tulip/TulipViewSettings.h>

namespace PorgyConstants {

#define LAYOUT_ALGO "FM^3 (OGDF);GEM (Frick);Sugiyama (OGDF);Tree Leaf;none"

// Algorithm names
const static char APPLY_STRATEGY_ALGORITHM[] = "StrategySpirit";
const static char APPLY_RULE_ALGORITHM[] = "RuleAlgorithm";
const static char APPLY_RULE_CONDITION[] = "RuleCondition";
const static char REDRAW_GRAPH[] = "Redraw PortGraph Model";
const static char REDRAW_RULE[] = "Redraw PortGraph Rule";
const static char REDRAW_TRACE[] = "Redraw Trace Tree";
const static char CHECK_APPLY_RULE[] = "Check and Apply a Rule";
const static char SIMPLECHECKAPPLY[] = "Check and Apply a Rule (Simplified version)";
const static char APPLY_RULE[] = "Apply a rule";
const static char CHECK_RULE[] = "Check Rule";
const static char FIND_ISOMORPHIC_GRAPHS[] = "Find Isomorphic graphs";

const static std::string LAYOUTALGORITHM = "LayoutAlgorithm";
const static char NONE[] = "none";
const static char GEM[] = "GEM (Frick)";
const static char GEM_VERSION[] = "1.2";
const static char SUGIYAMA[] = "Sugiyama (OGDF)";

const static char RULES_GRAPH_NAME[] = "Rules";
const static char GRAPHS_GRAPH_NAME[] = "Models";
const static char TRACE_GRAPH_NAME[] = "TraceRoot";
const static char SMALL_MULTIPLES_NAME[] = "SmallMultiples";
const static char TRACEMAIN[] = "TraceMain";

const static char HISTOGRAM_NAME[] = "Histogram view";
const static char SCATTER_PLOT_NAME[] = "Scatter Plot 2D view";

const static char GRAPH_VIEW_NAME[] = "Graph view";
const static char RULE_VIEW_NAME[] = "Rule view";
const static char TRACE_VIEW_NAME[] = "Derivation Tree view";
const static char SMALL_MULTIPLE_VIEW_NAME[] = "Porgy Small Multiples view";
const static char ANIMATION_VIEW_NAME[] = "Porgy Animations view";

const static char CATEGORY_NAME[] = "PORGY";
const static char CATEGORY_NAME_INTERNAL[] = "PORGY (internal)";

// name of some properties
const static std::string TAG = "tag";

// Rule properties
const static std::string ARITY = "Arity";
const static std::string INARITY = "InArity";
const static std::string OUTARITY = "OutArity";
const static std::set<std::string> ArityProps = {ARITY, INARITY, OUTARITY};
const static std::string M = "M"; // property to indicate if a RHS member has to
                                  // be put in the current position set after
                                  // rewriting
const static std::string N = "N"; // property to indicate if a RHS member has to
                                  // be put in the current ban set after
                                  // rewriting
const static std::string W =
    "W"; // property to indicate if the image of a node from LHS has to be in P
const static std::string RULE_ALGORITHM = "Rule Algorithm";
const static std::string RULE_CONDITION = "Rule Condition";

// Trace graph properties
const static std::string TRANSFORMATION_RULE_PROPERTY =
    "Transformation_Rule"; // An integer property containing the id of the rule
                           // graph for the transformation.
const static std::string TRANSFORMATION_INSTANCE_PROPERTY =
    "Transformation_Instance"; // An integer property containing the id of the
                               // instance graph for the transformation.
const static std::string TRANSFORMATION_PARALLEL_INFORMATION_PROPERTY =
    "Transformation_Parallel_Information";
const static std::string TRANSFORMATION_STRATEGY_PROPERTY = "Transformation_Strategy";
const static std::string TRANSFORMATION_PPROPERTYNAME_PROPERTY = "Transformation_PPropertyName";
const static std::string TRANSFORMATION_BANPROPERTYNAME_PROPERTY =
    "Transformation_Ban_PropertyName";
// const static std::string
// TRANSFORMATION_PROBA_PROPERTY="Transformation_Proba_Property";
const static std::string TYPE_PROPERTY = "Type"; // Integer property.
// const static std::string MATCHING_PROPERTY_DEFAULT = "viewLabel";
const static std::string GUI_COLLAPSED_ELEMENT_ID = "Collapsed_Element_Id_Property";

const static char POSITION[] = "Property for Position"; // property name for the Position set
const static char BAN[] = "Property for Ban";           // property name for the ban set
const static char P_PREFIX[] = "P";
const static char BAN_PREFIX[] = "Q";
const static char STRATEGY[] = "Strategy";
const static char DEBUG[] = "Debug";

// portnode configuration
const static unsigned CENTER_ALPHA = 100;
const static unsigned PORT_ALPHA = 255;
const static unsigned PORTNODEEDGE_ALPHA = 0;
const static tlp::NodeShape::NodeShapes CENTER_SHAPE = tlp::NodeShape::RoundedBox;
const static tlp::NodeShape::NodeShapes BRIDGE_CENTER_SHAPE = tlp::NodeShape::Icon;
#define CENTER_ICON "far-circle-right"
const static tlp::NodeShape::NodeShapes PORT_DEFAULT_SHAPE = tlp::NodeShape::Square;

const static tlp::Color DEFAULT_COLOR = tlp::Color(0, 0, 255);
// const static tlp::Color DERIVATION_EDGE_PROBABILISTIC_OPERATION_COLOR =
// tlp::Color::BlueViolet;
const static tlp::Color DERIVATION_STRATEGY_EDGE_COLOR = tlp::Color::Green;

// constant for the drawing of portgraphs
const static unsigned SPACE = 2;

// name of the property which indicates if a node/edge is new after a rewriting
// operation
const static std::string NEW = "new";

// name of the property which indicates if a node/edge is new for a whole
// strategy application
const static std::string NEW_STRATEGY_APPLICATION = "New Strategy Application";

struct EnumClassHash {
  template <typename T>
  std::size_t operator()(T t) const {
    return static_cast<std::size_t>(t);
  }
};

// identifiers for the type of the nodes
const static std::string PORTNODE_TYPE = "NodeType";
enum PortNodeType { NO_TYPE, CENTER, PORT, ANTI };

const static std::unordered_map<PortNodeType, const char *, EnumClassHash>
    PortNodeTypeToString({{PorgyConstants::NO_TYPE, "None"},
                          {PorgyConstants::PORT, "Port"},
                          {PorgyConstants::CENTER, "Center"},
                          {PorgyConstants::ANTI, "Anti-Edge"}});

// rule side property
const static std::string RULE_SIDE = "RuleSide";
enum RuleSide { NO_SIDE, SIDE_LEFT, SIDE_RIGHT, SIDE_BRIDGE, SIDE_BRIDGE_PORT, SIDE_BRIDGE_OPP };

const static std::unordered_map<RuleSide, const char *, EnumClassHash>
    RuleSideToString({{PorgyConstants::SIDE_BRIDGE, "Bridge"},
                      {PorgyConstants::SIDE_LEFT, "Left"},
                      {PorgyConstants::SIDE_RIGHT, "Right"},
                      {PorgyConstants::SIDE_BRIDGE_OPP, "Bridge opp"},
                      {PorgyConstants::SIDE_BRIDGE_PORT, "Bridge port"},
                      {PorgyConstants::NO_SIDE, "None"}});

// graphs names
const static std::string GRAPH_PREFIX = "G";
const static std::string INITIAL_GRAPH = GRAPH_PREFIX + "0";
const static std::string NUMBER = "last number";
const static std::string MATCHING_PROPERTIES = "Matching Properties";
const static std::string MATCHING_PROPERTIES_SUFFIX = "_isused";
const static std::string MATCHING_PROPERTIES_COMPARISON_SUFFIX = "_comp";
const static std::string PROPERTIES_TO_DISPLAY = "Properties To Display";

const static std::unordered_set<std::string> PROPERTY_MANAGER_FORBIDDEN{PORTNODE_TYPE, RULE_SIDE,
                                                                        MATCHING_PROPERTIES_SUFFIX};

// rule comparison operator
enum class ComparisonOp { EQ, NEQ, GT, GEQ, LT, LEQ };

const static std::unordered_map<ComparisonOp, const char *, EnumClassHash>
    RuleComparisonToString({{ComparisonOp::EQ, "=="},
                            {ComparisonOp::NEQ, "!="},
                            {ComparisonOp::GT, ">"},
                            {ComparisonOp::GEQ, ">="},
                            {ComparisonOp::LT, "<"},
                            {ComparisonOp::LEQ, "<="}});

// Strategy
const static std::string FAILURE_NODE_ADDED = "Failure node added";
const static char STRAT_MACRO_REGEXP[] = "#([^#]*)#";

// checkrule
const static char maximum_number_of_instances[] = "Maximum number of instances to find";
const static char minimum_number_of_instances[] = "Minimum number of instances to find";
// const static std::string all_instances_found="All LHS instances already found
// before";
const static char isCondition[] = "Is Condition?";
const static char applyConnectedComponentPacking[] = "Apply Connected Components Packing?";
const static char newData[] = "new data";
const static char newModelAll[] = "new model vector";
const static char layoutNewModel[] = "Layout new model";
const static char checkRule[] = "Check rule?";
const static char applyRule[] = "Apply rule if possible?";
const static char RuleName[] = "Rule Name";

const static float hidden_alpha = 15.f;

// graph attributes
// Position and Ban Properties
const static std::string P_NAME = "P Property";
const static std::string BAN_NAME = "Ban Property";
const static std::string NO_P = "NoPosition";
const static std::string NO_BAN = "NoBan";
// rulename graph attribute
const static std::string RULENAME = "rulename";
const static std::string RULE_IMPORT_ALLOWED[] = {"name", "filename", LAYOUTALGORITHM,
                                                  RULE_ALGORITHM, RULE_CONDITION};
const static unsigned int RULE_IMPORT_ALLOWED_SIZE = 4;
// parallel rule
const static std::string IS_PARALLEL_RULE = "Is Parallel rule?";
const static std::string PARALLEL_RULENAME = "Parallel rule name";
// edge orientation
const static std::string EDGE_ORIENTATION_ENABLED = "Edge orientation enabled";

enum class strat_focus_op { SETPOS, SETBAN, COMPARISON };
// type of rule execution => condition or not.
enum class state { OTHER = 0, IF, ORELSE };
}

#endif // PORGYCONSTANTS_H
