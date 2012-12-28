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

#include "diagramtooltipitem.h"

#include <mongo/client/dbclient.h>

#include <QtGui>

#include "vtkContext2D.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkContextScene.h"
#include "vtkQImageToImageSource.h"

namespace MongoChem {

using namespace mongo;

vtkStandardNewMacro(DiagramTooltipItem)

DiagramTooltipItem::DiagramTooltipItem()
  : vtkTooltipItem()
{
  QSettings settings;
  std::string host = settings.value("hostname").toString().toStdString();

  try {
    m_db.connect(host);
  }
  catch (DBException &e) {
    std::cerr << "Failed to connect to MongoDB: " << e.what() << std::endl;
    return;
  }
}

void DiagramTooltipItem::PrintSelf(ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

bool DiagramTooltipItem::Paint(vtkContext2D *painter)
{
  // name of molecule corresponding to the point
  std::string name = this->GetText();
  if (name.empty())
    return false;

  // query database
  QSettings settings;
  std::string collection = settings.value("collection").toString().toStdString();
  std::string moleculesCollection = collection + ".molecules";

  BSONObj obj = m_db.findOne(moleculesCollection, QUERY("name" << name));
  if (obj.isEmpty()) {
    // just paint the name using the superclass
    return Superclass::Paint(painter);
  }

  BSONElement diagram = obj.getField("diagram");
  if (diagram.eoo()) {
    // just paint the name using the superclass
    return Superclass::Paint(painter);
  }

  // convert bson bin data in png format to a QImage
  int binDataLength = 0;
  const char *binData = diagram.binData(binDataLength);
  QImage image = QImage::fromData(reinterpret_cast<const unsigned char *>(binData),
                                  binDataLength,
                                  "png");

  // layout diagram and molecule name vertically and render to a QImage
  QWidget widget;
  QVBoxLayout *layout = new QVBoxLayout;
  QLabel imageLabel;
  imageLabel.setPixmap(QPixmap::fromImage(image));
  layout->addWidget(&imageLabel);
  QLabel nameLabel(name.c_str());
  nameLabel.setWordWrap(true);
  nameLabel.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  layout->addWidget(&nameLabel);
  widget.setLayout(layout);
  widget.resize(300, 300);

  QImage tooltipImage(widget.width(), widget.height(), QImage::Format_ARGB32);
  QPainter qpainter(&tooltipImage);
  widget.render(&qpainter);

  // convert QImage to vtkImageData
  vtkQImageToImageSource *converter = vtkQImageToImageSource::New();
  converter->SetQImage(&tooltipImage);
  converter->Update();
  vtkImageData *vtkImage = converter->GetOutput();

  // setup painter
  painter->ApplyPen(this->GetPen());
  painter->ApplyBrush(this->GetBrush());

  // determine where to draw the tooltip
  vtkVector2f drawPosition = this->GetPositionVector();
  int drawPositionX = static_cast<int>(drawPosition.GetX());
  int drawPositionY = static_cast<int>(drawPosition.GetY());
  if(drawPositionX + tooltipImage.width() > this->Scene->GetViewWidth()){
    drawPosition[0] -= static_cast<float>(tooltipImage.width());
  }
  if(drawPositionY + tooltipImage.height() > this->Scene->GetViewHeight()){
    drawPosition[1] -= static_cast<float>(tooltipImage.height());
  }

  // draw background rect
  painter->DrawRect(drawPosition.GetX(),
                    drawPosition.GetY(),
                    static_cast<float>(tooltipImage.width()),
                    static_cast<float>(tooltipImage.height()));

  // draw molecule diagram
  painter->DrawImage(drawPosition.GetX(),
                     drawPosition.GetY(),
                     vtkImage);

  // cleanup memory
  converter->Delete();

  return true;
}

} // end MongoChem namespace