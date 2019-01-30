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
#include <porgy/porgyitemdelegate.h>

#include <portgraph/PorgyConstants.h>

#include <tulip/TlpQtTools.h>

using namespace tlp;

PorgyItemDelegate::PorgyItemDelegate(QObject *parent) : TulipItemDelegate(parent) {
  registerCreator<PorgyConstants::ComparisonOp>(new PorgyCompOpEditorCreator);
}

QWidget *PorgyCompOpEditorCreator::createWidget(QWidget *parent) const {
  QComboBox *cb = new QComboBox(parent);
  for (const auto &op : PorgyConstants::RuleComparisonToString) {
    cb->addItem(op.second, QVariant::fromValue<PorgyConstants::ComparisonOp>(op.first));
  }
  return cb;
}

void PorgyCompOpEditorCreator::setEditorData(QWidget *editor, const QVariant &v, bool,
                                             tlp::Graph *) {
  QComboBox *cb = static_cast<QComboBox *>(editor);
  cb->setCurrentIndex(cb->count() - 1 -
                      static_cast<unsigned>(v.value<PorgyConstants::ComparisonOp>()));
}

QVariant PorgyCompOpEditorCreator::editorData(QWidget *editor, tlp::Graph *) {
  QComboBox *cb = static_cast<QComboBox *>(editor);
  return cb->itemData(cb->currentIndex());
}

QString PorgyCompOpEditorCreator::displayText(const QVariant &v) const {
  const char *tt = PorgyConstants::RuleComparisonToString.at(
      static_cast<PorgyConstants::ComparisonOp>(v.toUInt()));
  return QString(tt);
}
