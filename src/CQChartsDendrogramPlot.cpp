#include <CQChartsDendrogramPlot.h>
#include <CQChartsView.h>
#include <CQChartsModelUtil.h>
#include <CQChartsDrawUtil.h>
#include <CQCharts.h>
#include <CQChartsViewPlotPaintDevice.h>
#include <CQChartsHtml.h>
#include <CQChartsTip.h>

#include <CQPropertyViewItem.h>
#include <CQPerfMonitor.h>

#include <CBuchHeim.h>

namespace {

class Tree : public CBuchHeim::Tree {
 public:
  using Node = CQChartsDendrogram::Node;

 public:
  Tree(Node *node) :
   CBuchHeim::Tree(node->name().toStdString()), node_(node) {
  }

  Node *node() const { return node_; }

 private:
  Node *node_ { nullptr };
};

class PlotDendrogram : public CQChartsDendrogram {
 public:
  using ModelIndex = CQChartsModelIndex;
  using OptReal    = CQChartsOptReal;

 public:
  class PlotNode : public CQChartsDendrogram::Node {
   public:
    PlotNode(CQChartsDendrogram::HierNode *parent, const QString &name="", double size=1.0) :
     CQChartsDendrogram::Node(parent, name, size) {
    }

    const ModelIndex &modelInd() const { return modelInd_; }
    void setModelInd(const ModelIndex &v) { modelInd_ = v; }

    const OptReal &colorValue() const { return colorValue_; }
    void setColorValue(const OptReal &r) { colorValue_ = r; }

    const OptReal &sizeValue() const { return sizeValue_; }
    void setSizeValue(const OptReal &v) { sizeValue_ = v; }

   private:
    ModelIndex modelInd_;
    OptReal    colorValue_;
    OptReal    sizeValue_;
  };

 public:
  PlotDendrogram() :
   CQChartsDendrogram() {
  }

  Node *createNode(CQChartsDendrogram::HierNode *hier, const QString &name,
                   double size) const override {
    return new PlotNode(hier, name, size);
  }
};

}

//---

CQChartsDendrogramPlotType::
CQChartsDendrogramPlotType()
{
}

void
CQChartsDendrogramPlotType::
addParameters()
{
  startParameterGroup("Dendrogram");

  addColumnParameter("name" , "name" , "nameColumn").
   setStringColumn().setRequired().setPropPath("columns.name"  ).setTip("Name column");
  addColumnParameter("value", "Value", "valueColumn").
   setNumericColumn().setRequired().setPropPath("columns.value").setTip("Value column");
  addColumnParameter("size" , "Size" , "sizeColumn").
   setNumericColumn().setRequired().setPropPath("columns.size" ).setTip("Size column");

  endParameterGroup();

  //---

  // options
  addEnumParameter("orientation", "Orientation", "orientation").
    addNameValue("HORIZONTAL", int(Qt::Horizontal)).
    addNameValue("VERTICAL"  , int(Qt::Vertical  )).
    setTip("Draw orientation");

  //---

  CQChartsPlotType::addParameters();
}

QString
CQChartsDendrogramPlotType::
description() const
{
  auto IMG = [](const QString &src) { return CQChartsHtml::Str::img(src); };

  return CQChartsHtml().
   h2("Dendrogram Plot").
    h3("Summary").
     p("Draw hierarchical data using collapsible tree.").
    h3("Limitations").
     p("None.").
    h3("Example").
     p(IMG("images/dendogram.png"));
}

CQChartsPlot *
CQChartsDendrogramPlotType::
create(View *view, const ModelP &model) const
{
  return new CQChartsDendrogramPlot(view, model);
}

//------

CQChartsDendrogramPlot::
CQChartsDendrogramPlot(View *view, const ModelP &model) :
 CQChartsPlot(view, view->charts()->plotType("dendrogram"), model),
 CQChartsObjNodeShapeData<CQChartsDendrogramPlot>(this),
 CQChartsObjEdgeLineData <CQChartsDendrogramPlot>(this),
 CQChartsObjTextData     <CQChartsDendrogramPlot>(this)
{
}

CQChartsDendrogramPlot::
~CQChartsDendrogramPlot()
{
  term();
}

//---

void
CQChartsDendrogramPlot::
init()
{
  CQChartsPlot::init();

  //---

  NoUpdate noUpdate(this);

  setNodeFillColor(Color(Color::Type::PALETTE));

  //---

  addTitle();
}

void
CQChartsDendrogramPlot::
term()
{
  delete dendrogram_;
}

//---

void
CQChartsDendrogramPlot::
addProperties()
{
  addBaseProperties();

  // columns
  addProp("columns", "nameColumn" , "name" , "Name column" );
  addProp("columns", "valueColumn", "value", "Value column");
  addProp("columns", "sizeColumn" , "size" , "Size column" );

  // node
  addProp("node", "circleSize", "circleSize", "Circle size in pixels")->setMinValue(1.0);

  addFillProperties("node/fill"  , "nodeFill"  , "Node");
  addLineProperties("node/stroke", "nodeStroke", "Node");
  addLineProperties("edge/stroke", "edgeLines" , "Edge");

  // label
//addProp("label/text", "textVisible", "visible", "Labels visible");

  addTextProperties("label/text", "text", "", CQChartsTextOptions::ValueType::CONTRAST |
                    CQChartsTextOptions::ValueType::CLIP_LENGTH |
                    CQChartsTextOptions::ValueType::CLIP_ELIDE);

  addProp("label", "textMargin", "margin", "Text margin in pixels")->setMinValue(1.0);

  addProp("options", "placeType"  , "placeType"  , "Place type");
  addProp("options", "orientation", "orientation", "Draw orientation");
}

//---

void
CQChartsDendrogramPlot::
setNameColumn(const Column &c)
{
  CQChartsUtil::testAndSet(nameColumn_, c, [&]() {
    needsReload_ = true; updateRangeAndObjs(); emit customDataChanged();
  } );
}

void
CQChartsDendrogramPlot::
setValueColumn(const Column &c)
{
  CQChartsUtil::testAndSet(valueColumn_, c, [&]() {
    needsReload_ = true; updateRangeAndObjs(); emit customDataChanged();
  } );
}

void
CQChartsDendrogramPlot::
setSizeColumn(const Column &c)
{
  CQChartsUtil::testAndSet(sizeColumn_, c, [&]() {
    needsReload_ = true; updateRangeAndObjs(); emit customDataChanged();
  } );
}

//---

CQChartsColumn
CQChartsDendrogramPlot::
getNamedColumn(const QString &name) const
{
  Column c;
  if      (name == "name" ) c = this->nameColumn();
  else if (name == "value") c = this->valueColumn();
  else if (name == "size" ) c = this->sizeColumn();
  else                      c = CQChartsPlot::getNamedColumn(name);

  return c;
}

void
CQChartsDendrogramPlot::
setNamedColumn(const QString &name, const Column &c)
{
  if      (name == "name" ) this->setNameColumn(c);
  else if (name == "value") this->setValueColumn(c);
  else if (name == "size" ) this->setSizeColumn(c);
  else                      CQChartsPlot::setNamedColumn(name, c);
}

//---

void
CQChartsDendrogramPlot::
setCircleSize(double r)
{
  CQChartsUtil::testAndSet(circleSize_, r, [&]() {
    needsPlace_ = true; updateRangeAndObjs();
  } );
}

void
CQChartsDendrogramPlot::
setTextMargin(double r)
{
  CQChartsUtil::testAndSet(textMargin_, r, [&]() {
    drawObjs();
  } );
}

void
CQChartsDendrogramPlot::
setOrientation(const Qt::Orientation &orient)
{
  CQChartsUtil::testAndSet(orientation_, orient, [&]() {
    needsPlace_ = true; updateRangeAndObjs();
  } );
}

void
CQChartsDendrogramPlot::
setPlaceType(const PlaceType &t)
{
  CQChartsUtil::testAndSet(placeType_, t, [&]() {
    needsPlace_ = true; updateRangeAndObjs();
  } );
}

//---

CQChartsGeom::Range
CQChartsDendrogramPlot::
calcRange() const
{
  CQPerfTrace trace("CQChartsDendrogramPlot::calcRange");

  NoUpdate noUpdate(this);

  auto *th = const_cast<CQChartsDendrogramPlot *>(this);

  th->clearErrors();

  //---

  // check columns
  bool columnsValid = true;

  if (! checkColumn       (nameColumn (), "Name" )) columnsValid = false;
  if (! checkNumericColumn(valueColumn(), "Value")) columnsValid = false;
  if (! checkNumericColumn(sizeColumn (), "Size" )) columnsValid = false;

  if (! columnsValid)
    return Range(0.0, 0.0, 1.0, 1.0);

  //---

  Range dataRange;

  dataRange.updateRange(0, 0);
  dataRange.updateRange(1, 1);

  //---

  return dataRange;
}

void
CQChartsDendrogramPlot::
placeModel() const
{
  auto *th = const_cast<CQChartsDendrogramPlot *>(this);

  for (const auto &plotObj : plotObjects()) {
    auto *nodeObj = dynamic_cast<NodeObj *>(plotObj);

    if (nodeObj)
      nodeObj->resetNode();
  }

  delete th->dendrogram_;

  th->dendrogram_ = new PlotDendrogram;

  //---

  // add name values
  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(const CQChartsDendrogramPlot *plot) :
     plot_(plot) {
    }

    State visit(const QAbstractItemModel *model, const VisitData &data) override {
      auto path = CQChartsModelUtil::parentPath(model, data.parent);

      //---

      // get name
      ModelIndex nameModelInd(plot_, data.row, plot_->nameColumn(), data.parent);

    //auto nameInd  = modelIndex(nameModelInd);
    //auto nameInd1 = normalizeIndex(nameInd);

      bool ok1;
      auto name = plot_->modelString(nameModelInd, ok1);

      if (ok1 && path.length())
        name = path + "/" + name;

      //--

      // get value
      ModelIndex valueModelInd(plot_, data.row, plot_->valueColumn(), data.parent);

      bool ok2;
      double value = plot_->modelReal(valueModelInd, ok2);

      if (! ok2) {
        if (! plot_->isSkipBad())
          return addDataError(valueModelInd, "Invalid Value");

        value = 0.0;
      }

      if (CMathUtil::isNaN(value))
        return State::SKIP;

      //---

      // get color
      OptReal colorValue;

      if (plot_->colorColumn().isValid()) {
        ModelIndex colorModelInd(plot_, data.row, plot_->colorColumn(), data.parent);

        bool ok3;
        double color = plot_->modelReal(colorModelInd, ok3);

        if (! ok3) {
          if (! plot_->isSkipBad())
            return addDataError(valueModelInd, "Invalid Color");

          color = 0.0;
        }

        if (! CMathUtil::isNaN(color))
          colorValue = OptReal(color);
      }

      //---

      // get size
      OptReal sizeValue;

      if (plot_->sizeColumn().isValid()) {
        ModelIndex sizeModelInd(plot_, data.row, plot_->sizeColumn(), data.parent);

        bool ok3;
        double size = plot_->modelReal(sizeModelInd, ok3);

        if (! ok3) {
          if (! plot_->isSkipBad())
            return addDataError(valueModelInd, "Invalid Size");

          size = 0.0;
        }

        if (! CMathUtil::isNaN(size))
          sizeValue = OptReal(size);
      }

      //---

      plot_->addNameValue(name, value, nameModelInd, colorValue, sizeValue);

      //--

      return State::OK;
    }

   private:
    State addDataError(const ModelIndex &ind, const QString &msg) const {
      const_cast<CQChartsDendrogramPlot *>(plot_)->addDataError(ind , msg);
      return State::SKIP;
    }

   private:
    const CQChartsDendrogramPlot* plot_ { nullptr };
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  //---

  auto *root = dendrogram_->root();

  if (root)
    root->setOpen(true);

  //---

  place();
}

void
CQChartsDendrogramPlot::
place() const
{
  if (placeType() == PlaceType::BUCHHEIM) {
    delete buchheimTree_;
    delete buchheimDrawTree_;

    buchheimTree_ = new Tree(dendrogram_->root());

    addBuchheimHierNode(buchheimTree_, dendrogram_->root());

    buchheimDrawTree_ = new CBuchHeim::DrawTree(buchheimTree_);

    buchheimDrawTree_->place();

    buchheimDrawTree_->normalize(/*equalScale*/false);

    //---

    dendrogram_->root()->resetPlaced();

    moveBuchheimHierNode(buchheimDrawTree_);
  }
  else {
    dendrogram_->placeNodes();
  }
}

void
CQChartsDendrogramPlot::
addBuchheimHierNode(CBuchHeim::Tree *tree, HierNode *hierNode) const
{
  if (! hierNode->isOpen())
    return;

  for (auto &hierNode : hierNode->getChildren()) {
    auto *childTree = new Tree(hierNode);

    tree->addChild(CBuchHeim::TreeP(childTree));

    addBuchheimHierNode(childTree, hierNode);
  }

  for (auto &node : hierNode->getNodes()) {
    auto *childTree = new Tree(node);

    tree->addChild(CBuchHeim::TreeP(childTree));
  }
}

void
CQChartsDendrogramPlot::
moveBuchheimHierNode(CBuchHeim::DrawTree *drawTree) const
{
  auto *tree = static_cast<Tree *>(drawTree->tree());

  auto *node1 = tree->node();

  double x1 = drawTree->x1();
  double y1 = drawTree->y1();
  double x2 = drawTree->x2();
  double y2 = drawTree->y2();

  double xc = (x1 + x2)/2.0;
  double yc = (y1 + y2)/2.0;
  double r  = 0.4*std::min(x2 - x1, y2 - y1);

  if (orientation() == Qt::Horizontal)
    node1->setBBox(BBox(yc - r, xc - r, yc + r, xc + r));
  else
    node1->setBBox(BBox(xc - r, 1.0 - (yc - r), xc + r, 1.0 - (yc + r)));

  node1->setPlaced(true);

  for (const auto &node : drawTree->children())
    moveBuchheimHierNode(node.get());
}

void
CQChartsDendrogramPlot::
addNameValue(const QString &name, double value, const ModelIndex &modelInd,
             const OptReal &colorValue, OptReal &sizeValue) const
{
  QStringList names;

  auto name1 = name;

  int pos = name1.indexOf('/');

  if (pos != -1) {
    while (pos != -1) {
      auto lhs = name1.mid(0, pos);
      auto rhs = name1.mid(pos + 1);

      names.push_back(lhs);

      name1 = rhs;

      pos = name1.indexOf('/');
    }
  }
  else {
    names.push_back(name1);
  }

  //---

  // create nodes
  HierNode *hierNode = nullptr;

  for (const auto &n : names) {
    if (! hierNode) {
      if (! dendrogram_->root()) {
        hierNode = dendrogram_->addRootNode(n);

        hierNode->setOpen(false);
      }
      else
        hierNode = dendrogram_->root();
    }
    else {
      auto *hierNode1 = hierNode->findChild(n);

      if (! hierNode1) {
        hierNode = dendrogram_->addHierNode(hierNode, n);

        hierNode->setOpen(false);
      }
      else
        hierNode = hierNode1;
    }
  }

  if (! hierNode) {
    hierNode = dendrogram_->addRootNode(name1);

    hierNode->setOpen(false);
  }
  else {
    auto *node = dendrogram_->addNode(hierNode, name1, value);
    assert(node);

    auto *node1 = dynamic_cast<PlotDendrogram::PlotNode *>(node); assert(node1);

    node1->setModelInd(modelInd);

    if (colorValue.isSet())
      node1->setColorValue(colorValue);

    if (sizeValue.isSet())
      node1->setSizeValue(sizeValue);
  }
}

CQChartsGeom::BBox
CQChartsDendrogramPlot::
calcExtraFitBBox() const
{
  CQPerfTrace trace("CQChartsDendrogramPlot::calcExtraFitBBox");

  BBox bbox;

  for (const auto &plotObj : plotObjs_) {
    auto *nodeObj = dynamic_cast<NodeObj *>(plotObj);

    if (nodeObj)
      bbox += nodeObj->textRect();
  }

  return bbox;
}

//------

bool
CQChartsDendrogramPlot::
createObjs(PlotObjs &objs) const
{
  CQPerfTrace trace("QChartsDendrogramPlot::createObjs");

  NoUpdate noUpdate(this);

  //---

  if (needsReload_) {
    placeModel();

    needsReload_ = false;
  }
  else if (needsPlace_) {
    place();

    needsPlace_ = false;
  }

  //---

  auto *th = const_cast<CQChartsDendrogramPlot *>(this);

  auto *root = dendrogram_->root();

  if (root) {
    th->rootNodeObj_ = addNodeObj(root, objs, /*hier*/true);

    addNodeObjs(root, 0, rootNodeObj_, objs);
  }
  else
    th->rootNodeObj_ = nullptr;

  //---

  // init sum of child values for all hier nodes (including root)
  for (const auto &plotObj : objs) {
    auto *nodeObj = dynamic_cast<NodeObj *>(plotObj);
    if (! nodeObj) continue;

    auto *hierNode = dynamic_cast<const HierNode *>(nodeObj->node());

    if (hierNode) {
      nodeObj->setHierColor(calcHierColor(hierNode));
      nodeObj->setHierSize (calcHierSize (hierNode));
    }
  }

  return true;
}

double
CQChartsDendrogramPlot::
calcHierColor(const HierNode *hierNode) const
{
  double color = 0.0;

  for (const auto *child : hierNode->getChildren())
    color += calcHierColor(child);

  for (const auto *node : hierNode->getNodes()) {
    auto *node1 = dynamic_cast<const PlotDendrogram::PlotNode *>(node); assert(node1);

    color += node1->colorValue().realOr(0.0);
  }

  return color;
}

double
CQChartsDendrogramPlot::
calcHierSize(const HierNode *hierNode) const
{
  double size = 0.0;

  for (const auto *child : hierNode->getChildren())
    size += calcHierSize(child);

  for (const auto *node : hierNode->getNodes()) {
    auto *node1 = dynamic_cast<const PlotDendrogram::PlotNode *>(node); assert(node1);

    size += node1->sizeValue().realOr(0.0);
  }

  return size;
}

void
CQChartsDendrogramPlot::
addNodeObjs(HierNode *hier, int depth, NodeObj *parentObj, PlotObjs &objs) const
{
  for (auto &hierNode : hier->getChildren()) {
    auto *hierNodeObj = addNodeObj(hierNode, objs, /*hier*/true);
    if (! hierNodeObj) continue;

    if (parentObj) {
      hierNodeObj->setParent(parentObj);

      parentObj->addChild(hierNodeObj);
    }

    addNodeObjs(hierNode, depth + 1, hierNodeObj, objs);
  }

  //------

  for (auto &node : hier->getNodes()) {
    auto *nodeObj = addNodeObj(node, objs, /*hier*/false);
    if (! nodeObj) continue;

    if (parentObj) {
      nodeObj->setParent(parentObj);

      parentObj->addChild(nodeObj);
    }
  }
}

CQChartsDendrogramNodeObj *
CQChartsDendrogramPlot::
addNodeObj(Node *node, PlotObjs &objs, bool isHier) const
{
  if (! node->isPlaced())
    return nullptr;

  BBox rect = getBBox(node);

  auto *obj = createNodeObj(node, rect);

  obj->setHier(isHier);

  objs.push_back(obj);

  return obj;
}

CQChartsGeom::BBox
CQChartsDendrogramPlot::
getBBox(Node *node) const
{
  if (placeType() == PlaceType::BUCHHEIM)
    return node->bbox();

  double cs = std::max(circleSize(), 1.0);
//double tm = std::max(textMargin(), 1.0);

  double cw = pixelToWindowWidth (cs);
  double ch = pixelToWindowHeight(cs);

//double mw = pixelToWindowWidth(tm);

//double xc = node->x() + mw;
  double xc = node->x();
  double yc = node->yc();

  return BBox(xc - cw/2.0, yc - ch/2.0, xc + cw/2.0, yc + ch/2.0);
}

//------

bool
CQChartsDendrogramPlot::
handleSelectPress(const Point &p, SelMod /*selMod*/)
{
  Node *node = nullptr;

  PlotObjs plotObjs;

  plotObjsAtPoint(p, plotObjs, Constraints::SELECTABLE);

  if (plotObjs.size() == 1) {
    auto *nodeObj = dynamic_cast<NodeObj *>(plotObjs[0]);

    node = const_cast<Node *>(nodeObj->node());
  }

  if (! node) return false;

  node->setOpen(! node->isOpen());

  needsPlace_ = true;

  updateObjs();

  return true;
}

CQChartsDendrogramNodeObj *
CQChartsDendrogramPlot::
createNodeObj(Node *node, const BBox &rect) const
{
  return new CQChartsDendrogramNodeObj(this, node, rect);
}

//---

CQChartsPlotCustomControls *
CQChartsDendrogramPlot::
createCustomControls()
{
  auto *controls = new CQChartsDendrogramPlotCustomControls(charts());

  controls->init();

  controls->setPlot(this);

  controls->updateWidgets();

  return controls;
}

//------

CQChartsDendrogramNodeObj::
CQChartsDendrogramNodeObj(const CQChartsDendrogramPlot *plot, Node *node, const BBox &rect) :
 CQChartsPlotObj(const_cast<Plot *>(plot), rect), plot_(plot), node_(node), name_(node->name())
{
  setValue(node->size());
  setOpen (node->isOpen());

  auto *node1 = dynamic_cast<PlotDendrogram::PlotNode *>(node);

  if (node1) {
    setModelIndex(node1->modelInd());

    setColor(node1->colorValue());
    setSize (node1->sizeValue());
  }
}

//---

QString
CQChartsDendrogramNodeObj::
calcId() const
{
  return QString("%1:%2").arg(typeName()).arg(name());
}

QString
CQChartsDendrogramNodeObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  plot()->addNoTipColumns(tableTip);

  //---

  tableTip.addTableRow("Name", name());

  if (plot()->valueColumn().isValid())
    tableTip.addTableRow("Value", value());

  if (size().isSet())
    tableTip.addTableRow("Color", color().real());

  if (size().isSet())
    tableTip.addTableRow("Size", size().real());

  //---

  plot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

//---

CQChartsGeom::BBox
CQChartsDendrogramNodeObj::
textRect() const
{
  auto pbbox = plot()->windowToPixel(rect());

  auto pc = pbbox.getCenter();
  auto pr = pbbox.getWidth()/2.0;

  //---

  auto font = plot()->view()->plotFont(plot(), plot()->textFont());

  QFontMetricsF fm(font);

  const auto &name = this->name();

  bool is_hier = this->isHier();

  double dy = (fm.ascent() - fm.descent())/2;

  Point p;

  if (is_hier)
    p = Point(pc.x - pr - fm.width(name), pc.y + dy);
  else
    p = Point(pc.x + pr, pc.y + dy);

  Point p1(p.x                 , p.y - fm.ascent());
  Point p2(p.x + fm.width(name), p.y + fm.ascent());

  BBox ptbbox(p1, p2);

  auto tbbox = plot()->pixelToWindow(ptbbox);

  return tbbox;
}

void
CQChartsDendrogramNodeObj::
draw(PaintDevice *device) const
{
  double tm = std::max(plot()->textMargin(), 1.0);

  auto rect1 = displayRect();

  //---

  // set pen and brush
  ColorInd colorInd;

  PenBrush penBrush;

  auto strokeColor = plot()->interpNodeStrokeColor(colorInd);
  auto fillColor   = plot()->interpNodeFillColor  (colorInd);

  bool is_hier = this->isHier();

  bool closed = (is_hier && ! isOpen());

  bool colored = false;

  if (plot()->colorColumn().isValid()) {
    auto maxColor = (parent() ? parent()->hierColor() : this->color().realOr(0.0));

    double color = 0.0;

    if (isHier())
      color = hierColor();
    else
      color = this->color().real();

    auto c = plot()->colorFromColorMapPaletteValue(color/maxColor);

    fillColor = plot()->interpColor(c, colorInd);
    colored   = true;
  }

  plot()->setPenBrush(penBrush, plot()->nodePenData(strokeColor),
    (closed || colored) ? plot()->nodeBrushData(fillColor) : BrushData(false));

  plot()->updateObjPenBrushState(this, penBrush);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // draw node
  if (colored) {
    if (isHier()) {
      auto f = 1.0/std::sqrt(2.0);

      auto rect2 = rect1.centerScaled(f, f);

      CQChartsDrawUtil::drawRoundedRect(device, rect2, Length(),
                                        Sides(Sides::Side::ALL), Angle::degrees(45));
    }
    else
      device->drawEllipse(rect1);
  }
  else
    device->drawEllipse(rect1);

  //---

  // draw node text
  PenBrush tpenBrush;

  auto tc = plot()->interpTextColor(colorInd);

  plot()->setPen(tpenBrush, PenData(/*stroked*/true, tc, plot()->textAlpha()));

  device->setPen(tpenBrush.pen);

  //---

  plot()->setPainterFont(device, plot()->textFont());

  //---

  const auto &name = this->name();

  QFontMetricsF fm(device->font());

  double dy = (fm.ascent() - fm.descent())/2.0;
  double dx = fm.width(name);

  BBox pbbox = plot()->windowToPixel(rect1);

  Point p;

  if (plot()->orientation() == Qt::Horizontal) {
    if (is_hier)
      p = Point(pbbox.getXMin() - dx - tm, pbbox.getYMid() + dy); // align right
    else
      p = Point(pbbox.getXMax()      + tm, pbbox.getYMid() + dy); // align left
  }
  else {
    if (is_hier)
      p = Point(pbbox.getXMid() - dx/2.0, pbbox.getYMin() - tm + dy); // align top
    else
      p = Point(pbbox.getXMid() - dx/2.0, pbbox.getYMax() + tm + dy); // align bottom
  }

  // only support contrast
  auto textOptions = plot()->textOptions(device);

  textOptions.angle     = Angle();
  textOptions.align     = Qt::AlignLeft;
  textOptions.formatted = false;
  textOptions.scaled    = false;
  textOptions.html      = false;

  CQChartsDrawUtil::drawTextAtPoint(device, plot()->pixelToWindow(p), name, textOptions);

  //---

  for (const auto *child : children_) {
    drawEdge(device, child);
  }
}

void
CQChartsDendrogramNodeObj::
drawEdge(PaintDevice *device, const NodeObj *child) const
{
  auto rect1 = displayRect();
  auto rect2 = child->displayRect();

  auto pbbox1 = plot()->windowToPixel(rect1); // from
  auto pbbox2 = plot()->windowToPixel(rect2); // to

  //---

  PenBrush lPenBrush;

  plot()->setEdgeLineDataPen(lPenBrush.pen, ColorInd(0, 1));

  plot()->setBrush(lPenBrush, BrushData(false));

  CQChartsDrawUtil::setPenBrush(device, lPenBrush);

  //---

  // draw edge
  QPainterPath path;

  double x1, y1, x4, y4;

  if (plot()->orientation() == Qt::Horizontal) {
    x1 = pbbox1.getXMax(); y1 = pbbox1.getYMid();
    x4 = pbbox2.getXMin(); y4 = pbbox2.getYMid();
  }
  else {
    x1 = pbbox1.getXMid(); y1 = pbbox1.getYMax();
    x4 = pbbox2.getXMid(); y4 = pbbox2.getYMin();
  }

  auto p1 = plot()->pixelToWindow(Point(x1, y1));
  auto p4 = plot()->pixelToWindow(Point(x4, y4));

  CQChartsDrawUtil::curvePath(path, p1, p4, plot()->orientation());

  device->drawPath(path);
}

CQChartsGeom::BBox
CQChartsDendrogramNodeObj::
displayRect() const
{
  auto rect1 = rect();

  if (parent() && plot()->sizeColumn().isValid()) {
    auto maxSize = parent()->hierSize();

    double size = 0.0;

    if (isHier())
      size = hierSize();
    else
      size = this->size().real();

    auto symbolSize = CMathUtil::map(size, 0.0, maxSize, 0.0, rect1.getWidth());

    auto f = std::min(symbolSize/rect1.getWidth(), symbolSize/rect1.getHeight());

    rect1 = rect1.centerScaled(f, f);
  }

  return rect1;
}

//------

CQChartsDendrogramPlotCustomControls::
CQChartsDendrogramPlotCustomControls(CQCharts *charts) :
 CQChartsPlotCustomControls(charts, "dendrogram")
{
}

void
CQChartsDendrogramPlotCustomControls::
init()
{
  addWidgets();

  addLayoutStretch();

  connectSlots(true);
}

void
CQChartsDendrogramPlotCustomControls::
addWidgets()
{
  // columns group
  auto columnsFrame = createGroupFrame("Columns", "columnsFrame");

  //---

  addColumnWidgets(QStringList() << "name" << "value" << "color" << "size", columnsFrame);
}

void
CQChartsDendrogramPlotCustomControls::
connectSlots(bool b)
{
  CQChartsPlotCustomControls::connectSlots(b);
}

void
CQChartsDendrogramPlotCustomControls::
setPlot(CQChartsPlot *plot)
{
  plot_ = dynamic_cast<CQChartsDendrogramPlot *>(plot);

  CQChartsPlotCustomControls::setPlot(plot);
}

void
CQChartsDendrogramPlotCustomControls::
updateWidgets()
{
  connectSlots(false);

  //---

  CQChartsPlotCustomControls::updateWidgets();

  //---

  connectSlots(true);
}
