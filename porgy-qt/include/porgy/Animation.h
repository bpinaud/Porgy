#ifndef ANIMATION_H
#define ANIMATION_H

#include <QPropertyAnimation>

class Animation : public QPropertyAnimation {
  Q_OBJECT
  Q_PROPERTY(int frame READ currentFrame WRITE frameChanged)
  Q_PROPERTY(int frameCount READ frameCount WRITE setFrameCount)

  int _currentFrame;
  int _frameCount;

public:
  Animation(int frameCount = 0, QObject *parent = nullptr);
  virtual ~Animation();

  int currentFrame() const;
  int frameCount() const;

public slots:
  virtual void frameChanged(int) = 0;
  virtual void setFrameCount(int);
};


#endif // ANIMATION_H
