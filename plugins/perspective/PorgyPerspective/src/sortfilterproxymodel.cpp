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
#include "sortfilterproxymodel.h"
#include <QQueue>
SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent), _filteringOrder(TopToBottom) {}

void SortFilterProxyModel::setFilterOrder(FilteringOrder order) {
  // If the filtering order is the same don't update the data
  if (order != _filteringOrder) {
    _filteringOrder = order;
    // Need to perform a new filtering
    invalidateFilter();
  }
}

bool SortFilterProxyModel::filterAcceptsRow(int source_row,
                                            const QModelIndex &source_parent) const {
  if (_filteringOrder == TopToBottom) {
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
  } else {
    // First we test if the element validate the filter.
    // If true we don't need to explore it's children we display the element.
    // If false we test each of it's children. If a children validate the filter
    // we display the parent.
    if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)) {
      return true;
    } else {
      QQueue<QModelIndex> elementsToCheck;
      elementsToCheck.enqueue(sourceModel()->index(source_row, 0, source_parent));
      while (!elementsToCheck.empty()) {
        // Explore children
        QModelIndex currentIndex = elementsToCheck.dequeue();
        int childrenCount = sourceModel()->rowCount(currentIndex);
        for (int i = 0; i < childrenCount; ++i) {
          // Check if the children validate the filter.
          // If the child validate the filter return and display the parent as
          // at least one of its descendant validate the filter
          // Else enque the child to explore its children later
          if (QSortFilterProxyModel::filterAcceptsRow(i, currentIndex)) {
            return true;
          } else {
            elementsToCheck.enqueue(sourceModel()->index(i, 0, currentIndex));
          }
        }
      }
      // No descendant validate the filter hide this element.
      return false;
    }
  }
}
