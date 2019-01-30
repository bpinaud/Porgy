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
#ifndef VALIDATERULE_H
#define VALIDATERULE_H

#include <QDialog>

class QAbstractButton;

namespace Ui {
class ValidateRule;
}

namespace tlp {
class Graph;
}

class RuleValidation : public QDialog {
  Q_OBJECT

  Ui::ValidateRule *_ui;
  tlp::Graph *_rules;
  QString content;

public:
  explicit RuleValidation(tlp::Graph *rulesRoot, QWidget *parent = nullptr);
  ~RuleValidation() override;

public slots:
  void validate(QAbstractButton *);
};

#endif // VALIDATERULE_H
