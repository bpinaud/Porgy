#include "elementdeleter.h"
#include "../../StandardInteractorPriority.h"

#include <portgraph/PorgyTlpGraphStructure.h>
#include <portgraph/Trace.h>

#include <tulip/GlGraphComposite.h>
#include <tulip/GlMainWidget.h>
#include <tulip/MouseInteractors.h>
#include <tulip/View.h>

#include <QMouseEvent>

using namespace std;
using namespace tlp;

class DeleteElementTraceComponent : public tlp::MouseElementDeleter {
  void delElement(tlp::Graph *graph, tlp::SelectedEntity &selectedEntity) override {
    if (selectedEntity.getEntityType() == SelectedEntity::NODE_SELECTED) {
      node n(selectedEntity.getComplexEntityId());
      unsigned outdeg = graph->outdeg(n); unsigned indeg = graph->indeg(n);
      //Do not forget to remove self loops before checking if the node is a leaf
      for (auto e : graph->getInEdges(n)) {
        const std::pair<node, node> eEnds = graph->ends(e);
        if (eEnds.first == eEnds.second) {
            --outdeg;
            --indeg;
        }
      }
      if ((outdeg == 0) && (indeg != 0)) {
        if (PorgyTlpGraphStructure::getMainTrace(graph) == graph) {
          Graph *meta = graph->getNodeMetaInfo(n);
          graph->delNode(n, true);
          if (meta != nullptr) {
            PorgyTlpGraphStructure::getModelsRoot(graph)->delSubGraph(meta);
          }
        } else {
          graph->delNode(n);
        }
      } else {
        tlp::debug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": Cannot delete element. Only leaves can be deleted." << endl;
      }
    } else {
        tlp::debug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": Cannot delete element. Only leaves can be deleted." << endl;
    }
  }

public:
  DeleteElementTraceComponent() {}
  bool eventFilter(QObject *obj, QEvent *ev) override {
    QMouseEvent *qMouseEv = dynamic_cast<QMouseEvent *>(ev);

    if (qMouseEv != nullptr) {
      GlMainWidget *glMainWidget = static_cast<GlMainWidget *>(obj);
      Graph *g = glMainWidget->getScene()->getGlGraphComposite()->getInputData()->getGraph();
      SelectedEntity select;
      if (glMainWidget->pickNodesEdges(qMouseEv->x(), qMouseEv->y(), select, nullptr, false,
                                       true)) {
        return true;
      }
      if (g->numberOfNodes() > 1)
        return MouseElementDeleter::eventFilter(obj, ev);
    }
    return false;
  }
};

elementDeleter::elementDeleter(const tlp::PluginContext *)
    : NodeLinkDiagramComponentInteractor(":/tulip/gui/icons/i_del.png",
                                         "Delete a node from the derivation tree",StandardInteractorPriority::DeleteElement) {
}
bool elementDeleter::isCompatible(const string &viewName) const {
  return (viewName == PorgyConstants::TRACE_VIEW_NAME);
}

void elementDeleter::construct() {
  push_front(new MousePanNZoomNavigator);
  push_front(new DeleteElementTraceComponent);
}

PLUGIN(elementDeleter)
