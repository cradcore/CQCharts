#include <CQChartsTitle.h>
#include <CQChartsPlot.h>
#include <CQChartsAxis.h>
#include <CQChartsView.h>
#include <CQChartsUtil.h>
#include <CQPropertyViewModel.h>
#include <CQChartsRenderer.h>
#include <QRectF>

CQChartsTitle::
CQChartsTitle(CQChartsPlot *plot) :
 CQChartsTextBoxObj(plot)
{
  setText("Title");

  font_.setPointSizeF(1.5*font_.pointSizeF());
}

void
CQChartsTitle::
redraw()
{
  plot_->update();
}

QString
CQChartsTitle::
locationStr() const
{
  switch (location_) {
    case Location::TOP:    return "top";
    case Location::CENTER: return "center";
    case Location::BOTTOM: return "bottom";
    default:               return "none";
  }
}

void
CQChartsTitle::
setLocationStr(const QString &str)
{
  QString lstr = str.toLower();

  if      (lstr == "top"   ) location_ = Location::TOP;
  else if (lstr == "center") location_ = Location::CENTER;
  else if (lstr == "bottom") location_ = Location::BOTTOM;

  updatePosition();
}

void
CQChartsTitle::
updatePosition()
{
  plot_->updateTitlePosition();

  plot_->update();
}

void
CQChartsTitle::
updateLocation(const CQChartsGeom::BBox &bbox)
{
  QSizeF ts = calcSize();

  CQChartsTitle::Location location = this->location();

//double xm = plot_->pixelToWindowWidth (8);
  double ym = plot_->pixelToWindowHeight(8);

  double kx = bbox.getXMid() - ts.width()/2;

  double ky = 0.0;

  CQChartsAxis *xAxis = plot_->xAxis();

  if      (location == CQChartsTitle::Location::TOP) {
    if (! isInside()) {
      ky = bbox.getYMax() + ym;

      if (xAxis && xAxis->side() == CQChartsAxis::Side::TOP_RIGHT)
        ky += xAxis->bbox().getHeight();
    }
    else
      ky = bbox.getYMax() - ts.height() - ym;
  }
  else if (location == CQChartsTitle::Location::CENTER) {
    ky = bbox.getYMid() - ts.height()/2;
  }
  else if (location == CQChartsTitle::Location::BOTTOM) {
    if (! isInside()) {
      ky = bbox.getYMin() - ts.height() - ym;

      if (xAxis && xAxis->side() == CQChartsAxis::Side::BOTTOM_LEFT)
        ky -= xAxis->bbox().getHeight();
    }
    else
      ky = bbox.getYMin() + ym;
  }
  else {
    ky = bbox.getYMid() - ts.height()/2;
  }

  setPosition(QPointF(kx, ky));
}

void
CQChartsTitle::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  model->addProperty(path, this, "visible" );
  model->addProperty(path, this, "location");
  model->addProperty(path, this, "inside"  );

  CQChartsTextBoxObj::addProperties(model, path);
}

QSizeF
CQChartsTitle::
calcSize()
{
  if (text().length()) {
    QFontMetricsF fm(font());

    double pw = fm.width(text());
    double ph = fm.height();

    double ww = plot_->pixelToWindowWidth (pw);
    double wh = plot_->pixelToWindowHeight(ph);

    double xp = plot_->pixelToWindowWidth (padding());
    double yp = plot_->pixelToWindowHeight(padding());

    double xm = plot_->pixelToWindowWidth (margin());
    double ym = plot_->pixelToWindowHeight(margin());

    size_ = QSizeF(ww + 2*xp + 2*xm, wh + 2*yp + 2*ym);
  }
  else {
    size_ = QSizeF();
  }

  return size_;
}

bool
CQChartsTitle::
contains(const CQChartsGeom::Point &p) const
{
  if (! isVisible())
    return false;

  return bbox().inside(p);
}

void
CQChartsTitle::
draw(CQChartsRenderer *renderer)
{
  if (! isVisible())
    return;

  if (! text().length())
     return;

  //---

  renderer->save();

  QRectF clipRect = CQChartsUtil::toQRect(plot_->calcPixelRect());

  renderer->setClipRect(clipRect);

  //---

  double x = position_.x(); // bottom
  double y = position_.y(); // top
  double w = size_.width ();
  double h = size_.height();

  bbox_ = CQChartsGeom::BBox(x, y, x + w, y + h);

  double xp = plot_->pixelToWindowWidth (padding());
  double yp = plot_->pixelToWindowHeight(padding());

  CQChartsGeom::BBox ibbox = CQChartsGeom::BBox(x + xp, y + yp, x + w - xp, y + h - yp);

  //---

  CQChartsGeom::BBox prect, pirect;

  plot_->windowToPixel(bbox_, prect);
  plot_->windowToPixel(ibbox, pirect);

  //---

  CQChartsBoxObj::draw(renderer, CQChartsUtil::toQRect(pirect));

  //---

  QFontMetricsF fm(font());

  renderer->setFont(font());

  renderer->setPen(interpColor(0, 1));

  renderer->drawText(QPointF(pirect.getXMin() + margin(),
                             pirect.getYMax() - margin() - fm.descent()), text());

  //---

  if (plot_->showBoxes())
    plot_->drawWindowRedBox(renderer, bbox_);

  //---

  renderer->restore();
}
