#ifndef CQChartsPiePlot_H
#define CQChartsPiePlot_H

#include <CQChartsPlot.h>
#include <CQChartsPlotObj.h>
#include <CQChartsTextBoxObj.h>
#include <CQChartsValueSet.h>

#include <CPoint2D.h>

#include <boost/optional.hpp>
#include <string>

class CQChartsPiePlot;

//---

class CQChartsPieTextObj : public CQChartsRotatedTextBoxObj {
 public:
  CQChartsPieTextObj(CQChartsPiePlot *plot);

  void redrawBoxObj() override;

 private:
  CQChartsPiePlot* plot_ { nullptr };
};

//---

class CQChartsPieObj : public CQChartsPlotObj {
 public:
  CQChartsPieObj(CQChartsPiePlot *plot, const CBBox2D &rect, int i, int n);

  double angle1() const { return angle1_; }
  void setAngle1(double a) { angle1_ = a; }

  double angle2() const { return angle2_; }
  void setAngle2(double a) { angle2_ = a; }

  const QString &label() const { return label_; }
  void setLabel(const QString &s) { label_ = s; }

  double value() const { return value_; }
  void setValue(double r) { value_ = r; }

  void setColor(double r) { color_ = r; }

  bool isExploded() const { return exploded_; }
  void setExploded(bool b) { exploded_ = b; }

  //---

  bool inside(const CPoint2D &p) const override;

  void draw(QPainter *p, const CQChartsPlot::Layer &) override;

 protected:
  typedef boost::optional<double> OptReal;

  CQChartsPiePlot* plot_     { nullptr }; // parent plot
  int              i_        { -1 };      // index
  int              n_        { -1 };      // number of wedges
  double           angle1_   { 0 };       // wedge start angle
  double           angle2_   { 360 };     // wedge start angle
  QString          label_    { "" };      // label
  double           value_    { 0 };       // value
  OptReal          color_;                // color
  bool             exploded_ { false };   // exploded
};

//---

#include <CQChartsKey.h>

class CQChartsPieKeyColor : public CQChartsKeyColorBox {
  Q_OBJECT

 public:
  CQChartsPieKeyColor(CQChartsPiePlot *plot, int i, int n);

  bool mousePress(const CPoint2D &p) override;

  QColor fillColor() const override;
};

class CQChartsPieKeyText : public CQChartsKeyText {
  Q_OBJECT

 public:
  CQChartsPieKeyText(CQChartsPiePlot *plot, int i, const QString &text);

  QColor textColor() const override;

 private:
  int i_ { 0 };
};

//---

class CQChartsPiePlotType : public CQChartsPlotType {
 public:
  CQChartsPiePlotType();

  QString name() const override { return "pie"; }
  QString desc() const override { return "Pie"; }

  void addParameters() override;

  CQChartsPlot *create(CQChartsView *view, const ModelP &model) const override;
};

//---

class CQChartsPiePlot : public CQChartsPlot {
  Q_OBJECT

  // propeties
  //   donut

  Q_PROPERTY(int    labelColumn     READ labelColumn       WRITE setLabelColumn    )
  Q_PROPERTY(int    dataColumn      READ dataColumn        WRITE setDataColumn     )
  Q_PROPERTY(int    keyLabelColumn  READ keyLabelColumn    WRITE setKeyLabelColumn )
  Q_PROPERTY(int    colorColumn     READ colorColumn       WRITE setColorColumn    )
  Q_PROPERTY(bool   donut           READ isDonut           WRITE setDonut          )
  Q_PROPERTY(double innerRadius     READ innerRadius       WRITE setInnerRadius    )
  Q_PROPERTY(double outerRadius     READ outerRadius       WRITE setOuterRadius    )
  Q_PROPERTY(double labelRadius     READ labelRadius       WRITE setLabelRadius    )
  Q_PROPERTY(bool   rotatedText     READ isRotatedText     WRITE setRotatedText    )
  Q_PROPERTY(bool   explodeSelected READ isExplodeSelected WRITE setExplodeSelected)
  Q_PROPERTY(double startAngle      READ startAngle        WRITE setStartAngle     )
  Q_PROPERTY(bool   colorMapEnabled READ isColorMapEnabled WRITE setColorMapEnabled)
  Q_PROPERTY(double colorMapMin     READ colorMapMin       WRITE setColorMapMin    )
  Q_PROPERTY(double colorMapMax     READ colorMapMax       WRITE setColorMapMax    )

 public:
  CQChartsPiePlot(CQChartsView *view, const ModelP &model);

  int labelColumn() const { return labelColumn_; }
  void setLabelColumn(int i) { labelColumn_ = i; }

  int dataColumn() const { return dataColumn_; }
  void setDataColumn(int i) { dataColumn_ = i; }

  int keyLabelColumn() const { return keyLabelColumn_; }
  void setKeyLabelColumn(int i) { keyLabelColumn_ = i; }

  int colorColumn() const { return colorColumn_; }
  void setColorColumn(int i) { colorColumn_ = i; }

  //---

  bool isDonut() const { return donut_; }
  void setDonut(bool b) { donut_ = b; update(); }

  double innerRadius() const { return innerRadius_; }
  void setInnerRadius(double r) { innerRadius_ = r; update(); }

  double outerRadius() const { return outerRadius_; }
  void setOuterRadius(double r) { outerRadius_ = r; update(); }

  double labelRadius() const { return labelRadius_; }
  void setLabelRadius(double r) { labelRadius_ = r; updateRange(); }

  bool isRotatedText() const { return rotatedText_; }
  void setRotatedText(bool b) { rotatedText_ = b; update(); }

  bool isExplodeSelected() const { return explodeSelected_; }
  void setExplodeSelected(bool b) { explodeSelected_ = b; update(); }

  double startAngle() const { return startAngle_; }
  void setStartAngle(double r) { startAngle_ = r; }

  //---

  const CQChartsPieTextObj &textBox() const { return textBox_; }

  //---

  bool isColorMapEnabled() const { return colorSet_.isMapEnabled(); }
  void setColorMapEnabled(bool b) { colorSet_.setMapEnabled(b); initObjs(/*force*/true); update(); }

  double colorMapMin() const { return colorSet_.mapMin(); }
  void setColorMapMin(double r) { colorSet_.setMapMin(r); initObjs(/*force*/true); update(); }

  double colorMapMax() const { return colorSet_.mapMax(); }
  void setColorMapMax(double r) { colorSet_.setMapMax(r); initObjs(/*force*/true); update(); }

  //---

  void addProperties() override;

  void updateRange() override;

  void initObjs(bool force=false) override;

  void addKeyItems(CQChartsKey *key) override;

  //---

  void handleResize() override;

  void draw(QPainter *) override;

 private:
  int                labelColumn_     { 0 };
  int                dataColumn_      { 1 };
  int                keyLabelColumn_  { -1 };
  int                colorColumn_     { -1 };
  bool               donut_           { false };
  double             innerRadius_     { 0.6 };
  double             outerRadius_     { 0.9 };
  double             labelRadius_     { 0.5 };
  bool               rotatedText_     { false };
  bool               explodeSelected_ { true };
  double             startAngle_      { 90 };
  CQChartsValueSet   colorSet_;
  CQChartsPieTextObj textBox_;
};

#endif
