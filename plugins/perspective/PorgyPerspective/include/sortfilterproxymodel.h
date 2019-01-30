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
#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H
#include <QSortFilterProxyModel>
class SortFilterProxyModel : public QSortFilterProxyModel {

public:
  enum FilteringOrder { TopToBottom, BottomToTop };
  explicit SortFilterProxyModel(QObject *parent = nullptr);

  FilteringOrder filterOrder() const {
    return _filteringOrder;
  }
  void setFilterOrder(FilteringOrder order);

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
  Q_PROPERTY(FilteringOrder filterOrder READ filterOrder WRITE setFilterOrder)

  FilteringOrder _filteringOrder;
};

#endif // SORTFILTERPROXYMODEL_H
