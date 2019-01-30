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
#ifndef PORGYSETTINGS_H
#define PORGYSETTINGS_H

#include <QObject>
#include <QVariant>

#include <portgraph/porgyconf.h>

class PORGY_SCOPE PorgySettings : public QObject {

  Q_OBJECT

  QVariantMap _cache;
  QMap<QString, QString> _toolTips;
  QMap<QString, QString> _whatsThis;

public:
  PorgySettings(QObject *parent = nullptr);

  void registerParamter(const QString &key, const QVariant &defaultValue,
                        const QString &toolTip = QString(), const QString &whatThis = QString());

  QVariant value(const QString &key) const;
  void setValue(const QString &key, const QVariant &value);
  QStringList keys() const;

  QString toolTip(const QString &key) const;
  QString whatThis(const QString &key) const;
  QStringList groups(const QString &key) const;

  // Porgy specific functions
  unsigned int traceViewZoomAnimationTime() const;
  void setTraceViewZoomAnimationTime(unsigned int animationTime);

signals:
  void parameterChanged(QString key);

public slots:
  void setAsDefault();
  void restoreDefault();
};

#endif // PORGYSETTINGS_H
