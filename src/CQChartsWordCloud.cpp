#include <CQChartsWordCloud.h>
#include <CQChartsPlot.h>
#include <CQChartsRand.h>

#include <QKeyEvent>
#include <QPainter>

#include <cmath>

CQChartsWordCloud::
CQChartsWordCloud()
{
}

CQChartsWordCloud::
~CQChartsWordCloud()
{
  for (auto &wordData : wordDataArray_)
    delete wordData;
}

void
CQChartsWordCloud::
addWord(const QString &word, int count)
{
  assert(count > 0);

  auto *wordData = new WordData(word, count);

  wordDatas_[word] = wordData;

  wordDataArray_.push_back(wordData);

  minCount_ = std::min((minCount_ > 0 ? minCount_ : count), count);
  maxCount_ = std::max((maxCount_ > 0 ? maxCount_ : count), count);
}

void
CQChartsWordCloud::
place(const Plot *plot)
{
  auto mapCount = [](double value, double srcMin, double srcMax, double destMin, double destMax) {
    double s = (value - srcMin)/(srcMax - srcMin);

    return s*(destMax - destMin) + destMin;
  };

  tree_.reset();

  QFont font = plot->font().font();

  CountWordDatas countWordDatas;

  for (const auto &pw : wordDatas_) {
    auto *wordData = pw.second;

    countWordDatas[-wordData->count].push_back(wordData);
  }

  CQChartsRand::RealInRange rand(0.0, 1.0);

  for (auto &cw : countWordDatas) {
    for (auto &wordData : cw.second) {
      wordData->fontSize =
        mapCount(wordData->count, minCount_, maxCount_, minFontSize(), maxFontSize());

      wordData->x = rand.gen();
      wordData->y = rand.gen();

      //---

      font.setPointSize(wordData->fontSize);

      QFontMetricsF fm(font);

      double ptw = fm.width(wordData->word);
      double pth = fm.height();

      double tw = plot->pixelToWindowWidth (ptw);
      double th = plot->pixelToWindowHeight(pth);

      //---

      double t = 0.0;

      for (int i = 0; i < spiralTurns_; ++i) {
        double x, y;

        spiralPos(t, x, y);

        Rect r(x - tw/2.0, y - th/2.0, x + tw/2.0, y + th/2.0);

        if (! tree_.isDataTouchingRect(r)) {
          wordData->x        = x;
          wordData->y        = y;
          wordData->wordRect = r;
          break;
        }

        t += spiralDelta_;
      }

      //---

      tree_.add(wordData);
    }
  }
}

void
CQChartsWordCloud::
spiralPos(double t, double &x, double &y) const
{
  x = spiralWidth_*t*std::cos(t) + 0.5;
  y = spiralWidth_*t*std::sin(t) + 0.5;
}
