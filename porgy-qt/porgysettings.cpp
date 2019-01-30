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
#include <porgy/porgypersistentsettings.h>
#include <porgy/porgysettings.h>

#include <QStringList>

#include <iostream>

#include <tulip/tulipconf.h>

PorgySettings::PorgySettings(QObject *parent) : QObject(parent) {
  registerParamter(QString("Trace view/Zoom animation duration"), QVariant(3000),
                   QString("The duration of the zoom animation of the trace view"));
}

void PorgySettings::registerParamter(const QString &key, const QVariant &defaultValue,
                                     const QString &toolTip, const QString &whatThis) {
  // Check if the parameter already exists.
  if (_cache.find(key) != _cache.end()) {
    std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__
              << " Error a parameter with a different type already exists it "
                 "will be erased"
              << std::endl;
    _cache.remove(key);
  }
  if (_toolTips.find(key) != _toolTips.end()) {
    std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__
              << " Error a parameter with a different type already exists it "
                 "will be erased"
              << std::endl;
    _toolTips.remove(key);
  }
  if (_whatsThis.find(key) != _whatsThis.end()) {
    std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__
              << " Error a parameter with a different type already exists it "
                 "will be erased"
              << std::endl;
    _whatsThis.remove(key);
  }

  // Add into the persistent system if the default value don't already exist.
  PorgyPersistentSettings &persistentSettings = PorgyPersistentSettings::instance();
  if (!persistentSettings.contains(key)) {
    persistentSettings.setValue(key, defaultValue);
    std::cout << __PRETTY_FUNCTION__ << " " << __LINE__ << " Registering the parameter "
              << key.toStdString() << std::endl;
  }

  if (!toolTip.isEmpty()) {
    _toolTips[key] = toolTip;
  }
  if (!whatThis.isEmpty()) {
    _whatsThis[key] = whatThis;
  }
}

QStringList PorgySettings::keys() const {
  return PorgyPersistentSettings::instance().allKeys();
}

QVariant PorgySettings::value(const QString &key) const {
  if (!PorgyPersistentSettings::instance().contains(key)) {
    std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << " Error : The parameter "
              << key.toStdString() << " was not declared " << std::endl;
    return QVariant();
  }
  auto it = _cache.find(key);
  if (it != _cache.end()) {
    return it.value();
  } else {
    return PorgyPersistentSettings::instance().value(key);
  }
}
void PorgySettings::setValue(const QString &key, const QVariant &value) {
  if (!PorgyPersistentSettings::instance().contains(key)) {
    std::cerr << __PRETTY_FUNCTION__ << " " << __LINE__ << " Error : The parameter "
              << key.toStdString() << " was not declared " << std::endl;
    return;
  }
  _cache[key] = value;
  emit parameterChanged(key);
}

void PorgySettings::setAsDefault() {
  PorgyPersistentSettings &persistence = PorgyPersistentSettings::instance();
  for (auto it = _cache.begin(); it != _cache.end(); ++it) {
    persistence.setValue(it.key(), it.value());
  }
  // No need to use the value in cache.
  _cache.clear();
}

void PorgySettings::restoreDefault() {
  // Erase all the values in the cache
  _cache.clear();
  QStringList keys = _cache.keys();
  foreach (const QString &s, keys) { emit parameterChanged(s); }
}

QString PorgySettings::toolTip(const QString &key) const {
  auto it = _toolTips.find(key);
  if (it != _toolTips.end()) {
    return it.value();
  } else {
    return QString();
  }
}
QString PorgySettings::whatThis(const QString &key) const {
  auto it = _whatsThis.find(key);
  if (it != _toolTips.end()) {
    return it.value();
  } else {
    return QString();
  }
}

unsigned int PorgySettings::traceViewZoomAnimationTime() const {
  return value("Trace view/Zoom animation duration").toUInt();
}
void PorgySettings::setTraceViewZoomAnimationTime(unsigned int animationTime) {
  setValue("Trace view/Zoom animation duration", QVariant(animationTime));
}

QStringList PorgySettings::groups(const QString &key) const {
  return key.split(QChar('/'));
}
