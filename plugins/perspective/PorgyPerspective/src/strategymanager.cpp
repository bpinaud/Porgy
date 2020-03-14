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
#include "strategymanager.h"

#include <tulip/TlpQtTools.h>

#include <porgy/porgymimedata.h>
#include <portgraph/PorgyConstants.h>

#include <QMimeData>
#include <QRegularExpression>

using namespace tlp;
using namespace std;

StrategyManager::StrategyManager(QObject *parent) : QAbstractListModel(parent), _graph(nullptr) {}

unsigned int StrategyManager::addStrategy(const QString &name, StrategyErrors &error,
                                          const QString &code) {
  if (indexOfStrategy(name) == UINT_MAX) {
    if (name.isEmpty()) {
      error = StrategyErrors::STRATEGY_EMPTY_NAME;
      return UINT_MAX;
    } else {
      unsigned int newIndex = strategies.size();
      beginInsertRows(QModelIndex(), newIndex, newIndex);
      strategies.push_back(Strategy(name, code));
      endInsertRows();
      error = StrategyErrors::STRATEGY_NO_ERROR;
      return newIndex;
    }
  } else {
    error = StrategyErrors::STRATEGY_EXISTING_NAME;
    return -1;
  }
}
void StrategyManager::removeStrategy(unsigned index) {
  assert(strategies.size() > index);
  beginRemoveRows(QModelIndex(), index, index);
  strategies.erase(strategies.begin() + index);
  endRemoveRows();
}

const QString StrategyManager::getStrategyCode(unsigned index, StrategyErrors &error) const {
  assert(strategies.size() > index);
  return expandStrategyCode(strategies[index].code(), error);
}

const QString &StrategyManager::getStrategy(unsigned index) const {
  assert(strategies.size() > index);
  return strategies[index].code();
}

const QString &StrategyManager::getStrategyName(unsigned index) const {
  assert(strategies.size() > index);
  return strategies[index].name();
}

Strategy StrategyManager::getStrategy(unsigned index, StrategyErrors &error) const {
  assert(strategies.size() > index);
  Strategy strat(getStrategyName(index), getStrategyCode(index, error));
  return strat;
}

StrategyManager::StrategyErrors StrategyManager::setStrategyName(unsigned currentIndex,
                                                                 const QString &newName) {
  assert(strategies.size() > currentIndex);
  if (!newName.isEmpty()) {
    unsigned existingStrategyIndex = indexOfStrategy(newName);
    if (existingStrategyIndex == UINT_MAX || existingStrategyIndex == currentIndex) {
      strategies[currentIndex].setName(newName);
      emit dataChanged(index(currentIndex), index(currentIndex));
      return StrategyErrors::STRATEGY_NO_ERROR;
    } else {
      return StrategyErrors::STRATEGY_EXISTING_NAME;
    }
  } else {
    return StrategyErrors::STRATEGY_EMPTY_NAME;
  }
}

DataSet StrategyManager::getData() const {
  DataSet data;
  unsigned int currentIndex = 0;
  for (const Strategy &p : strategies) {
    DataSet temp;
    temp.set<string>("name", QStringToTlpString(p.name()));
    temp.set<string>("code", QStringToTlpString(p.code()));
    data.set(QStringToTlpString(QString::number(currentIndex)), temp);
    ++currentIndex;
  }
  return data;
}
void StrategyManager::setData(const DataSet &data) {
  beginResetModel();
  strategies.clear();
  for(auto value:data.getValues()) {
    if (value.second->getTypeName().compare(typeid(DataSet).name()) == 0) {
      DataSet *temp = static_cast<DataSet *>(value.second->value);
      if (temp->exists("name") && temp->exists("code")) {
        string name;
        temp->get("name", name);
        string code;
        temp->get("code", code);
        StrategyErrors error;
        addStrategy(tlpStringToQString(name), error, tlpStringToQString(code));
        if (error != StrategyErrors::STRATEGY_NO_ERROR) {
          tlp::warning() << "Error when loading strategy \"" << name << "\" "
                         << errorMessage(error).toStdString() << endl;
        }
      }
    }
  }
  endResetModel();
}

void StrategyManager::setStrategyCode(unsigned currentIndex, const QString &newCode) {
  assert(strategies.size() > currentIndex);
  strategies[currentIndex].setCode(newCode);
  emit dataChanged(index(currentIndex), index(currentIndex));
}

int StrategyManager::rowCount(const QModelIndex &) const {
  return count();
}
QVariant StrategyManager::data(const QModelIndex &index, int role) const {
  if (index.isValid()) {
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
      return getStrategyName(index.row());
    }
  }
  return QVariant();
}
QVariant StrategyManager::headerData(int, Qt::Orientation, int role) const {
  if (role == Qt::DisplayRole) {
    return QVariant(QString("Name"));
  }
  return QVariant();
}
bool StrategyManager::setData(const QModelIndex &curindex, const QVariant &value, int role) {
  if (curindex.isValid()) {
    if (role == Qt::EditRole) {
      QString strategyName = value.toString();
      QRegExp rx(PorgyConstants::STRAT_MACRO_REGEXP);
      StrategyErrors errorCode = StrategyErrors::STRATEGY_INVALID_NAME;
      if (rx.indexIn(strategyName) == -1)
        errorCode = setStrategyName(curindex.row(), strategyName);
      if (errorCode == StrategyErrors::STRATEGY_NO_ERROR) {
        return true;
      } else {
        tlp::warning() << errorMessage(errorCode).toStdString() << endl;
        return false;
      }
    }
  }
  return false;
}

Qt::ItemFlags StrategyManager::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

QMimeData *StrategyManager::mimeData(const QModelIndexList &indexes) const {
  PorgyMimeData *data = nullptr;
  foreach (const QModelIndex &m, indexes) {
    if (m.isValid()) {
      StrategyErrors error;
      Strategy strategy = getStrategy(m.row(), error);
      if (error == StrategyErrors::STRATEGY_NO_ERROR && strategy.isValid()) {
        if (data == nullptr) {
          data = new PorgyMimeData();
        }
        data->setStrategy(strategy);
      } else {
        if (error != StrategyErrors::STRATEGY_NO_ERROR) {
          tlp::warning() << tlp::QStringToTlpString(errorMessage(error)) << endl;
        } else {
          tlp::warning() << "Cannot use an empty strategy." << endl;
        }
      }
    }
  }
  return data;
}

QString StrategyManager::expandStrategyCode(QString strategy, StrategyErrors &error) const {
    QRegularExpression rx(PorgyConstants::STRAT_MACRO_REGEXP);
    QRegularExpressionMatch m = rx.match(strategy);
    unsigned count(0);
    while (m.hasMatch()) {
        count++;
        //too many nested levels of macro expansion. Probably a circular call.
        //FIXME: this test is not correct. It should be possible to use more than 10 macro calls.
//        if(count==10) {
//            error = StrategyErrors::STRATEGY_CIRCULAR_CALL;
//            return "";
//        }
        QString strat = m.captured(1);
        unsigned strategyIndex = indexOfStrategy(strat);
        if (strategyIndex != UINT_MAX) {
            QRegularExpression re(QString("#")+strat+QString("#"));
            //add an empty-line at the end to avoid problems with comments in strategy or weird behavior
            strategy.replace(re, strategies[strategyIndex].code()+"\n");
        }
        else {
            error = StrategyErrors::STRATEGY_MISSING_STRATEGY;
            return "";
        }
        m = rx.match(strategy);
    }
//  int pos = 0;
//  while ((pos = rx.indexIn(strategy, pos)) != -1) {
//    // Get tokens
//    QString strategyName = rx.cap(0);
//    int length = strategyName.size();
//    // Remove #
//    strategyName = strategyName.mid(1, strategyName.size() - 2);
//    unsigned strategyIndex = indexOfStrategy(strategyName);
//    QString strategyCode;
//    if (strategyIndex != UINT_MAX) {
//      // Check if we have already expanded this strategy to avoid cycle
//      if (alreadyExpandedStrategies.find(strategies[strategyIndex].name()) !=
//          alreadyExpandedStrategies.end()) {
//        error = StrategyErrors::STRATEGY_CIRCULAR_CALL;
//        return "";
//      } else {
//        // Store the name of the properties we will expand to avoid circular
//        // inclusion
//        auto it = alreadyExpandedStrategies.insert(strategies[strategyIndex].name());
//        strategyCode =
//            expandStrategyCode(strategies[strategyIndex].code(), error, alreadyExpandedStrategies);
//        alreadyExpandedStrategies.erase(it.first);
//        if (error != StrategyErrors::STRATEGY_NO_ERROR) {
//          return "";
//        }
//      }
//    } else {
//      error = StrategyErrors::STRATEGY_MISSING_STRATEGY;
//      return "";
//    }
//    // Replace the # token by the strategy code.
//    strategy.replace(pos, length, strategyCode);
//    pos += strategyCode.size();
//  }
  error = StrategyErrors::STRATEGY_NO_ERROR;
  return strategy;
}

unsigned StrategyManager::indexOfStrategy(const QString &strategyName) const {
  for (unsigned i = 0; i < strategies.size(); ++i) {
    if (strategies[i].name() == strategyName) {
      return i;
    }
  }
  return UINT_MAX;
}
QString StrategyManager::errorMessage(StrategyErrors &error) {
  switch (error) {
  case StrategyErrors::STRATEGY_NO_ERROR:
    return QString();
  case StrategyErrors::STRATEGY_CIRCULAR_CALL:
    return QString("Cannot generate strategy with circular calls");

  case StrategyErrors::STRATEGY_MISSING_STRATEGY:
    return QString("The strategy uses non existing strategy");

  case StrategyErrors::STRATEGY_EMPTY_NAME:
    return QString("Strategies cannot have an empty name");

  case StrategyErrors::STRATEGY_EXISTING_NAME:
    return QString("Strategies must have unique names");

  case StrategyErrors::STRATEGY_INVALID_NAME:
    return QString("The name of a strategy cannot contains two sharps (#)");
  }
  return QString();
}
