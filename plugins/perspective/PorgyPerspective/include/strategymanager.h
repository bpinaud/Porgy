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
#ifndef STRATEGYMANAGER_H
#define STRATEGYMANAGER_H

#include <tulip/DataSet.h>

#include <porgy/Strategy.h>

#include <QAbstractListModel>
#include <QString>

#include <set>

namespace tlp {
class Graph;
}

/**
  * @brief Class model managing a list of strategy.
  **/
class StrategyManager : public QAbstractListModel {

public:
  enum class StrategyErrors {
    STRATEGY_NO_ERROR,
    STRATEGY_CIRCULAR_CALL,
    STRATEGY_MISSING_STRATEGY,
    STRATEGY_EMPTY_NAME,
    STRATEGY_EXISTING_NAME,
    STRATEGY_INVALID_NAME
  };

  StrategyManager(QObject *parent = nullptr);

  int count() const {
    return strategies.size();
  }

  bool empty() const {
    return strategies.empty();
  }

  void setGraph(tlp::Graph *g) {
    _graph = g;
  }

  tlp::Graph *graph() const {
    return _graph;
  }

  unsigned int addStrategy(const QString &name, StrategyErrors &error, const QString &code = "");

  void removeStrategy(unsigned index);

  const QString &getStrategyName(unsigned index) const;
  /**
    * @brief Return the last commited values of the document.
    **/
  const QString getStrategyCode(unsigned index, StrategyErrors &error) const;

  Strategy getStrategy(unsigned index, StrategyErrors &error) const;

  /**
    * @brief Return the working document for the index.
    **/
  const QString &getStrategy(unsigned index) const;

  /**
    * @brief Update the strategy name.
    **/
  StrategyErrors setStrategyName(unsigned index, const QString &newName);
  /**
    * @brief Update the strategy code. If there is a buffer for this strategy it
    *will be erased.
    **/
  void setStrategyCode(unsigned index, const QString &newCode);

  /**
    * @brief Search if a strategy with the given name exists and return it's
    *index else return -1.
    **/
  unsigned indexOfStrategy(const QString &strategyName) const;

  tlp::DataSet getData() const;
  void setData(const tlp::DataSet &data);

  // QabstractListModel interface
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QMimeData *mimeData(const QModelIndexList &indexes) const override;

  /**
    * @brief Return the label corresponding to the error code.
    **/
  static QString errorMessage(StrategyErrors &error);

private:
  /**
    * @brief Replace all the strategy reference in the code by the code of its
    *strategies. Set the error variable with the rigth error code.
    **/
  QString expandStrategyCode(QString metaStrategy, StrategyErrors &error) const;

  std::vector<Strategy> strategies;
  tlp::Graph *_graph;
};

#endif // STRATEGYMANAGER_H
