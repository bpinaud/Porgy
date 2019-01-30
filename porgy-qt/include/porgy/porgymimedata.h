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
#ifndef PORGYMIMEDATA_H
#define PORGYMIMEDATA_H

#include <QMimeData>
#include <QString>

#include <portgraph/porgyconf.h>

class Strategy;
class PortNodeQt;

/**
  * @brief Class to store and retrieve porgy types in QMimeData.
  **/
class PORGY_SCOPE PorgyMimeData : public QMimeData {
public:
  static const QString StrategyMimeType;
  static const QString portNodeQtPtrListMimeTypes;

  /**
    * @brief Check if the mime data object contains a Porgy strategy.
    **/
  bool hasStrategy() const;
  Strategy getStrategy() const;
  void setStrategy(const Strategy &strategy);

  /**
    * @brief Checks if the given mime data contains port node qt pointers list.
    **/
  bool hasPortNodeQtPtrList() const;
  QList<PortNodeQt *> getPortNodeQtPtrList() const;
  void setPortNodeQtPtrList(const QList<PortNodeQt *> &ptrs);
};

QDataStream &operator>>(QDataStream &in, PortNodeQt *&portNodeQt);
QDataStream &operator<<(QDataStream &out, const PortNodeQt *portNodeQt);
#endif // PORGYMIMEDATA_H
