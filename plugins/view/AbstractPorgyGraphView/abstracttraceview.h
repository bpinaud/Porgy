#ifndef ABSTRACTTRACEVIEW_H
#define ABSTRACTTRACEVIEW_H

#include "AbstractPorgyGraphView.h"

#include <tulip/GlMainWidget.h>

class AbstractTraceView : public AbstractPorgyGraphView {

  Q_OBJECT

public:
  AbstractTraceView();

protected:
  tlp::SelectedEntity entity;

protected slots:
  void openModel();
  void createNewAction();

signals:
  void openModelView(tlp::Graph *g);
  void newTrace(tlp::node n);
};

#endif // ABSTRACTTRACEVIEW_H
