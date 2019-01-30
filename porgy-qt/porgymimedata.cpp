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
#include <QDataStream>
#include <QMimeData>

#include <porgy/PortNodeQt.h>
#include <porgy/Strategy.h>
#include <porgy/porgymimedata.h>

const QString PorgyMimeData::StrategyMimeType = QString("application/porgy;value=\"strategy\"");
const QString PorgyMimeData::portNodeQtPtrListMimeTypes =
    QString("application/porgy;value=\"PortNodeQtPtrList\"");

bool PorgyMimeData::hasStrategy() const {
  return hasFormat(StrategyMimeType);
}

Strategy PorgyMimeData::getStrategy() const {
  return Strategy::fromByteArray(data(StrategyMimeType));
}

void PorgyMimeData::setStrategy(const Strategy &strategy) {
  setData(StrategyMimeType, strategy.toByteArray());
}

bool PorgyMimeData::hasPortNodeQtPtrList() const {
  return hasFormat(portNodeQtPtrListMimeTypes);
}

QList<PortNodeQt *> PorgyMimeData::getPortNodeQtPtrList() const {
  QList<PortNodeQt *> list;
  if (hasPortNodeQtPtrList()) {
    QByteArray dat = data(portNodeQtPtrListMimeTypes);
    QDataStream stream(&dat, QIODevice::ReadOnly);
    stream >> list;
  }
  return list;
}

void PorgyMimeData::setPortNodeQtPtrList(const QList<PortNodeQt *> &ptrs) {
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);
  stream << ptrs;
  setData(portNodeQtPtrListMimeTypes, data);
}

QDataStream &operator>>(QDataStream &in, PortNodeQt *&portNodeQt) {
  // Need to hack to read a pointer stored in a QDataStream
  in.readRawData(reinterpret_cast<char *>(&portNodeQt), sizeof(portNodeQt));
  return in;
}

QDataStream &operator<<(QDataStream &out, const PortNodeQt *portNodeQt) {
  // Need to hack to store a pointer in a QDataStream
  out.writeRawData(reinterpret_cast<char *>(&portNodeQt), sizeof(portNodeQt));
  return out;
}
