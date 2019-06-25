#ifndef PROPERTIESANIMATION_H
#define PROPERTIESANIMATION_H

#include <tulip/Graph.h>
#include <tulip/ColorProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/BooleanProperty.h>
#include <tulip/SizeProperty.h>
#include <tulip/LayoutProperty.h>

#include <porgy/Animation.h>

template <typename PropType, typename NodeType, typename EdgeType>
class PropertyAnimation : public Animation {
public:
  PropertyAnimation(tlp::Graph *graph, PropType *start, PropType *end, PropType *out,
                    tlp::BooleanProperty *selection = nullptr, int frameCount = 1,
                    bool computeNodes = true, bool computeEdges = true, QObject *parent = nullptr);

  virtual ~PropertyAnimation();
  void frameChanged(int f) override;

protected:
  tlp::Graph *_graph;
  PropType *_start;
  PropType *_end;
  PropType *_out;
  tlp::BooleanProperty *_selection;
  bool _computeNodes;
  bool _computeEdges;

  virtual NodeType getNodeFrameValue(const NodeType &startValue, const NodeType &endValue,
                                     int frame) = 0;
  virtual EdgeType getEdgeFrameValue(const EdgeType &startValue, const EdgeType &endValue,
                                     int frame) = 0;

  virtual bool equalNodes(const NodeType &v1, const NodeType &v2) {
    return v1 == v2;
  }

  virtual bool equalEdges(const EdgeType &v1, const EdgeType &v2) {
    return v1 == v2;
  }
};

#include "cxx/PropertyAnimation.cxx"

//////////////////////
template <typename PropType, typename NodeType, typename EdgeType>
class CachedPropertyAnimation : public PropertyAnimation<PropType, NodeType, EdgeType> {
public:
  CachedPropertyAnimation(tlp::Graph *graph, PropType *start, PropType *end, PropType *out,
                          tlp::BooleanProperty *selection = nullptr, int frameCount = 1,
                          bool computeNodes = true, bool computeEdges = true,
                          QObject *parent = nullptr);
  ~CachedPropertyAnimation() override;

  void frameChanged(int frame) override;

protected:
  std::map<std::pair<NodeType, NodeType>, NodeType> computedNodeSteps;
  std::map<std::pair<EdgeType, EdgeType>, EdgeType> computedEdgeSteps;
};

#include "cxx/CachedPropertyAnimation.cxx"
////////////////////////////////////

template <typename PropType, typename RealType, typename VectorType, unsigned int SIZE>
class VectorPropertyAnimation : public CachedPropertyAnimation<PropType, RealType, RealType> {
public:
  VectorPropertyAnimation(tlp::Graph *graph, PropType *start, PropType *end, PropType *out,
                          tlp::BooleanProperty *selection = nullptr, int frameCount = 1,
                          bool computeNodes = true, bool computeEdges = true,
                          QObject *parent = nullptr);

  virtual ~VectorPropertyAnimation() {}

protected:
  virtual RealType getNodeFrameValue(const RealType &startValue, const RealType &endValue,
                                     int frame);
  virtual RealType getEdgeFrameValue(const RealType &startValue, const RealType &endValue,
                                     int frame);

private:
  typedef tlp::Vector<VectorType, SIZE> PropVector;

  std::map<std::pair<tlp::Vector<VectorType, SIZE>, tlp::Vector<VectorType, SIZE>>,
           tlp::Vector<double, SIZE>>
      steps;
};

#include "cxx/VectorPropertyAnimation.cxx"

///////////////////
typedef VectorPropertyAnimation<tlp::ColorProperty, tlp::Color, unsigned char, 4> ColorPropertyAnimation;
typedef VectorPropertyAnimation<tlp::SizeProperty, tlp::Size, float, 3> SizePropertyAnimation;
//////////////
class DoublePropertyAnimation
    : public CachedPropertyAnimation<tlp::DoubleProperty, double, double> {
public:
  DoublePropertyAnimation(tlp::Graph *graph, tlp::DoubleProperty *start, tlp::DoubleProperty *end,
                          tlp::DoubleProperty *out, tlp::BooleanProperty *selection = nullptr,
                          int frameCount = 1, bool computeNodes = true, bool computeEdges = false,
                          QObject *parent = nullptr);

protected:
  double getNodeFrameValue(const double &startValue, const double &endValue, int frame) override;
  double getEdgeFrameValue(const double &startValue, const double &endValue, int frame) override;
};

class LayoutPropertyAnimation
    : public CachedPropertyAnimation<tlp::LayoutProperty, tlp::Coord, std::vector<tlp::Coord>> {
public:
  LayoutPropertyAnimation(tlp::Graph *g, tlp::LayoutProperty *start, tlp::LayoutProperty *end,
                          tlp::LayoutProperty *out, tlp::BooleanProperty *selection = nullptr,
                          int frameCount = 1, bool computeNodes = true, bool computeEdges = true,
                          QObject *parent = nullptr);

protected:
  tlp::Coord getNodeFrameValue(const tlp::Coord &startValue, const tlp::Coord &endValue,
                               int frame) override;
  std::vector<tlp::Coord> getEdgeFrameValue(const std::vector<tlp::Coord> &startValue,
                                            const std::vector<tlp::Coord> &endValue,
                                            int frame) override;
  bool equalEdges(const std::vector<tlp::Coord> &v1, const std::vector<tlp::Coord> &v2) override;

private:
  std::map<std::pair<tlp::Coord, tlp::Coord>, tlp::Vector<double, 3>> steps;
};

#endif // PROPERTIESANIMATION_H
