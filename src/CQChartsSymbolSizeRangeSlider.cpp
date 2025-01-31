#include <CQChartsSymbolSizeRangeSlider.h>
#include <CQChartsPointPlot.h>
#include <CQCharts.h>

#include <QPainter>

CQChartsSymbolSizeRangeSlider::
CQChartsSymbolSizeRangeSlider(QWidget *parent) :
 CQDoubleRangeSlider(parent)
{
  auto font = this->font();

  font.setPointSizeF(font.pointSizeF()*0.9);

  setTextFont(font);
}

void
CQChartsSymbolSizeRangeSlider::
setPlot(CQChartsPlot *plot)
{
  plot_ = plot;

  auto *pointPlot = dynamic_cast<CQChartsPointPlot *>(plot_);

  if (pointPlot) {
    setRangeMinMax(pointPlot->symbolSizeMapMin(), pointPlot->symbolSizeMapMax(),
                   /*reset*/false);
    setSliderMinMax(pointPlot->symbolSizeUserMapMin(), pointPlot->symbolSizeUserMapMax(),
                    /*reset*/false);
  }
  else
    setRangeMinMax(CQChartsSymbolSize::minValue(), CQChartsSymbolSize::maxValue(),
                   /*reset*/false);

  update();
}
