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
#ifndef CREATEPORTNODE_H
#define CREATEPORTNODE_H

/**
 * Fenetre pour créer ou éditer un type de portnode
 */

#include <QDialog>
#include <QTableWidgetItem>

#include <map>
#include <vector>

class PortNodeQt;
class PortQt;

namespace Ui {
class DialogPortNode;
}

class CreatePortNode : public QDialog {
  Q_OBJECT

  Ui::DialogPortNode *ui;

  std::map<QTableWidgetItem *, PortQt> ports;
  void init(const QColor &color);

public:
  CreatePortNode(QWidget *);
  CreatePortNode(QWidget *, const PortNodeQt &);

  std::vector<PortQt> getPortQts() const;

  ~CreatePortNode() override;
  QString getName() const;
  QColor getColor() const;
  void done(int r) override;

private slots:
  /**
    * Ouvre la boite de dialogue pour créer un port
    */
  void showPortDialog();

  /**
    * Ouvre la boite de dialogue pour éditer un port existant
    */
  void showPortDialogModify(QTableWidgetItem *item = nullptr);

  /**
    * Efface un port
    */
  void removePort();

  /**
    * Autorise ou interdit les boutons suivant la sélection
    */
  void enableButtons();
};

#endif
