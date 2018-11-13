#include <CQChartsSankeyPlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsUtil.h>
#include <CQCharts.h>
#include <CQChartsModelDetails.h>
#include <CQChartsNamePair.h>
#include <CQPerfMonitor.h>

#include <QPainter>

CQChartsSankeyPlotType::
CQChartsSankeyPlotType()
{
}

void
CQChartsSankeyPlotType::
addParameters()
{
  startParameterGroup("Sankey");

  addColumnParameter("link", "Source/Target", "linkColumn").
    setRequired().setTip("Name pair for Source/Target connection");

  addColumnParameter("value", "Value", "valueColumn").
    setRequired().setTip("Connection value");

  endParameterGroup();

  //---

  CQChartsPlotType::addParameters();
}

QString
CQChartsSankeyPlotType::
description() const
{
  return "<h2>Summary</h2>\n"
         "<p>Draw connected objects as a connected flow graph.</p>\n";
}

bool
CQChartsSankeyPlotType::
isColumnForParameter(CQChartsModelColumnDetails *columnDetails,
                     CQChartsPlotParameter *parameter) const
{
  if (parameter->name() == "link") {
    if (columnDetails->type() == CQChartsPlot::ColumnType::NAME_PAIR)
      return true;

    return false;
  }

  return CQChartsPlotType::isColumnForParameter(columnDetails, parameter);
}

CQChartsPlot *
CQChartsSankeyPlotType::
create(CQChartsView *view, const ModelP &model) const
{
  return new CQChartsSankeyPlot(view, model);
}

//------

CQChartsSankeyPlot::
CQChartsSankeyPlot(CQChartsView *view, const ModelP &model) :
 CQChartsPlot(view, view->charts()->plotType("sankey"), model),
 CQChartsObjTextData     <CQChartsSankeyPlot>(this),
 CQChartsObjNodeShapeData<CQChartsSankeyPlot>(this),
 CQChartsObjEdgeShapeData<CQChartsSankeyPlot>(this)
{
  setLayerActive(CQChartsLayer::Type::FG_PLOT, true);

  CQChartsColor bg(CQChartsColor::Type::PALETTE);

  setNodeFilled(true);
  setNodeFillColor(bg);
  setNodeFillAlpha(1.00);

  setNodeBorder(true);
  setNodeBorderAlpha(0.2);

  setEdgeFilled(true);
  setEdgeFillColor(bg);
  setEdgeFillAlpha(0.25);

  setEdgeBorder(true);
  setEdgeBorderAlpha(0.2);

  //---

  bbox_ = CQChartsGeom::BBox(-1.0, -1.0, 1.0, 1.0);
}

CQChartsSankeyPlot::
~CQChartsSankeyPlot()
{
  for (const auto &nameNode : nameNodeMap_)
    delete nameNode.second;

  for (const auto &edge : edges_)
    delete edge;
}

//---

void
CQChartsSankeyPlot::
addProperties()
{
  CQChartsPlot::addProperties();

  addProperty("columns", this, "linkColumn" , "link" );
  addProperty("columns", this, "valueColumn", "value");

  addProperty("options", this, "align", "align");

  addProperty("node/stroke", this, "nodeBorder", "visible");

  addLineProperties("node/stroke", "nodeBorder");

  addProperty("node/fill", this, "nodeFilled", "visible");

  addFillProperties("node/fill", "nodeFill");

  addProperty("edge/stroke", this, "edgeBorder", "visible");

  addLineProperties("edge/stroke", "edgeBorder");

  addProperty("edge/fill", this, "edgeFilled", "visible");

  addFillProperties("edge/fill", "edgeFill");

  addProperty("text", this, "textVisible" , "visible" );

  addTextProperties("text", "text");
}

CQChartsGeom::Range
CQChartsSankeyPlot::
calcRange()
{
  CQChartsGeom::Range dataRange;

  QAbstractItemModel *model = this->model().data();

  if (! model)
    return dataRange;

  if (bbox_.isSet()) {
    double xm = bbox_.getHeight()*0.01;
    double ym = bbox_.getWidth ()*0.01;

    dataRange.updateRange(bbox_.getXMin() - xm, bbox_.getYMin() - ym);
    dataRange.updateRange(bbox_.getXMax() + xm, bbox_.getYMax() + ym);
  }

  return dataRange;
}

bool
CQChartsSankeyPlot::
createObjs()
{
  CQPerfTrace trace("CQChartsSankeyPlot::createObjs");

  for (const auto &nameNode : nameNodeMap_)
    delete nameNode.second;

  for (const auto &edge : edges_)
    delete edge;

  nameNodeMap_.clear();
  indNodeMap_ .clear();
  edges_      .clear();

  //---

  QAbstractItemModel *model = this->model().data();

  if (! model)
    return false;

  //---

  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(CQChartsSankeyPlot *plot) :
     plot_(plot) {
    }

    State visit(QAbstractItemModel *, const VisitData &data) override {
      bool ok1, ok2;

      QString linkStr = plot_->modelString(data.row, plot_->linkColumn (), data.parent, ok1);
      double  value   = plot_->modelReal  (data.row, plot_->valueColumn(), data.parent, ok2);

      if (! ok1 || ! ok2)
        return State::SKIP;

      CQChartsNamePair namePair(linkStr);

      if (! namePair.isValid())
        return State::SKIP;

      QString srcStr  = namePair.name1();
      QString destStr = namePair.name2();

      CQChartsSankeyPlotNode *srcNode  = plot_->findNode(srcStr);
      CQChartsSankeyPlotNode *destNode = plot_->findNode(destStr);

      CQChartsSankeyPlotEdge *edge = plot_->createEdge(value, srcNode, destNode);

      srcNode ->addDestEdge(edge);
      destNode->addSrcEdge (edge);

      return State::OK;
    }

    CQChartsSankeyPlot *plot_ { nullptr };
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  //---

  createGraph();

  return true;
}

void
CQChartsSankeyPlot::
createGraph()
{
//double xs = bbox_.getWidth ();
  double ys = bbox_.getHeight();

  //---

  updateMaxDepth();

  //---

  using DepthNodesMap = std::map<int,IndNodeMap>;
  using DepthSizeMap  = std::map<int,double>;

  DepthNodesMap depthNodesMap;
  DepthSizeMap  depthSizeMap;

  for (const auto &idNode : indNodeMap_) {
    CQChartsSankeyPlotNode *node = idNode.second;

    int xpos = node->calcXPos();

    depthSizeMap[xpos] += node->edgeSum();

    depthNodesMap[xpos][node->ind()] = node;
  }

  //---

  maxHeight_ = 0;

  for (const auto &depthNodes : depthNodesMap)
    maxHeight_ = std::max(maxHeight_, int(depthNodes.second.size()));

  //---

  double totalSize = 0.0;

  for (const auto &depthSize : depthSizeMap)
    totalSize = std::max(totalSize, depthSize.second);

  //---

  margin_ = (maxHeight_ > 1 ? 0.2*ys/(maxHeight_ - 1) : 0.0);

  valueScale_ = (totalSize > 0 ? ys/totalSize : 0.0);

  //---

  for (const auto &depthNodes : depthNodesMap)
    createNodes(depthNodes.second);

  //--

  for (const auto &edge : edges_)
    createEdge(edge);

  //---

  adjustNodes();

  //---

  for (const auto &idNode : indNodeMap_) {
    CQChartsSankeyPlotNode *node = idNode.second;

    addPlotObject(node->obj());
  }

  for (const auto &edge : edges_)
    addPlotObject(edge->obj());
}

void
CQChartsSankeyPlot::
createNodes(const IndNodeMap &nodes)
{
  double xs = bbox_.getWidth ();
  double ys = bbox_.getHeight();

  double dx = xs/maxDepth();

  double xm = pixelToWindowWidth(16);

  //---

  double height = margin_*(nodes.size() - 1);

  for (const auto &idNode : nodes) {
    CQChartsSankeyPlotNode *node = idNode.second;

    height += valueScale()*node->edgeSum();
  }

  //---

  double y1 = bbox_.getYMax() - (ys - height)/2.0;

  for (const auto &idNode : nodes) {
    CQChartsSankeyPlotNode *node = idNode.second;

    // draw src box
    double h = valueScale()*node->edgeSum();

    //---

    int srcDepth  = node->srcDepth ();
    int destDepth = node->destDepth();

    int xpos = node->calcXPos();

    double x = bbox_.getXMin() + xpos*dx;

    double y2 = y1 - h;

    CQChartsGeom::BBox rect;

    if      (srcDepth == 0)
      rect = CQChartsGeom::BBox(x, y1, x + xm, y2);
    else if (destDepth == 0)
      rect = CQChartsGeom::BBox(x - xm, y1, x, y2);
    else
      rect = CQChartsGeom::BBox(x - xm/2, y1, x + xm/2, y2);

    CQChartsSankeyNodeObj *nodeObj = new CQChartsSankeyNodeObj(this, rect, node);

    node->setObj(nodeObj);

    //---

    y1 = y2 - margin_;
  }
}

void
CQChartsSankeyPlot::
createEdge(CQChartsSankeyPlotEdge *edge)
{
  double xm = bbox_.getHeight()*0.01;
  double ym = bbox_.getWidth ()*0.01;

  CQChartsGeom::BBox rect(bbox_.getXMin() - xm, bbox_.getYMin() - ym,
                          bbox_.getXMax() + xm, bbox_.getYMax() + ym);

  CQChartsSankeyEdgeObj *edgeObj = new CQChartsSankeyEdgeObj(this, rect, edge);

  edge->setObj(edgeObj);
}

void
CQChartsSankeyPlot::
updateMaxDepth()
{
  bbox_ = CQChartsGeom::BBox(-1.0, -1.0, 1.0, 1.0);

  maxDepth_ = 0;

  for (const auto &idNode : indNodeMap_) {
    CQChartsSankeyPlotNode *node = idNode.second;

    int srcDepth  = node->srcDepth ();
    int destDepth = node->destDepth();

    if      (align() == CQChartsSankeyPlot::Align::SRC)
      maxDepth_ = std::max(maxDepth_, srcDepth);
    else if (align() == CQChartsSankeyPlot::Align::DEST)
      maxDepth_ = std::max(maxDepth_, destDepth);
    else
      maxDepth_ = std::max(std::max(maxDepth_, srcDepth), destDepth);
  }
}

void
CQChartsSankeyPlot::
adjustNodes()
{
  // update range
  bbox_ = CQChartsGeom::BBox();

  for (const auto &idNode : indNodeMap_) {
    CQChartsSankeyPlotNode *node = idNode.second;

    bbox_ += node->obj()->rect();
  }

  updateRange();

  //---

  initPosNodesMap();

  //---

  int numPasses = 25;

  for (int pass = 0; pass < numPasses; ++pass) {
    //std::cerr << "Pass " << pass << "\n";

    adjustNodeCenters();
  }

  //---

  reorderNodeEdges();
}

void
CQChartsSankeyPlot::
initPosNodesMap()
{
  // get nodes by x pos
  posNodesMap_ = PosNodesMap();

  for (const auto &idNode : indNodeMap_) {
    CQChartsSankeyPlotNode *node = idNode.second;

    posNodesMap_[node->xpos()][node->ind()] = node;
  }
}

void
CQChartsSankeyPlot::
adjustNodeCenters()
{
  // adjust nodes so centered on src nodes

  // second to last
  for (int xpos = 1; xpos <= maxDepth(); ++xpos) {
    const IndNodeMap &indNodeMap = posNodesMap_[xpos];

    for (const auto &idNode : indNodeMap) {
      CQChartsSankeyPlotNode *node = idNode.second;

      adjustNode(node);
    }
  }

  removeOverlaps();

  // second to last to first
  for (int xpos = maxDepth() - 1; xpos >= 0; --xpos) {
    const IndNodeMap &indNodeMap = posNodesMap_[xpos];

    for (const auto &idNode : indNodeMap) {
      CQChartsSankeyPlotNode *node = idNode.second;

      adjustNode(node);
    }
  }

  removeOverlaps();
}

void
CQChartsSankeyPlot::
removeOverlaps()
{
  using PosNodeMap = std::map<double,CQChartsSankeyPlotNode *>;

  double ym = pixelToWindowHeight(4);

  for (const auto &posNodes : posNodesMap_) {
    const IndNodeMap &indNodeMap = posNodes.second;

    // get nodes sorted by y (max to min)
    PosNodeMap posNodeMap;

    for (const auto &idNode : indNodeMap) {
      CQChartsSankeyPlotNode *node = idNode.second;

      const CQChartsGeom::BBox &rect = node->obj()->rect();

      double y = bbox_.getYMax() - rect.getYMid();

      auto p = posNodeMap.find(y);

      while (p != posNodeMap.end()) {
        y -= 0.001;

        p = posNodeMap.find(y);
      }

      posNodeMap[y] = node;
    }

    //---

    // remove overlaps between nodes
    CQChartsSankeyPlotNode *node1 = nullptr;

    for (const auto &posNode : posNodeMap) {
      CQChartsSankeyPlotNode *node2 = posNode.second;

      if (node1) {
        const CQChartsGeom::BBox &rect1 = node1->obj()->rect();
        const CQChartsGeom::BBox &rect2 = node2->obj()->rect();

        if (rect2.getYMax() >= rect1.getYMin() - ym) {
          double dy = rect1.getYMin() - ym - rect2.getYMax();

          node2->obj()->moveBy(CQChartsGeom::Point(0, dy));
        }
      }

      node1 = node2;
    }

    // move back inside bbox
    if (node1) {
      const CQChartsGeom::BBox &rect1 = node1->obj()->rect();

      if (rect1.getYMin() < bbox_.getYMin()) {
        double dy = bbox_.getYMin() - rect1.getYMin();

        for (const auto &idNode : indNodeMap) {
          CQChartsSankeyPlotNode *node = idNode.second;

          node->obj()->moveBy(CQChartsGeom::Point(0, dy));
        }
      }
    }
  }
}

void
CQChartsSankeyPlot::
reorderNodeEdges()
{
  // sort node edges nodes by bbox
  using PosEdgeMap = std::map<double,CQChartsSankeyPlotEdge *>;

  for (const auto &idNode : indNodeMap_) {
    CQChartsSankeyPlotNode *node = idNode.second;

    //---

    PosEdgeMap srcPosEdgeMap;

    for (const auto &edge : node->srcEdges()) {
      CQChartsSankeyPlotNode *srcNode = edge->srcNode();

      const CQChartsGeom::BBox &rect = srcNode->obj()->rect();

      double y = bbox_.getYMax() - rect.getYMid();

      auto p = srcPosEdgeMap.find(y);

      while (p != srcPosEdgeMap.end()) {
        y -= 0.001;

        p = srcPosEdgeMap.find(y);
      }

      srcPosEdgeMap[y] = edge;
    }

    Edges srcEdges;

    for (const auto &srcPosNode : srcPosEdgeMap)
      srcEdges.push_back(srcPosNode.second);

    //---

    PosEdgeMap destPosEdgeMap;

    for (const auto &edge : node->destEdges()) {
      CQChartsSankeyPlotNode *destNode = edge->destNode();

      const CQChartsGeom::BBox &rect = destNode->obj()->rect();

      double y = bbox_.getYMax() - rect.getYMid();

      auto p = destPosEdgeMap.find(y);

      while (p != destPosEdgeMap.end()) {
        y -= 0.001;

        p = destPosEdgeMap.find(y);
      }

      destPosEdgeMap[y] = edge;
    }

    Edges destEdges;

    for (const auto &destPosNode : destPosEdgeMap)
      destEdges.push_back(destPosNode.second);

    //---

    node->setSrcEdges (srcEdges);
    node->setDestEdges(destEdges);
  }
}

void
CQChartsSankeyPlot::
adjustNode(CQChartsSankeyPlotNode *node)
{
  CQChartsGeom::BBox bbox;

  for (const auto &edge : node->srcEdges()) {
    CQChartsSankeyPlotNode *srcNode = edge->srcNode();

    bbox += srcNode->obj()->rect();
  }

  for (const auto &edge : node->destEdges()) {
    CQChartsSankeyPlotNode *destNode = edge->destNode();

    bbox += destNode->obj()->rect();
  }

  double dy = bbox.getYMid() - node->obj()->rect().getYMid();

  node->obj()->moveBy(CQChartsGeom::Point(0, dy));
}

CQChartsSankeyPlotNode *
CQChartsSankeyPlot::
findNode(const QString &name)
{
  auto p = nameNodeMap_.find(name);

  if (p == nameNodeMap_.end()) {
    CQChartsSankeyPlotNode *node = new CQChartsSankeyPlotNode(this, name);

    node->setInd(nameNodeMap_.size());

    p = nameNodeMap_.insert(p, NameNodeMap::value_type(name, node));

    //--

    indNodeMap_[node->ind()] = node;
  }

  return (*p).second;
}

CQChartsSankeyPlotEdge *
CQChartsSankeyPlot::
createEdge(double value, CQChartsSankeyPlotNode *srcNode, CQChartsSankeyPlotNode *destNode)
{
  CQChartsSankeyPlotEdge *edge = new CQChartsSankeyPlotEdge(this, value, srcNode, destNode);

  edges_.push_back(edge);

  return edge;
}

void
CQChartsSankeyPlot::
keyPress(int key, int modifier)
{
  if (key == Qt::Key_A) {
    adjustNodes();

    invalidateLayers();
  }
  else
    CQChartsPlot::keyPress(key, modifier);
}

//------

CQChartsSankeyPlotNode::
CQChartsSankeyPlotNode(CQChartsSankeyPlot *plot, const QString &str) :
 plot_(plot), str_(str)
{
}

CQChartsSankeyPlotNode::
~CQChartsSankeyPlotNode()
{
}

void
CQChartsSankeyPlotNode::
addSrcEdge(CQChartsSankeyPlotEdge *edge)
{
  edge->destNode()->parent_ = edge->srcNode();

  srcEdges_.push_back(edge);

  srcDepth_ = -1;
}

void
CQChartsSankeyPlotNode::
addDestEdge(CQChartsSankeyPlotEdge *edge)
{
  edge->destNode()->parent_ = edge->srcNode();

  destEdges_.push_back(edge);

  destDepth_ = -1;
}

int
CQChartsSankeyPlotNode::
srcDepth() const
{
  NodeSet visited;

  visited.insert(const_cast<CQChartsSankeyPlotNode *>(this));

  return srcDepth(visited);
}

int
CQChartsSankeyPlotNode::
srcDepth(NodeSet &visited) const
{
  if (srcDepth_ >= 0)
    return srcDepth_;

  if (srcEdges_.empty())
    srcDepth_ = 0;
  else {
    int depth = 0;

    for (const auto &edge : srcEdges_) {
      CQChartsSankeyPlotNode *node = edge->srcNode();

      auto p = visited.find(node);

      if (p == visited.end()) {
        visited.insert(node);

        depth = std::max(depth, node->srcDepth(visited));
      }
    }

    srcDepth_ = depth + 1;
  }

  return srcDepth_;
}

int
CQChartsSankeyPlotNode::
destDepth() const
{
  NodeSet visited;

  visited.insert(const_cast<CQChartsSankeyPlotNode *>(this));

  return destDepth(visited);
}

int
CQChartsSankeyPlotNode::
destDepth(NodeSet &visited) const
{
  if (destDepth_ >= 0)
    return destDepth_;

  if (destEdges_.empty())
    destDepth_ = 0;
  else {
    int depth = 0;

    for (const auto &edge : destEdges_) {
      CQChartsSankeyPlotNode *node = edge->destNode();

      auto p = visited.find(node);

      if (p == visited.end()) {
        visited.insert(node);

        depth = std::max(depth, node->destDepth(visited));
      }
    }

    destDepth_ = depth + 1;
  }

  return destDepth_;
}

int
CQChartsSankeyPlotNode::
calcXPos() const
{
  int srcDepth  = this->srcDepth ();
  int destDepth = this->destDepth();

  if      (srcDepth == 0)
    xpos_ = 0;
  else if (destDepth == 0)
    xpos_ = plot_->maxDepth();
  else {
    if      (plot_->align() == CQChartsSankeyPlot::Align::SRC)
      xpos_ = srcDepth;
    else if (plot_->align() == CQChartsSankeyPlot::Align::DEST)
      xpos_ = plot_->maxDepth() - destDepth;
    else if (plot_->align() == CQChartsSankeyPlot::Align::JUSTIFY) {
      double f = 1.0*srcDepth/(srcDepth + destDepth);

      xpos_ = int(f*plot_->maxDepth());
    }
  }

  return xpos_;
}

double
CQChartsSankeyPlotNode::
edgeSum() const
{
  return std::max(srcEdgeSum(), destEdgeSum());
}

double
CQChartsSankeyPlotNode::
srcEdgeSum() const
{
  double value = 0.0;

  for (const auto &edge : srcEdges_)
    value += edge->value();

  return value;
}

double
CQChartsSankeyPlotNode::
destEdgeSum() const
{
  double value = 0.0;

  for (const auto &edge : destEdges_)
    value += edge->value();

  return value;
}

void
CQChartsSankeyPlotNode::
setObj(CQChartsSankeyNodeObj *obj)
{
  obj_ = obj;
}

//------

CQChartsSankeyPlotEdge::
CQChartsSankeyPlotEdge(CQChartsSankeyPlot *plot, double value,
                       CQChartsSankeyPlotNode *srcNode, CQChartsSankeyPlotNode *destNode) :
 plot_(plot), value_(value), srcNode_(srcNode), destNode_(destNode)
{
}

CQChartsSankeyPlotEdge::
~CQChartsSankeyPlotEdge()
{
}

void
CQChartsSankeyPlotEdge::
setObj(CQChartsSankeyEdgeObj *obj)
{
  obj_ = obj;
}

//------

CQChartsSankeyNodeObj::
CQChartsSankeyNodeObj(CQChartsSankeyPlot *plot, const CQChartsGeom::BBox &rect,
                      CQChartsSankeyPlotNode *node) :
 CQChartsPlotObj(plot, rect), plot_(plot), node_(node)
{
  double x1 = rect.getXMin();
  double x2 = rect.getXMax();
  double y3 = rect.getYMax();

  for (const auto &edge : node_->srcEdges()) {
    double h1 = plot_->valueScale()*edge->value();

    double y4 = y3 - h1;

    auto p = srcEdgeRect_.find(edge);

    if (p == srcEdgeRect_.end())
      srcEdgeRect_[edge] = CQChartsGeom::BBox(x1, y4, x2, y3);

    y3 = y4;
  }

  y3 = rect.getYMax();

  for (const auto &edge : node->destEdges()) {
    double h1 = plot_->valueScale()*edge->value();

    double y4 = y3 - h1;

    auto p = destEdgeRect_.find(edge);

    if (p == destEdgeRect_.end())
      destEdgeRect_[edge] = CQChartsGeom::BBox(x1, y4, x2, y3);

    y3 = y4;
  }
}

QString
CQChartsSankeyNodeObj::
calcId() const
{
  double value = node_->edgeSum();

  return QString("node:%1:%2").arg(node_->str()).arg(value);
}

void
CQChartsSankeyNodeObj::
moveBy(const CQChartsGeom::Point &delta)
{
  //std::cerr << "  Move " << node_->str().toStdString() << " by " << delta.y << "\n";

  rect_.moveBy(delta);

  for (auto &edgeRect : srcEdgeRect_)
    edgeRect.second.moveBy(delta);

  for (auto &edgeRect : destEdgeRect_)
    edgeRect.second.moveBy(delta);
}

void
CQChartsSankeyNodeObj::
draw(QPainter *painter)
{
  int numNodes = plot_->numNodes();

  // set fill and stroke
  QPen   pen;
  QBrush brush;

  plot_->setPenBrush(pen, brush,
                     plot_->isNodeBorder(), plot_->interpNodeBorderColor(node_->ind(), numNodes),
                     plot_->nodeBorderAlpha(), plot_->nodeBorderWidth(), plot_->nodeBorderDash(),
                     plot_->isNodeFilled(), plot_->interpNodeFillColor(node_->ind(), numNodes),
                     plot_->nodeFillAlpha(), plot_->nodeFillPattern());

  plot_->updateObjPenBrushState(this, pen, brush);

  painter->setBrush(brush);
  painter->setPen(pen);

  //---

  // draw node
  CQChartsGeom::BBox prect;

  plot_->windowToPixel(rect_, prect);

  double px1 = prect.getXMin();
  double py1 = prect.getYMin();
  double px2 = prect.getXMax();
  double py2 = prect.getYMax();

  QPainterPath path;

  path.moveTo(QPointF(px1, py1));
  path.lineTo(QPointF(px2, py1));
  path.lineTo(QPointF(px2, py2));
  path.lineTo(QPointF(px1, py2));

  path.closeSubpath();

  painter->drawPath(path);
}

void
CQChartsSankeyNodeObj::
drawFg(QPainter *painter)
{
  if (! plot_->isTextVisible())
    return;

  CQChartsGeom::BBox prect;

  plot_->windowToPixel(rect_, prect);

  int numNodes = plot_->numNodes();

  //---

  // set font
  plot_->view()->setPlotPainterFont(plot_, painter, plot_->textFont());

  QFontMetricsF fm(painter->font());

  //---

  // set color
  QPen pen;

  QColor c = plot_->interpTextColor(node_->ind(), numNodes);

  plot_->setPen(pen, true, c, plot_->textAlpha(), CQChartsLength("0px"));

  painter->setPen(pen);

  //---

  //double value = node_->edgeSum();

  QString str = node_->str();

  double tx = (rect_.getXMid() < 0.5 ?
    prect.getXMax() + 4 : prect.getXMin() - 4 - fm.width(str));
  double ty = prect.getYMid() + fm.ascent()/2;

  if (plot_->isTextContrast())
    plot_->drawContrastText(painter, tx, ty, str, pen);
  else
    painter->drawText(tx, ty, str);
}

//------

CQChartsSankeyEdgeObj::
CQChartsSankeyEdgeObj(CQChartsSankeyPlot *plot, const CQChartsGeom::BBox &rect,
                      CQChartsSankeyPlotEdge *edge) :
 CQChartsPlotObj(plot, rect), plot_(plot), edge_(edge)
{
}

QString
CQChartsSankeyEdgeObj::
calcId() const
{
  return QString("edge:%1:%2:%3").arg(edge_->srcNode()->str()).
          arg(edge_->destNode()->str()).arg(edge_->value());
}

bool
CQChartsSankeyEdgeObj::
inside(const CQChartsGeom::Point &p) const
{
  CQChartsGeom::Point p1;

  plot_->windowToPixel(p, p1);

  return path_.contains(CQChartsUtil::toQPoint(p1));
}

void
CQChartsSankeyEdgeObj::
draw(QPainter *painter)
{
  int numNodes = plot_->numNodes();

  // set fill and stroke
  QPen   pen;
  QBrush brush;

  QColor fc1 = plot_->interpEdgeFillColor(edge_->srcNode ()->ind(), numNodes);
  QColor fc2 = plot_->interpEdgeFillColor(edge_->destNode()->ind(), numNodes);

  QColor fc = CQChartsUtil::blendColors(fc1, fc2, 0.5);

  QColor sc1 = plot_->interpEdgeBorderColor(edge_->srcNode ()->ind(), numNodes);
  QColor sc2 = plot_->interpEdgeBorderColor(edge_->destNode()->ind(), numNodes);

  QColor sc = CQChartsUtil::blendColors(sc1, sc2, 0.5);

  plot_->setPenBrush(pen, brush,
                     plot_->isEdgeBorder(), sc,
                     plot_->edgeBorderAlpha(), plot_->edgeBorderWidth(), plot_->edgeBorderDash(),
                     plot_->isEdgeFilled(), fc,
                     plot_->edgeFillAlpha(), plot_->edgeFillPattern());

  plot_->updateObjPenBrushState(this, pen, brush);

  painter->setBrush(brush);
  painter->setPen(pen);

  //---

  // draw edge
  const CQChartsGeom::BBox &srcRect  = edge_->srcNode ()->obj()->destEdgeRect(edge_);
  const CQChartsGeom::BBox &destRect = edge_->destNode()->obj()->srcEdgeRect (edge_);

  CQChartsGeom::BBox psrcRect, pdestRect;

  plot_->windowToPixel(srcRect , psrcRect );
  plot_->windowToPixel(destRect, pdestRect);

  double px1 = psrcRect .getXMax();
  double px2 = pdestRect.getXMin();

  double py11 = psrcRect .getYMax();
  double py12 = psrcRect .getYMin();
  double py21 = pdestRect.getYMax();
  double py22 = pdestRect.getYMin();

  path_ = QPainterPath();

  double px3 = px1 + (px2 - px1)/3.0;
  double px4 = px2 - (px2 - px1)/3.0;

  path_.moveTo (QPointF(px1, py11));
  path_.cubicTo(QPointF(px3, py11), QPointF(px4, py21), QPointF(px2, py21));
  path_.lineTo (QPointF(px2, py22));
  path_.cubicTo(QPointF(px4, py22), QPointF(px3, py12), QPointF(px1, py12));

  path_.closeSubpath();

  //---

  painter->drawPath(path_);
}
