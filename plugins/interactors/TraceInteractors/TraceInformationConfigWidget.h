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

#ifndef INFO_TRACE_CONFIG_W_H_
#define INFO_TRACE_CONFIG_W_H_

#include <QWidget>

namespace Ui {
class InformationInteractorConfig;
}

class TraceInformationConfigWidget : public QWidget {
  Q_OBJECT

  Ui::InformationInteractorConfig *ui;

public:
  TraceInformationConfigWidget(QWidget *parent = nullptr);
  ~TraceInformationConfigWidget() override;

  bool ShowPProperty() const;
  bool ShowInstance() const;
  bool ShowBanProperty() const;
};

#endif
