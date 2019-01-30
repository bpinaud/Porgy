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
#ifndef STRATEGY_H
#define STRATEGY_H

#include <QByteArray>
#include <QString>

#include <portgraph/porgyconf.h>

/**
  * @brief Simple strategy container.
  **/
class PORGY_SCOPE Strategy {
public:
  Strategy(const QString &name, const QString &code = QString());
  Strategy(const Strategy &other);

  inline const QString &name() const {
    return _name;
  }
  inline const QString &code() const {
    return _code;
  }

  inline void setName(const QString &name) {
    _name = name;
  }

  inline void setCode(const QString &code) {
    _code = code;
  }

  inline bool isValid() const {
    return !_name.isEmpty() && !_code.isEmpty();
  }

  QByteArray toByteArray() const;
  static Strategy fromByteArray(const QByteArray &byteArray);

private:
  QString _name;
  QString _code;
};

#endif // STRATEGY_H
