#ifndef CQChartsPiePlot_H
#define CQChartsPiePlot_H

#include <CQChartsPlot.h>
#include <CQChartsPlotObj.h>
#include <CQChartsTextBoxObj.h>
#include <CQChartsColumnBucket.h>
#include <CQChartsGeom.h>

#include <boost/optional.hpp>
#include <string>

class CQChartsPiePlot;

//---

class CQChartsPieTextObj : public CQChartsRotatedTextBoxObj {
 public:
  CQChartsPieTextObj(CQChartsPiePlot *plot);

 private:
  CQChartsPiePlot* plot_ { nullptr };
};

//---

class CQChartsPieGroupObj;

class CQChartsPieObj : public CQChartsPlotObj {
 public:
  using OptColor = boost::optional<CQChartsPaletteColor>;

 public:
  CQChartsPieObj(CQChartsPiePlot *plot, const CQChartsGeom::BBox &rect, const QModelIndex &ind);

  QString calcTipId() const override;

  QString calcId() const override;

  double angle1() const { return angle1_; }
  void setAngle1(double a) { angle1_ = a; }

  double angle2() const { return angle2_; }
  void setAngle2(double a) { angle2_ = a; }

  double innerRadius() const { return ri_; }
  void setInnerRadius(double r) { ri_ = r; }

  double outerRadius() const { return ro_; }
  void setOuterRadius(double r) { ro_ = r; }

  const QString &label() const { return label_; }
  void setLabel(const QString &s) { label_ = s; }

  double value() const { return value_; }
  void setValue(double r) { value_ = r; }

  const QString &keyLabel() const { return keyLabel_; }
  void setKeyLabel(const QString &s) { keyLabel_ = s; }

  void setColor(const CQChartsPaletteColor &c) { color_ = c; }

  CQChartsPieGroupObj *groupObj() const { return groupObj_; }
  void setGroupObj(CQChartsPieGroupObj *p) { groupObj_ = p; }

  bool isExploded() const { return exploded_; }
  void setExploded(bool b) { exploded_ = b; }

  //---

  bool calcExploded() const;

  bool inside(const CQChartsGeom::Point &p) const override;

  void addSelectIndex() override;

  bool isIndex(const QModelIndex &) const override;

  void draw(QPainter *painter, const CQChartsPlot::Layer &) override;

  void drawSegmentLabel(QPainter *painter, const CQChartsGeom::Point &c);

 protected:
  CQChartsPiePlot*     plot_     { nullptr }; // parent plot
  QModelIndex          ind_;                  // model index
  double               angle1_   { 0 };       // wedge start angle
  double               angle2_   { 360 };     // wedge end angle
  double               ri_       { 0.0 };     // inner radius
  double               ro_       { 0.0 };     // outer radius
  QString              label_    { "" };      // label
  double               value_    { 0 };       // value
  QString              keyLabel_ { "" };      // key label
  OptColor             color_;                // color
  CQChartsPieGroupObj* groupObj_ { nullptr }; // parent group object
  bool                 exploded_ { false };   // exploded
};

//---

class CQChartsPieGroupObj : public CQChartsGroupObj {
 public:
  using PieObjs = std::vector<CQChartsPieObj *>;

 public:
  CQChartsPieGroupObj(CQChartsPiePlot *plot, const QString &name);

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  double total() const { return total_; }
  void setTotal(double r) { total_ = r; }

  double innerRadius() const { return innerRadius_; }
  void setInnerRadius(double r) { innerRadius_ = r; }

  double outerRadius() const { return outerRadius_; }
  void setOuterRadius(double r) { outerRadius_ = r; }

  void addObject(CQChartsPieObj *obj);

  CQChartsPieObj *lookupObj(const QString &name) const;

  int numObjs() const { return objs_.size(); }

  const PieObjs &objs() const { return objs_; }

  QString calcId() const override { return name_; }

  void draw(QPainter *, const CQChartsPlot::Layer &) override { }

 private:
  CQChartsPiePlot* plot_        { nullptr }; // parent plot
  QString          name_;                    // group name
  double           total_       { 0.0 };     // value total
  double           innerRadius_ { 0.0 };     // inner radius
  double           outerRadius_ { 0.0 };     // outer radius
  PieObjs          objs_;                    // objects
};

//---

#include <CQChartsKey.h>

class CQChartsPieKeyColor : public CQChartsKeyColorBox {
  Q_OBJECT

 public:
  CQChartsPieKeyColor(CQChartsPiePlot *plot, CQChartsPlotObj *obj);

  bool mousePress(const CQChartsGeom::Point &p) override;

  QBrush fillBrush() const override;

 private:
  CQChartsPlotObj *obj_ { nullptr };
};

class CQChartsPieKeyText : public CQChartsKeyText {
  Q_OBJECT

 public:
  CQChartsPieKeyText(CQChartsPiePlot *plot, CQChartsPlotObj *obj);

  QColor interpTextColor(int i, int n) const override;

 private:
  CQChartsPlotObj *obj_ { nullptr };
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

  // properties
  //   donut, inner radius, outer radius, label radius, start angle, end angle,
  //   explode/explode radius

  Q_PROPERTY(int     labelColumn     READ labelColumn       WRITE setLabelColumn    )
  Q_PROPERTY(int     dataColumn      READ dataColumn        WRITE setDataColumn     )
  Q_PROPERTY(QString dataColumns     READ dataColumnsStr    WRITE setDataColumnsStr )
  Q_PROPERTY(int     groupColumn     READ groupColumn       WRITE setGroupColumn    )
  Q_PROPERTY(bool    rowGrouping     READ isRowGrouping     WRITE setRowGrouping    )
  Q_PROPERTY(int     keyLabelColumn  READ keyLabelColumn    WRITE setKeyLabelColumn )
  Q_PROPERTY(int     colorColumn     READ colorColumn       WRITE setColorColumn    )
  Q_PROPERTY(bool    donut           READ isDonut           WRITE setDonut          )
  Q_PROPERTY(double  innerRadius     READ innerRadius       WRITE setInnerRadius    )
  Q_PROPERTY(double  outerRadius     READ outerRadius       WRITE setOuterRadius    )
  Q_PROPERTY(double  labelRadius     READ labelRadius       WRITE setLabelRadius    )
  Q_PROPERTY(double  startAngle      READ startAngle        WRITE setStartAngle     )
  Q_PROPERTY(double  angleExtent     READ angleExtent       WRITE setAngleExtent    )
  Q_PROPERTY(bool    rotatedText     READ isRotatedText     WRITE setRotatedText    )
  Q_PROPERTY(bool    explodeSelected READ isExplodeSelected WRITE setExplodeSelected)
  Q_PROPERTY(double  explodeRadius   READ explodeRadius     WRITE setExplodeRadius  )
  Q_PROPERTY(bool    colorMapEnabled READ isColorMapEnabled WRITE setColorMapEnabled)
  Q_PROPERTY(double  colorMapMin     READ colorMapMin       WRITE setColorMapMin    )
  Q_PROPERTY(double  colorMapMax     READ colorMapMax       WRITE setColorMapMax    )

 public:
  using OptColor = boost::optional<CQChartsPaletteColor>;

 public:
  CQChartsPiePlot(CQChartsView *view, const ModelP &model);

 ~CQChartsPiePlot();

  //---

  int labelColumn() const { return labelColumn_; }
  void setLabelColumn(int i);

  int dataColumn() const { return dataColumn_; }
  void setDataColumn(int i);

  const Columns &dataColumns() const { return dataColumns_; }
  void setDataColumns(const Columns &dataColumns);

  QString dataColumnsStr() const;
  bool setDataColumnsStr(const QString &s);

  int dataColumnAt(int i) const {
    assert(i >= 0 && i < int(dataColumns_.size()));

    return dataColumns_[i];
  }

  int groupColumn() const { return groupColumn_; }
  void setGroupColumn(int i);

  int keyLabelColumn() const { return keyLabelColumn_; }
  void setKeyLabelColumn(int i);

  //---

  bool isDonut() const { return donut_; }
  void setDonut(bool b) { donut_ = b; updateRangeAndObjs(); }

  double innerRadius() const { return innerRadius_; }
  void setInnerRadius(double r) { innerRadius_ = r; updateRangeAndObjs(); }

  double outerRadius() const { return outerRadius_; }
  void setOuterRadius(double r) { outerRadius_ = r; updateRangeAndObjs(); }

  double labelRadius() const { return labelRadius_; }
  void setLabelRadius(double r) { labelRadius_ = r; updateRangeAndObjs(); }

  double startAngle() const { return startAngle_; }
  void setStartAngle(double r) { startAngle_ = r; updateRangeAndObjs(); }

  double angleExtent() const { return angleExtent_; }
  void setAngleExtent(double r);

  //---

  bool isRotatedText() const { return rotatedText_; }
  void setRotatedText(bool b) { rotatedText_ = b; update(); }

  bool isExplodeSelected() const { return explodeSelected_; }
  void setExplodeSelected(bool b) { explodeSelected_ = b; update(); }

  double explodeRadius() const { return explodeRadius_; }
  void setExplodeRadius(double r) { explodeRadius_ = r; update(); }

  //---

  CQChartsPieTextObj *textBox() const { return textBox_; }

  //---

  int colorColumn() const { return valueSetColumn("color"); }
  void setColorColumn(int i) { setValueSetColumn("color", i); updateRangeAndObjs(); }

  bool isColorMapEnabled() const { return isValueSetMapEnabled("color"); }
  void setColorMapEnabled(bool b) { setValueSetMapEnabled("color", b); updateObjs(); }

  double colorMapMin() const { return valueSetMapMin("color"); }
  void setColorMapMin(double r) { setValueSetMapMin("color", r); updateObjs(); }

  double colorMapMax() const { return valueSetMapMax("color"); }
  void setColorMapMax(double r) { setValueSetMapMax("color", r); updateObjs(); }

  //---

  void addProperties() override;

  void updateRange(bool apply=true) override;

  void updateObjs() override;

  bool initObjs() override;

  void adjustObjAngles();

  void addKeyItems(CQChartsPlotKey *key) override;

  //---

  int numGroups() const { return groupBucket_.numUnique(); }

  int numGroupObjs() const { return groupObjs_.size(); }

  //---

  void handleResize() override;

  //---

  void draw(QPainter *) override;

 private:
  void addRow(QAbstractItemModel *model, const QModelIndex &parent, int r);

  void addRowColumn(QAbstractItemModel *model, const QModelIndex &parent, int r, int c);

  void calcDataTotal();

  void addRowDataTotal(QAbstractItemModel *model, const QModelIndex &parent, int r);

  void addRowColumnDataTotal(QAbstractItemModel *model, const QModelIndex &parent, int r, int c);

  bool getDataColumnValue(QAbstractItemModel *model, const QModelIndex &ind, double &value) const;

 private:
  struct GroupData {
    GroupData(const QString &name) :
     name(name) {
    }

    QString              name;                 // name
    double               total    { 0.0 };     // total
    CQChartsPieGroupObj *groupObj { nullptr }; // associated group obj
  };

  using GroupInd   = std::map<QString,int>;
  using GroupDatas = std::map<int,GroupData>;
  using GroupObjs  = std::vector<CQChartsPieGroupObj *>;

  int                  labelColumn_     { 0 };       // label column
  int                  dataColumn_      { 1 };       // data column
  Columns              dataColumns_;                 // data columns
  int                  groupColumn_     { -1 };      // group column
  int                  keyLabelColumn_  { -1 };      // key label column
  bool                 donut_           { false };   // is donut
  double               innerRadius_     { 0.6 };     // relative inner donut radius
  double               outerRadius_     { 0.9 };     // relative outer donut radius
  double               labelRadius_     { 1.1 };     // label radus
  double               startAngle_      { 90 };      // first pie start angle
  double               angleExtent_     { 360.0 };   // pie angle extent
  bool                 rotatedText_     { false };   // is label rotated
  bool                 explodeSelected_ { true };    // explode selected pie
  double               explodeRadius_   { 0.05 };    // expose radus
  GroupDatas           groupDatas_;                  // data per group
  CQChartsPieTextObj*  textBox_         { nullptr }; // text box
  CQChartsGeom::Point  center_;                      // center point
  GroupObjs            groupObjs_;
};

#endif
