#ifndef CQChartsSunburstPlot_H
#define CQChartsSunburstPlot_H

#include <CQChartsPlot.h>
#include <CQChartsPlotObj.h>
#include <CQChartsPaletteColor.h>
#include <QModelIndex>

class CQChartsSunburstPlot;
class CQChartsSunburstRootNode;
class CQChartsSunburstHierNode;
class CQChartsSunburstNode;
class CQChartsTextBoxObj;

//---

class CQChartsSunburstNodeObj : public CQChartsPlotObj {
  Q_OBJECT

 public:
  CQChartsSunburstNodeObj(CQChartsSunburstPlot *plot, const CQChartsGeom::BBox &rect,
                          CQChartsSunburstNode *node);

  CQChartsSunburstNode *node() const { return node_; }

  QString calcId() const override;

  QString calcTipId() const override;

  bool inside(const CQChartsGeom::Point &p) const override;

  void addSelectIndex() override;

  bool isIndex(const QModelIndex &ind) const override;

  void draw(QPainter *painter, const CQChartsPlot::Layer &) override;

 private:
  CQChartsSunburstPlot *plot_ { nullptr };
  CQChartsSunburstNode *node_ { nullptr };
};

//---

class CQChartsSunburstNode {
 private:
  static uint nextId() {
    static int lastId = 0;

    return ++lastId;
  }

 public:
  CQChartsSunburstNode(CQChartsSunburstPlot *plot, CQChartsSunburstHierNode *parent,
                       const QString &name="");

  virtual ~CQChartsSunburstNode() { }

  CQChartsSunburstPlot *plot() const { return plot_; }

  CQChartsSunburstHierNode *parent() const { return parent_; }

  uint id() const { return id_; }

  const QString &name() const { return name_; }

  virtual double size() const { return size_; }
  void setSize(double size) { size_ = size; }

  virtual int depth() const { return 1; }

  virtual int numNodes() const { return 1; }

  const QModelIndex &ind() const { return ind_; }
  void setInd(const QModelIndex &v) { ind_ = v; }

  double r () const { return r_ ; }
  double a () const { return a_ ; }
  double dr() const { return dr_; }
  double da() const { return da_; }

  int colorId() const { return colorId_; }
  virtual void setColorId(int colorId) { colorId_ = colorId; }

  const CQChartsPaletteColor &color() const { return color_; }
  void setColor(const CQChartsPaletteColor &v) { color_ = v; }

  bool isFiller() const { return filler_; }
  void setFiller(bool b) { filler_ = b; }

  virtual double hierSize() const { return size(); }

  virtual QString hierName() const;

  virtual void setPosition(double r, double a, double dr, double da);

  void unplace() { placed_ = false; }

  bool placed() const { return placed_; }

  CQChartsSunburstNodeObj *obj() const { return obj_; }
  void setObj(CQChartsSunburstNodeObj *obj) { obj_ = obj; }

  //---

  static double normalizeAngle(double a) {
    while (a <    0.0) a += 360.0;
    while (a >= 360.0) a -= 360.0;

    return a;
  }

  bool pointInside(double x, double y);

  virtual QColor interpColor(CQChartsSunburstPlot *plot, int n) const;

 protected:
  CQChartsSunburstPlot*     plot_    { nullptr }; // parent plot
  CQChartsSunburstHierNode* parent_  { nullptr }; // parent hier node
  uint                      id_      { 0 };       // node id
  QString                   name_;                // node name
  double                    size_    { 0.0 };     // node size
  QModelIndex               ind_;                 // node index
  double                    r_       { 0.0 };     // node radius
  double                    a_       { 0.0 };     // node angle
  double                    dr_      { 0.0 };     // node delta radius
  double                    da_      { 0.0 };     // node delta angle
  int                       colorId_ { -1 };      // node color index
  CQChartsPaletteColor      color_   { };         // node explicit color
  bool                      filler_  { false };   // is filler
  bool                      placed_  { false };   // is place
  CQChartsSunburstNodeObj*  obj_     { nullptr }; // associated object
};

//---

struct CQChartsSunburstNodeNameCmp {
  // sort reverse alphabetic no case
  bool operator()(const CQChartsSunburstNode *n1, const CQChartsSunburstNode *n2);
};

struct CQChartsSunburstNodeSizeCmp {
  bool operator()(const CQChartsSunburstNode *n1, const CQChartsSunburstNode *n2) {
    return n1->size() < n2->size();
  }
};

struct CQChartsSunburstNodeCountCmp {
  bool operator()(const CQChartsSunburstNode *n1, const CQChartsSunburstNode *n2) {
    return n1->numNodes() < n2->numNodes();
  }
};

//---

class CQChartsSunburstHierNode : public CQChartsSunburstNode {
 public:
  enum class Order {
    SIZE,
    COUNT
  };

 public:
  using Nodes    = std::vector<CQChartsSunburstNode*>;
  using Children = std::vector<CQChartsSunburstHierNode*>;

 public:
  CQChartsSunburstHierNode(CQChartsSunburstPlot *plot, CQChartsSunburstHierNode *parent=nullptr,
                           const QString &name="");

 ~CQChartsSunburstHierNode();

  double hierSize() const override;

  int depth() const;

  int numNodes() const;

  bool hasNodes() const { return ! nodes_.empty(); }

  const Nodes &getNodes() const { return nodes_; }

  bool hasChildren() const { return ! children_.empty(); }

  const Children &getChildren() const { return children_; }

  void unplace();

  void unplaceNodes();

  void packNodes(CQChartsSunburstHierNode *root, double ri, double ro,
                 double dr, double a, double da, const Order &order, bool sort);

  void packSubNodes(CQChartsSunburstHierNode *root, double ri,
                    double dr, double a, double da, const Order &order, bool sort);

  void addNode(CQChartsSunburstNode *node);

  void removeNode(CQChartsSunburstNode *node);

  QColor interpColor(CQChartsSunburstPlot *plot, int n) const override;

 private:
  Nodes    nodes_;    // child nodes
  Children children_; // child hier nodes
};

//---

class CQChartsSunburstRootNode : public CQChartsSunburstHierNode {
 public:
  CQChartsSunburstRootNode(CQChartsSunburstPlot *plot, const QString &name="") :
   CQChartsSunburstHierNode(plot, 0, name), sort_(true), order_(Order::SIZE) {
  }

  bool sort() const { return sort_; }
  void setSort(bool sort) { sort_ = sort; }

  Order order() const { return order_; }
  void setOrder(Order order) { order_ = order; }

  void packNodes(double ri=0.5, double ro=1.0, double dr=0.0, double a=0.0, double da=360.0) {
    CQChartsSunburstHierNode::packNodes(this, ri, ro, dr, a, da, order(), sort());
  }

 private:
  bool  sort_  { true };        // is sorted
  Order order_ { Order::SIZE }; // sort order
};

//---

class CQChartsSunburstPlotType : public CQChartsHierPlotType {
 public:
  CQChartsSunburstPlotType();

  QString name() const override { return "sunburst"; }
  QString desc() const override { return "Sunburst"; }

  void addParameters() override;

  CQChartsPlot *create(CQChartsView *view, const ModelP &model) const override;
};

//---

class CQChartsSunburstPlot : public CQChartsHierPlot {
  Q_OBJECT

  Q_PROPERTY(double  innerRadius     READ innerRadius       WRITE setInnerRadius    )
  Q_PROPERTY(double  outerRadius     READ outerRadius       WRITE setOuterRadius    )
  Q_PROPERTY(double  startAngle      READ startAngle        WRITE setStartAngle     )
  Q_PROPERTY(bool    multiRoot       READ isMultiRoot       WRITE setMultiRoot      )
  Q_PROPERTY(bool    border          READ isBorder          WRITE setBorder         )
  Q_PROPERTY(QString borderColor     READ borderColorStr    WRITE setBorderColorStr )
  Q_PROPERTY(double  borderAlpha     READ borderAlpha       WRITE setBorderAlpha    )
  Q_PROPERTY(double  borderWidth     READ borderWidth       WRITE setBorderWidth    )
  Q_PROPERTY(bool    filled          READ isFilled          WRITE setFilled         )
  Q_PROPERTY(QString fillColor       READ fillColorStr      WRITE setFillColorStr   )
  Q_PROPERTY(double  fillAlpha       READ fillAlpha         WRITE setFillAlpha      )
  Q_PROPERTY(Pattern fillPattern     READ fillPattern       WRITE setFillPattern    )
  Q_PROPERTY(QFont   textFont        READ textFont          WRITE setTextFont       )
  Q_PROPERTY(QString textColor       READ textColorStr      WRITE setTextColorStr   )
  Q_PROPERTY(bool    textContrast    READ isTextContrast    WRITE setTextContrast   )
  Q_PROPERTY(bool    colorMapEnabled READ isColorMapEnabled WRITE setColorMapEnabled)
  Q_PROPERTY(double  colorMapMin     READ colorMapMin       WRITE setColorMapMin    )
  Q_PROPERTY(double  colorMapMax     READ colorMapMax       WRITE setColorMapMax    )

  Q_ENUMS(Pattern);

 public:
  enum class Pattern {
    SOLID,
    HATCH,
    DENSE,
    HORIZ,
    VERT,
    FDIAG,
    BDIAG
  };

  using OptColor = boost::optional<CQChartsPaletteColor>;

  using RootNodes = std::vector<CQChartsSunburstRootNode*>;

 public:
  CQChartsSunburstPlot(CQChartsView *view, const ModelP &model);

 ~CQChartsSunburstPlot();

  //---

  double innerRadius() const { return innerRadius_; }
  void setInnerRadius(double r) { innerRadius_ = r; resetRoots(); updateObjs(); }

  double outerRadius() const { return outerRadius_; }
  void setOuterRadius(double r) { outerRadius_ = r; resetRoots(); updateObjs(); }

  double startAngle() const { return startAngle_; }
  void setStartAngle(double r) { startAngle_ = r; resetRoots(); updateObjs(); }

  //---

  const RootNodes &roots() const { return roots_; }

  bool isMultiRoot() const { return multiRoot_; }
  void setMultiRoot(bool b) { multiRoot_ = b; resetRoots(); updateObjs(); }

  //---

  bool isBorder() const;
  void setBorder(bool b);

  QString borderColorStr() const;
  void setBorderColorStr(const QString &str);

  QColor interpBorderColor(int i, int n) const;

  double borderAlpha() const;
  void setBorderAlpha(double a);

  double borderWidth() const;
  void setBorderWidth(double r);

  //---

  bool isFilled() const;
  void setFilled(bool b);

  QString fillColorStr() const;
  void setFillColorStr(const QString &s);

  QColor interpFillColor(int i, int n) const;

  double fillAlpha() const;
  void setFillAlpha(double a);

  Pattern fillPattern() const;
  void setFillPattern(Pattern pattern);

  //---

  const QFont &textFont() const;
  void setTextFont(const QFont &f);

  QString textColorStr() const;
  void setTextColorStr(const QString &s);

  QColor interpTextColor(int i, int n) const;

  bool isTextContrast() const;
  void setTextContrast(bool b);

  //---

  CQChartsSunburstHierNode *currentRoot() const;
  void setCurrentRoot(CQChartsSunburstHierNode *r, bool update=true);

  //---

  int colorId() const { return colorId_; }

  int numColorIds() const { return numColorIds_; }

  void initColorIds() {
    colorId_     = -1;
    numColorIds_ = 0;
  }

  int nextColorId() {
    ++colorId_;

    if (colorId_ >= numColorIds_)
      numColorIds_ = colorId_ + 1;

    return colorId_;
  }

  //---

  void addProperties() override;

  void updateRange(bool apply=true) override;

  void updateObjs() override;

  bool initObjs() override;

  //---

  void handleResize() override;

  //---

  void draw(QPainter *) override;

  void drawNode(QPainter *painter, CQChartsSunburstNodeObj *nodeObj,
                CQChartsSunburstNode *node);

  //---

  bool addMenuItems(QMenu *menu) override;

 private:
  void resetRoots();

  void initRoots();

  void replaceRoots();

  void colorNodes(CQChartsSunburstHierNode *hier);

  void colorNode(CQChartsSunburstNode *node);

  //---

  void loadChildren(CQChartsSunburstHierNode *hier, const QModelIndex &index=QModelIndex(),
                    int depth=1);

  void loadFlat(CQChartsSunburstHierNode *hier);

  void addExtraNodes(CQChartsSunburstHierNode *hier);

  //---

  CQChartsSunburstRootNode *createRootNode(const QString &name="");

  CQChartsSunburstRootNode *rootNode(const QString &name) const;

  CQChartsSunburstHierNode *childHierNode(CQChartsSunburstHierNode *parent,
                                          const QString &name) const;

  CQChartsSunburstNode *childNode(CQChartsSunburstHierNode *parent,
                                  const QString &name) const;

  //---

  void addPlotObjs(CQChartsSunburstHierNode *parent);

  void addPlotObj(CQChartsSunburstNode *node);

  void drawNodes(QPainter *painter, CQChartsSunburstHierNode *hier);

 public slots:
  void pushSlot();
  void popSlot();
  void popTopSlot();

 private:
  double              innerRadius_ { 0.5 };     // inner radius
  double              outerRadius_ { 1.0 };     // outer radius
  double              startAngle_  { -90 };     // start angle
  bool                multiRoot_   { false };   // has multiple roots
  RootNodes           roots_;                   // root nodes
  QString             currentRootName_;         // current root name
  CQChartsTextBoxObj* textBoxObj_  { nullptr }; // arc fill/border/text object
  int                 colorId_     { -1 };      // current color id
  int                 numColorIds_ { 0 };       // num used color ids
};

#endif
