#include <CQChartsNoDataObj.h>
#include <CQChartsView.h>
#include <QPainter>

CQChartsNoDataObj::
CQChartsNoDataObj(CQChartsPlot *plot) :
 CQChartsPlotObj(plot, CQChartsGeom::BBox(0, 0, 1, 1))
{
}

void
CQChartsNoDataObj::
draw(QPainter *painter)
{
  CQChartsView *view = plot_->view();

  // get plot inside bbox in view coords
  CQChartsGeom::BBox bbox = plot_->innerViewBBox();

  // get pixel position of center
  double xm = bbox.getXMid();
  double ym = bbox.getXMid();

  CQChartsGeom::Point c = view->windowToPixel(CQChartsGeom::Point(xm, ym));

  //--

  // draw no data text
  QString msg = "NO DATA";

  QFontMetricsF fm(view->font());

  double fw = fm.width(msg);

  painter->setFont(view->font());

  painter->drawText(QPointF(c.x - fw/2, c.y + (fm.ascent() - fm.descent())/2), msg);
}
