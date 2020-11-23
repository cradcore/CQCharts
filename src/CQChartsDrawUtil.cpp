#include <CQChartsDrawUtil.h>
#include <CQChartsRotatedText.h>
#include <CQChartsPlotSymbol.h>
#include <CQChartsRoundedPolygon.h>
#include <CQChartsViewPlotPaintDevice.h>
#include <CQChartsUtil.h>

#include <CMathUtil.h>

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

namespace CQChartsDrawUtil {

void
setPenBrush(CQChartsPaintDevice *device, const PenBrush &penBrush)
{
  device->setPen      (penBrush.pen);
  device->setBrush    (penBrush.brush);
  device->setAltColor (penBrush.altColor);
  device->setFillAngle(penBrush.fillAngle);
}

void
setPenBrush(QPainter *painter, const PenBrush &penBrush)
{
  painter->setPen  (penBrush.pen);
  painter->setBrush(penBrush.brush);
}

void
setBrush(QBrush &brush, const BrushData &data)
{
  CQChartsUtil::setBrush(brush, data.isVisible(), data.color(), data.alpha(), data.pattern());
}

//---

void
drawDotLine(PaintDevice *device, const PenBrush &penBrush, const BBox &bbox,
            const Length &lineWidth, bool horizontal,
            const Symbol &symbolType, const Length &symbolSize)
{
  // draw solid line
  double lw = (! horizontal ? device->lengthPixelWidth (lineWidth) :
                              device->lengthPixelHeight(lineWidth));

  if (! horizontal) {
    if (lw < 3) {
      setPenBrush(device, penBrush);

      double xc = bbox.getXMid();

      device->drawLine(Point(xc, bbox.getYMin()), Point(xc, bbox.getYMax()));
    }
    else {
      auto pbbox = device->windowToPixel(bbox);

      double xc = pbbox.getXMid();

      BBox pbbox1(xc - lw/2, pbbox.getYMin(), xc + lw/2, pbbox.getYMax());

      drawRoundedPolygon(device, penBrush, device->pixelToWindow(pbbox1));
    }
  }
  else {
    if (lw < 3) {
      setPenBrush(device, penBrush);

      double yc = bbox.getYMid();

      device->drawLine(Point(bbox.getXMin(), yc), Point(bbox.getXMax(), yc));
    }
    else {
      auto pbbox = device->windowToPixel(bbox);

      double yc = pbbox.getYMid();

      BBox pbbox1(pbbox.getXMid(), yc - lw/2, pbbox.getXMax(), yc + lw/2);

      drawRoundedPolygon(device, penBrush, device->pixelToWindow(pbbox1));
    }
  }

  //---

  // draw dot
  Point p;

  if (! horizontal)
    p = Point(bbox.getXMid(), bbox.getYMax());
  else
    p = Point(bbox.getXMax(), bbox.getYMid());

  drawSymbol(device, penBrush, symbolType, p, symbolSize);
}

//---

void
drawRoundedPolygon(PaintDevice *device, const PenBrush &penBrush, const BBox &bbox,
                   const Length &cornerSize, const Sides &sides)
{
  setPenBrush(device, penBrush);

  drawRoundedPolygon(device, bbox, cornerSize, cornerSize, sides);
}

void
drawRoundedPolygon(PaintDevice *device, const BBox &bbox,
                   const Length &cornerSize, const Sides &sides)
{
  drawRoundedPolygon(device, bbox, cornerSize, cornerSize, sides);
}

void
drawRoundedPolygon(PaintDevice *device, const BBox &bbox, const Length &xCornerSize,
                   const Length &yCornerSize, const Sides &sides)
{
  static double minSize1 = 2.5; // pixels
  static double minSize2 = 1.5; // pixels

  auto pbbox = device->windowToPixel(bbox);

  double minSize = pbbox.getMinSize();

  double xsize = device->lengthPixelWidth (xCornerSize);
  double ysize = device->lengthPixelHeight(yCornerSize);

  if      (minSize >= minSize1) {
    CQChartsRoundedPolygon::draw(device, bbox, xsize, ysize, sides);
  }
  else if (minSize >= minSize2) {
    auto pen = device->pen();

    auto pc = pen.color();
    auto f  = (minSize - minSize2)/(minSize1 - minSize2);

    pc.setAlphaF(f*pc.alphaF());

    pen.setColor(pc);

    device->setPen(pen);

    CQChartsRoundedPolygon::draw(device, bbox, xsize, ysize, sides);
  }
  else {
    auto bc = device->brush().color();

    device->setPen(bc);
    device->setBrush(Qt::NoBrush);

    if (pbbox.getWidth() > pbbox.getHeight())
      device->drawLine(Point(bbox.getXMin(), bbox.getYMid()),
                       Point(bbox.getXMax(), bbox.getYMid()));
    else
      device->drawLine(Point(bbox.getXMid(), bbox.getYMin()),
                       Point(bbox.getXMid(), bbox.getYMax()));
  }
}

void
drawRoundedPolygon(PaintDevice *device, const PenBrush &penBrush, const Polygon &poly,
                   const Length &cornerSize)
{
  setPenBrush(device, penBrush);

  drawRoundedPolygon(device, poly, cornerSize, cornerSize);
}

void
drawRoundedPolygon(PaintDevice *device, const Polygon &poly, const Length &cornerSize)
{
  drawRoundedPolygon(device, poly, cornerSize, cornerSize);
}

void
drawRoundedPolygon(PaintDevice *device, const Polygon &poly,
                   const Length &xCornerSize, const Length &yCornerSize)
{
  static double minSize = 2.5; // pixels

  auto bbox = poly.boundingBox();

  double pw = device->lengthPixelWidth (Length(bbox.getWidth (), device->parentUnits()));
  double ph = device->lengthPixelHeight(Length(bbox.getHeight(), device->parentUnits()));

  if (pw > minSize && ph > minSize) {
    double xsize = device->lengthPixelWidth (xCornerSize);
    double ysize = device->lengthPixelHeight(yCornerSize);

    CQChartsRoundedPolygon::draw(device, poly, xsize, ysize);
  }
  else {
    auto bc = device->brush().color();

    device->setPen(bc);
    device->setBrush(Qt::NoBrush);

    if (bbox.getWidth() > minSize) // horizontal line
      device->drawLine(Point(bbox.getXMin(), bbox.getYMid()),
                       Point(bbox.getXMax(), bbox.getYMin()));
    else                           // vertical line
      device->drawLine(Point(bbox.getXMid(), bbox.getYMin()),
                       Point(bbox.getXMid(), bbox.getYMax()));
  }
}

//------

void
drawTextInBox(PaintDevice *device, const BBox &rect,
              const QString &text, const TextOptions &options)
{
  assert(rect.isValid());

  if (! rect.isValid())
    return;

  //---

  // handle html separately
  if (options.html) {
    if (options.scaled)
      CQChartsDrawPrivate::drawScaledHtmlText(device, rect, text, options);
    else
      CQChartsDrawPrivate::drawHtmlText(device, rect.getCenter(), rect, text, options);

    return;
  }

  //---

  auto pen = device->pen();

  if (options.clipped)
    device->save();

  if (options.angle.isZero()) {
    auto text1 = clipTextToLength(text, device->font(), options.clipLength, options.clipElide);

    if (options.clipped)
      device->setClipRect(rect, Qt::IntersectClip);

    device->setPen(pen);

    //---

    QStringList strs;

    if (options.formatted) {
      auto prect = device->windowToPixel(rect);

      CQChartsUtil::formatStringInRect(text1, device->font(), prect, strs,
                                       CQChartsUtil::FormatData(options.formatSeps));
    }
    else
      strs << text1;

    //---

    drawStringsInBox(device, rect, strs, options);
  }
  else {
    if (options.clipped)
      device->setClipRect(rect, Qt::IntersectClip);

    drawRotatedTextInBox(device, rect, text, pen, options);
  }

  if (options.clipped)
    device->restore();
}

void
drawStringsInBox(PaintDevice *device, const BBox &rect,
                 const QStringList &strs, const TextOptions &options)
{
  auto prect = device->windowToPixel(rect);

  QFontMetricsF fm(device->font());

  double th = strs.size()*fm.height() + 2*options.margin;

  if (options.scaled) {
    // calc text scale
    double s = options.scale;

    if (s <= 0.0) {
      double tw = 0;

      for (int i = 0; i < strs.size(); ++i)
        tw = std::max(tw, fm.width(strs[i]));

      tw += 2*options.margin;

      double sx = (tw > 0 ? prect.getWidth ()/tw : 1);
      double sy = (th > 0 ? prect.getHeight()/th : 1);

      s = std::min(sx, sy);
    }

    // scale font
    device->setFont(CQChartsUtil::scaleFontSize(
      device->font(), s, options.minScaleFontSize, options.maxScaleFontSize));

    fm = QFontMetricsF(device->font());

    th = strs.size()*fm.height();
  }

  //---

  double dy = 0.0;

  if      (options.align & Qt::AlignVCenter)
    dy = (prect.getHeight() - th)/2.0;
  else if (options.align & Qt::AlignBottom)
    dy = prect.getHeight() - th;

  double y = prect.getYMin() + dy + fm.ascent();

  for (int i = 0; i < strs.size(); ++i) {
    double dx = 0.0;

    double tw = fm.width(strs[i]);

    if      (options.align & Qt::AlignHCenter)
      dx = (prect.getWidth() - tw)/2;
    else if (options.align & Qt::AlignRight)
      dx = prect.getWidth() - tw;

    double x = prect.getXMin() + dx;

    auto pt = device->pixelToWindow(Point(x, y));

    if (options.contrast)
      drawContrastText(device, pt, strs[i], options.contrastAlpha);
    else
      drawSimpleText(device, pt, strs[i]);

    y += fm.height();
  }
}

void
drawRotatedTextInBox(PaintDevice *device, const BBox &rect,
                     const QString &text, const QPen &pen, const TextOptions &options)
{
  assert(rect.isValid());

  device->setPen(pen);

  CQChartsRotatedText::drawInBox(device, rect, text, options, /*alignBox*/false);
}

//------

CQChartsGeom::BBox
calcTextAtPointRect(PaintDevice *device, const Point &point, const QString &text,
                    const TextOptions &options, bool centered, double pdx, double pdy)
{
  auto text1 = clipTextToLength(text, device->font(), options.clipLength, options.clipElide);

  //---

  // handle html separately
  if (options.html) {
    Size psize = CQChartsDrawPrivate::calcHtmlTextSize(text1, device->font(), options.margin);

    auto sw = device->pixelToWindowWidth (psize.width () + 4);
    auto sh = device->pixelToWindowHeight(psize.height() + 4);

    BBox rect(point.x - sw/2.0, point.y - sh/2.0,
              point.x + sw/2.0, point.y + sh/2.0);

    return rect;
  }

  //---

  QFontMetricsF fm(device->font());

  double ta = fm.ascent();
  double td = fm.descent();

  double tw = fm.width(text1);

  //---

  if (options.angle.isZero()) {
    // calc dx : point is left or hcenter of text (
    // drawContrastText and drawSimpleText wants left aligned
    double dx1 = 0.0, dy1 = 0.0; // pixel

    if (! centered) { // point is left
      if      (options.align & Qt::AlignHCenter) dx1 = -tw/2.0;
      else if (options.align & Qt::AlignRight  ) dx1 = -tw - pdx;
    }
    else {            // point is center
      if      (options.align & Qt::AlignLeft ) dx1 =  tw/2.0 + pdx;
      else if (options.align & Qt::AlignRight) dx1 = -tw/2.0 - pdx;
    }

    if      (options.align & Qt::AlignTop    ) dy1 =  ta + pdy;
    else if (options.align & Qt::AlignBottom ) dy1 = -td - pdy;
    else if (options.align & Qt::AlignVCenter) dy1 = (ta - td)/2.0;

    auto tp = point;

    if (dx1 != 0.0 || dy1 != 0.0) {
      // apply delta (pixels)
      auto pp = device->windowToPixel(tp);

      tp = device->pixelToWindow(Point(pp.x + dx1, pp.y + dy1));
    }

    auto pp = device->windowToPixel(tp);

    BBox prect(pp.x, pp.y - ta, pp.x + tw, pp.y + td);

    return device->pixelToWindow(prect);
  }
  else {
    // calc dx : point is left or hcenter of text
    // CQChartsRotatedText::draw wants center aligned
    auto tp = point;

    if (! centered) {
      double dx1 = -tw/2.0;

      auto pp = device->windowToPixel(tp);

      tp = device->pixelToWindow(Point(pp.x + dx1, pp.y));
    }

    auto pp = device->windowToPixel(tp);

    BBox prect(pp.x, pp.y - ta, pp.x + tw, point.y + td);

    return CQChartsRotatedText::calcBBox(tp.x, tp.y, text1, device->font(),
                                         options, 0, /*alignBox*/true);
  }
}

void
drawTextAtPoint(PaintDevice *device, const Point &point, const QString &text,
                const TextOptions &options, bool centered, double pdx, double pdy)
{
  auto text1 = clipTextToLength(text, device->font(), options.clipLength, options.clipElide);

  //---

  QFontMetricsF fm(device->font());

  double ta = fm.ascent();
  double td = fm.descent();

  double tw = fm.width(text1);

  //---

  // handle html separately
  if (options.html) {
    auto psize = CQChartsDrawPrivate::calcHtmlTextSize(text1, device->font(), options.margin);

    auto sw = device->pixelToWindowWidth (psize.width () + 4);
    auto sh = device->pixelToWindowHeight(psize.height() + 4);

    auto dx = device->pixelToSignedWindowWidth (pdx);
    auto dy = device->pixelToSignedWindowHeight(pdy);

    auto c = Point(point.x + dx, point.y + dy);

    BBox rect;

    if (centered)
      rect = BBox(c.x - sw/2.0, c.y - sh/2.0, c.x + sw/2.0, c.y + sh/2.0);
    else
      rect = BBox(c.x, c.y, c.x + sw, c.y + sh);

    if (options.scaled)
      CQChartsDrawPrivate::drawScaledHtmlText(device, rect, text1, options);
    else
      CQChartsDrawPrivate::drawHtmlText(device, c, rect, text1, options, pdx, pdy);

    return;
  }

  //---

  if (options.angle.isZero()) {
    // calc dx : point is left or hcenter of text (
    // drawContrastText and drawSimpleText wants left aligned
    double dx1 = 0.0, dy1 = 0.0; // pixel

    if (! centered) { // point is left
      if      (options.align & Qt::AlignHCenter) dx1 = -tw/2.0;
      else if (options.align & Qt::AlignRight  ) dx1 = -tw - pdx;
    }
    else {            // point is center
      if      (options.align & Qt::AlignLeft ) dx1 =  tw/2.0 + pdx;
      else if (options.align & Qt::AlignRight) dx1 = -tw/2.0 - pdx;
    }

    if      (options.align & Qt::AlignTop    ) dy1 =  ta + pdy;
    else if (options.align & Qt::AlignBottom ) dy1 = -td - pdy;
    else if (options.align & Qt::AlignVCenter) dy1 = (ta - td)/2.0;

    auto tp = point;

    if (dx1 != 0.0 || dy1 != 0.0) {
      // apply delta (pixels)
      auto pp = device->windowToPixel(tp);

      tp = device->pixelToWindow(Point(pp.x + dx1, pp.y + dy1));
    }

    if (options.contrast)
      drawContrastText(device, tp, text1, options.contrastAlpha);
    else
      drawSimpleText(device, tp, text1);
  }
  else {
    // calc dx : point is left or hcenter of text
    // CQChartsRotatedText::draw wants center aligned
    auto tp = point;

    if (! centered) {
      double dx1 = -tw/2.0;

      auto pp = device->windowToPixel(tp);

      tp = device->pixelToWindow(Point(pp.x + dx1, pp.y));
    }

    CQChartsRotatedText::draw(device, tp, text1, options, /*alignBox*/true);
  }
}

//------

void
drawAlignedText(PaintDevice *device, const Point &p, const QString &text,
                Qt::Alignment align, double pdx, double pdy)
{
  QFontMetricsF fm(device->font());

  double tw = fm.width(text);
  double ta = fm.ascent ();
  double td = fm.descent();

  double dx1 = 0.0, dy1 = 0.0;

  if      (align & Qt::AlignLeft   ) dx1 = pdx;
  else if (align & Qt::AlignRight  ) dx1 = -tw - pdx;
  else if (align & Qt::AlignHCenter) dx1 = -tw/2;

  if      (align & Qt::AlignTop    ) dy1 =  ta + pdy;
  else if (align & Qt::AlignBottom ) dy1 = -td - pdy;
  else if (align & Qt::AlignVCenter) dy1 = (ta - td)/2;

  auto pp = device->windowToPixel(p);
  auto pt = device->pixelToWindow(Point(pp.x + dx1, pp.y + dy1));

  drawSimpleText(device, pt, text);
}

//------

CQChartsGeom::BBox
calcAlignedTextRect(PaintDevice *device, const QFont &font, const Point &p,
                    const QString &text, Qt::Alignment align, double pdx, double pdy)
{
  QFontMetricsF fm(font);

  double tw = fm.width(text);
  double ta = fm.ascent ();
  double td = fm.descent();

  double dx1 = 0.0, dy1 = 0.0;

  if      (align & Qt::AlignLeft   ) dx1 = pdx;
  else if (align & Qt::AlignRight  ) dx1 = -tw - pdx;
  else if (align & Qt::AlignHCenter) dx1 = -tw/2;

  if      (align & Qt::AlignTop    ) dy1 =  ta + pdy;
  else if (align & Qt::AlignBottom ) dy1 = -td - pdy;
  else if (align & Qt::AlignVCenter) dy1 = (ta - td)/2;

  auto pp = device->windowToPixel(p);
  auto pt = device->pixelToWindow(Point(pp.x + dx1, pp.y + dy1));

  BBox pbbox(pt.x, pt.y - ta, pt.x + tw, pt.y + td);

  return device->pixelToWindow(pbbox);
}

//------

void
drawContrastText(PaintDevice *device, const Point &p, const QString &text, const Alpha &alpha)
{
  auto pen = device->pen();

  //---

  // set contrast color
  // TODO: allow set type (invert, bw) and alpha
//auto icolor = CQChartsUtil::invColor(pen.color());
  auto icolor = CQChartsUtil::bwColor(pen.color());

  icolor.setAlphaF(alpha.value());

  //---

  auto pp = device->windowToPixel(p);

  // draw contrast outline
  device->setPen(icolor);

  for (int dy = -2; dy <= 2; ++dy) {
    for (int dx = -2; dx <= 2; ++dx) {
      if (dx != 0 || dy != 0) {
        auto p1 = device->pixelToWindow(Point(pp.x + dx, pp.y + dy));

        drawSimpleText(device, p1, text);
      }
    }
  }

  //---

  // draw text
  device->setPen(pen);

  drawSimpleText(device, p, text);
}

//------

CQChartsGeom::Size
calcTextSize(const QString &text, const QFont &font, const TextOptions &options)
{
  if (options.html)
    return CQChartsDrawPrivate::calcHtmlTextSize(text, font, options.margin);

  //---

  QFontMetricsF fm(font);

  return Size(fm.width(text), fm.height());
}

//------

void
drawCenteredText(PaintDevice *device, const Point &pos, const QString &text)
{
  QFontMetricsF fm(device->font());

  auto ppos = device->windowToPixel(pos);

  Point ppos1(ppos.x - fm.width(text)/2, ppos.y + (fm.ascent() - fm.descent())/2);

  drawSimpleText(device, device->pixelToWindow(ppos1), text);
}

//------

void
drawSimpleText(PaintDevice *device, const Point &pos, const QString &text)
{
  device->drawText(pos, text);
}

//---

void
drawSymbol(PaintDevice *device, const PenBrush &penBrush, const Symbol &symbol,
           const Point &c, const Length &size)
{
  setPenBrush(device, penBrush);

  drawSymbol(device, symbol, c, size);
}

void
drawSymbol(PaintDevice *device, const Symbol &symbol, const Point &c, const Length &size)
{
  if (! size.isValid())
    return;

  CQChartsPlotSymbolRenderer srenderer(device, Point(c), size);

  if (device->brush().style() != Qt::NoBrush) {
    CQChartsPlotSymbolMgr::fillSymbol(symbol, &srenderer);

    if (device->pen().style() != Qt::NoPen)
      CQChartsPlotSymbolMgr::strokeSymbol(symbol, &srenderer);
  }
  else {
    if (device->pen().style() != Qt::NoPen)
      CQChartsPlotSymbolMgr::drawSymbol(symbol, &srenderer);
  }
}

void
drawSymbol(PaintDevice *device, const Symbol &symbol, const BBox &bbox)
{
  assert(bbox.isValid());

  auto pbbox = device->windowToPixel(bbox);

  double cx = bbox.getXMid();
  double cy = bbox.getYMid();
  double ss = pbbox.getMinSize();

  Length symbolSize(ss/2.0, device->parentUnits());

  drawSymbol(device, symbol, Point(cx, cy), symbolSize);
}

//---

void
drawPieSlice(PaintDevice *device, const Point &c, double ri, double ro,
             const Angle &a1, const Angle &a2, bool isInvertX, bool isInvertY)
{
  QPainterPath path;

  pieSlicePath(path, c, ri, ro, a1, a2, isInvertX, isInvertY);

  device->drawPath(path);
}

void
pieSlicePath(QPainterPath &path, const Point &c, double ri, double ro, const Angle &a1,
             const Angle &a2, bool isInvertX, bool isInvertY)
{
  BBox bbox(c.x - ro, c.y - ro, c.x + ro, c.y + ro);

  //---

  if (! CMathUtil::isZero(ri)) {
    BBox bbox1(c.x - ri, c.y - ri, c.x + ri, c.y + ri);

    //---

    double da = (isInvertX != isInvertY ? -1 : 1);

    double ra1 = da*a1.radians();
    double ra2 = da*a2.radians();

    auto p1 = CQChartsGeom::circlePoint(c, ri, ra1);
    auto p2 = CQChartsGeom::circlePoint(c, ro, ra1);
    auto p3 = CQChartsGeom::circlePoint(c, ri, ra2);
    auto p4 = CQChartsGeom::circlePoint(c, ro, ra2);

    path.moveTo(p1.x, p1.y);
    path.lineTo(p2.x, p2.y);

    path.arcTo(bbox.qrect(), -a1.value(), a1.value() - a2.value());

    path.lineTo(p4.x, p4.y);
    path.lineTo(p3.x, p3.y);

    path.arcTo(bbox1.qrect(), -a2.value(), a2.value() - a1.value());
  }
  else {
    double a21 = a2.value() - a1.value();

    if (std::abs(a21) < 360.0) {
      path.moveTo(QPointF(c.x, c.y));

      path.arcTo(bbox.qrect(), -a1.value(), a1.value() - a2.value());
    }
    else {
      path.addEllipse(bbox.qrect());
    }
  }

  path.closeSubpath();
}

//---

void
drawEllipse(PaintDevice *device, const BBox &bbox)
{
  QPainterPath path;

  ellipsePath(path, bbox);

  device->drawPath(path);
}

void
ellipsePath(QPainterPath &path, const BBox &bbox)
{
  double xc = bbox.getXMid();
  double yc = bbox.getYMid();

  double w = bbox.getWidth ();
  double h = bbox.getHeight();

  double xr = w/2.0;
  double yr = h/2.0;

  double f = 4.0*(std::sqrt(2.0) - 1.0)/3.0;

  double dx = xr*f;
  double dy = yr*f;

  path.moveTo (                                    xc + xr, yc     );
  path.cubicTo(xc + xr, yc + dy, xc + dx, yc + yr, xc     , yc + yr);
  path.cubicTo(xc - dx, yc + yr, xc - xr, yc + dy, xc - xr, yc     );
  path.cubicTo(xc - xr, yc - dy, xc - dx, yc - yr, xc     , yc - yr);
  path.cubicTo(xc + dx, yc - yr, xc + xr, yc - dy, xc + xr, yc     );

  path.closeSubpath();
}

//---

void
drawArc(PaintDevice *device, const BBox &bbox, const Angle &angle, const Angle &dangle)
{
  QPainterPath path;

  arcPath(path, bbox, angle, dangle);

  device->drawPath(path);
}

void
arcPath(QPainterPath &path, const BBox &bbox, const Angle &angle, const Angle &dangle)
{
  auto c = bbox.getCenter();

  auto rect = bbox.qrect();

  path.arcMoveTo(rect, -angle.value());
  path.arcTo    (rect, -angle.value(), -dangle.value());
  path.lineTo   (c.x, c.y);

  path.closeSubpath();
}

//---

void
drawArcSegment(PaintDevice *device, const BBox &ibbox, const BBox &obbox,
               const Angle &angle, const Angle &dangle)
{
  QPainterPath path;

  arcSegmentPath(path, ibbox, obbox, angle, dangle);

  device->drawPath(path);
}

void
arcSegmentPath(QPainterPath &path, const BBox &ibbox, const BBox &obbox,
               const Angle &angle, const Angle &dangle)
{
  // arc segment for start angle and delta angle for circles in inner and outer boxes
  auto angle2 = angle + dangle;

  auto irect = ibbox.qrect();
  auto orect = obbox.qrect();

  //---

  path.arcMoveTo(orect, -angle.value());
  path.arcTo    (orect, -angle.value() , -dangle.value());
  path.arcTo    (irect, -angle2.value(),  dangle.value());

  path.closeSubpath();
}

//---

void
drawArcsConnector(PaintDevice *device, const BBox &ibbox, const Angle &a1,
                  const Angle &da1, const Angle &a2, const Angle &da2, bool isSelf)
{
  QPainterPath path;

  arcsConnectorPath(path, ibbox, a1, da1, a2, da2, isSelf);

  // draw path
  device->drawPath(path);
}

void
arcsConnectorPath(QPainterPath &path, const BBox &ibbox, const Angle &a1,
                  const Angle &da1, const Angle &a2, const Angle &da2,
                  bool isSelf)
{
  // draw connecting arc between inside of two arc segments
  // . arc segments have start angle and delta angle for circles in inner and outer boxes
  // isSelf is true if connecting arcs are the same
  auto a11 = a1 + da1;
  auto a21 = a2 + da2;

  auto irect = ibbox.qrect();
  auto c     = irect.center();

  //---

  path.arcMoveTo(irect, -a1 .value());   auto p1 = path.currentPosition();
  path.arcMoveTo(irect, -a11.value());   auto p2 = path.currentPosition();
  path.arcMoveTo(irect, -a2 .value()); //auto p3 = path.currentPosition();
  path.arcMoveTo(irect, -a21.value());   auto p4 = path.currentPosition();

  //--

  if (! isSelf) {
    path.moveTo(p1);
    path.quadTo(c, p4);
    path.arcTo (irect, -a21.value(), da2.value());
    path.quadTo(c, p2);
    path.arcTo (irect, -a11.value(), da1.value());

    path.closeSubpath();
  }
  else {
    path.moveTo(p1);
    path.quadTo(c, p2);
    path.arcTo (irect, -a11.value(), da1.value());

    path.closeSubpath();
  }
}

//---

void
edgePath(QPainterPath &path, const BBox &ibbox, const BBox &obbox, bool isLine)
{
  // x from right of source rect to left of dest rect
  bool swapped = false;

  double x1 = ibbox.getXMax(), x2 = obbox.getXMin();

  if (x1 > x2) {
    x1 = obbox.getXMax(), x2 = ibbox.getXMin();
    swapped = true;
  }

  // start y range from source node, and end y range from dest node
  double y11 = ibbox.getYMax(), y12 = ibbox.getYMin();
  double y21 = obbox.getYMax(), y22 = obbox.getYMin();

  if (swapped) {
    std::swap(y11, y21);
    std::swap(y12, y22);
  }

  // curve control point x at 1/3 and 2/3
  double x3 = CMathUtil::lerp(1.0/3.0, x1, x2);
  double x4 = CMathUtil::lerp(2.0/3.0, x1, x2);

  if (isLine) {
    double y1m = CMathUtil::avg(y11, y12);
    double y2m = CMathUtil::avg(y21, y22);

    path.moveTo (QPointF(x1, y1m));
    path.cubicTo(QPointF(x3, y1m), QPointF(x4, y2m), QPointF(x2, y2m));
  }
  else {
    path.moveTo (QPointF(x1, y11));
    path.cubicTo(QPointF(x3, y11), QPointF(x4, y21), QPointF(x2, y21));
    path.lineTo (QPointF(x2, y22));
    path.cubicTo(QPointF(x4, y22), QPointF(x3, y12), QPointF(x1, y12));

    path.closeSubpath();
  }
}

//---

QString
clipTextToLength(PaintDevice *device, const QString &text,
                 const Length &clipLength, const Qt::TextElideMode &clipElide)
{
  if (! clipLength.isValid())
    return text;

  double clipLengthPixels = device->lengthPixelWidth(clipLength);

  return clipTextToLength(text, device->font(), clipLengthPixels, clipElide);
}

QString
clipTextToLength(const QString &text, const QFont &font, double clipLength,
                 const Qt::TextElideMode &clipElide)
{
  if (clipLength <= 0.0)
    return text;

  if (clipElide != Qt::ElideLeft && clipElide != Qt::ElideRight)
    return text;

  //---

  QFontMetricsF fm(font);

  double ellipsisWidth = fm.width("...");

  if (ellipsisWidth > clipLength)
    return "";

  double clipLength1 = clipLength - ellipsisWidth;

  //---

  auto isClipped = [&](const QString &str) {
    double w = fm.width(str);

    return (w > clipLength1);
  };

  auto isLenClipped = [&](const QString &str, int len) {
    if      (clipElide == Qt::ElideLeft)
      return isClipped(str.right(len));
    else if (clipElide == Qt::ElideRight)
      return isClipped(str.left(len));
    else
      assert(false);
  };

  //---

  if (! isClipped(text))
    return text;

  //---

  using LenClipped = std::map<int, bool>;

  LenClipped lenClipped;

  int len = text.length();

  int len1 = 0;
  int len2 = len;

  int midLen = (len1 + len2)/2;

  while (midLen > 0) {
    auto pl = lenClipped.find(midLen);

    bool clipped;

    if (pl == lenClipped.end()) {
      clipped = isLenClipped(text, midLen);

      lenClipped[midLen] = clipped;
    }
    else {
      clipped = (*pl).second;

      if (! clipped && midLen == len1)
        break;
    }

    if (clipped)
      len2 = midLen;
    else
      len1 = midLen;

    midLen = (len1 + len2)/2;
  }

  QString text1;

  if      (clipElide == Qt::ElideLeft)
    text1 = "..." + text.right(midLen);
  else if (clipElide == Qt::ElideRight)
    text1 = text.left(midLen) + "...";
  else
    assert(false);

  return text1;
}

}

//------

namespace CQChartsDrawPrivate {

CQChartsGeom::Size
calcHtmlTextSize(const QString &text, const QFont &font, int margin)
{
  QTextDocument td;

  td.setDocumentMargin(margin);
  td.setHtml(text);
  td.setDefaultFont(font);

  auto *layout = td.documentLayout();

  return Size(layout->documentSize());
}

//------

void
drawScaledHtmlText(PaintDevice *device, const BBox &tbbox, const QString &text,
                   const TextOptions &options)
{
  assert(tbbox.isValid());

  // calc scale
  double s = options.scale;

  if (s <= 0.0) {
    auto psize = calcHtmlTextSize(text, device->font(), options.margin);

    double pw = psize.width ();
    double ph = psize.height();

    auto ptbbox = device->windowToPixel(tbbox);

    double xs = ptbbox.getWidth ()/pw;
    double ys = ptbbox.getHeight()/ph;

    s = std::min(xs, ys);
  }

  //---

  // scale font
  device->setFont(CQChartsUtil::scaleFontSize(device->font(), s));

  //---

  drawHtmlText(device, tbbox.getCenter(), tbbox, text, options);
}

void
drawHtmlText(PaintDevice *device, const Point &center, const BBox &tbbox,
             const QString &text, const TextOptions &options, double pdx, double pdy)
{
  assert(tbbox.isValid());

  auto ptbbox = device->windowToPixel(tbbox);

  //---

  auto psize = calcHtmlTextSize(text, device->font(), options.margin);

  double c = options.angle.cos();
  double s = options.angle.sin();

  double pdx1 = 0.0, pdy1 = 0.0;

  if (! options.formatted) {
    if      (options.align & Qt::AlignHCenter)
      pdx1 = (ptbbox.getWidth() - psize.width())/2.0;
    else if (options.align & Qt::AlignRight)
      pdx1 = ptbbox.getWidth() - psize.width();

    if      (options.align & Qt::AlignVCenter)
      pdy1 = (ptbbox.getHeight() - psize.height())/2.0;
    else if (options.align & Qt::AlignBottom)
      pdy1 = ptbbox.getHeight() - psize.height();
  }

  if (! options.angle.isZero()) {
    double pdx2 = c*pdx1 - s*pdy1;
    double pdy2 = s*pdx1 + c*pdy1;

    pdx1 = pdx2;
    pdy1 = pdy2;
  }

  pdx += pdx1;
  pdy += pdy1;

  auto ptbbox1 = ptbbox.translated(pdx, pdy); // inner text rect

  //---

  QImage    image;
  QPainter *painter  = nullptr;
  QPainter *ipainter = nullptr;

  if (device->isInteractive()) {
    painter = dynamic_cast<CQChartsViewPlotPaintDevice *>(device)->painter();
  }
  else {
    image = CQChartsUtil::initImage(QSize(int(ptbbox.getWidth()), int(ptbbox.getHeight())));

    ipainter = new QPainter(&image);

    painter = ipainter;
  }

  //---

  painter->save();

  if (! options.angle.isZero()) {
  //auto tc = ptbbox1.getCenter().qpoint();
    auto tc = device->windowToPixel(center).qpoint();

    painter->translate(tc);
    painter->rotate(-options.angle.value());
    painter->translate(-tc);
  }

  QTextDocument td;

  td.setDocumentMargin(options.margin);
  td.setHtml(text);
  td.setDefaultFont(device->font());

  auto ptbbox2 = ptbbox1.translated(-ptbbox.getXMin(), -ptbbox.getYMin()); // move to origin

  if (device->isInteractive())
    painter->translate(ptbbox1.getXMin(), ptbbox1.getYMin());

  if (options.angle.isZero())
    painter->setClipRect(ptbbox2.qrect(), Qt::IntersectClip);

  int pm = 8;

  td.setPageSize(QSizeF(ptbbox.getWidth() + pm, ptbbox.getHeight() + pm));

  //---

  QTextCursor cursor(&td);

  cursor.select(QTextCursor::Document);

  QTextBlockFormat f;

  f.setAlignment(options.align);

  cursor.setBlockFormat(f);

  //---

  QAbstractTextDocumentLayout::PaintContext ctx;

  auto pc = device->pen().color();

  ctx.palette.setColor(QPalette::Text, pc);

  auto *layout = td.documentLayout();

  layout->setPaintDevice(painter->device());

  if (options.contrast) {
  //auto ipc = CQChartsUtil::invColor(pc);
    auto ipc = CQChartsUtil::bwColor(pc);

    ctx.palette.setColor(QPalette::Text, ipc);

    for (int dy = -2; dy <= 2; ++dy) {
      for (int dx = -2; dx <= 2; ++dx) {
        if (dx != 0 || dy != 0) {
          painter->translate(dx, dy);

          layout->draw(painter, ctx);

          painter->translate(-dx, -dy);
        }
      }
    }

    ctx.palette.setColor(QPalette::Text, pc);

    layout->draw(painter, ctx);
  }
  else {
    layout->draw(painter, ctx);
  }

  if (device->isInteractive())
    painter->translate(-ptbbox1.getXMin(), -ptbbox1.getYMin());

  //---

  painter->restore();

  delete ipainter;
}

}
