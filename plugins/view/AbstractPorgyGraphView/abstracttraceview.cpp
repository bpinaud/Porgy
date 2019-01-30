#include "abstracttraceview.h"

#include <tulip/TulipMetaTypes.h>

#include <QAction>

AbstractTraceView::AbstractTraceView():AbstractPorgyGraphView(false) {}

void AbstractTraceView::openModel() {
  QVariant data = qobject_cast<QAction *>(sender())->data();
  if (data.isValid() && data.userType() == qMetaTypeId<tlp::Graph *>()) {
    emit openModelView(data.value<tlp::Graph *>());
  }
}

void AbstractTraceView::createNewAction() {
  QVariant data = qobject_cast<QAction *>(sender())->data();
  if (data.isValid() && data.userType() == qMetaTypeId<tlp::node>()) {
    emit newTrace(data.value<tlp::node>());
  }
}
