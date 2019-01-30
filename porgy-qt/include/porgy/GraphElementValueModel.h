/**
 *
 * This file is part of Porgy (http://tulip.labri.fr/TulipDrupal/?q=porgy)
 *
 * from LaBRI, University of Bordeaux, Inria and King's College London
 *
 * Porgy is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Porgy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this Porgy.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GRAPHELEMENTVALUEMODEL_H
#define GRAPHELEMENTVALUEMODEL_H
#include <QAbstractTableModel>

#include <tulip/Graph.h>

class PropertyValueToQVariantManager;
/**
 * @brief The GraphElementValueModel class Displays properties for given element
 * according to it's type and id.
 * To filter the displayed properties give the name of the properties to
 * visualize
 */
class GraphElementValueModel : public QAbstractTableModel {

public:
  /**
   * @brief GraphElementValueModel Display only properties with the name given
   * in the propertiesToDisplay parameter.
   * @param graph
   * @param eltType
   * @param eltId
   * @param propertiesToDisplay
   * @param parent
   */
  GraphElementValueModel(tlp::Graph *graph, tlp::ElementType eltType, unsigned int eltId,
                         const std::unordered_set<std::string> &propertiesToDisplay,
                         QObject *parent = nullptr);
  /**
   * @brief GraphElementValueModel Displays all properties available in the
   * graph.
   * @param graph
   * @param eltType
   * @param eltId
   * @param parent
   */
  GraphElementValueModel(tlp::Graph *graph, tlp::ElementType eltType, unsigned int eltId,
                         QObject *parent = nullptr);
  ~GraphElementValueModel() override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

protected:
  PropertyValueToQVariantManager *_propertyDataConverter;
  tlp::Graph *_graph;
  std::vector<tlp::PropertyInterface *> _propertyTable;
  tlp::ElementType _displayType;
  unsigned int _eltId;
  void initPropertiesList(const std::unordered_set<std::string> &propertiesToDisplay);
  //    void addWithoutDuplicate(std::vector<std::string>& propertiesToDisplay,
  //    const std::string value);
};

#endif // GRAPHELEMENTVALUEMODEL_H
