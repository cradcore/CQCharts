#include <CQChartsTreeMapPlot.h>
#include <CQChartsView.h>
#include <CQChartsUtil.h>
#include <CQCharts.h>
#include <CQChartsTip.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsViewPlotPaintDevice.h>
#include <CQChartsScriptPaintDevice.h>
#include <CQChartsHtml.h>

#include <CQPropertyViewItem.h>
#include <CQPerfMonitor.h>

#include <QMenu>

CQChartsTreeMapPlotType::
CQChartsTreeMapPlotType()
{
}

void
CQChartsTreeMapPlotType::
addParameters()
{
  CQChartsHierPlotType::addParameters();
}

QString
CQChartsTreeMapPlotType::
description() const
{
  auto B   = [](const QString &str) { return CQChartsHtml::Str::bold(str); };
  auto IMG = [](const QString &src) { return CQChartsHtml::Str::img(src); };

  return CQChartsHtml().
   h2("Tree Map Plot").
    h3("Summary").
     p("Draw hierarchical data values using sized boxes.").
     p("Each level of the tree map can have an optional title (if large enough) "
       "and can be separately colored.").
     p("The user can push into, or pop out of, a level of hierarchy by selecting the node "
       "and using the Push and Pop operations on the context menu.").
    h3("Columns").
     p("The hierarchical data comes from the " + B("Name") + " columns and " +
       B("Value") + " column.").
     p("If the name columns is a hierarchical path then the separator can be specified width "
       "the " + B("Separator") + " option (default '/').").
    h3("Limitations").
     p("This plot does not support a user specified range, axes, logarithm scales, "
       "or probing.").
    h3("Example").
     p(IMG("images/treemap.png"));
}

CQChartsPlot *
CQChartsTreeMapPlotType::
create(CQChartsView *view, const ModelP &model) const
{
  return new CQChartsTreeMapPlot(view, model);
}

//------

CQChartsTreeMapPlot::
CQChartsTreeMapPlot(CQChartsView *view, const ModelP &model) :
 CQChartsHierPlot(view, view->charts()->plotType("treemap"), model),
 CQChartsObjHeaderShapeData<CQChartsTreeMapPlot>(this),
 CQChartsObjHeaderTextData <CQChartsTreeMapPlot>(this),
 CQChartsObjShapeData      <CQChartsTreeMapPlot>(this),
 CQChartsObjTextData       <CQChartsTreeMapPlot>(this)
{
  NoUpdate noUpdate(this);

  setHeaderFillColor(CQChartsColor(CQChartsColor::Type::INTERFACE_VALUE, 0.4));
  setHeaderTextColor(CQChartsColor(CQChartsColor::Type::INTERFACE_VALUE, 1.0));

  setHeaderTextFontSize(12.0);
  setHeaderTextAlign(Qt::AlignLeft | Qt::AlignVCenter);

  setHeaderStroked(true);
  setHeaderStrokeAlpha(CQChartsAlpha(0.5));

  setHeaderFilled(true);

  setFillColor(CQChartsColor(CQChartsColor::Type::PALETTE ));
  setTextColor(CQChartsColor(CQChartsColor::Type::CONTRAST));

  setFilled (true);
  setStroked(true);

  setTextFontSize(14.0);
  setTextAlign(Qt::AlignHCenter | Qt::AlignVCenter);

  setOuterMargin(CQChartsLength("4px"), CQChartsLength("4px"),
                 CQChartsLength("4px"), CQChartsLength("4px"));

  addTitle();
}

CQChartsTreeMapPlot::
~CQChartsTreeMapPlot()
{
  delete root_;
}

//---

void
CQChartsTreeMapPlot::
setTitles(bool b)
{
  CQChartsUtil::testAndSet(titleData_.visible, b, [&]() { updateCurrentRoot(); } );
}

void
CQChartsTreeMapPlot::
setTitleAutoHide(bool b)
{
  CQChartsUtil::testAndSet(titleData_.autoHide, b, [&]() { updateCurrentRoot(); } );
}

void
CQChartsTreeMapPlot::
setTitleMaxExtent(const CQChartsOptReal &r)
{
  CQChartsUtil::testAndSet(titleData_.maxExtent, r, [&]() { updateCurrentRoot(); } );
}

void
CQChartsTreeMapPlot::
setTitleHeight(const CQChartsOptLength &l)
{
  CQChartsUtil::testAndSet(titleData_.height, l, [&]() { updateCurrentRoot(); } );
}

void
CQChartsTreeMapPlot::
setTitleHierName(bool b)
{
  CQChartsUtil::testAndSet(titleData_.hierName, b, [&]() { drawObjs(); } );
}

void
CQChartsTreeMapPlot::
setTitleTextClipped(bool b)
{
  CQChartsUtil::testAndSet(titleData_.textClipped, b, [&]() { drawObjs(); } );
}

void
CQChartsTreeMapPlot::
setTitleMargin(double m)
{
  CQChartsUtil::testAndSet(titleData_.margin, m, [&]() { updateCurrentRoot(); } );
}

void
CQChartsTreeMapPlot::
setTitleDepth(int d)
{
  CQChartsUtil::testAndSet(titleData_.depth, d, [&]() { updateCurrentRoot(); } );
}

//--

double
CQChartsTreeMapPlot::
calcTitleHeight() const
{
  QFont font = view()->plotFont(this, headerTextFont());

  QFontMetricsF fm(font);

  if (titleHeight().isSet()) {
    double hh = lengthPixelHeight(*titleHeight().value());

    return std::max(hh, 4.0);
  }

  return fm.height() + 4.0;
}

//----

void
CQChartsTreeMapPlot::
setHierName(bool b)
{
  CQChartsUtil::testAndSet(nodeData_.hierName, b, [&]() { drawObjs(); } );
}

void
CQChartsTreeMapPlot::
setNumSkipHier(int n)
{
  CQChartsUtil::testAndSet(nodeData_.numSkipHier, n, [&]() { drawObjs(); } );
}

void
CQChartsTreeMapPlot::
setTextClipped(bool b)
{
  CQChartsUtil::testAndSet(nodeData_.textClipped, b, [&]() { drawObjs(); } );
}

void
CQChartsTreeMapPlot::
setValueLabel(bool b)
{
  CQChartsUtil::testAndSet(nodeData_.valueLabel, b, [&]() { drawObjs(); } );
}

void
CQChartsTreeMapPlot::
setMarginWidth(const CQChartsLength &l)
{
  CQChartsUtil::testAndSet(nodeData_.marginWidth, l, [&]() { updateCurrentRoot(); } );
}

//----

void
CQChartsTreeMapPlot::
setColorById(bool b)
{
  CQChartsUtil::testAndSet(colorById_, b, [&]() { drawObjs(); } );
}

//----

void
CQChartsTreeMapPlot::
setHeaderTextFontSize(double s)
{
  if (s != headerTextData_.font().pointSizeF()) {
    CQChartsFont f = headerTextData_.font(); f.setPointSizeF(s); headerTextData_.setFont(f);

    drawObjs();
  }
}

//----

void
CQChartsTreeMapPlot::
setTextFontSize(double s)
{
  if (s != textData_.font().pointSizeF()) {
    CQChartsFont f = textData_.font(); f.setPointSizeF(s); textData_.setFont(f);

    drawObjs();
  }
}

//---

void
CQChartsTreeMapPlot::
addProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  //---

  addHierProperties();

  // options
  addProp("options", "valueLabel"      , "", "Show value label");
  addProp("options", "followViewExpand", "", "Follow view expand");

  // margins
  addProp("margins", "marginWidth", "box", "Margin size for tree map boxes");

  // coloring
  addProp("coloring", "colorById", "colorById", "Color by id");

  //---

  // header
  addProp("header", "titles"        , "visible"  ,
          "Header title visible for each hierarchical group");
  addProp("header", "titleAutoHide", "autoHide",
          "Auto hide header if extent too large");
  addProp("header", "titleMaxExtent", "maxExtent",
          "Max extent of hierarchical group header (0.0 - 1.0)")->
    setMinValue(0.0).setMaxValue(1.0);
  addProp("header", "titleHeight"  , "height"   ,
          "Explicit hierarchical group header height");
  addProp("header", "titleMargin"  , "margin"   ,
          "Hierarchical group header margin in pixels")->setMinValue(0.0);
  addProp("header", "titleDepth"   , "depth"   ,
          "Maximum depth to show header")->setMinValue(-1);

  // header/fill
  addProp("header/fill", "headerFilled", "visible", "Header fill visible");

  addFillProperties("header/fill", "headerFill", "Header");

  // header/stroke
  addProp("header/stroke", "headerStroked", "visible", "Header stroke visible");

  addLineProperties("header/stroke", "headerStroke", "Header");

  addProp("header/text", "titleHierName"   , "hierName", "Show hierarchical name on title");
  addProp("header/text", "titleTextClipped", "clipped" , "Clip text to header");

  addTextProperties("header/text", "headerText", "Header", CQChartsTextOptions::ValueType::ALL);

  //---

  // fill
  addProp("fill", "filled", "visible", "Fill visible");

  addFillProperties("fill", "fill", "");

  // stroke
  addProp("stroke", "stroked", "visible", "Stroke visible");

  addLineProperties("stroke", "stroke", "");

  // text
  addProp("text", "textVisible", "visible", "Text visible");

  addProp("text", "hierName"   , "hierName"   , "Show hierarchical name in box");
  addProp("text", "numSkipHier", "numSkipHier", "Number of hierarchical name levels to skip");
  addProp("text", "textClipped", "clipped"    , "Clip text to box");

  addTextProperties("text", "text", "", CQChartsTextOptions::ValueType::ALL);
}

//------

CQChartsTreeMapHierNode *
CQChartsTreeMapPlot::
currentRoot() const
{
  QStringList names = currentRootName_.split(separator(), QString::SkipEmptyParts);

  if (names.empty())
    return firstHier();

  auto *currentRoot = root();

  if (! currentRoot)
    return nullptr;

  for (int i = 0; i < names.size(); ++i) {
    auto *hier = childHierNode(currentRoot, names[i]);

    if (! hier)
      return currentRoot;

    currentRoot = hier;
  }

  return currentRoot;
}

void
CQChartsTreeMapPlot::
setCurrentRoot(CQChartsTreeMapHierNode *hier, bool update)
{
  if (hier)
    currentRootName_ = hier->hierName();
  else
    currentRootName_ = "";

  if (update)
    updateCurrentRoot();
}

void
CQChartsTreeMapPlot::
updateCurrentRoot()
{
  //replaceNodes();

  updateObjs();
}

CQChartsGeom::Range
CQChartsTreeMapPlot::
calcRange() const
{
  CQPerfTrace trace("CQChartsTreeMapPlot::calcRange");

  CQChartsGeom::Range dataRange;

  double r = 1.0;

  dataRange.updateRange(-r, -r);
  dataRange.updateRange( r,  r);

  if (isEqualScale()) {
    double aspect = this->aspect();

    dataRange.equalScale(aspect);
  }

  return dataRange;
}

//------

void
CQChartsTreeMapPlot::
clearPlotObjects()
{
  resetNodes();

  CQChartsPlot::clearPlotObjects();
}

bool
CQChartsTreeMapPlot::
createObjs(PlotObjs &objs) const
{
  CQPerfTrace trace("CQChartsTreeMapPlot::createObjs");

  NoUpdate noUpdate(this);

  auto *th = const_cast<CQChartsTreeMapPlot *>(this);

  th->clearErrors();

  //---

  // check columns
  bool columnsValid = true;

  // name column required
  // value, color columns optional

  if (! checkColumns(nameColumns(), "Name", /*required*/true))
    columnsValid = false;

  if (! checkColumn(valueColumn(), "Value", th->valueColumnType_))
    columnsValid = false;

  if (! checkColumn(colorColumn(), "Color")) columnsValid = false;

  if (! columnsValid)
    return false;

  //---

  // init value sets
  //initValueSets();

  //---

  if (! root())
    initNodes();
  else
    replaceNodes();

  //---

  th->initColorIds();

  colorNodes(firstHier());

  //---

  ig_ = 0;
  in_ = 0;

  if (currentRoot())
    initNodeObjs(currentRoot(), nullptr, 0, objs);

  //---

  for (auto &obj : objs) {
    auto *hierObj = dynamic_cast<CQChartsTreeMapHierObj *>(obj);
    auto *nodeObj = dynamic_cast<CQChartsTreeMapNodeObj *>(obj);

    if      (hierObj) {
      if (hierObj->parent())
        hierObj->setIg(ColorInd(hierObj->parent()->ind(), ig_));

      hierObj->setIv(ColorInd(hierObj->ind(), ig_));
    }
    else if (nodeObj) {
      if (nodeObj->parent())
        nodeObj->setIg(ColorInd(nodeObj->parent()->ind(), ig_));

      nodeObj->setIv(ColorInd(nodeObj->ind(), in_));
    }
  }

  //---

  th->modelViewExpansionChanged();

  return true;
}

void
CQChartsTreeMapPlot::
initNodeObjs(CQChartsTreeMapHierNode *hier, CQChartsTreeMapHierObj *parentObj,
             int depth, PlotObjs &objs) const
{
  CQChartsTreeMapHierObj *hierObj = nullptr;

  if (hier != root()) {
    CQChartsGeom::BBox rect(hier->x(), hier->y(), hier->x() + hier->w(), hier->y() + hier->h());

    ColorInd is(hier->depth(), maxDepth() + 1);

    hierObj = createHierObj(hier, parentObj, rect, is);

    hierObj->setInd(ig_);

    if (parentObj)
      parentObj->addChild(hierObj);

    objs.push_back(hierObj);

    ++ig_;
  }

  //---

  for (auto &hierNode : hier->getChildren())
    initNodeObjs(hierNode, hierObj, depth + 1, objs);

  //---

  int n = 0;

  for (auto &node : hier->getNodes()) {
    if (! node->placed()) continue;

    ++n;
  }

  int i = 0;

  for (auto &node : hier->getNodes()) {
    if (! node->placed()) continue;

    //---

    CQChartsGeom::BBox rect(node->x(), node->y(), node->x() + node->w(), node->y() + node->h());

    ColorInd is(node->depth(), maxDepth() + 1);

    auto *obj = createNodeObj(node, parentObj, rect, is);

    obj->setInd(in_);

    if (parentObj)
      parentObj->addChild(obj);

    objs.push_back(obj);

    ++i;

    ++in_;
  }
}

void
CQChartsTreeMapPlot::
resetNodes()
{
  delete root_;

  root_      = nullptr;
  firstHier_ = nullptr;
}

void
CQChartsTreeMapPlot::
initNodes() const
{
  auto *th = const_cast<CQChartsTreeMapPlot *>(this);

  //---

  th->hierInd_ = 0;

  th->root_ = new CQChartsTreeMapHierNode(this, nullptr, "<root>");

  root_->setDepth(0);
  root_->setHierInd(th->hierInd_++);

  //---

  if (isHierarchical())
    loadHier();
  else
    loadFlat();

  //---

  th->firstHier_ = root();

  while (firstHier_ && firstHier_->numChildren() == 1)
    th->firstHier_ = firstHier_->childAt(0);

  //---

  replaceNodes();
}

void
CQChartsTreeMapPlot::
replaceNodes() const
{
  auto *th = const_cast<CQChartsTreeMapPlot *>(this);

  th->windowHeaderHeight_ = pixelToWindowHeight(calcTitleHeight());
//th->windowMarginWidth_  = lengthPixelWidth   (marginWidth());
  th->windowMarginWidth_  = lengthPlotWidth    (marginWidth());

  auto *hier = currentRoot();

  if (hier)
    placeNodes(hier);
}

void
CQChartsTreeMapPlot::
placeNodes(CQChartsTreeMapHierNode *hier) const
{
  hier->setPosition(-1, -1, 2, 2);
}

void
CQChartsTreeMapPlot::
colorNodes(CQChartsTreeMapHierNode *hier) const
{
  if (! hier->hasNodes() && ! hier->hasChildren()) {
    colorNode(hier);
  }
  else {
    for (const auto &node : hier->getNodes())
      colorNode(node);

    for (const auto &child : hier->getChildren())
      colorNodes(child);
  }
}

void
CQChartsTreeMapPlot::
colorNode(CQChartsTreeMapNode *node) const
{
  if (! node->color().isValid()) {
    auto *th = const_cast<CQChartsTreeMapPlot *>(this);

    node->setColorId(th->nextColorId());
  }
}

void
CQChartsTreeMapPlot::
loadHier() const
{
  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(const CQChartsTreeMapPlot *plot, CQChartsTreeMapHierNode *root) :
     plot_(plot) {
      hierStack_.push_back(root);
    }

    State hierVisit(const QAbstractItemModel *, const VisitData &data) override {
      QString     name;
      QModelIndex nameInd;

      (void) getName(data, name, nameInd);

      //---

      auto *hier = plot_->addHierNode(parentHier(), name, nameInd);

      //---

      hierStack_.push_back(hier);

      return State::OK;
    }

    State hierPostVisit(const QAbstractItemModel *, const VisitData &) override {
      auto *node = hierStack_.back();

      hierStack_.pop_back();

      assert(! hierStack_.empty());

      if (node->hierSize() == 0) {
        auto *plot = const_cast<CQChartsTreeMapPlot *>(plot_);

        plot->removeHierNode(node);
      }

      return State::OK;
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      QString     name;
      QModelIndex nameInd;

      (void) getName(data, name, nameInd);

      //---

      double size = 1.0;

      if (plot_->valueColumn().isValid()) {
        CQChartsModelIndex valueModelInd(data.row, plot_->valueColumn(), data.parent);

        if (! plot_->getValueSize(valueModelInd, size))
          return State::SKIP;

        if (size == 0.0)
          return State::SKIP;
      }

      //---

      auto *node = plot_->hierAddNode(parentHier(), name, size, nameInd);

      if (node && plot_->colorColumn().isValid()) {
        CQChartsColor color;

        CQChartsModelIndex colorInd(data.row, plot_->colorColumn(), data.parent);

        if (plot_->modelIndexColor(colorInd, color))
          node->setColor(color);
      }

      return State::OK;
    }

   private:
    CQChartsTreeMapHierNode *parentHier() const {
      assert(! hierStack_.empty());

      return hierStack_.back();
    }

    bool getName(const VisitData &data, QString &name, QModelIndex &nameInd) const {
      CQChartsModelIndex nameModelInd(data.row, plot_->nameColumns().column(), data.parent);

      nameInd = plot_->modelIndex(nameModelInd);

      bool ok;

      name = plot_->modelString(nameModelInd, ok);

      return ok;
    }

   private:
    using HierStack = std::vector<CQChartsTreeMapHierNode *>;

    const CQChartsTreeMapPlot* plot_ { nullptr };
    HierStack                  hierStack_;
  };

  RowVisitor visitor(this, root());

  visitModel(visitor);
}

CQChartsTreeMapHierNode *
CQChartsTreeMapPlot::
addHierNode(CQChartsTreeMapHierNode *hier, const QString &name, const QModelIndex &nameInd) const
{
  auto *th = const_cast<CQChartsTreeMapPlot *>(this);

  //---

  int depth1 = hier->depth() + 1;

  QModelIndex nameInd1 = normalizeIndex(nameInd);

  auto *hier1 = new CQChartsTreeMapHierNode(this, hier, name, nameInd1);

  hier1->setDepth(depth1);

  hier1->setHierInd(th->hierInd_++);

  th->maxDepth_ = std::max(maxDepth_, depth1);

  return hier1;
}

void
CQChartsTreeMapPlot::
removeHierNode(CQChartsTreeMapHierNode *hier)
{
  if (hier->parent())
    hier->parent()->removeChild(hier);
}

CQChartsTreeMapNode *
CQChartsTreeMapPlot::
hierAddNode(CQChartsTreeMapHierNode *hier, const QString &name, double size,
            const QModelIndex &nameInd) const
{
  auto *th = const_cast<CQChartsTreeMapPlot *>(this);

  //---

  int depth1 = hier->depth() + 1;

  QModelIndex nameInd1 = normalizeIndex(nameInd);

  auto *node = new CQChartsTreeMapNode(this, hier, name, size, nameInd1);

  node->setDepth(depth1);

  hier->addNode(node);

  th->maxDepth_ = std::max(maxDepth_, depth1);

  return node;
}

void
CQChartsTreeMapPlot::
loadFlat() const
{
  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(const CQChartsTreeMapPlot *plot) :
     plot_(plot) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      // get hier names from name columns
      QStringList  nameStrs;
      ModelIndices nameInds;

      if (! plot_->getHierColumnNames(data.parent, data.row, plot_->nameColumns(),
                                      plot_->separator(), nameStrs, nameInds))
        return State::SKIP;

      QString     name;
      QModelIndex nameInd;

      if (plot_->idColumn().isValid()) {
        CQChartsModelIndex idModelInd(data.row, plot_->idColumn(), data.parent);

        bool ok;

        name = plot_->modelString(idModelInd, ok);

        if (! ok)
          name = nameStrs.back();

        nameInd = plot_->modelIndex(idModelInd);
      }
      else {
        name    = nameStrs.back();
        nameInd = nameInds[0];
      }

      QModelIndex nameInd1 = plot_->normalizeIndex(nameInd);

      //---

      double size = 1.0;

      if (plot_->valueColumn().isValid()) {
        CQChartsModelIndex valueModelInd(data.row, plot_->valueColumn(), data.parent);

        if (! plot_->getValueSize(valueModelInd, size))
          return State::SKIP;

        if (size == 0.0)
          return State::SKIP;
      }

      //---

      auto *node = plot_->flatAddNode(nameStrs, size, nameInd1, name);

      if (node && plot_->colorColumn().isValid()) {
        CQChartsColor color;

        CQChartsModelIndex colorInd(data.row, plot_->colorColumn(), data.parent);

        if (plot_->modelIndexColor(colorInd, color))
          node->setColor(color);
      }

      return State::OK;
    }

   private:
    const CQChartsTreeMapPlot* plot_ { nullptr };
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  //---

  addExtraNodes(root());
}

CQChartsTreeMapNode *
CQChartsTreeMapPlot::
flatAddNode(const QStringList &nameStrs, double size,
            const QModelIndex &nameInd, const QString &name) const
{
  auto *th = const_cast<CQChartsTreeMapPlot *>(this);

  //---

  int depth = nameStrs.length();

  th->maxDepth_ = std::max(maxDepth_, depth + 1);

  //---

  auto *parent = root();

  for (int j = 0; j < nameStrs.length() - 1; ++j) {
    auto *child = childHierNode(parent, nameStrs[j]);

    if (! child) {
      // remove any existing leaf node (save size to use in new hier node)
      QModelIndex nameInd1 = nameInd;
      double      size1    = 0.0;

      auto *node = childNode(parent, nameStrs[j]);

      if (node) {
        nameInd1 = node->ind();
        size1    = node->size();

        parent->removeNode(node);

        delete node;
      }

      //---

      child = new CQChartsTreeMapHierNode(this, parent, nameStrs[j], nameInd1);

      child->setSize(size1);

      child->setDepth(j + 1);
      child->setHierInd(th->hierInd_++);
    }

    parent = child;
  }

  //---

  QString nodeName = nameStrs.back();

  auto *node = childNode(parent, nodeName);

  if (! node) {
    // use hier node if already created
    auto *child = childHierNode(parent, nodeName);

    if (child) {
      child->setSize(size);
      return nullptr;
    }

    //---

    node = new CQChartsTreeMapNode(this, parent, name, size, nameInd);

    node->setDepth(depth);

    parent->addNode(node);
  }

  return node;
}

void
CQChartsTreeMapPlot::
addExtraNodes(CQChartsTreeMapHierNode *hier) const
{
  if (hier->size() > 0) {
    auto *node = new CQChartsTreeMapNode(this, hier, "", hier->size(), hier->ind());

    QModelIndex ind1 = unnormalizeIndex(hier->ind());

    if (colorColumn().isValid()) {
      CQChartsColor color;

      if (colorColumnColor(ind1.row(), ind1.parent(), color))
        node->setColor(color);
    }

    node->setDepth (hier->depth() + 1);
    node->setFiller(true);

    hier->addNode(node);

    hier->setSize(0.0);
  }

  for (const auto &child : hier->getChildren())
    addExtraNodes(child);
}

CQChartsTreeMapHierNode *
CQChartsTreeMapPlot::
childHierNode(CQChartsTreeMapHierNode *parent, const QString &name) const
{
  for (const auto &child : parent->getChildren())
    if (child->name() == name)
      return child;

  return nullptr;
}

CQChartsTreeMapNode *
CQChartsTreeMapPlot::
childNode(CQChartsTreeMapHierNode *parent, const QString &name) const
{
  for (const auto &node : parent->getNodes())
    if (node->name() == name)
      return node;

  return nullptr;
}

bool
CQChartsTreeMapPlot::
getValueSize(const CQChartsModelIndex &ind, double &size) const
{
  auto addDataError = [&](const QString &msg) {
    const_cast<CQChartsTreeMapPlot *>(this)->addDataError(ind, msg);
    return false;
  };

  size = 1.0;

  if (! ind.isValid())
    return false;

  bool ok = true;

  if      (valueColumnType() == ColumnType::REAL)
    size = modelReal(ind, ok);
  else if (valueColumnType() == ColumnType::INTEGER)
    size = (double) modelInteger(ind, ok);
  else
    ok = false;

  if (! ok)
    return addDataError("Invalid numeric value");

  if (size < 0.0)
    return addDataError("Non-positive value");

  return true;
}

//------

bool
CQChartsTreeMapPlot::
addMenuItems(QMenu *menu)
{
  auto addMenuAction = [&](QMenu *menu, const QString &name, const char *slot) {
    auto *action = new QAction(name, menu);

    connect(action, SIGNAL(triggered()), this, slot);

    menu->addAction(action);

    return action;
  };

  //---

  PlotObjs objs;

  selectedPlotObjs(objs);

  menu->addSeparator();

  auto *pushAction   = addMenuAction(menu, "Push"   , SLOT(pushSlot()));
  auto *popAction    = addMenuAction(menu, "Pop"    , SLOT(popSlot()));
  auto *popTopAction = addMenuAction(menu, "Pop Top", SLOT(popTopSlot()));

  pushAction  ->setEnabled(! objs.empty());
  popAction   ->setEnabled(currentRoot() != firstHier());
  popTopAction->setEnabled(currentRoot() != firstHier());

  return true;
}

void
CQChartsTreeMapPlot::
pushSlot()
{
  PlotObjs objs;

  selectedPlotObjs(objs);

  if (objs.empty()) {
    auto gpos = view()->menuPos();
    auto pos  = view()->mapFromGlobal(QPoint(gpos.x, gpos.y));

    auto w = pixelToWindow(CQChartsGeom::Point(pos));

    plotObjsAtPoint(w, objs);
  }

  if (objs.empty())
    return;

  for (const auto &obj : objs) {
    auto *hierObj = dynamic_cast<CQChartsTreeMapHierObj *>(obj);

    if (hierObj) {
      auto *hnode = hierObj->hierNode();

      setCurrentRoot(hnode, /*update*/true);

      break;
    }

    auto *nodeObj = dynamic_cast<CQChartsTreeMapNodeObj *>(obj);

    if (nodeObj) {
      auto *node = nodeObj->node();

      auto *hnode = node->parent();

      if (hnode) {
        setCurrentRoot(hnode, /*update*/true);

        break;
      }
    }
  }
}

void
CQChartsTreeMapPlot::
popSlot()
{
  auto *root = currentRoot();

  if (root && root->parent()) {
    setCurrentRoot(root->parent(), /*update*/true);
  }
}

void
CQChartsTreeMapPlot::
popTopSlot()
{
  auto *root = currentRoot();

  if (root != firstHier()) {
    setCurrentRoot(firstHier(), /*update*/true);
  }
}

//------

void
CQChartsTreeMapPlot::
postResize()
{
  CQChartsPlot::postResize();

  //replaceNodes();

  updateObjs();
}

//------

void
CQChartsTreeMapPlot::
followViewExpandChanged()
{
  if (isFollowViewExpand())
    modelViewExpansionChanged();
  else
    resetNodeExpansion();

  drawObjs();
}

void
CQChartsTreeMapPlot::
modelViewExpansionChanged()
{
  if (! isFollowViewExpand())
    return;

  std::set<QModelIndex> indSet;

  expandedModelIndices(indSet);

  for (auto &hierNode : root()->getChildren())
    setNodeExpansion(hierNode, indSet);

  drawObjs();
}

void
CQChartsTreeMapPlot::
setNodeExpansion(CQChartsTreeMapHierNode *hierNode, const std::set<QModelIndex> &indSet)
{
  hierNode->setExpanded(indSet.find(hierNode->ind()) != indSet.end());

  for (auto &hierNode1 : hierNode->getChildren())
    setNodeExpansion(hierNode1, indSet);
}

void
CQChartsTreeMapPlot::
resetNodeExpansion()
{
  for (auto &hierNode : root()->getChildren())
    resetNodeExpansion(hierNode);
}

void
CQChartsTreeMapPlot::
resetNodeExpansion(CQChartsTreeMapHierNode *hierNode)
{
  hierNode->setExpanded(true);

  for (auto &hierNode1 : hierNode->getChildren())
    resetNodeExpansion(hierNode1);
}

//---

CQChartsTreeMapHierObj *
CQChartsTreeMapPlot::
createHierObj(CQChartsTreeMapHierNode *hier, CQChartsTreeMapHierObj *hierObj,
              const CQChartsGeom::BBox &rect, const ColorInd &is) const
{
  return new CQChartsTreeMapHierObj(this, hier, hierObj, rect, is);
}

CQChartsTreeMapNodeObj *
CQChartsTreeMapPlot::
createNodeObj(CQChartsTreeMapNode *node, CQChartsTreeMapHierObj *hierObj,
              const CQChartsGeom::BBox &rect, const ColorInd &is) const
{
  return new CQChartsTreeMapNodeObj(this, node, hierObj, rect, is);
}

//------

CQChartsTreeMapHierObj::
CQChartsTreeMapHierObj(const CQChartsTreeMapPlot *plot, CQChartsTreeMapHierNode *hier,
                       CQChartsTreeMapHierObj *hierObj, const CQChartsGeom::BBox &rect,
                       const ColorInd &is) :
 CQChartsTreeMapNodeObj(plot, hier, hierObj, rect, is), hier_(hier)
{
  setModelInd(hier_->ind());
}

QString
CQChartsTreeMapHierObj::
calcId() const
{
  //return QString("%1:%2").arg(hier_->name()).arg(hier_->hierSize());
  return CQChartsTreeMapNodeObj::calcId();
}

QString
CQChartsTreeMapHierObj::
calcTipId() const
{
  //return QString("%1:%2").arg(hier_->hierName()).arg(hier_->hierSize());
  return CQChartsTreeMapNodeObj::calcTipId();
}

bool
CQChartsTreeMapHierObj::
inside(const CQChartsGeom::Point &p) const
{
  CQChartsGeom::BBox bbox(hier_->x(), hier_->y(), hier_->x() + hier_->w(), hier_->y() + hier_->h());

  if (bbox.inside(p))
    return true;

  return false;
}

void
CQChartsTreeMapHierObj::
getObjSelectIndices(Indices &inds) const
{
  return addColumnSelectIndex(inds, plot_->valueColumn());
}

void
CQChartsTreeMapHierObj::
draw(CQChartsPaintDevice *device)
{
  auto *pnode = node()->parent();

  if (pnode && ! pnode->isHierExpanded())
    return;

  //---

  if (! hierNode()->isExpanded())
    return CQChartsTreeMapNodeObj::draw(device);

  //---

  CQChartsGeom::Point p1(hier_->x()             , hier_->y()             );
  CQChartsGeom::Point p2(hier_->x() + hier_->w(), hier_->y() + hier_->h());

  CQChartsGeom::BBox bbox(p1.x, p2.y, p2.x, p1.y);

  //---

  // calc header stroke and brush
  CQChartsPenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  // draw rectangle
  device->setColorNames();

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  device->drawRect(bbox);

  device->resetColorNames();

  //---

  if (hier_->isShowTitle())
    drawText(device, bbox);
}

void
CQChartsTreeMapHierObj::
drawText(CQChartsPaintDevice *device, const BBox &bbox)
{
  // get label (name)
  QString name = (plot_->isTitleHierName() ? hier_->hierName() : hier_->name());

  //---

  CQChartsPenBrush penBrush;

  calcPenBrush(penBrush, /*updateState*/false);

  plot_->charts()->setContrastColor(penBrush.brush.color());

  //---

  // calc text pen
  CQChartsPenBrush tPenBrush = penBrush;

  QColor tc = plot_->interpHeaderTextColor(ColorInd());

  plot_->setPen(tPenBrush.pen, true, tc, plot_->headerTextAlpha());

  plot_->updateObjPenBrushState(this, tPenBrush);

  //---

  device->save();

  //---

  // set font
  plot_->view()->setPlotPainterFont(plot_, device, plot_->headerTextFont());

  //---

  // check if text visible (font dependent)
  auto pbbox = device->windowToPixel(bbox);

  //bool visible = plot_->isTextVisible();
  bool visible = true;

  if (visible) {
    QFontMetricsF fm(device->font());

    double minTextWidth  = fm.width("X") + 4;
    double minTextHeight = fm.height() + 4;

    visible = (pbbox.getWidth() >= minTextWidth && pbbox.getHeight() >= minTextHeight);
  }

  //---

  // draw label
  if (visible) {
    CQChartsTextOptions textOptions;

    textOptions.angle         = plot_->headerTextAngle();
    textOptions.align         = plot_->headerTextAlign();
    textOptions.contrast      = plot_->isHeaderTextContrast();
    textOptions.contrastAlpha = plot_->headerTextContrastAlpha();
    textOptions.formatted     = plot_->isHeaderTextFormatted();
    textOptions.scaled        = plot_->isHeaderTextScaled();
    textOptions.html          = plot_->isHeaderTextHtml();
    textOptions.clipped       = plot_->isTitleTextClipped();

    textOptions = plot_->adjustTextOptions(textOptions);

    device->setPen(tPenBrush.pen);

    double m = plot_->titleMargin(); // margin in pixels

    double hh = plot_->calcTitleHeight(); // title height in pixels

    CQChartsGeom::BBox pbbox1(pbbox.getXMin() + m, pbbox.getYMin(),
                              pbbox.getXMax() - m, pbbox.getYMin() + hh);

    CQChartsDrawUtil::drawTextInBox(device, device->pixelToWindow(pbbox1), name, textOptions);
  }

  //---

  plot_->charts()->resetContrastColor();

  device->restore();
}

void
CQChartsTreeMapHierObj::
calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const
{
  ColorInd colorInd = calcColorInd();

  QColor bc = plot_->interpHeaderStrokeColor(colorInd);

  if (isChildSelected())
    bc.setAlphaF(1.0);

  QColor hierColor = hier_->interpColor(plot_, plot_->fillColor(), colorInd, plot_->numColorIds());

  QColor fc;

  if (hierNode()->isExpanded()) {
    QColor c = plot_->interpHeaderFillColor(colorInd);

    fc = CQChartsUtil::blendColors(c, hierColor, 0.8);
  }
  else
    fc = hierColor;

  plot_->setPenBrush(penBrush,
    CQChartsPenData  (plot_->isHeaderStroked(), bc, plot_->headerStrokeAlpha(),
                      plot_->headerStrokeWidth(), plot_->headerStrokeDash()),
    CQChartsBrushData(plot_->isHeaderFilled(), fc, plot_->headerFillAlpha(),
                      plot_->headerFillPattern()));

  if (updateState)
    plot_->updateObjPenBrushState(this, penBrush);
}

void
CQChartsTreeMapHierObj::
writeScriptData(CQChartsScriptPaintDevice *device) const
{
  calcPenBrush(penBrush_, /*updateState*/ false);

  writeObjScriptData(device);

  std::ostream &os = device->os();

  os << "\n";
  os << "  this.name = \"" << node_->name().toStdString() << "\";\n";
  os << "  this.size = " << node_->hierSize() << ";\n";
}

//------

CQChartsTreeMapNodeObj::
CQChartsTreeMapNodeObj(const CQChartsTreeMapPlot *plot, CQChartsTreeMapNode *node,
                       CQChartsTreeMapHierObj *hierObj, const CQChartsGeom::BBox &rect,
                       const ColorInd &is) :
 CQChartsPlotObj(const_cast<CQChartsTreeMapPlot *>(plot), rect, is, ColorInd(), ColorInd()),
 plot_(plot), node_(node), hierObj_(hierObj)
{
  setDetailHint(DetailHint::MAJOR);

  setModelInd(node_->ind());
}

QString
CQChartsTreeMapNodeObj::
calcId() const
{
//if (node_->isFiller())
//  return hierObj_->calcId();

  QModelIndex ind1 = plot_->unnormalizeIndex(node_->ind());

  QString idStr;

  if (calcColumnId(ind1, idStr))
    return idStr;

  return QString("%1:%2:%3").arg(typeName()).
    arg(node_->isFiller() ? node_->parent()->name() : node_->name()).arg(node_->hierSize());
}

QString
CQChartsTreeMapNodeObj::
calcTipId() const
{
//if (node_->isFiller())
//  return hierObj_->calcTipId();

  CQChartsTableTip tableTip;

  if (plot_->idColumn().isValid())
    tableTip.addTableRow("Name", (node_->isFiller() ?
      node_->parent()->name() : node_->name()));
  else
    tableTip.addTableRow("Name", (node_->isFiller() ?
      node_->parent()->hierName() : node_->hierName()));

  tableTip.addTableRow("Size", node_->hierSize());

  if (plot_->colorColumn().isValid()) {
    QModelIndex ind1 =
      plot_->unnormalizeIndex(node_->isFiller() ? node_->parent()->ind() : node_->ind());

    CQChartsModelIndex colorInd1(ind1.row(), plot_->colorColumn(), ind1.parent());

    bool ok;

    QString colorStr = plot_->modelString(colorInd1, ok);

    tableTip.addTableRow("Color", colorStr);
  }

  //---

  plot()->addTipColumns(tableTip, node_->ind());

  //---

  return tableTip.str();
}

bool
CQChartsTreeMapNodeObj::
inside(const CQChartsGeom::Point &p) const
{
  CQChartsGeom::BBox bbox(node_->x(), node_->y(), node_->x() + node_->w(), node_->y() + node_->h());

  if (bbox.inside(p))
    return true;

  return false;
}

void
CQChartsTreeMapNodeObj::
getObjSelectIndices(Indices &inds) const
{
  for (const auto &c : plot_->nameColumns())
    addColumnSelectIndex(inds, c);

  addColumnSelectIndex(inds, plot_->valueColumn());
}

void
CQChartsTreeMapNodeObj::
draw(CQChartsPaintDevice *device)
{
  auto *pnode = node()->parent();

  if (pnode && ! pnode->isHierExpanded())
    return;

  //---

  CQChartsGeom::Point p1 =
    plot_->windowToPixel(CQChartsGeom::Point(node_->x()             , node_->y()             ));
  CQChartsGeom::Point p2 =
    plot_->windowToPixel(CQChartsGeom::Point(node_->x() + node_->w(), node_->y() + node_->h()));

  bool isPoint = this->isPoint();

  CQChartsGeom::BBox  bbox;
  CQChartsGeom::Point point;

  if (isPoint)
    point = CQChartsGeom::Point(CMathUtil::avg(p1.x, p2.x), CMathUtil::avg(p1.y, p2.y));
  else
    bbox = CQChartsGeom::BBox(p1.x + 1, p2.y + 1, p2.x - 1, p1.y - 1);

  //---

  // calc stroke and brush
  CQChartsPenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, isPoint, updateState);

  //---

  // draw rectangle
  device->setColorNames();

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  if (isPoint)
    device->drawPoint(device->pixelToWindow(point));
  else
    device->drawRect(device->pixelToWindow(bbox));

  device->resetColorNames();

  //---

  if (isPoint)
    return;

  //---

  if (plot_->isTextVisible())
    drawText(device, bbox);
}

void
CQChartsTreeMapNodeObj::
drawText(CQChartsPaintDevice *device, const CQChartsGeom::BBox &bbox)
{
  // get labels (name and optional size)
  QStringList strs;

  QString name;

  if (plot_->isHierName()) {
    QChar sep = '/';

    name = (! node_->isFiller() ? node_->hierName(sep) : node_->parent()->hierName(sep));

    int nh = plot_->numSkipHier();

    if (nh > 0) {
      QStringList strs = name.split(sep);

      int ns = strs.size();

      QStringList strs1;

      for (int i = nh; i < ns; ++i)
        strs1 += strs[i];

      name = strs1.join(sep);
    }
  }
  else
    name = (! node_->isFiller() ? node_->name() : node_->parent()->name());

  strs.push_back(name);

  if (plot_->isValueLabel())
    strs.push_back(QString("%1").arg(node_->size()));

  //---

  CQChartsPenBrush penBrush;

  calcPenBrush(penBrush, /*isPoint*/false, /*updateState*/false);

  plot_->charts()->setContrastColor(penBrush.brush.color());

  //---

  // calc text pen
  ColorInd colorInd = calcColorInd();

  CQChartsPenBrush tPenBrush;

  QColor tc = plot_->interpTextColor(colorInd);

  plot_->setPen(tPenBrush.pen, true, tc, plot_->textAlpha());

  plot_->updateObjPenBrushState(this, tPenBrush);

  //---

  device->save();

  //---

  // set font
  plot_->view()->setPlotPainterFont(plot_, device, plot_->textFont());

  //---

  // check if text visible
  QFontMetricsF fm(device->font());

  double minTextWidth  = fm.width("X") + 4;
  double minTextHeight = fm.height() + 4;

  bool visible = (bbox.getWidth() >= minTextWidth && bbox.getHeight() >= minTextHeight);

  //---

  // draw label
  if (visible) {
    CQChartsTextOptions textOptions;

    textOptions.angle         = plot_->textAngle();
    textOptions.align         = plot_->textAlign();
    textOptions.contrast      = plot_->isTextContrast();
    textOptions.contrastAlpha = plot_->textContrastAlpha();
    textOptions.formatted     = plot_->isTextFormatted();
    textOptions.formatSeps    = plot_->separator();
    textOptions.scaled        = plot_->isTextScaled();
    textOptions.html          = plot_->isTextHtml();
    textOptions.clipped       = plot_->isTextClipped();

    textOptions = plot_->adjustTextOptions(textOptions);

    device->setPen(tPenBrush.pen);

    auto ibbox = bbox.adjusted(3, 3, -3, -3);

    auto bbox1 = device->pixelToWindow(ibbox);

    if (! plot_->isValueLabel()) {
      CQChartsDrawUtil::drawTextInBox(device, bbox1, name, textOptions);
    }
    else {
      if (plot_->isTextClipped())
        device->setClipRect(bbox1);

      if (textOptions.formatted) {
        QString str1 = strs.join('\n');

        CQChartsDrawUtil::drawTextInBox(device, bbox1, str1, textOptions);
      }
      else
        CQChartsDrawUtil::drawStringsInBox(device, bbox1, strs, textOptions);

#if 0
      double th = fm.height();

      auto pc = ibbox.getCenter();

      CQChartsGeom::Point tp1(pc.x, pc.y - th/2);
      CQChartsGeom::Point tp2(pc.x, pc.y + th/2);

      CQChartsDrawUtil::drawTextAtPoint(device, device->pixelToWindow(tp1), strs[0], textOptions);
      CQChartsDrawUtil::drawTextAtPoint(device, device->pixelToWindow(tp2), strs[1], textOptions);
#endif
    }
  }

  //---

  plot_->charts()->resetContrastColor();

  device->restore();
}

void
CQChartsTreeMapNodeObj::
calcPenBrush(CQChartsPenBrush &penBrush, bool isPoint, bool updateState) const
{
  ColorInd colorInd = calcColorInd();

  QColor bc = plot_->interpStrokeColor(colorInd);
  QColor fc = node_->interpColor(plot_, plot_->fillColor(), colorInd, plot_->numColorIds());

  if (isPoint) {
    if      (plot_->isFilled())
      plot_->setPenBrush(penBrush,
        CQChartsPenData  (true, fc, plot_->fillAlpha()),
        CQChartsBrushData(true, fc, plot_->fillAlpha(), plot_->fillPattern()));
    else if (plot_->isStroked())
      plot_->setPenBrush(penBrush,
        CQChartsPenData  (true, bc, plot_->strokeAlpha(),
                          plot_->strokeWidth(), plot_->strokeDash()),
        CQChartsBrushData(true, bc, plot_->strokeAlpha()));
  }
  else {
    plot_->setPenBrush(penBrush,
      CQChartsPenData  (plot_->isStroked(), bc, plot_->strokeAlpha(),
                        plot_->strokeWidth(), plot_->strokeDash()),
      CQChartsBrushData(plot_->isFilled(), fc, plot_->fillAlpha(), plot_->fillPattern()));
  }

  if (updateState)
    plot_->updateObjPenBrushState(this, penBrush);
}

bool
CQChartsTreeMapNodeObj::
isPoint() const
{
  CQChartsGeom::Point p1 =
    plot_->windowToPixel(CQChartsGeom::Point(node_->x()             , node_->y()             ));
  CQChartsGeom::Point p2 =
    plot_->windowToPixel(CQChartsGeom::Point(node_->x() + node_->w(), node_->y() + node_->h()));

  double pw = std::abs(p2.x - p1.x) - 2;
  double ph = std::abs(p2.y - p1.y) - 2;

  return (pw <= 1.5 || ph <= 1.5);
}

void
CQChartsTreeMapNodeObj::
writeScriptData(CQChartsScriptPaintDevice *device) const
{
  bool isPoint = this->isPoint();

  calcPenBrush(penBrush_, isPoint, /*updateState*/ false);

  CQChartsPlotObj::writeScriptData(device);

  std::ostream &os = device->os();

  os << "\n";
  os << "  this.name = \"" << node_->name().toStdString() << "\";\n";
  os << "  this.size = " << node_->hierSize() << ";\n";
}

bool
CQChartsTreeMapNodeObj::
isChildSelected() const
{
  for (const auto &child : children_) {
    if (child->isSelected())
      return true;
  }

  for (const auto &child : children_) {
    if (child->isChildSelected())
      return true;
  }

  return false;
}

//------

CQChartsTreeMapHierNode::
CQChartsTreeMapHierNode(const CQChartsTreeMapPlot *plot, CQChartsTreeMapHierNode *parent,
                        const QString &name, const QModelIndex &ind) :
 CQChartsTreeMapNode(plot, parent, name, 0.0, ind)
{
  if (parent_)
    parent_->addChild(this);
}

CQChartsTreeMapHierNode::
~CQChartsTreeMapHierNode()
{
  for (auto &child : children_)
    delete child;

  for (auto &node : nodes_)
    delete node;
}

bool
CQChartsTreeMapHierNode::
isHierExpanded() const
{
  if (plot_->currentRoot() == this)
    return true;

  if (! isExpanded())
    return false;

  if (parent() && ! parent()->isHierExpanded())
    return false;

  return true;
}

double
CQChartsTreeMapHierNode::
hierSize() const
{
  double s = size();

  for (auto &child : children_)
    s += child->hierSize();

  for (auto &node : nodes_)
    s += node->hierSize();

  return s;
}

void
CQChartsTreeMapHierNode::
addChild(CQChartsTreeMapHierNode *child)
{
  children_.push_back(child);
}

void
CQChartsTreeMapHierNode::
removeChild(CQChartsTreeMapHierNode *child)
{
  int nc = children_.size();

  int i = 0;

  for ( ; i < nc; ++i)
    if (children_[i] == child)
      break;

  assert(i < nc);

  delete children_[i];

  ++i;

  for ( ; i < nc; ++i)
    children_[i - 1] = children_[i];

  children_.pop_back();
}

void
CQChartsTreeMapHierNode::
packNodes(double x, double y, double w, double h)
{
  double whh = plot()->windowHeaderHeight();
  double wmw = plot()->windowMarginWidth();

  //---

  // check if title bar should be shown
  showTitle_ = plot()->isTitles();

  if (plot()->isTitleAutoHide()) {
    double maxExtent = 0.2;

    if (plot()->titleMaxExtent().isSet())
      maxExtent = CMathUtil::clamp(*plot()->titleMaxExtent().value(), 0.0, 1.0);

    if (whh > h*maxExtent)
      showTitle_ = false;
  }

  if (! parent())
    showTitle_ = false;

  if (plot()->titleDepth() > 0 && depth() > plot()->titleDepth())
    showTitle_ = false;

  //---

  double dh = (showTitle_ ? whh : 0.0);
  double m  = (w > wmw ? wmw : 0.0);

  // make single list of nodes to pack
  Nodes nodes;

  for (const auto &child : children_)
    nodes.push_back(child);

  for (const auto &node : nodes_)
    nodes.push_back(node);

  // sort nodes by size (largest to smallest)
  std::sort(nodes.begin(), nodes.end(), CQChartsTreeMapNodeCmp());

  //std::cerr << name() << "\n";
  //for (uint i = 0; i < nodes.size(); ++i)
  //  std::cerr << " " << nodes[i]->name() << ":" << nodes[i]->hierSize() << "\n";

  packSubNodes(x + m/2, y + m/2, w - m, h - dh - m, nodes);
}

void
CQChartsTreeMapHierNode::
packSubNodes(double x, double y, double w, double h, const Nodes &nodes)
{
  // place nodes
  int n = nodes.size();
  if (n == 0) return;

  if (n >= 2) {
    Nodes  nodes1, nodes2;
    double size1 = 0.0, size2 = 0.0;

#if 1
    double size12 = 0.0;

    for (int i = 0; i < n; ++i)
      size12 += nodes[i]->hierSize();

    double hsize = size12/2;

    int i = 0;

    for ( ; i < n - 1; ++i) {
      if (size1 > hsize)
        break;

      size1 += nodes[i]->hierSize();

      nodes1.push_back(nodes[i]);
    }

    for ( ; i < n; ++i) {
      size2 += nodes[i]->hierSize();

      nodes2.push_back(nodes[i]);
    }
#else
    int n1 = n/2;

    for (int i = 0; i < n1; ++i) {
      size1 += nodes[i]->hierSize();

      nodes1.push_back(nodes[i]);
    }

    for (int i = n1; i <  n; ++i) {
      size2 += nodes[i]->hierSize();

      nodes2.push_back(nodes[i]);
    }

    double size12 = size1 + size2;
#endif

    // split area = (w*h) if largest direction
    // e.g. split at w1. area1 = w1*h; area2 = (w - w1)*h;
    // area1/area2 = w1/(w - w1) = size1/size2;
    // w1*size2 = w*size1 - w1*size1;
    // w1 = (w*size1)/(size1 + size2);

    if (size12 == 0.0)
      return;

    assert(nodes1.size() > 0 && nodes2.size() > 0);

    double f = size1/size12;

    if (w >= h) {
      double w1 = f*w;

      packSubNodes(x     , y,     w1, h, nodes1);
      packSubNodes(x + w1, y, w - w1, h, nodes2);
    }
    else {
      double h1 = f*h;

      packSubNodes(x, y     , w, h1    , nodes1);
      packSubNodes(x, y + h1, w, h - h1, nodes2);
    }
  }
  else {
    auto *node = nodes[0];

    node->setPosition(x, y, w, h);
  }
}

void
CQChartsTreeMapHierNode::
setPosition(double x, double y, double w, double h)
{
  CQChartsTreeMapNode::setPosition(x, y, w, h);

  packNodes(x, y, w, h);
}

void
CQChartsTreeMapHierNode::
addNode(CQChartsTreeMapNode *node)
{
  nodes_.push_back(node);
}

void
CQChartsTreeMapHierNode::
removeNode(CQChartsTreeMapNode *node)
{
  int n = nodes_.size();

  int i = 0;

  for ( ; i < n; ++i) {
    if (nodes_[i] == node)
      break;
  }

  assert(i < n);

  ++i;

  for ( ; i < n; ++i)
    nodes_[i - 1] = nodes_[i];

  nodes_.pop_back();
}

QColor
CQChartsTreeMapHierNode::
interpColor(const CQChartsTreeMapPlot *plot, const CQChartsColor &c,
            const ColorInd &colorInd, int n) const
{
  using Colors = std::vector<QColor>;

  Colors colors;

  for (auto &child : children_)
    colors.push_back(child->interpColor(plot, c, colorInd, n));

  for (auto &node : nodes_)
    colors.push_back(node->interpColor(plot, c, colorInd, n));

  if (colors.empty())
    return plot->interpColor(c, colorInd);

  return CQChartsUtil::blendColors(colors);
}

//------

CQChartsTreeMapNode::
CQChartsTreeMapNode(const CQChartsTreeMapPlot *plot, CQChartsTreeMapHierNode *parent,
                    const QString &name, double size, const QModelIndex &ind) :
 plot_(plot), parent_(parent), id_(nextId()), name_(name), size_(size), ind_(ind)
{
}

QString
CQChartsTreeMapNode::
hierName(QChar sep) const
{
  QString hname;

  if (parent() && parent() != plot_->root())
    hname = parent()->hierName(sep) + sep + name();
  else
    hname = name();

  return hname;
}

void
CQChartsTreeMapNode::
setPosition(double x, double y, double w, double h)
{
  assert(! CMathUtil::isNaN(x) && ! CMathUtil::isNaN(y) &&
         ! CMathUtil::isNaN(w) && ! CMathUtil::isNaN(h));

  x_ = x; y_ = y;
  w_ = w; h_ = h;

  //std::cerr << "Node: " << name() << " @ ( " << x_ << "," << y_ << ")" <<
  //             " [" << w_ << "," << h_ << "]" << "\n";

  placed_ = true;
}

bool
CQChartsTreeMapNode::
contains(double x, double y) const
{
  return (x >= x_ && x <= (x_ + w_) && y >= y_ && y <= (y_ + h_));
}

CQChartsTreeMapHierNode *
CQChartsTreeMapNode::
rootNode(CQChartsTreeMapHierNode *root) const
{
  auto *parent = this->parent();

  while (parent && parent->parent() && parent->parent() != root)
    parent = parent->parent();

  return parent;
}

QColor
CQChartsTreeMapNode::
interpColor(const CQChartsTreeMapPlot *plot, const CQChartsColor &c,
            const ColorInd &colorInd, int n) const
{
  if      (color().isValid())
    return plot->interpColor(color(), ColorInd());
  else if (colorId() >= 0 && plot_->isColorById())
    return plot->interpColor(c, ColorInd(colorId(), n));
  else
    return plot->interpColor(c, colorInd);
}
