template <typename PropType, typename NodeType, typename EdgeType>
CachedPropertyAnimation<PropType, NodeType, EdgeType>::CachedPropertyAnimation(
    tlp::Graph *graph, PropType *start, PropType *end, PropType *out,
    tlp::BooleanProperty *selection, int frameCount, bool computeNodes, bool computeEdges,
    QObject *parent)
    : PropertyAnimation<PropType, NodeType, EdgeType>(graph, start, end, out, selection, frameCount,
                                                      computeNodes, computeEdges, parent) {

  if (this->_computeNodes) {
    for (auto n : this->_graph->nodes()) {
      if (this->equalNodes(this->_end->getNodeValue(n), this->_start->getNodeValue(n))) {
        this->_selection->setNodeValue(n, false);
        // Init the out properties with the default value.
        this->_out->setNodeValue(n, this->_end->getNodeValue(n));
      }
    }
  }

  if (this->_computeEdges) {
    for (auto e : this->_graph->edges()) {
      if (this->equalEdges(this->_end->getEdgeValue(e), this->_start->getEdgeValue(e))) {
        this->_selection->setEdgeValue(e, false);
        // Init the out properties with the default value.
        this->_out->setEdgeValue(e, end->getEdgeValue(e));
      }
    }
  }
}

template <typename PropType, typename NodeType, typename EdgeType>
CachedPropertyAnimation<PropType, NodeType, EdgeType>::~CachedPropertyAnimation() {}

template <typename PropType, typename NodeType, typename EdgeType>
void CachedPropertyAnimation<PropType, NodeType, EdgeType>::frameChanged(int f) {
  if (this->_computeNodes) {
    computedNodeSteps.clear();
    for (auto n : this->_graph->nodes()) {
      if (this->_selection && !this->_selection->getNodeValue(n))
        continue;

      std::pair<NodeType, NodeType> values(this->_start->getNodeValue(n),
                                           this->_end->getNodeValue(n));
      NodeType frameValue;

      if (computedNodeSteps.find(values) == computedNodeSteps.end()) {
        frameValue = this->getNodeFrameValue(values.first, values.second, f);
        computedNodeSteps[values] = frameValue;
      } else
        frameValue = computedNodeSteps[values];

      this->_out->setNodeValue(n, frameValue);
    }
  }

  if (this->_computeEdges) {
    computedEdgeSteps.clear();
    for (auto e : this->_graph->edges()) {
      if (this->_selection && !this->_selection->getEdgeValue(e))
        continue;

      std::pair<EdgeType, EdgeType> values(this->_start->getEdgeValue(e),
                                           this->_end->getEdgeValue(e));
      EdgeType frameValue;

      if (computedEdgeSteps.find(values) == computedEdgeSteps.end()) {
        frameValue = this->getEdgeFrameValue(values.first, values.second, f);
        computedEdgeSteps[values] = frameValue;
      } else
        frameValue = computedEdgeSteps[values];

      this->_out->setEdgeValue(e, frameValue);
    }
  }
}
