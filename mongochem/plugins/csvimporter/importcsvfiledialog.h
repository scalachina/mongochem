/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef IMPORTCSVFILEDIALOG_H
#define IMPORTCSVFILEDIALOG_H

#include <QSet>

#include <mongochem/gui/abstractimportdialog.h>

namespace Ui {
class ImportCsvFileDialog;
}

namespace MongoChem {
class SvgGenerator;
}

/**
 * The ImportCsvFileDialog class provides a dialog allowing for the user
 * to import molecular descriptor data from a CSV file.
 */
class ImportCsvFileDialog : public MongoChem::AbstractImportDialog
{
  Q_OBJECT

public:
  explicit ImportCsvFileDialog(QWidget *parent_ = 0);
  ~ImportCsvFileDialog();

  void setFileName(const QString &fileName_);
  QString fileName() const;

public slots:
  void openFile();

private slots:
  void import();
  void columnMappingTableCellChanged(int row, int column);
  void updateTypeComboBox(const QString &role);
  void delimiterComboBoxChanged(int index);
  void delimiterLineEditChanged(const QString &text);
  QChar delimiterCharacter() const;
  void closeCurrentFile();
  void moleculeDiagramReady(int errorCode);

private:
  Ui::ImportCsvFileDialog *ui;
  QString m_fileName;
  QSet<MongoChem::SvgGenerator *> m_svgGenerators;
};

#endif // IMPORTCSVFILEDIALOG_H
