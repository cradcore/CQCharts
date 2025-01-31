#ifndef CQChartsDistributionPlot_H
#define CQChartsDistributionPlot_H

#include <CQChartsBarPlot.h>
#include <CQChartsPlotObj.h>
#include <CQChartsColor.h>
#include <CQChartsReals.h>
#include <CQStatData.h>

class CQChartsDensity;

//---

/*!
 * \brief Distribution Bar Value Data
 * \ingroup Charts
 */
struct CQChartsDistributionBarValue {
  using RangeValue = CQChartsGeom::RangeValue;

  double     n1 { 0 };
  double     n2 { 0 };
  RangeValue xr;
  RangeValue yr;
};

//---

/*!
 * \brief Distribution Plot Bucket
 * \ingroup Charts
 */
class CQChartsDistributionBucket {
 public:
  enum class Type {
    VALUE,
    UNDERFLOW,
    OVERFLOW
  };

  CQChartsDistributionBucket() { }

  explicit CQChartsDistributionBucket(int value) :
   type_(Type::VALUE), value_(value) {
  }

  explicit CQChartsDistributionBucket(Type type) :
   type_(type) {
  }

  bool hasValue() const { return (type_ == Type::VALUE); }

  int value() const { assert(type_ == Type::VALUE); return value_; }

  bool isUnderflow() const { return (type_ == Type::UNDERFLOW); }
  bool isOverflow () const { return (type_ == Type::OVERFLOW ); }

  int outsideValue(int min, int max) const {
    if (isUnderflow()) return min;
    if (isOverflow ()) return max;
    assert(false);
    return false;
  }

  bool operator<(const CQChartsDistributionBucket &rhs) const {
    if      (type_ == Type::UNDERFLOW) {
      if (rhs.type_ == Type::UNDERFLOW)
        return false;

      return true;
    }
    else if (type_ == Type::OVERFLOW) {
      if (rhs.type_ == Type::OVERFLOW)
        return false;

      return false;
    }
    else {
      if      (rhs.type_ == Type::UNDERFLOW)
        return false;
      else if (rhs.type_ == Type::OVERFLOW)
        return true;
      else
        return value_ < rhs.value_;
    }
  }

 private:
  Type type_  { Type::VALUE }; //!< bucket type
  int  value_ { -1 };          //!< bucket value
};

//---

class CQChartsDistributionPlot;

/*!
 * \brief Distribution plot type
 * \ingroup Charts
 */
class CQChartsDistributionPlotType : public CQChartsGroupPlotType {
 public:
  using Plot = CQChartsDistributionPlot;

 public:
  CQChartsDistributionPlotType();

  QString name() const override { return "distribution"; }
  QString desc() const override { return "Distribution"; }

  Category category() const override { return Category::ONE_D; }

  void addParameters() override;

  QString yColumnName() const override { return "value"; }

  bool allowXAxisIntegral() const override { return false; }

  bool allowXLog() const override { return false; }

  bool canProbe() const override { return true; }

  bool supportsImageColumn() const override { return true; }

  QString description() const override;

  CQChartsPlot *create(View *view, const ModelP &model) const override;
};

//---

class CQChartsDistributionPlot;

/*!
 * \brief Distribution Plot Bar object
 * \ingroup Charts
 */
class CQChartsDistributionBarObj : public CQChartsPlotObj {
  Q_OBJECT

  Q_PROPERTY(QString groupName READ groupName)
  Q_PROPERTY(QString bucketStr READ bucketStr)
  Q_PROPERTY(int     count     READ count    )
  Q_PROPERTY(double  minValue  READ minValue )
  Q_PROPERTY(double  maxValue  READ maxValue )

 public:
  using Color = CQChartsColor;

  struct IndColor {
    int   ind { 0 };
    Color color;

    IndColor(int ind, const Color &color) :
     ind(ind), color(color) {
    }

    friend bool operator<(const IndColor &lhs, const IndColor &rhs) {
      if (lhs.ind != rhs.ind) return lhs.ind < rhs.ind;
      return lhs.color  < rhs.color;
    }
  };

  struct IndColorSize {
    IndColor indColor;
    int      size { 0 };

    IndColorSize(const IndColor &indColor, int size) :
     indColor(indColor), size(size) {
    }
  };

  using IndColorSizes = std::vector<IndColorSize>;

  using ColorCount  = std::map<int, int>;
  using IndColorSet = std::map<IndColor, int>;
  using ColorRows   = std::set<int>;

  /*!
   * \brief Bar Color Data (for multiple stacked bars)
   * \ingroup Charts
   */
  struct ColorData {
    ColorCount    colorCount;
    IndColorSet   colorSet;
    ColorRows     colorRows;
    int           nv { 0 };
    IndColorSizes colorSizes;
  };

 public:
  using Plot     = CQChartsDistributionPlot;
  using Bucket   = CQChartsDistributionBucket;
  using BarValue = CQChartsDistributionBarValue;
  using Column   = CQChartsColumn;
  using Symbol   = CQChartsSymbol;
  using Length   = CQChartsLength;

  CQChartsDistributionBarObj(const Plot *plot, const BBox &rect, int groupInd,
                             const Bucket &bucket, const BarValue &barValue, bool isLine,
                             const ColorInd &ig, const ColorInd &iv);

 ~CQChartsDistributionBarObj();

  int groupInd() const { return groupInd_; }

  //---

  QString typeName() const override { return "bar"; }

  QString calcId() const override;

  QString calcTipId() const override;

  bool isLine() const { return isLine_; }

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path) override;

  //---

  QString groupName() const;

  //---

  const Bucket &bucket() const { return bucket_; }

  QString bucketStr() const;

  bool bucketXValue(double x, double &value) const;
  bool bucketYValue(double y, double &value) const;

  //---

  int count() const;

  double minValue() const;
  double maxValue() const;

  //---

  BBox dataLabelRect() const;

  //---

  void getObjSelectIndices(Indices &inds) const override;

  void addColumnSelectIndex(Indices &inds, const Column &column) const override;

  //---

  void draw(PaintDevice *device) const override;

  void drawColoredRects(PaintDevice *device) const;

  void drawFg(PaintDevice *device) const override;

  void drawRug(PaintDevice *device) const;

  CQChartsImage getImage() const;

  void calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const override;

  //---

  BBox calcRect() const;

  //---

  double mapValue(double v) const;

 private:
  bool getBarColoredRects(ColorData &colorData) const;

  //---

  void drawShape(PaintDevice *device, const BBox &pbbox, const Color &color,
                 bool useLine) const;

  void drawRect   (PaintDevice *device, const BBox &pbbox, bool useLine) const;
  void drawDotLine(PaintDevice *device, const BBox &pbbox,
                   const CQChartsPenBrush &barPenBrush) const;
  void drawBox    (PaintDevice *device, const BBox &pbbox) const;
  void drawScatter(PaintDevice *device, const BBox &pbbox) const;
  void drawViolin (PaintDevice *device, const BBox &pbbox) const;
  void drawTreeMap(PaintDevice *device, const BBox &bbox, const PenBrush &penBrush) const;

  void calcBarPenBrush(const Color &color, bool useLine, CQChartsPenBrush &barPenBrush,
                       bool updateState) const;

  QColor calcBarColor() const;

  //---

  bool isUseLine() const;

  //---

  double xColorValue(bool relative=true) const override;
  double yColorValue(bool relative=true) const override;

 private:
  using DensityP = std::unique_ptr<CQChartsDensity>;

  const Plot* plot_     { nullptr }; //!< parent plot
  int         groupInd_ { -1 };      //!< group ind
  Bucket      bucket_;               //!< bucket
  BarValue    barValue_;             //!< bar value
  double      value1_   { 0.0 };     //!< bucket start value
  double      value2_   { 1.0 };     //!< bucket end value
  bool        isLine_   { false };   //!< is drawn as line

  mutable ColorData colorData_; //!< color data
  mutable QColor    barColor_;  //!< bar color
  mutable DensityP  density_;   //!< density data
};

//---

/*!
 * \brief Distribution Plot Density object
 * \ingroup Charts
 */
class CQChartsDistributionDensityObj : public CQChartsPlotObj {
  Q_OBJECT

  Q_PROPERTY(QString groupName  READ groupName )
  Q_PROPERTY(int     numSamples READ numSamples)

 public:
  using Plot   = CQChartsDistributionPlot;
  using Bucket = CQChartsDistributionBucket;
  using Symbol = CQChartsSymbol;

  using Points = std::vector<Point>;

  /*!
   * \brief Bucket count data
   * \ingroup Charts
   */
  struct BucketCount {
    Bucket bucket;
    int    n;

    BucketCount(const Bucket &bucket, int n) :
     bucket(bucket), n(n) {
    }
  };

  using BucketCounts = std::vector<BucketCount>;

  /*!
   * \brief Density Point Data
   * \ingroup Charts
   */
  struct Data {
    Points       points;
    double       xmin { 0.0 };
    double       xmax { 0.0 };
    double       ymin { 0.0 };
    double       ymax { 0.0 };
    CQStatData   statData;
    BucketCounts buckets;
  };

 public:
  CQChartsDistributionDensityObj(const Plot *plot, const BBox &rect, int groupInd,
                                 const Data &data, double doffset, const ColorInd &is);

  int groupInd() const { return groupInd_; }

  //---

  QString typeName() const override { return "density"; }

  QString calcId() const override;

  QString calcTipId() const override;

  //---

  QString groupName() const;

  int numSamples() const;

  //---

  bool isPolygon() const override { return true; }
  Polygon polygon() const override { return poly_; }

  //---

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) const override;

  void drawFg(PaintDevice *device) const override;

  void drawStatsLines(PaintDevice *device) const;

  void drawRug(PaintDevice *device) const;

  void calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const override;

  //---

  BBox calcRect() const;

 private:
  const Plot* plot_        { nullptr };
  int         groupInd_    { -1 };
  Data        data_;
  double      doffset_     { 0.0 };
  ColorInd    is_;
  Polygon     poly_;
  double      bucketScale_ { 1.0 };
};

//---

/*!
 * \brief Distribution Plot Scatter object
 * \ingroup Charts
 */
class CQChartsDistributionScatterObj : public CQChartsPlotObj {
  Q_OBJECT

 public:
  using Plot       = CQChartsDistributionPlot;
  using Bucket     = CQChartsDistributionBucket;
  using Length     = CQChartsLength;
  using Symbol     = CQChartsSymbol;
  using SymbolType = CQChartsSymbolType;

 public:
  CQChartsDistributionScatterObj(const Plot *plot, const BBox &rect, int groupInd,
                                 const Bucket &bucket, int n, const ColorInd &is,
                                 const ColorInd &iv);

  int groupInd() const { return groupInd_; }

  const Bucket &bucket() const { return bucket_; }

  //---

  QString typeName() const override { return "scatter"; }

  //---

  bool isPoint() const override { return true; }

  //---

  QString calcId() const override;

  QString calcTipId() const override;

  //---

  void draw(PaintDevice *device) const override;

  void calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const override;

  //---

 private:
  using Points = std::vector<Point>;

  const Plot* plot_     { nullptr };
  int         groupInd_ { -1 };
  Bucket      bucket_;
  int         n_        { 0 };
  ColorInd    is_;
  ColorInd    iv_;
  Points      points_;
};

//---

#include <CQChartsKey.h>

/*!
 * \brief Distribution Plot Key Color Box
 * \ingroup Charts
 */
class CQChartsDistColorKeyItem : public CQChartsColorBoxKeyItem {
  Q_OBJECT

 public:
  using Plot = CQChartsDistributionPlot;

 public:
  CQChartsDistColorKeyItem(Plot *plot, const ColorInd &ig, const ColorInd &iv,
                           const RangeValue &xv, const RangeValue &yv);

  const CQChartsColor &color() const { return color_; }
  void setColor(const CQChartsColor &v) { color_ = v; }

#if 0
  //! handle select press
  bool selectPress(const Point &p, SelData &selData) override;
#endif

  QBrush fillBrush() const override;

#if 0
  // get/set hidden
  bool isSetHidden() const override;
  void setSetHidden(bool b) override;
#endif

 private:
  Plot*         plot_ { nullptr }; //!< plot
  CQChartsColor color_;            //!< custom color
};

/*!
 * \brief Distribution Plot Key Text
 * \ingroup Charts
 */
class CQChartsDistTextKeyItem : public CQChartsTextKeyItem {
  Q_OBJECT

 public:
  using Plot = CQChartsDistributionPlot;

 public:
  CQChartsDistTextKeyItem(Plot *plot, const QString &text, const ColorInd &ig, const ColorInd &iv);

  QColor interpTextColor(const ColorInd &ind) const override;

  bool isSetHidden() const override;

 private:
  ColorInd ig_;
  ColorInd iv_;
};

//---

/*!
 * \brief Distribution plot
 * \ingroup Charts
 */
class CQChartsDistributionPlot : public CQChartsBarPlot,
 public CQChartsObjStatsShapeData<CQChartsDistributionPlot>,
 public CQChartsObjDotPointData  <CQChartsDistributionPlot>,
 public CQChartsObjRugPointData  <CQChartsDistributionPlot> {
  Q_OBJECT

  // columns
  Q_PROPERTY(CQChartsColumn nameColumn READ nameColumn WRITE setNameColumn)
  Q_PROPERTY(CQChartsColumn dataColumn READ dataColumn WRITE setDataColumn)

  // bucketer
  Q_PROPERTY(bool   bucketed         READ isBucketed       WRITE setBucketed        )
  Q_PROPERTY(bool   autoBucket       READ isAutoBucket     WRITE setAutoBucket      )
  Q_PROPERTY(double startBucketValue READ startBucketValue WRITE setStartBucketValue)
  Q_PROPERTY(double deltaBucketValue READ deltaBucketValue WRITE setDeltaBucketValue)
  Q_PROPERTY(double minBucketValue   READ minBucketValue   WRITE setMinBucketValue  )
  Q_PROPERTY(double maxBucketValue   READ maxBucketValue   WRITE setMaxBucketValue  )
  Q_PROPERTY(int    numAutoBuckets   READ numAutoBuckets   WRITE setNumAutoBuckets  )

  // underflow/overflow bucket
  Q_PROPERTY(CQChartsOptReal underflowBucket READ underflowBucket WRITE setUnderflowBucket)
  Q_PROPERTY(CQChartsOptReal overflowBucket  READ overflowBucket  WRITE setOverflowBucket )

  // options
  Q_PROPERTY(PlotType        plotType  READ plotType  WRITE setPlotType )
  Q_PROPERTY(ValueType       valueType READ valueType WRITE setValueType)
  Q_PROPERTY(ShapeType       shapeType READ shapeType WRITE setShapeType)
  Q_PROPERTY(CQChartsOptReal minValue  READ minValue  WRITE setMinValue )

  Q_PROPERTY(bool percent   READ isPercent   WRITE setPercent  )
  Q_PROPERTY(bool skipEmpty READ isSkipEmpty WRITE setSkipEmpty)
  Q_PROPERTY(bool sorted    READ isSorted    WRITE setSorted   )

  // density
  Q_PROPERTY(bool   density         READ isDensity         WRITE setDensity        )
  Q_PROPERTY(double densityOffset   READ densityOffset     WRITE setDensityOffset  )
  Q_PROPERTY(int    densitySamples  READ densitySamples    WRITE setDensitySamples )
  Q_PROPERTY(bool   densityGradient READ isDensityGradient WRITE setDensityGradient)
  Q_PROPERTY(bool   densityBars     READ isDensityBars     WRITE setDensityBars    )

  // scatter
  Q_PROPERTY(bool   scatter       READ isScatter     WRITE setScatter      )
  Q_PROPERTY(double scatterFactor READ scatterFactor WRITE setScatterFactor)

  // stats data
  Q_PROPERTY(bool statsLines READ isStatsLines WRITE setStatsLines)

  CQCHARTS_NAMED_SHAPE_DATA_PROPERTIES(Stats, stats)

  Q_PROPERTY(bool includeOutlier READ isIncludeOutlier WRITE setIncludeOutlier)

  // min bar size
  Q_PROPERTY(double minBarSize READ minBarSize WRITE setMinBarSize)

  // scatter margin
  Q_PROPERTY(double scatterMargin READ scatterMargin WRITE setScatterMargin)

  // dot line data
  Q_PROPERTY(CQChartsLength dotLineWidth READ dotLineWidth WRITE setDotLineWidth)

  CQCHARTS_NAMED_POINT_DATA_PROPERTIES(Dot, dot)

  // rug symbols
  Q_PROPERTY(bool rug READ isRug WRITE setRug)

  CQCHARTS_NAMED_POINT_DATA_PROPERTIES(Rug, rug)

  Q_ENUMS(PlotType)
  Q_ENUMS(ValueType)
  Q_ENUMS(ShapeType)

 public:
  enum class PlotType {
    NORMAL,
    STACKED,
    SIDE_BY_SIDE,
    OVERLAY,
    SCATTER,
    DENSITY
  };

  enum class ValueType {
    COUNT,
    RANGE,
    MIN,
    MAX,
    MEAN,
    SUM
  };

  enum class ShapeType {
    RECT,
    DOT_LINE,
    BOX,
    SCATTER,
    VIOLIN,
    TREEMAP
  };

  /*!
   * \brief Filter Data (for Push/Pop)
   * \ingroup Charts
   */
  struct Filter {
    Filter(int groupInd, double min, double max) :
     groupInd(groupInd), minValue(min), maxValue(max) {
    }

    int    groupInd { -1 };
    double minValue { 1.0 };
    double maxValue { 1.0 };
  };

  enum class BucketValueType {
    START,
    END,
    ALL
  };

  using Bucket     = CQChartsDistributionBucket;
  using RangeValue = CQChartsGeom::RangeValue;
  using LineDash   = CQChartsLineDash;
  using Symbol     = CQChartsSymbol;
  using SymbolType = CQChartsSymbolType;
  using Length     = CQChartsLength;
  using Color      = CQChartsColor;
  using Alpha      = CQChartsAlpha;
  using ColorInd   = CQChartsUtil::ColorInd;
  using PenBrush   = CQChartsPenBrush;

 public:
  CQChartsDistributionPlot(View *view, const ModelP &model);
 ~CQChartsDistributionPlot();

  //---

  void init() override;
  void term() override;

  //---

  void setValueColumns(const Columns &c) override;

  const Column &nameColumn() const { return nameColumn_; }
  void setNameColumn(const Column &c);

  const Column &dataColumn() const { return dataColumn_; }
  void setDataColumn(const Column &c);

  //---

  Column getNamedColumn(const QString &name) const override;
  void setNamedColumn(const QString &name, const Column &c) override;

  Columns getNamedColumns(const QString &name) const override;
  void setNamedColumns(const QString &name, const Columns &c) override;

  //---

  QString valueColumnName(const QString &def="value") const;

  //---

  // bucket data
  bool isBucketed() const { return bucketed_; }
  void setBucketed(bool b);

  bool isAutoBucket() const;
  void setAutoBucket(bool b);

  double startBucketValue() const;
  void setStartBucketValue(double r);

  double deltaBucketValue() const;
  void setDeltaBucketValue(double r);

  double minBucketValue() const;
  void setMinBucketValue(double r);

  double maxBucketValue() const;
  void setMaxBucketValue(double r);

  void calcMinMaxBucketValue(double &rmin, double &rmax) const;

  int numAutoBuckets() const;
  void setNumAutoBuckets(int i);

  const CQChartsOptReal &underflowBucket() const { return underflowBucket_; }
  void setUnderflowBucket(const CQChartsOptReal &r);

  const CQChartsOptReal &overflowBucket() const { return overflowBucket_; }
  void setOverflowBucket(const CQChartsOptReal &r);

  const CQChartsReals &bucketStops() const { return bucketStops_; }
  void setBucketStops(const CQChartsReals &r);

  int numUniqueValues() const;

  bool isExactBucketValue() const { return exactValue_; }
  void setExactBucketValue(bool b);

  //--

  CQBucketer::Type bucketType() const;
  void setBucketType(const CQBucketer::Type &type);

  CQBucketer::Type bucketRealType() const;

  void updateGroupBucketers();

  void initBucketer(CQBucketer &bucketer);

  //---

  PlotType plotType() const { return plotType_; }

  bool isNormal    () const { return (plotType() == PlotType::NORMAL      ); }
  bool isStacked   () const { return (plotType() == PlotType::STACKED     ); }
  bool isSideBySide() const { return (plotType() == PlotType::SIDE_BY_SIDE); }
  bool isOverlaid  () const { return (plotType() == PlotType::OVERLAY     ); }
  bool isDensity   () const { return (plotType() == PlotType::DENSITY     ); }

  //---

  ValueType valueType() const { return valueType_; }

  bool isValueCount() const { return (valueType() == ValueType::COUNT); }
  bool isValueRange() const { return (valueType() == ValueType::RANGE); }
  bool isValueMin  () const { return (valueType() == ValueType::MIN  ); }
  bool isValueMax  () const { return (valueType() == ValueType::MAX  ); }
  bool isValueMean () const { return (valueType() == ValueType::MEAN ); }
  bool isValueSum  () const { return (valueType() == ValueType::SUM  ); }

  //---

  ShapeType shapeType() const { return shapeType_; }

  //---

  bool isPercent  () const { return percent_  ; }
  bool isSkipEmpty() const { return skipEmpty_; }
  bool isSorted   () const { return sorted_   ; }

  //---

  //! get/set min value
  const CQChartsOptReal &minValue() const { return minValue_; }
  void setMinValue(const CQChartsOptReal &r);

  bool isEmptyValue(double r) const;

  //---

  // density
  double densityOffset() const { return densityData_.offset; }
  void setDensityOffset(double o);

  int densitySamples() const { return densityData_.numSamples; }
  void setDensitySamples(int n);

  bool isDensityGradient() const { return densityData_.gradient; }
  void setDensityGradient(bool b);

  bool isDensityBars() const { return densityData_.bars; }
  void setDensityBars(bool b);

  //---

  // scatter
  bool isScatter() const { return (plotType_ == PlotType::SCATTER); }

  double scatterFactor() const { return scatterData_.factor; }
  void setScatterFactor(double r);

  double scatterMargin() const { return scatterData_.margin; }
  void setScatterMargin(double m);

  //---

  // stats lines
  bool isStatsLines() const { return statsLines_; }

  //---

  const CQChartsLength &dotLineWidth() const { return dotLineData_.width; }
  void setDotLineWidth(const CQChartsLength &l);

  //---

  bool isRug() const { return rug_; }

  //---

  bool isIncludeOutlier() const { return includeOutlier_; }
  void setIncludeOutlier(bool b);

  //---

  double minBarSize() const { return minBarSize_; }
  void setMinBarSize(double s);

  //---

  bool checkFilter(int groupInd, const QVariant &value) const;

  Bucket calcBucket(int groupInd, double v) const;

  //---

  BBox calcExtraFitBBox() const override;

  //---

  bool allowZoomX() const override;
  bool allowZoomY() const override;

  bool allowPanX() const override;
  bool allowPanY() const override;

  //---

  void addProperties() override;

  //---

  Range calcRange() const override;

  bool createObjs(PlotObjs &objs) const override;

  //---

  CQChartsAxis *valueAxis() const;
  CQChartsAxis *countAxis() const;

  //---

  void addKeyItems(PlotKey *key) override;

  //---

  QString posStr(const Point &w) const override;

  //---

  bool addMenuItems(QMenu *) override;

  //---

  bool hasForeground() const override;

  void execDrawForeground(PaintDevice *) const override;

  void drawStatsLines(PaintDevice *) const;

  //---

  virtual QString bucketValuesStr(int groupInd, const Bucket &bucket,
                                  BucketValueType type=BucketValueType::ALL) const;

  QString bucketStr(int groupInd, const Bucket &bucket,
                    BucketValueType type=BucketValueType::ALL) const;

  virtual void bucketValues(int groupInd, const Bucket &bucket,
                            double &value1, double &value2) const;

  //--

  CQBucketer &groupBucketer(int groupInd);
  const CQBucketer &groupBucketer(int groupInd) const;

  bool isConsistentBucketer() const;

  //---

 public:
  /*!
   * \brief Model Index Variant Data
   * \ingroup Charts
   */
  struct VariantInd {
    QVariant   var;
    ModelIndex ind;
    QVariant   dvar;

    VariantInd(const QVariant &var, const ModelIndex &ind, const QVariant &dvar) :
     var(var), ind(ind), dvar(dvar) {
    }
  };

  using VariantInds = std::vector<VariantInd>;

  /*!
   * \brief Model Index Variant Set Data
   * \ingroup Charts
   */
  struct VariantIndsData {
    VariantInds inds;         //!< model indices
    double      min  { 0.0 }; //!< min value
    double      max  { 0.0 }; //!< max value
    CQStatData  statData;     //!< stats data
    RMinMax     valueRange;   //!< value range
  };

  using BarValue = CQChartsDistributionBarValue;

 public:
  void calcVarIndsData(VariantIndsData &varInds) const;

  BarValue varIndsValue(const VariantIndsData &varInds) const;

  void getInds(int groupInd, const Bucket &bucket, VariantInds &inds) const;

  void getXVals(int groupInd, const Bucket &bucket, std::vector<double> &xvals) const;

  bool getStatData(int groupInd, CQStatData &statData) const;

  bool getRealValues(int groupInd, std::vector<double> &xvals, CQStatData &statData) const;

  bool hasGroups() const;

 protected:
  using DensityObj     = CQChartsDistributionDensityObj;
  using DensityObjData = CQChartsDistributionDensityObj::Data;
  using ScatterObj     = CQChartsDistributionScatterObj;
  using BarObj         = CQChartsDistributionBarObj;

  virtual DensityObj *createDensityObj(const BBox &rect, int groupInd, const DensityObjData &data,
                                       double doffset, const ColorInd &is) const;

  virtual ScatterObj *createScatterObj(const BBox &rect, int groupInd, const Bucket &bucket, int n,
                                       const ColorInd &is, const ColorInd &iv) const;

  virtual BarObj *createBarObj(const BBox &rect, int groupInd, const Bucket &bucket,
                               const BarValue &barValue, bool isLine, const ColorInd &ig,
                               const ColorInd &iv) const;

 private:
  using Inds         = std::vector<ModelIndex>;
  using BucketValues = std::map<Bucket, VariantIndsData>;

  /*!
   * \brief Bucketed Value Set Data
   * \ingroup Charts
   */
  struct Values {
    using DensityP = std::shared_ptr<CQChartsDensity>;

    Inds              inds;                      //!< value indices
    CQChartsValueSet* valueSet      { nullptr }; //!< value set
    BucketValues      bucketValues;              //!< values in each bucket
    DensityP          densityData;               //!< density data
    CQStatData        statData;                  //!< stat data
    RMinMax           xValueRange;               //!< x value range
    RMinMax           yValueRange;               //!< y value range

    Values(CQChartsValueSet *valueSet);
   ~Values();

    Values(const Values &) = delete;
    Values &operator=(const Values &) = delete;
  };

  using GroupValues   = std::map<int, Values *>;
  using GroupBucketer = std::map<int, CQBucketer>;

  using Filters     = std::vector<Filter>;
  using FilterStack = std::vector<Filters>;

  using GroupBucketRange = std::map<int, IMinMax>;

 private:
  void bucketGroupValues() const;

  Range calcBucketRanges() const;

  void clearGroupValues() const;
  void clearGroupBuckets() const;

  void addRow(const ModelVisitor::VisitData &data) const;

  void addRowColumn(const ModelIndex &ind) const;

  //---

  QString bucketValuesStr(int groupInd, const Bucket &bucket, const Values *values,
                          BucketValueType type=BucketValueType::ALL) const;

  Values *getGroupIndValues(int groupInd, const ModelIndex &ind) const;

  const Values *getGroupValues(int groupInd) const;

  //---

  double getPanX(bool is_shift) const override;
  double getPanY(bool is_shift) const override;

  //---

 public slots:
  // set plot type
  void setPlotType(PlotType plotType);

  // set normal, stacked, side by side, overlay
  void setNormal    (bool b);
  void setStacked   (bool b);
  void setSideBySide(bool b);
  void setOverlaid  (bool b);

  //---

  // set value type
  void setValueType(ValueType valueType);

  void setValueCount(bool b);
  void setValueRange(bool b);
  void setValueMin  (bool b);
  void setValueMax  (bool b);
  void setValueMean (bool b);
  void setValueSum  (bool b);

  //---

  void setShapeType(ShapeType shapeType);

  //---

  // set percent
  void setPercent(bool b);

  // set skip empty
  void setSkipEmpty(bool b);

  // set sorted
  void setSorted(bool b);

  // set density
  void setDensity(bool b);

  // set scatter
  void setScatter(bool b);

  // stats lines
  void setStatsLines(bool b);

  // set rug
  void setRug(bool b);

  //---

  // push to bar range
  void pushSlot();
  // pop out of bar range
  void popSlot();
  // pop out of all bar ranges
  void popTopSlot();

 protected:
  CQChartsPlotCustomControls *createCustomControls() override;

 private:
  /*!
   * \brief Density Data
   * \ingroup Charts
   */
  struct DensityData {
    double offset     { 0.0 };   //!< offset
    int    numSamples { 100 };   //!< num samples
    bool   gradient   { false }; //!< gradient
    bool   bars       { false }; //!< show bars
  };

  /*!
   * \brief Scatter Point Data
   * \ingroup Charts
   */
  struct ScatterData {
    double factor { 1.0 };  //!< factor
    double margin { 0.05 }; //!< point margin
  };

  /*!
   * \brief Dot Line Data
   * \ingroup Charts
   *
   * TODO: symbol, ends ...
   */
  struct DotLineData {
    CQChartsLength width { Length::pixel(3) }; //!< width
  };

  /*!
   * \brief Group Data (Values. Bucketer and Range)
   * \ingroup Charts
   */
  struct GroupData {
    GroupValues      groupValues;      //!< grouped value sets
    GroupBucketer    groupBucketer;    //!< group bucketer
    GroupBucketRange groupBucketRange; //!< bucketer per group
  };

  Column      nameColumn_;                          //!< name column
  Column      dataColumn_;                          //!< data column
  PlotType    plotType_       { PlotType::NORMAL }; //!< plot type
  ValueType   valueType_      { ValueType::COUNT }; //!< show value count
  ShapeType   shapeType_      { ShapeType::RECT };  //!< bar object shape type
  OptReal     minValue_;                            //!< min value
  bool        percent_        { false };            //!< percent values
  bool        skipEmpty_      { false };            /*!< skip empty buckets
                                                        (non contiguous range) */
  bool        sorted_         { false };            //!< sort by count
  DensityData densityData_;                         //!< density data
  ScatterData scatterData_;                         //!< scatter data
  bool        statsLines_     { false };            //!< stats lines data
  DotLineData dotLineData_;                         //!< dot line data
  bool        rug_            { false };            //!< show rug
  bool        includeOutlier_ { true };             //!< include outlier values

  double minBarSize_ { 3.0 };  //!< min bar size (pixels)

  // bucketer data
  CQChartsOptReal underflowBucket_;          //!< underflow bucket threshold
  CQChartsOptReal overflowBucket_;           //!< overflow bucket threshold
  CQBucketer      bucketer_;                 //!< shared bucketer
  bool            bucketed_       { true };  //!< is bucketed
  int             numUnique_      { 0 };     //!< num unique values
  CQChartsReals   bucketStops_;              //!< explicit bucket stops
  bool            exactValue_     { false }; //!< force bucket of exact values

  FilterStack        filterStack_;          //!< filter stack
  GroupData          groupData_;            //!< grouped value sets
  double             barWidth_    { 1.0 };  //!< bar width
  mutable std::mutex mutex_;                //!< mutex
};

//---

#include <CQChartsGroupPlotCustomControls.h>

class CQIconRadio;
class CQDoubleRangeSlider;
class CQRealSpin;
class CQIntegerSpin;
class CQLabel;

class QLineEdit;
class QButtonGroup;

/*!
 * \brief Distribution Plot plot custom controls
 * \ingroup Charts
 */
class CQChartsDistributionPlotCustomControls : public CQChartsGroupPlotCustomControls {
  Q_OBJECT

 public:
  CQChartsDistributionPlotCustomControls(CQCharts *charts);

  void init() override;

  void setPlot(CQChartsPlot *plot) override;

 public slots:
  void updateWidgets() override;

 protected:
  void addWidgets() override;

  void addColumnWidgets() override;

  virtual void addBucketGroup();

  void addOptionsWidgets() override;

  void connectSlots(bool b) override;

  //---

  CQChartsColor getColorValue() override;
  void setColorValue(const CQChartsColor &c) override;

 protected slots:
  void orientationSlot();
  void plotTypeSlot();
  void valueTypeSlot();
  void shapeTypeSlot();

  void bucketRadioGroupSlot(QAbstractButton *);
  void bucketRangeSlot();
  void startBucketSlot();
  void deltaBucketSlot();
  void numBucketsSlot();
  void bucketStopsSlot();
  void statsLinesSlot(int);

 protected:
  CQChartsDistributionPlot* plot_ { nullptr };

  FrameData optionsFrame_;

  CQChartsEnumParameterEdit* orientationCombo_{ nullptr };
  CQChartsEnumParameterEdit* plotTypeCombo_   { nullptr };
  CQChartsEnumParameterEdit* valueTypeCombo_  { nullptr };
  CQChartsEnumParameterEdit* shapeTypeCombo_  { nullptr };
  CQChartsBoolParameterEdit* statsCheck_      { nullptr };

  QButtonGroup*        bucketRadioGroup_  { nullptr };
  CQIconRadio*         fixedBucketRadio_  { nullptr };
  CQIconRadio*         rangeBucketRadio_  { nullptr };
  CQIconRadio*         stopsBucketRadio_  { nullptr };
  CQIconRadio*         uniqueBucketRadio_ { nullptr };
  CQDoubleRangeSlider* bucketRange_       { nullptr };
  CQRealSpin*          startBucketEdit_   { nullptr };
  CQRealSpin*          deltaBucketEdit_   { nullptr };
  CQIntegerSpin*       numBucketsEdit_    { nullptr };
  QLineEdit*           bucketStopsEdit_   { nullptr };
  CQLabel*             uniqueCount_       { nullptr };
  QLabel*              rangeLabel_        { nullptr };
};

#endif
