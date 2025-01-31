#include <CQChartsPlotDrawUtil.h>
#include <CQChartsPlot.h>
#include <CQChartsPaintDevice.h>
#include <CQChartsValueSet.h>
#include <CQChartsColor.h>
#include <CQChartsTreeMapPlace.h>

namespace CQChartsPlotDrawUtil {

using PenBrush  = CQChartsPenBrush;
using BrushData = CQChartsBrushData;
using PenData   = CQChartsPenData;
using Color     = CQChartsColor;
using ColorInd  = CQChartsUtil::ColorInd;
using Angle     = CQChartsAngle;

void
drawPie(Plot *plot, PaintDevice *device, const Values &values, const BBox &bbox,
        const PaletteName &paletteName, const QPen &pen)
{
  int nv = values.size();
  if (! nv) return;

  double sum = values.sum();
  if (sum <= 0.0) return;

  auto color = Color::makePalette();

  color.setPaletteName(paletteName.name());

  double a  = 90.0;
  double da = 360.0/sum;

  auto c = bbox.getCenter();
  auto r = std::min(bbox.getWidth(), bbox.getHeight())/2.0;

  for (int i = 0; i < nv; ++i) {
    double da1 = da*values.value(i).value();

    PenBrush penBrush;

    plot->setPenBrush(penBrush,
      PenData(false), BrushData(true, plot->interpColor(color, ColorInd(i, nv))));

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    device->setPen(pen);

    CQChartsDrawUtil::drawPieSlice(device, c, 0.0, r, Angle(a), Angle(a + da1));

    a += da1;
  }
}

void
drawTreeMap(Plot *plot, PaintDevice *device, const Values &values, const BBox &bbox,
            const PaletteName &paletteName, const QPen &pen)
{
  int nv = values.size();
  if (! nv) return;

  double sum = values.sum();
  if (sum <= 0.0) return;

  CQChartsTreeMapPlace place(bbox);

  for (int i = 0; i < nv; ++i)
    place.addValue(values.value(i).value());

  place.placeValues();

  //---

  auto color = Color::makePalette();

  color.setPaletteName(paletteName.name());

  place.processAreas([&](const BBox &bbox, const CQChartsTreeMapPlace::IArea &iarea) {
    PenBrush penBrush;

    plot->setPenBrush(penBrush,
      PenData(false), BrushData(true, plot->interpColor(color, ColorInd(iarea.i, nv))));

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    device->setPen(pen);

    device->drawRect(bbox);
  });
}

}
