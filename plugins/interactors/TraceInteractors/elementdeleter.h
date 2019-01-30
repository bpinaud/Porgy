#ifndef ELEMENTDELETER_H
#define ELEMENTDELETER_H

#include <portgraph/PorgyConstants.h>
#include <tulip/NodeLinkDiagramComponentInteractor.h>

class elementDeleter : public tlp::NodeLinkDiagramComponentInteractor {
public:
  elementDeleter(const tlp::PluginContext *);
  PLUGININFORMATION("elementDeleter", "Bruno Pinaud", "05/09/2016",
                    "Delete  an element from the derivation tree", "1.0",
                    PorgyConstants::CATEGORY_NAME)

  void construct() override;
  bool isCompatible(const std::string &) const override;
};

#endif // ELEMENTDELETER_H
