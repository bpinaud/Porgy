#include <porgy/Animation.h>

Animation::Animation(int frameCount, QObject *parent)
    : QPropertyAnimation(parent), _currentFrame(0), _frameCount(0) {
  setPropertyName("frame");
  setTargetObject(this);
  setStartValue(0);
  setFrameCount(frameCount);
}

Animation::~Animation() {}

int Animation::currentFrame() const {
  return _currentFrame;
}

int Animation::frameCount() const {
  return _frameCount;
}

void Animation::setFrameCount(int fc) {
  _frameCount = fc;
  setEndValue(frameCount() - 1);
}
