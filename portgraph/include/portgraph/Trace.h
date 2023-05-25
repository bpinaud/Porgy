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

#ifndef TRACE_H_
#define TRACE_H_

#include <portgraph/PorgyConstants.h>
#include <portgraph/porgyconf.h>
#include <tulip/GraphDecorator.h>

namespace tlp {
class BooleanProperty;
class IntegerProperty;
class StringProperty;
class DoubleProperty;
}

const static std::string TRACETYPEATTRIBUTE = "Type";
const static std::string PORTNODENAMEATTRIBUTE = "Portnode";
const static std::string DEPTHPROPERTY = "Depth";

class PORTGRAPH_SCOPE Trace : public tlp::GraphDecorator {

public:
  enum TransformationType {
    Simple_Transformation,
    Strategy_Transformation,
    Focus_Transformation,
    Ban_Transformation,
    Update_Transformation
  };
  enum NodeType { Failure_Node, Model_Node };
  enum TransformationParallelInformation { No_parallel, Parallel };

  const static std::unordered_map<TransformationType, const char *, PorgyConstants::EnumClassHash>
      TransformationTypeToString;
  const static std::unordered_map<TransformationParallelInformation, const char *,
                                  PorgyConstants::EnumClassHash>
      TransformationParallelInformationToString;
  const static std::unordered_map<NodeType, const char *, PorgyConstants::EnumClassHash>
      NodeTypeToString;

  enum class TraceType { DerivationTree, Histogram };

  Trace(tlp::Graph *);
  bool redraw(std::string &errMsg, tlp::PluginProgress *pp = nullptr);
  // Access methods
  tlp::node findNode(const std::string &) const;
  //    bool existEdge(const std::string &, const tlp::node, const std::string
  //    &) const;
  //    std::string existEdge(const std::string &, const std::string &) const;
  std::vector<tlp::node> getModelsMetaNodes() const;
  tlp::BooleanProperty *getSelectionProperty();

  /**
        * @brief Find the node corresponding to the given model graph in the
    *trace.
        *
        * @return The node corresponding the model or an invalid node.
        **/
  tlp::node nodeForModel(tlp::Graph *modelGraph) const;

  /**
        * @brief Return the property containing the node type or transformation
    *type for edges.
        *
        * See TransformationType and NodeType for available types.
        **/
  tlp::IntegerProperty *getTypeProperty() const;

  // Properties for simple transformations
  /**
        * @brief Return the property containing the parallel information for the
    *transformation.
        **/
  tlp::IntegerProperty *getTransformationParallelInformationProperty() const;
  /**
        * @brief Return the property containing the rule used for
    *transformations.
        **/
  tlp::IntegerProperty *getTransformationRuleProperty() const;
  /**
        * @brief Return the property containing the instance used for
    *transformations.
        **/
  tlp::IntegerProperty *getTransformationInstanceProperty() const;

  // Properties for strategies transformations
  /**
        * @brief Return the property containing the strategy code for strategy
    *transformations
        **/
  tlp::StringProperty *getTransformationStrategyProperty() const;

  /**
        * @brief Return the property containing the p property name for
    *transformations
        **/
  tlp::StringProperty *getTransformationPPropertyNameProperty() const;
  tlp::StringProperty *getTransformationBanPropertyNameProperty() const;

  /**
        * @brief Convinience function get the type of the transformation for the
    *transformation edge. The transformation edge must be an element of the
    *trace graph.
        **/
  TransformationType getTransformationType(const tlp::edge &transformation) const;

  /**
        *
        **/
  tlp::Graph *getRuleForTransformation(tlp::edge e) const;
  tlp::Graph *getRuleForTransformation(tlp::node n) const;

  /**
        * @brief Find the rule used in the transformation e.
        **/
  std::string getRuleNameForTransformation(tlp::edge e) const;

  /**
   * @brief Return the instance of the source model used to rewrite it into the
   *destination model.
   **/
  tlp::Graph *getInstanceForTransformation(tlp::edge e) const;

  /**
        * @brief Search the source P property used in the transformation e.
        **/
  inline bool getSourcePPropertyForTransformation(tlp::edge e, tlp::BooleanProperty *result) const {
    return getBanFocusPropertyForTransformation(e, result, SourceNode, false);
  }

  /**
        * @brief Search the target P property used in the transformation e.
        **/
  inline bool getTargetPPropertyForTransformation(tlp::edge e, tlp::BooleanProperty *result) const {
    return getBanFocusPropertyForTransformation(e, result, TargetNode, false);
  }

  /**
        * @brief Search the source Ban property used in the transformation e.
        **/
  inline bool getSourceBanPropertyForTransformation(tlp::edge e,
                                                    tlp::BooleanProperty *result) const {
    return getBanFocusPropertyForTransformation(e, result, SourceNode, true);
  }

  /**
        * @brief Search the target Ban property used in the transformation e.
        **/
  inline bool getTargetBanPropertyForTransformation(tlp::edge e,
                                                    tlp::BooleanProperty *result) const {
    return getBanFocusPropertyForTransformation(e, result, TargetNode, true);
  }

  /**
   * @brief Get the left members used in the transformation e and fill the
   *result property with them.
   **/
  bool getLeftMembersForTransformation(tlp::edge e, tlp::BooleanProperty *result) const;
  /**
   * @brief Get the created members with the transformation e.
   **/
  bool getRightMembersForTransformation(tlp::edge e, tlp::BooleanProperty *result) const;

  // Modification methods.
  /**
        * @brief Create a new strategy transformation edge and set properties.
        **/
  tlp::edge addStrategyTransformationEdge(const tlp::node, const tlp::node, const std::string &p,
                                          const std::string &ban,
                                          const std::string &strategyCode) const;
  /**
        * @brief Create a new simple transformation edge and set properties.
        **/
  tlp::edge addSimpleTransformationEdge(const tlp::node, const tlp::node, const std::string &p,
                                        const std::string &ban, tlp::Graph *instanceGraph,
                                        const int rule_graph_id) const;
  tlp::edge addOtherEdge(const tlp::node src, const tlp::node tgt,
                         TransformationType transformationType) const;

  tlp::edge addFocusTransformationEdge(const tlp::node source, const tlp::node target,
                                       const std::string &p, tlp::Graph *instanceGraph,
                                       const PorgyConstants::strat_focus_op op) const;

  tlp::node createMetaNode(Graph *subGraph, bool multiEdges = true,
                           bool delAllEdge = true, bool allGrouped=true) override;
  /**
        * @brief Add a failure node
        **/
  tlp::node addFailureNode(const std::string &, const int rule_graph_id,
                           const std::string &PProperty,
                           const std::string &BanProperty) const;

  bool isFailureNode(const tlp::node &n) const;
  bool isModelNode(const tlp::node &n) const;

  bool isStrategyEdge(const tlp::edge &e) const;
  bool isSimpleTransformationEdge(const tlp::edge &e) const;

  std::list<std::string> computeStrategy(tlp::node from, tlp::node to) const;

  /**
        * @brief Compute the strategy for the path given in parameters. If one
    *of the node in the path does not exist or if an edge between two next
    *elements does not exist returns an empty string.
        **/
  std::list<std::string> computeStrategy(const std::list<tlp::edge> &path) const;

  /**
        * @brief Try to compute the strategy code from a selection.
        * If a strategy can be computed from the selection this two values will
    *be filled.
        **/
  std::string computeStrategyText(tlp::BooleanProperty *selection, tlp::node &sourceNode,
                                  tlp::node &destinationNode) const;
  tlp::IntegerProperty *depthProperty();

  /**
        * @brief Return the type of the graph.
        **/
  TraceType type() const;

  std::vector<std::string> followedPortnodes() const;
  std::vector<tlp::IntegerProperty *> followedPortnodesProperties();

  /**
        * @brief Follow the given portnode : count all
        **/
  std::list<tlp::IntegerProperty *> followPortNode(const std::list<std::string> &toFollow);

  /**
      * @brief Used in NewTrace to build a scatter plot, this variant allows to
    *follow the number of elements "typeElement"
      * for which the value of the property "analysedPropertyName" is
    *"comparisonOperator" than "propertyValue"
      *
      * @param newPropertyName Used to name the returned property
      * @param typeElement
      * @param analysedPropertyName
      * @param comparisonOperator
      * @param propertyValue
      *
      * @return A new property specifying the number of occurences verifying the
    *condition specified.
      **/
  tlp::IntegerProperty *followPropertyValue(const std::string &newPropertyName,
                                            const std::string &typeElement,
                                            const std::string &analysedPropertyName,
                                            const std::string &comparisonOperator,
                                            const std::string &propertyValue);

  /**
      * @brief Used in NewTrace to compute the ratio of the property
    *"currentPropertyName" at step t against the property
    *"antecedentPropertyName" at step t-1.
      * DFS performed by ratioComputingDFS()
      *
      * @param antecedentPropertyName
      * @param currentPropertyName
      *
      * @return A new property specifying the ratio
      **/
  tlp::DoubleProperty *computePropertyRatioValue(const std::string &antecedentPropertyName,
                                                 const std::string &currentPropertyName);

  void ratioComputingDFS(tlp::node n, double previousValue,
                         tlp::PropertyInterface *antecedentProperty,
                         tlp::PropertyInterface *currentProperty, tlp::DoubleProperty *prop);

private:
  enum EdgeExtremitiy { SourceNode, TargetNode };

  /**
        * @brief Create an edge between two states and set generals information
    *on newly created edge.
        **/
  tlp::edge addTransformationEdge(const tlp::node, const tlp::node, const std::string &pname,
                                  const std::string &ban_name,
                                  TransformationType transformationType) const;

  /**
       * @brief Search the P property used in the transformation e. You can
    *search P either in source or in destination model using the gPInSourceModel
    *flag.
       * @return The property with the P value or nullptr if the transformation
    *don't exists.
       **/
  bool getBanFocusPropertyForTransformation(tlp::edge e, tlp::BooleanProperty *result,
                                            EdgeExtremitiy origin, const bool isBan) const;
};

#endif /* TRACE_H_ */
