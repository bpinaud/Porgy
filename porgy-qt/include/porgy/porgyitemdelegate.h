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
#ifndef PORGYITEMDELEGATE_H
#define PORGYITEMDELEGATE_H

#include <tulip/TulipItemDelegate.h>
#include <tulip/TulipItemEditorCreators.h>

#include <porgy/PorgyQtConstants.h>

class PorgyItemDelegate : public tlp::TulipItemDelegate {
public:
  PorgyItemDelegate(QObject *parent = nullptr);
};

class PorgyCompOpEditorCreator : public tlp::TulipItemEditorCreator {
public:
  QWidget *createWidget(QWidget *) const override;
  void setEditorData(QWidget *, const QVariant &, bool, tlp::Graph *) override;
  QVariant editorData(QWidget *, tlp::Graph *) override;
  QString displayText(const QVariant &) const override;
};

#endif // PORGYITEMDELEGATE_H
