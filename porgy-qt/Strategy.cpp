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
#include <porgy/Strategy.h>

#include <QDataStream>
#include <QIODevice>

using namespace std;

Strategy::Strategy(const QString &name, const QString &code) : _name(name), _code(code) {}

Strategy::Strategy(const Strategy &other) : _name(other._name), _code(other._code) {}

QByteArray Strategy::toByteArray() const {
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);
  stream << _name << _code;
  return data;
}

Strategy Strategy::fromByteArray(const QByteArray &byteArray) {
  QDataStream stream(byteArray);
  QString name;
  QString code;
  stream >> name >> code;
  return Strategy(name, code);
}
