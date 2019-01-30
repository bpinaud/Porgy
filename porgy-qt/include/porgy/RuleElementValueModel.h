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
#ifndef RULEELEMENTVALUEMODEL_H
#define RULEELEMENTVALUEMODEL_H

#include <porgy/GraphElementValueModel.h>

#include <QVariant>

class RuleElementValueModel : public GraphElementValueModel {
public:
  RuleElementValueModel(tlp::Graph *graph, tlp::ElementType elementType, unsigned int elementId,
                        QObject *parent = nullptr);
  QVariant data(const QModelIndex &index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
  std::unordered_set<std::string> buildPropertiesList();
};

#endif // RULEELEMENTVALUEMODEL_H
