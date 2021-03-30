#ifndef CQChartsMapKey_H
#define CQChartsMapKey_H

#include <CQChartsTextBoxObj.h>

class CQChartsMapKey : public CQChartsTextBoxObj {
  Q_OBJECT

  Q_PROPERTY(double           margin    READ margin     WRITE setMargin   )
  Q_PROPERTY(CQChartsPosition position  READ position   WRITE setPosition )
  Q_PROPERTY(Qt::Alignment    align     READ align      WRITE setAlign    )
  Q_PROPERTY(bool             numeric   READ isNumeric  WRITE setNumeric  )
  Q_PROPERTY(bool             integral  READ isIntegral WRITE setIntegral )
  Q_PROPERTY(int              numUnique READ numUnique  WRITE setNumUnique)

 public:
  using Plot          = CQChartsPlot;
  using Position      = CQChartsPosition;
  using PropertyModel = CQPropertyViewModel;
  using PaintDevice   = CQChartsPaintDevice;
  using Font          = CQChartsFont;
  using Color         = CQChartsColor;
  using Alpha         = CQChartsAlpha;
  using BBox          = CQChartsGeom::BBox;
  using Point         = CQChartsGeom::Point;

 public:
  CQChartsMapKey(Plot *plot);

  //---

  //! get plot
  Plot *plot() const { return plot_; }

  //---

  //! get/set margin
  double margin() const { return margin_; }
  void setMargin(double r);

  //! get/set position
  const Position &position() const { return position_; }
  void setPosition(const Position &p);

  //! get/set align
  const Qt::Alignment &align() const { return align_; }
  void setAlign(const Qt::Alignment &a);

  //! get/set is numeric
  bool isNumeric() const { return numeric_; }
  void setNumeric(bool b) { numeric_ = b; }

  //! get/set is integral
  bool isIntegral() const { return integral_; }
  void setIntegral(bool b) { integral_ = b; }

  //! get/set num unique
  int numUnique() const { return numUnique_; }
  void setNumUnique(int i) { numUnique_ = i; }

  //! get/set unique values
  const QVariantList &uniqueValues() const { return uniqueValues_; }
  void setUniqueValues(const QVariantList &v) { uniqueValues_ = v; }

  //---

  //! get/set bbox
  const BBox &bbox() const { return bbox_; }
  void setBBox(const BBox &b) { bbox_ = b; }

  //---

  // Implement edit interface
  bool editPress (const Point &) override;
  bool editMove  (const Point &) override;
  bool editMotion(const Point &) override;

  //---

  virtual void draw(PaintDevice *device, bool usePenBrush=false) = 0;

 protected:
  virtual void invalidate() = 0;

  void calcCenter();

  void calcAlignedBBox();

 protected:
  using EditHandlesP = std::unique_ptr<EditHandles>;

  double        margin_    { 4.0 };                                 //!< margin in pixels
  Position      position_;                                          //!< key position
  Qt::Alignment align_     { Qt::AlignHCenter | Qt::AlignVCenter }; //!< key align
  bool          numeric_   { false };                               //!< is numeric
  bool          integral_  { false };                               //!< is integral
  int           numUnique_ { -1 };                                  //!< num unique
  QVariantList  uniqueValues_;                                      //!< unique values

  mutable double kw_ { 0.0 };
  mutable double kh_ { 0.0 };
  mutable double xm_ { 0.0 };
  mutable double ym_ { 0.0 };
  mutable BBox   pbbox_;
};

//-----

class CQChartsColorMapKey : public CQChartsMapKey {
  Q_OBJECT

  Q_PROPERTY(double              dataMin     READ dataMin     WRITE setDataMin    )
  Q_PROPERTY(double              dataMax     READ dataMax     WRITE setDataMax    )
  Q_PROPERTY(double              mapMin      READ mapMin      WRITE setMapMin     )
  Q_PROPERTY(double              mapMax      READ mapMax      WRITE setMapMax     )
  Q_PROPERTY(CQChartsPaletteName paletteName READ paletteName WRITE setPaletteName)

 public:
  using PenBrush    = CQChartsPenBrush;
  using BrushData   = CQChartsBrushData;
  using PenData     = CQChartsPenData;
  using PaletteName = CQChartsPaletteName;
  using ColorInd    = CQChartsUtil::ColorInd;

  using BBox  = CQChartsGeom::BBox;
  using Point = CQChartsGeom::Point;

 public:
  CQChartsColorMapKey(Plot *plot);

  //---

  QString calcId() const override { return "color_map_key"; }

  //---

  // data range
  double dataMin() const { return dataMin_; }
  void setDataMin(double r) { dataMin_ = r; invalidate(); }

  double dataMax() const { return dataMax_; }
  void setDataMax(double r) { dataMax_ = r; invalidate(); }

  //---

  // map range
  double mapMin() const { return mapMin_; }
  void setMapMin(double r) { mapMin_ = r; invalidate(); }

  double mapMax() const { return mapMax_; }
  void setMapMax(double r) { mapMax_ = r; invalidate(); }

  //---

  //! get/set color palette name
  const PaletteName &paletteName() const { return paletteName_; }
  void setPaletteName(const PaletteName &n) { paletteName_ = n; invalidate(); }

  //---

  void addProperties(PropertyModel *model, const QString &path, const QString &desc="") override;

  //---

  void draw(PaintDevice *device, bool useBrush=false) override;

  void drawContiguous(PaintDevice *device);
  void drawDiscreet  (PaintDevice *device);

  //---

 private:
  void invalidate() override;

 signals:
  void dataChanged();

 private:
  QString valueText(double value) const;

 private:
  double dataMin_ { 0.0 }; //!< model data min
  double dataMax_ { 1.0 }; //!< model data max

  double mapMin_ { 0.0 }; //!< mapped color min
  double mapMax_ { 1.0 }; //!< mapped color max

  PaletteName paletteName_; //!< custom palette

  BBox tbbox_;
};

//-----

// TODO:
//  . custom palette
//  . shape
//  . spread/overlay
//  . text align left/center/right
//  . separate border style
class CQChartsSymbolSizeMapKey : public CQChartsMapKey {
  Q_OBJECT

  Q_PROPERTY(double              dataMin     READ dataMin     WRITE setDataMin    )
  Q_PROPERTY(double              dataMax     READ dataMax     WRITE setDataMax    )
  Q_PROPERTY(double              mapMin      READ mapMin      WRITE setMapMin     )
  Q_PROPERTY(double              mapMax      READ mapMax      WRITE setMapMax     )
  Q_PROPERTY(double              scale       READ scale       WRITE setScale      )
  Q_PROPERTY(bool                stacked     READ isStacked   WRITE setStacked    )
  Q_PROPERTY(int                 rows        READ rows        WRITE setRows       )
  Q_PROPERTY(CQChartsAlpha       alpha       READ alpha       WRITE setAlpha      )
  Q_PROPERTY(Qt::Alignment       textAlign   READ textAlign   WRITE setTextAlign  )
  Q_PROPERTY(CQChartsPaletteName paletteName READ paletteName WRITE setPaletteName)

 public:
  using Alpha       = CQChartsAlpha;
  using PenBrush    = CQChartsPenBrush;
  using BrushData   = CQChartsBrushData;
  using PenData     = CQChartsPenData;
  using PaletteName = CQChartsPaletteName;
  using ColorInd    = CQChartsUtil::ColorInd;

  using BBox  = CQChartsGeom::BBox;
  using Point = CQChartsGeom::Point;

 public:
  CQChartsSymbolSizeMapKey(Plot *plot);

  //---

  QString calcId() const override { return "symbol_size_map_key"; }

  //---

  // data range
  double dataMin() const { return dataMin_; }
  void setDataMin(double r) { dataMin_ = r; invalidate(); }

  double dataMax() const { return dataMax_; }
  void setDataMax(double r) { dataMax_ = r; invalidate(); }

  //---

  // map range
  double mapMin() const { return mapMin_; }
  void setMapMin(double r) { mapMin_ = r; invalidate(); }

  double mapMax() const { return mapMax_; }
  void setMapMax(double r) { mapMax_ = r; invalidate(); }

  //---

  double scale() const { return scale_; }
  void setScale(double r) { scale_ = r; invalidate(); }

  bool isStacked() const { return stacked_; }
  void setStacked(bool b) { stacked_ = b; invalidate(); }

  int rows() const { return rows_; }
  void setRows(int i) { rows_ = i; invalidate(); }

  //---

  const Alpha &alpha() const { return alpha_; }
  void setAlpha(const Alpha &a) { alpha_ = a; invalidate(); }

  const Qt::Alignment &textAlign() const { return textAlign_; }
  void setTextAlign(const Qt::Alignment &a) { textAlign_ = a; invalidate(); }

  const PaletteName &paletteName() const { return paletteName_; }
  void setPaletteName(const PaletteName &n) { paletteName_ = n; invalidate(); }

  //---

  void addProperties(PropertyModel *model, const QString &path, const QString &desc="") override;

  //---

  void draw(PaintDevice *device, bool usePenBrush=false) override;

  //---

  void initDraw();

  void drawParts(PaintDevice *device, bool usePenBrush=false);

  void drawBorder(PaintDevice *device, bool usePenBrush=false);

  void drawCircles(PaintDevice *device, bool usePenBrush=false);

  void drawText(PaintDevice *device, const CQChartsTextOptions &textOptions,
                bool usePenBrush=false);

 private:
  void invalidate() override;

 signals:
  void dataChanged();

 private:
  void initCenter     () const;
  void calcSymbolBoxes() const;
  void calcTextBBox   () const;
  void alignBoxes     () const;

  QString valueText(double value) const;

 private:
  using BBoxes = std::vector<BBox>;

  double dataMin_ { 0.0 }; //!< model data min
  double dataMax_ { 1.0 }; //!< model data max

  double mapMin_ { 5.0 };  //!< mapped symbol size min (pixels)
  double mapMax_ { 17.0 }; //!< mapped symbol size max (pixels)

  double scale_ { 1.0 }; //! scale symbol sizes

  bool stacked_ { false }; //! draw stacked
  int  rows_    { 3 };     //! number of symbol rows

  Alpha         alpha_     { 0.6 };                               //!< background alpha
  Qt::Alignment textAlign_ { Qt::AlignRight | Qt::AlignVCenter }; //!< text align
  PaletteName   paletteName_;                                     //!< custom palette

  mutable BBox   tbbox_;
  mutable BBox   sbbox_;
  mutable BBoxes symbolBoxes_;
  mutable Point  pcenter_;
  mutable Point  center_;
};

//-----

class CQChartsSymbolTypeMapKey : public CQChartsMapKey {
  Q_OBJECT

  Q_PROPERTY(int     dataMin   READ dataMin   WRITE setDataMin  )
  Q_PROPERTY(int     dataMax   READ dataMax   WRITE setDataMax  )
  Q_PROPERTY(int     mapMin    READ mapMin    WRITE setMapMin   )
  Q_PROPERTY(int     mapMax    READ mapMax    WRITE setMapMax   )
  Q_PROPERTY(QString symbolSet READ symbolSet WRITE setSymbolSet)

 public:
  using PenBrush  = CQChartsPenBrush;
  using BrushData = CQChartsBrushData;
  using PenData   = CQChartsPenData;
  using ColorInd  = CQChartsUtil::ColorInd;

  using BBox  = CQChartsGeom::BBox;
  using Point = CQChartsGeom::Point;

 public:
  CQChartsSymbolTypeMapKey(Plot *plot);

  //---

  QString calcId() const override { return "symbol_type_map_key"; }

  //---

  // data range
  int dataMin() const { return dataMin_; }
  void setDataMin(int i) { dataMin_ = i; invalidate(); }

  int dataMax() const { return dataMax_; }
  void setDataMax(int i) { dataMax_ = i; invalidate(); }

  //---

  // map range
  int mapMin() const { return mapMin_; }
  void setMapMin(int i) { mapMin_ = i; invalidate(); }

  int mapMax() const { return mapMax_; }
  void setMapMax(int i) { mapMax_ = i; invalidate(); }

  //---

  const QString &symbolSet() const { return symbolSet_; }
  void setSymbolSet(const QString &s) { symbolSet_ = s; }

  //---

  void addProperties(PropertyModel *model, const QString &path, const QString &desc="") override;

  //---

  void draw(PaintDevice *device, bool usePenBrush=false) override;

 private:
  void invalidate() override;

  QSize pixelSize() const;

 signals:
  void dataChanged();

 private:
  QString valueText(double value) const;

 private:
  int dataMin_ { 0 }; //!< model data min
  int dataMax_ { 1 }; //!< model data max

  int mapMin_ { 0 }; //!< mapped symbol type min
  int mapMax_ { 1 }; //!< mapped symbol type max

  QString symbolSet_;
};

#endif
