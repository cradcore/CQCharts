#include <CQChartsPaintDevice.h>
#include <CQChartsPlot.h>
#include <QBuffer>

void
CQChartsPaintDevice::
drawPolygonSides(const BBox &bbox, int n, const Angle &angle)
{
  QPainterPath path;

  if (! CQChartsDrawUtil::polygonSidesPath(path, bbox, n, angle))
    return;

  drawPath(path);
}

void
CQChartsPaintDevice::
drawDiamond(const BBox &bbox)
{
  QPainterPath path;

  CQChartsDrawUtil::diamondPath(path, bbox);

  drawPath(path);
}

void
CQChartsPaintDevice::
drawRoundedLine(const Point &p1, const Point &p2, double w)
{
  QPainterPath path;

  CQChartsDrawUtil::roundedLinePath(path, p1, p2, w);

  fillPath(path, QBrush(pen().color()));
}

CQChartsGeom::BBox
CQChartsPaintDevice::
windowToPixel(const BBox &r) const
{
  return (! view_ ? (! plot_ ? r : plot_->windowToPixel(r)) : view_->windowToPixel(r));
}

CQChartsGeom::BBox
CQChartsPaintDevice::
pixelToWindow(const BBox &r) const
{
  return (! view_ ? (! plot_ ? r : plot_->pixelToWindow(r)) : view_->pixelToWindow(r));
}

CQChartsGeom::Point
CQChartsPaintDevice::
windowToPixel(const Point &p) const
{
  return (! view_ ? (! plot_ ? p : plot_->windowToPixel(p)) : view_->windowToPixel(p));
}

CQChartsGeom::Point
CQChartsPaintDevice::
pixelToWindow(const Point &p) const
{
  return (! view_ ? (! plot_ ? p : plot_->pixelToWindow(p)) : view_->pixelToWindow(p));
}

CQChartsGeom::Polygon
CQChartsPaintDevice::
windowToPixel(const Polygon &p) const
{
  return (! view_ ? (! plot_ ? p : plot_->windowToPixel(p)) : view_->windowToPixel(p));
}

CQChartsGeom::Size
CQChartsPaintDevice::
pixelToWindowSize(const Size &s) const
{
  return (! view_ ? (! plot_ ? s : plot_->pixelToWindowSize(s)) : view_->pixelToWindowSize(s));
}

double
CQChartsPaintDevice::
lengthPixelWidth(const CQChartsLength &w) const
{
  return (! view_ ? (! plot_ ? w.value() :
    plot_->lengthPixelWidth(w)) : view_->lengthPixelWidth(w));
}

double
CQChartsPaintDevice::
lengthPixelHeight(const CQChartsLength &h) const
{
  return (! view_ ? (! plot_ ? h.value() :
    plot_->lengthPixelHeight(h)) : view_->lengthPixelHeight(h));
}

double
CQChartsPaintDevice::
lengthWindowWidth(const CQChartsLength &w) const
{
  return (! view_ ? (! plot_ ? w.value() :
    plot_->lengthPlotWidth(w)) : view_->lengthViewWidth(w));
}

double
CQChartsPaintDevice::
lengthWindowHeight(const CQChartsLength &h) const
{
  return (! view_ ? (! plot_ ? h.value() :
    plot_->lengthPlotHeight(h)) : view_->lengthViewHeight(h));
}

double
CQChartsPaintDevice::
pixelToWindowWidth(double pw) const
{
  return (! view_ ? (! plot_ ? 0.0 :
    plot_->pixelToWindowWidth(pw)) : view_->pixelToWindowWidth(pw));
}

double
CQChartsPaintDevice::
pixelToWindowHeight(double ph) const
{
  return (! view_ ? (! plot_ ? 0.0 :
    plot_->pixelToWindowHeight(ph)) : view_->pixelToWindowHeight(ph));
}

double
CQChartsPaintDevice::
pixelToSignedWindowWidth(double pw) const
{
  return (! view_ ? (! plot_ ? 0.0 :
    plot_->pixelToSignedWindowWidth(pw)) : view_->pixelToSignedWindowWidth(pw));
}

double
CQChartsPaintDevice::
pixelToSignedWindowHeight(double ph) const
{
  return (! view_ ? (! plot_ ? 0.0 :
    plot_->pixelToSignedWindowHeight(ph)) : view_->pixelToSignedWindowHeight(ph));
}

QPainterPath
CQChartsPaintDevice::
windowToPixel(const QPainterPath &path) const
{
  return (! view_ ? (! plot_ ? path : plot_->windowToPixel(path)) : view_->windowToPixel(path));
}

bool
CQChartsPaintDevice::
isInvertX() const
{
  return (plot_ ? plot_->isInvertX() : false);
}

bool
CQChartsPaintDevice::
isInvertY() const
{
  return (plot_ ? plot_->isInvertY() : false);
}
