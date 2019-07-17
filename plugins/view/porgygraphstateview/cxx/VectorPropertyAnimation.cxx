template <typename PropType, typename RealType, typename VectorType, unsigned int SIZE>
VectorPropertyAnimation<PropType, RealType, VectorType, SIZE>::VectorPropertyAnimation(
    tlp::Graph *graph, PropType *start, PropType *end, PropType *out,
    tlp::BooleanProperty *selection, int frameCount, bool computeNodes, bool computeEdges,
    QObject *parent)
    : CachedPropertyAnimation<PropType, RealType, RealType>(
          graph, start, end, out, selection, frameCount, computeNodes, computeEdges, parent) {

  assert(frameCount > 1);

  if (this->_computeNodes) {
    for (auto n : this->_graph->nodes()) {
      if (this->_selection && !this->_selection->getNodeValue(n))
        continue;

      std::pair<PropVector, PropVector> values(PropVector(this->_start->getNodeValue(n)),
                                               PropVector(this->_end->getNodeValue(n)));

      if (steps.find(values) == steps.end()) {
        tlp::Vector<double, SIZE> stepsVector;

        for (unsigned int i = 0; i < SIZE; ++i)
          stepsVector[i] =
              (double(values.second[i]) - double(values.first[i])) * 1. / (frameCount - 1);

        steps[values] = stepsVector;
      }
    }
  }

  if (this->_computeEdges) {
    for (auto e : this->_graph->edges()) {
      if (this->_selection && !this->_selection->getEdgeValue(e))
        continue;

      std::pair<PropVector, PropVector> values(PropVector(this->_start->getEdgeValue(e)),
                                               PropVector(this->_end->getEdgeValue(e)));

      if (steps.find(values) == steps.end()) {
        tlp::Vector<double, SIZE> stepsVector;

        for (unsigned int i = 0; i < SIZE; ++i)
          stepsVector[i] =
              (double((values.second[i]) - double(values.first[i]))) * 1. / (frameCount - 1);

        steps[values] = stepsVector;
      }
    }
  }
}

template <typename PropType, typename RealType, typename VectorType, unsigned int SIZE>
RealType VectorPropertyAnimation<PropType, RealType, VectorType, SIZE>::getNodeFrameValue(
    const RealType &startValue, const RealType &endValue, int frame) {
  PropVector result(static_cast<PropVector>(startValue));
  std::pair<PropVector, PropVector> values(result, static_cast<PropVector>(endValue));
  tlp::Vector<double, SIZE> stepVal(steps[values]);

  for (unsigned i = 0; i < SIZE; ++i)
    result[i] += stepVal[i] * frame;

  return RealType(result);
}

template <typename PropType, typename RealType, typename VectorType, unsigned int SIZE>
RealType VectorPropertyAnimation<PropType, RealType, VectorType, SIZE>::getEdgeFrameValue(
    const RealType &startValue, const RealType &endValue, int frame) {
  return getNodeFrameValue(startValue, endValue, frame);
}
