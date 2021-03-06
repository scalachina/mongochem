/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MongoModel_H
#define MongoModel_H

#include <QtCore/QModelIndex>
#include <QtCore/QList>

#include <vector>

namespace mongo {
class DBClientConnection;
class Query;
}

class vtkTable;

namespace MongoChem {

class MoleculeRef;

class MongoModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit MongoModel(mongo::DBClientConnection *db, QObject *parent = 0);
  ~MongoModel();

  void setQuery(const mongo::Query &query);

  /** Adds a new column with @p name. */
  void addFieldColumn(const QString &name);

  /** Removes the column at @p index. */
  void removeFieldColumn(int index);

  QModelIndex parent(const QModelIndex & index) const;
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex & index, const QVariant & value,
               int role = Qt::EditRole);
  Qt::ItemFlags flags(const QModelIndex & index) const;

  QModelIndex index(int row, int column,
                    const QModelIndex & parent = QModelIndex()) const;

  /** Sets the molecules to be displayed in the model. */
  void setMolecules(const std::vector<MoleculeRef> &molecules);

  /** Returns the molecules being displayed in the model. */
  std::vector<MoleculeRef> molecules() const;

  void clear();

  /** Set the 2D image for the molecule at row. */
  bool setImage2D(int row, const QByteArray &image);

  /**
   * Returns @c true if the model can load more data for the current query.
   */
  bool hasMoreData() const;

  /**
   * Requests that the model load @p count more rows of data.
   */
  void loadMoreData(int count = 100);

  /**
   * Sorts the model by @p column in @order. column of -1 indicates
   * no sorting.
   */
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:
  /**
   * Sets the field to sort by to @p field. @p direction of 1 indicates
   * ascending order, -1 indicates descending order.
   */
  void setSortField(const std::string &field, int direction = 1);

  /**
   * Sets the column to sort by to @p index. @p direction of 1 indicates
   * ascending order, -1 indicates descending order.
   */
  void setSortColumn(int index, int direction = 1);

  class Private;
  Private *d;

private slots:
};

} // End namespace

#endif
