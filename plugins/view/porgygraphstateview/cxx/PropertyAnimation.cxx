template <typename PropType, typename NodeType, typename EdgeType>
PropertyAnimation<PropType, NodeType, EdgeType>::PropertyAnimation(
    tlp::Graph *graph, PropType *start, PropType *end, PropType *out,
    tlp::BooleanProperty *selection, int frameCount, bool computeNodes, bool computeEdges,
    QObject *parent)
    :

      Animation(frameCount, parent), _graph(graph), _start(nullptr), _end(nullptr), _out(out),
      _computeNodes(computeNodes), _computeEdges(computeEdges) {

  assert(out);
  assert(start);
  assert(end);
  assert(end->getGraph()->getRoot() == start->getGraph()->getRoot());
  assert(end->getGraph()->getRoot() == out->getGraph()->getRoot());
  assert(graph);
  assert(frameCount > 0);

  this->_start = new PropType(start->getGraph());
  *(this->_start) = *start;
  this->_end = new PropType(end->getGraph());
  *(this->_end) = *end;

  if (!selection) {
    this->_selection = new tlp::BooleanProperty(graph);
    this->_selection->setAllNodeValue(true);
    this->_selection->setAllEdgeValue(true);
  } else {
    this->_selection = new tlp::BooleanProperty(selection->getGraph());
    *(this->_selection) = *selection;
  }
}

template <typename PropType, typename NodeType, typename EdgeType>
PropertyAnimation<PropType, NodeType, EdgeType>::~PropertyAnimation() {
  delete _start;
  delete _end;
  delete _selection;
}

template <typename PropType, typename NodeType, typename EdgeType>
void PropertyAnimation<PropType, NodeType, EdgeType>::frameChanged(int f) {
  if (_computeNodes) {
    for (auto n : _graph->nodes()) {
      if (_selection->getNodeValue(n))
        _out->setNodeValue(n, getNodeFrameValue(_start->getNodeValue(n), _end->getNodeValue(n), f));
    }
  }

  if (_computeEdges) {
    for (auto e : _graph->edges()) {
      if (_selection->getEdgeValue(e))
        _out->setEdgeValue(e, getEdgeFrameValue(_start->getEdgeValue(e), _end->getEdgeValue(e), f));
    }
  }
}
