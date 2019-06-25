#include "propertiesanimation.h"

using namespace std;
using namespace tlp;

LayoutPropertyAnimation::LayoutPropertyAnimation(Graph *g, LayoutProperty *start,
                                                 LayoutProperty *end, LayoutProperty *out,
                                                 BooleanProperty *selection, int frameCount,
                                                 bool computeNodes, bool computeEdges,
                                                 QObject *parent)
    : CachedPropertyAnimation<LayoutProperty, Coord, vector<Coord>>(
          g, start, end, out, selection, frameCount, computeNodes, computeEdges, parent) {}

Coord LayoutPropertyAnimation::getNodeFrameValue(const Coord &startValue, const Coord &endValue,
                                                 int frame) {
  pair<Coord, Coord> values(startValue, endValue);
  map<pair<Coord, Coord>, Vector<double, 3>>::iterator it = steps.find(values);
  Vector<double, 3> stepVal;

  if (it != steps.end())
    stepVal = it->second;
  else {
    for (unsigned i = 0; i < 3; ++i)
      stepVal[i] = (double(endValue[i]) - double(startValue[i])) * 1. / (frameCount() - 1);
  }

  Coord result;

  for (unsigned i = 0; i < 3; ++i)
    result[i] = startValue[i] + frame * stepVal[i];

  return result;
}

vector<Coord> LayoutPropertyAnimation::getEdgeFrameValue(const vector<Coord> &startValue,
                                                         const vector<Coord> &endValue, int frame) {
  vector<Coord> result;

  for (unsigned i = 0; i < min<unsigned>(startValue.size(), endValue.size()); ++i)
    result.push_back(getNodeFrameValue(startValue[i], endValue[i], frame));

  return result;
}

bool LayoutPropertyAnimation::equalEdges(const vector<Coord> &v1, const vector<Coord> &v2) {
  if (v1.size() != v2.size())
    return false;

  for (unsigned i = 0; i < v1.size(); ++i) {
    if (v1[i] != v2[i])
      return false;
  }

  return true;
}

DoublePropertyAnimation::DoublePropertyAnimation(Graph *graph, DoubleProperty *start,
                                                 DoubleProperty *end, DoubleProperty *out,
                                                 BooleanProperty *selection, int frameCount,
                                                 bool computeNodes, bool computeEdges,
                                                 QObject *parent)
    : CachedPropertyAnimation<DoubleProperty, double, double>(
          graph, start, end, out, selection, frameCount, computeNodes, computeEdges, parent) {}

double DoublePropertyAnimation::getNodeFrameValue(const double &startValue, const double &endValue,
                                                  int frame) {
  if (frame != 0) {
    return startValue + ((endValue - startValue) / (frameCount())) * frame;
  } else {
    return startValue;
  }
}

double DoublePropertyAnimation::getEdgeFrameValue(const double &startValue, const double &endValue,
                                                  int frame) {
  if (frame != 0) {
    return startValue + ((endValue - startValue) / (frameCount())) * frame;
  } else {
    return startValue;
  }
}
