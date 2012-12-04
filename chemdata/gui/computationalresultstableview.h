/******************************************************************************

  This source file is part of the ChemData project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef CHEMDATA_COMPUTATIONALRESULTSTABLEVIEW_H
#define CHEMDATA_COMPUTATIONALRESULTSTABLEVIEW_H

#include <QTableView>

namespace ChemData {

class OpenInEditorHandler;

/// \class ComputationalResultsTableView
///
/// The ComputationalResultsTableView class shows a list of
/// computational job results and associated data in a table.
class ComputationalResultsTableView : public QTableView
{
  Q_OBJECT

public:
  explicit ComputationalResultsTableView(QWidget *parent_ = 0);
  ~ComputationalResultsTableView();

  void contextMenuEvent(QContextMenuEvent *e);

protected slots:
  void showLogFile();
  void showMoleculeDetailsDialog();

private:
  Q_DISABLE_COPY(ComputationalResultsTableView)

  int m_row;
  bool m_enableShowDetailsAction;
  OpenInEditorHandler *m_openInEditorHandler;
};

} // end ChemData namespace

#endif // CHEMDATA_COMPUTATIONALRESULTSTABLEVIEW_H