#ifndef CQChartsDendrogram_H
#define CQChartsDendrogram_H

#include <CQChartsGeom.h>
#include <QString>
#include <vector>
#include <algorithm>
#include <boost/optional.hpp>

/*!
 * \brief Dendrogram plot data
 * \ingroup Charts
 */
class CQChartsDendrogram {
 public:
  using BBox = CQChartsGeom::BBox;

  class RootNode;
  class HierNode;
  class Node;

  //! Node
  class Node {
   private:
    static uint nextId() {
      static int lastId = 0;

      return ++lastId;
    }

   public:
    Node(HierNode *parent, const QString &name="", double size=1.0);

    virtual ~Node() { }

    HierNode *parent() const { return parent_; }

    uint id() const { return id_; }

    const QString &name() const { return name_; }
    void setName(const QString &name) { name_ = name; }

    virtual double size() const { return size_; }

    //! get/set depth
    int depth() const { return depth_; }
    void setDepth(int depth) { depth_ = depth; }

    //! get/set row
    double row() const { return row_; }
    void setRow(double row) { row_ = row; }

    //! get/set number of rows
    double numRows() const { return nr_; }
    void setNumRows(double nr) { nr_ = nr; }

    double gap() const { return gap_; }

    void setGap(double gap);

    //! get/set is placed
    bool isPlaced() const { return placed_; }
    void setPlaced(bool placed) { placed_ = placed; }

    virtual void resetPlaced() { bbox_ = BBox(); placed_ = false; }

    //! get/set is open
    bool isOpen() const { return open_; }
    void setOpen(bool open) { open_ = open; }

    //---

    double x() const {
      if (bbox_.isValid()) return bbox_.getXMin();

      return depth()*root()->dx();
    }

    double y() const {
      if (bbox_.isValid()) return bbox_.getYMin();

      return row()*root()->dy();
    }

    double w() const {
      if (bbox_.isValid()) return bbox_.getWidth();

      return root()->dx();
    }

    double h() const {
      if (bbox_.isValid()) return bbox_.getHeight();

      return numRows()*root()->dy();
    }

    double xc() const { return x() + w()/2.0; }
    double yc() const { return y() + h()/2.0; }

    const BBox &bbox() const { return bbox_; }
    void setBBox(const BBox &b) { bbox_ = b; }

    //---

    virtual int maxNodes() { return 1; }

    RootNode *root();
    const RootNode *root() const;

    virtual bool hasChildren() const { return false; }

    virtual void moveNode(double d);

    virtual void compressNode(double d);

    bool isNodeAtPoint(double x, double y, double tol) const;

   protected:
    friend class HierNode;

    void setParent(HierNode *parent) { parent_ = parent; }

   protected:
    HierNode* parent_ { nullptr }; //!< parent hier node
    uint      id_;                 //!< id
    QString   name_;               //!< name
    double    size_   { 0.0 };     //!< size
    int       depth_  { 0 };       //!< depth
    double    row_    { 0.0 };     //!< row
    double    nr_     { 0.0 };     //!< number of rows
    double    gap_    { 0.0 };     //!< gap
    bool      open_   { false };   //!< is open
    bool      placed_ { false };   //!< is placed
    BBox      bbox_;               //!< bbox
  };

  //---

  using OptReal = boost::optional<double>;

  struct Child {
    Node*   node { nullptr };
    OptReal value;

    Child() { }

    Child(Node *node, const OptReal &value=OptReal()) :
     node(node), value(value) {
    }
  };

  using Nodes    = std::vector<Node *>;
  using Children = std::vector<Child>;

  //---

  //! Hierarchical Node
  class HierNode : public Node {
   public:
    struct HierChild {
      HierNode *node { nullptr };
      OptReal   value;

      HierChild() { }

      HierChild(HierNode *node, const OptReal &value=OptReal()) :
       node(node), value(value) {
      }
    };

    using HierChildren = std::vector<HierChild>;

   public:
    HierNode(HierNode *parent=nullptr, const QString &name="");

   ~HierNode();

    double size() const override;

    const Children     &getNodes   () const { return nodes_; }
    const HierChildren &getChildren() const { return children_; }

    int calcDepth() const;

    int numNodes() const;

    int maxNodes() override;

    bool hasChildren() const override;

    HierNode *findChild(const QString &name) const;

    void resetPlaced() override;

    void placeSubNodes(RootNode *root, int depth, double row);

    void addChild(HierNode *child, const OptReal &value=OptReal());

    void setChildValue(Node *child, double value);

    void addNode(Node *node, const OptReal &value=OptReal());

    void compressNode(double d) override;

    Node *getNodeAtPoint(double x, double y, double tol);
    const Node *getNodeAtPoint(double x, double y, double tol) const;

    void clear();

   protected:
    Children     nodes_;    //!< child leaf nodes
    HierChildren children_; //!< child hier nodes
  };

  //---

  //! Root Node
  class RootNode : public HierNode {
   public:
    explicit RootNode(const QString &name="");

    virtual ~RootNode() { }

    void setDebug(bool debug) { debug_ = debug; }
    bool debug() const { return debug_; }

    void setSingleStep(bool singleStep) { singleStep_ = singleStep; }
    bool singleStep() const { return singleStep_; }

    double dx() const { return dx_; }
    double dy() const { return dy_; }

    // place child nodes in (1.0 by 1.0 rectangle)
    void placeNodes();

    // set gaps beneath each node
    void setGaps();

    // print node gaps
    void printGaps() const;

    // compress nodes by removing space below
    void compressNodes();

    // check if we can move node and children by delta
    bool canMoveNode(Node *node, double &move_gap, Nodes &lowestChildren);

    // get the child of node with lowest row
    Node *getLowestChild(HierNode *hierNode);

    // open all nodes down to depth and close all node below depth
    void setOpenDepth(int depth);

    // open node with name at depth
    void openNode(int depth, const QString &name);

    // compress node and children
    void compressNodeAndChildren(Node *node, const Nodes &lowestChildren, double d);

    //void moveChildNodes(HierNode *hierNode, double d);

    // recursively move node, higher nodes and parent
    void compressNodeUp(Node *node, double d);

    // move all nodes higher than specified node
    void moveHigherNodes(Node *node, double d);

    // place node (set depth, row and number of rows)
    void placeNode(Node *node, int depth, double row, double num_rows);

   protected:
    using DepthNodes = std::vector<Nodes>;

    double     dx_         { 0.0 };   //!< dx
    double     dy_         { 0.0 };   //!< dy
    double     max_rows_   { 0.0 };   //!< max rows
    bool       debug_      { false }; //!< is debug
    bool       singleStep_ { false }; //!< single step
    DepthNodes depthNodes_;           //!< nodes by depth
  };

  //------

 public:
  CQChartsDendrogram();

  virtual ~CQChartsDendrogram();

  //! get/set debug
  bool debug() const;
  void setDebug(bool b);

  //! get/set single setp
  bool singleStep() const;
  void setSingleStep(bool b);

  HierNode *root() const { return root_; }

  HierNode *addRootNode(const QString &name);

  HierNode *addHierNode(HierNode *hier, const QString &name, const OptReal &value=OptReal());

  Node *addNode(HierNode *hier, const QString &name, double size);

  virtual RootNode *createRootNode(const QString &name) const;
  virtual HierNode *createHierNode(HierNode *hier, const QString &name) const;
  virtual Node *createNode(HierNode *hier, const QString &name, double size) const;

  void placeNodes();

  void compressNodes();

  void setOpenDepth(int depth);

  void openNode(int depth, const QString &name);

  Node *getNodeAtPoint(double x, double y, double tol=1E-3);
  const Node *getNodeAtPoint(double x, double y, double tol=1E-3) const;

  void printGaps();

 private:
  RootNode* root_ { nullptr }; //!< root node
};

#endif
