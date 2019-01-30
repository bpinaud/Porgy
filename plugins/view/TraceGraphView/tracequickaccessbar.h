#ifndef TRACEQUICKACCESSBAR_H
#define TRACEQUICKACCESSBAR_H

#include <tulip/QuickAccessBar.h>

class QPushButton;

class TraceQuickAccessBar : public tlp::QuickAccessBarImpl {
  Q_OBJECT

  QPushButton *_displayMetaNode;
  QPushButton *_metanodelabel;
  QPushButton *_arrows;

public:
  TraceQuickAccessBar(QWidget *parent = nullptr);

private slots:
  void togglemetanodelabel(bool t);
  void showArrows(bool t);
  void renderMetaNodes(bool t);
};

#endif // TRACEQUICKACCESSBAR_H
