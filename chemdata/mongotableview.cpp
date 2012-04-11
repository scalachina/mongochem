/******************************************************************************

  This source file is part of the ChemData project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "mongotableview.h"

#include "mongomodel.h"
#include "openineditorhandler.h"
#include "moleculedetaildialog.h"

#include <boost/make_shared.hpp>

#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace ChemData
{

MongoTableView::MongoTableView(QWidget *parent) : QTableView(parent),
  m_network(0),
  m_row(-1)
{
  m_network = new QNetworkAccessManager(this);
  connect(m_network, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(replyFinished(QNetworkReply*)));

  m_openInEditorHandler = new OpenInEditorHandler(this);
}

void MongoTableView::contextMenuEvent(QContextMenuEvent *e)
{
  QModelIndex index = indexAt(e->pos());

  // convert index from filter model to source model
  if (QSortFilterProxyModel *filterModel = qobject_cast<QSortFilterProxyModel *>(model())) {
    index = filterModel->mapToSource(index);
  }

  if (index.isValid()) {
    mongo::BSONObj *obj = static_cast<mongo::BSONObj *>(index.internalPointer());
    QMenu *menu = new QMenu(this);

    QAction *action;
    mongo::BSONElement inchi = obj->getField("inchi");

    if (!inchi.eoo()) {
      action = menu->addAction("&Open in Editor");
      m_openInEditorHandler->setMolecule(
        boost::make_shared<chemkit::Molecule>(inchi.str(), "inchi"));
      connect(action, SIGNAL(triggered()),
              m_openInEditorHandler, SLOT(openInEditor()));
    }

    mongo::BSONElement diagram = obj->getField("diagram");

    if (diagram.eoo()) {
      // The field exists, there is more we can do here!
      action = menu->addAction("&Fetch 2D depiction");
      action->setData(inchi.str().c_str());
      connect(action, SIGNAL(triggered()), this, SLOT(fetchImage()));
    }

    // add details action
    action = menu->addAction("Show &Details", this, SLOT(showMoleculeDetailsDialog()));

    m_row = index.row();

    menu->exec(QCursor::pos());
  }
}

void MongoTableView::fetchByCas()
{
  if (!m_network) {
    m_network = new QNetworkAccessManager(this);
    connect(m_network, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
  }
  QAction *action = static_cast<QAction*>(sender());
  // Prompt for a chemical structure name
  QString structureName = action->data().toString();
  if (structureName.isEmpty())
    return;
  else
    while (structureName[0] == '0')
      structureName.remove(0, 1);
  // Fetch the CAS from the NIH database
  m_network->get(QNetworkRequest(
      QUrl("http://cactus.nci.nih.gov/chemical/structure/" + structureName + "/sdf?get3d=true")));

  m_moleculeName = structureName + ".sdf";
  qDebug() << "structure to fetch:" << m_moleculeName;
}

void MongoTableView::fetchIUPAC()
{
  if (!m_network) {
    m_network = new QNetworkAccessManager(this);
    connect(m_network, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
  }
  QAction *action = static_cast<QAction*>(sender());
  // Prompt for a chemical structure name
  QString inchi = action->data().toString();
  if (inchi.isEmpty())
    return;

  // Fetch the IUPAC name from the NIH database
  QString url = "http://cactus.nci.nih.gov/chemical/structure/" + inchi
      + "/iupac_name";
  m_network->get(QNetworkRequest(QUrl(url)));
  qDebug() << url;

  m_moleculeName = "IUPAC";
}

void MongoTableView::fetchImage()
{
  QAction *action = static_cast<QAction*>(sender());
  // Prompt for a chemical structure name
  QString inchi = action->data().toString();
  if (inchi.isEmpty())
    return;

  // Fetch the diagram name from the NIH database
  QString url = "http://cactus.nci.nih.gov/chemical/structure/" + inchi
      + "/image?format=png";
  m_network->get(QNetworkRequest(QUrl(url)));

  m_moleculeName = "diagram";
}

void MongoTableView::showMoleculeDetailsDialog()
{
  MoleculeDetailDialog *dialog = new MoleculeDetailDialog(this);

  mongo::BSONObj *obj = static_cast<mongo::BSONObj *>(currentIndex().internalPointer());
  dialog->setMoleculeObject(obj);

  dialog->show();
}

void MongoTableView::replyFinished(QNetworkReply *reply)
{
  // Read in all the data
  if (!reply->isReadable()) {
    QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                         tr("Network Download Failed"),
                         tr("Network timeout or other error."));
    reply->deleteLater();
    return;
  }

  QByteArray data = reply->readAll();
  MongoModel *model = qobject_cast<MongoModel *>(this->model());

  // Check if the structure was successfully downloaded
  if (data.contains("Page not found (404)")) {
    QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                         tr("Network Download Failed"),
                         tr("Specified molecule could not be found: %1").arg(m_moleculeName));
    reply->deleteLater();
    return;
  }

  if (m_moleculeName == "IUPAC") {
    qDebug() << "IUPAC Name:" << data;
//    model->setIUPACName(m_row, data);
    return;
  }
  else if (m_moleculeName == "diagram") {
    model->setImage2D(m_row, data);
    return;
  }

  QFile file(m_moleculeName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;
  file.write(data);
  file.close();

  QProcess descriptors;
  descriptors.start(QCoreApplication::applicationDirPath() +
                    "/descriptors " + m_moleculeName);
  if (!descriptors.waitForFinished()) {
    qDebug() << "Failed to calculate descriptors.";
    return;
  }
  QByteArray result = descriptors.readAllStandardOutput();
//  model->addIdentifiers(m_row, result);
}

} // End of namespace
