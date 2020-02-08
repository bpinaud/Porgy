#ifndef PORGYQUICKACCESSBAR_H
#define PORGYQUICKACCESSBAR_H

#include <tulip/QuickAccessBar.h>

class QCheckBox;

namespace tlp{
class BooleanProperty;
}

class PorgyQuickAccessBar : public tlp::QuickAccessBarImpl {
  Q_OBJECT

  tlp::Graph *rule_graph;
  QCheckBox *checkRHS;
  QCheckBox *checkLHS;
  QCheckBox *checkArrowNodeEdges;
  tlp::BooleanProperty* sidep;

public:
  PorgyQuickAccessBar(tlp::Graph *rule, QWidget *parent = nullptr);
  ~PorgyQuickAccessBar() override;
  void reset() override;
  void setGraph(tlp::Graph *g);
  inline tlp::Graph *getGraph() {
    return rule_graph;
  }
  bool LHSVisible() const;
  bool RHSVisible() const;
  bool allVisible() const;
  enum SideToDisplay { RHSFLAG = 0x1, LHSFLAG = 0x2, ARROW_EDGES_FLAG = 0x4, ALL = 0xF };
  Q_DECLARE_FLAGS(SideToDisplayFlags, SideToDisplay)

signals:
  void new_graph_to_display(tlp::BooleanProperty *);

private:
  void computeGraphToDisplay();
  SideToDisplayFlags flags;

public slots:
  void RHS(bool state);
  void LHS(bool state);
  void arrowNodeEdges(bool state);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(PorgyQuickAccessBar::SideToDisplayFlags)
#endif // PORGYQUICKACCESSBAR_H
