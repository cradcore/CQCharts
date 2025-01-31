#ifndef CBuchHeim_H
#define CBuchHeim_H

#include <CSafeIndex.h>

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>

namespace CBuchHeim {

class Tree;

using TreeP = std::shared_ptr<Tree>;

class Tree {
 public:
  using Children = std::vector<TreeP>;

 public:
  Tree(const std::string &name);

  virtual ~Tree();

  std::string name() const { return name_; }

  Tree *parent() const { return parent_; }

  const Children &children() const { return children_; }

  int numChildren() const { return int(children_.size()); }

  Tree *child(int i) const { return CUtil::safeIndex(children_, i).get(); }

  Tree *addChild(TreeP tree);

  void print(std::ostream &os) {
    printDepth(os, 0);
  }

  void printDepth(std::ostream &os, int depth) {
    for (int i = 0; i < depth; ++i)
      os << "  ";

    os << name() << "\n";

    for (const auto &child : children())
      child->printDepth(os, depth + 1);
  }

 private:
  std::string name_;
  Tree*       parent_ { nullptr };
  Children    children_;
};

//---

class DrawTree;

using DrawTreeP = std::shared_ptr<DrawTree>;

class DrawTree {
 public:
  using Children = std::vector<DrawTreeP>;

  using DrawTrees   = std::vector<DrawTree *>;
  using YMap        = std::map<int, DrawTrees>;
  using PositionMap = std::map<int, YMap>;

 public:
  DrawTree(Tree *tree, DrawTree *parent=nullptr, int depth=0, int number=1);

 ~DrawTree();

  //---

  DrawTree *parent() const { return parent_; }

  const Children &children() const { return children_; }

  int numChildren() const { return int(children_.size()); }

  DrawTree *child(int i) const { return CUtil::safeIndex(children_, i).get(); }

  DrawTree *addChild(DrawTreeP tree);

  //---

  double x() const { return x_; }
  double y() const { return y_; }
  double r() const { return r_; }

  Tree *tree() const { return tree_; }

  std::string name() const { return tree()->name(); }

  int number() const { return number_; }

  DrawTree *ancestor() const { return ancestor_; }

  DrawTree *thread() const { return thread_; }

  //---

  double x1() const { return x1_; }
  void setX1(double r) { x1_ = r; }

  double y1() const { return y1_; }
  void setY1(double r) { y1_ = r; }

  double x2() const { return x2_; }
  void setX2(double r) { x2_ = r; }

  double y2() const { return y2_; }
  void setY2(double r) { y2_ = r; }

  //---

  std::string hierName() const {
    if (parent_)
      return parent_->hierName() + "/" + name();
    else
      return name();
  }

  //---

  void place();

  void normalize(bool equalScale=false);

  //---

  void knuthPlace();

  void wsPlace();

  void wsPlaceCentered();

  //---

  void checkOverlaps() const;

  void fixOverlaps();

  DrawTree *commonParent(DrawTree *tree1, DrawTree *tree2) const;

 private:
  DrawTree *left();

  DrawTree *right();

  DrawTree *left_brother();

  DrawTree *get_lmost_sibling();

  static DrawTree *buchheim(Tree *tree);

  static DrawTree *first_walk(DrawTree *v, double distance=1.0);

  static DrawTree *ancestor(DrawTree *vil, DrawTree *v, DrawTree *default_ancestor);

  static void move_subtree(DrawTree *wl, DrawTree *wr, int shift);

  static DrawTree *apportion(DrawTree *v, DrawTree *default_ancestor, double distance);

  static void execute_shifts(DrawTree *v);

  static double second_walk(DrawTree *v, int m=0, int depth=0, double min=-9999);

  static void third_walk(DrawTree *tree, int n);

  static void sizeTree(DrawTree *root, int depth, double &xmin, double &ymin,
                       double &xmax, double &ymax);

  static void moveScale(DrawTree *root, int depth, double xmin, double ymin,
                        double xmax, double ymax);

  //---

  int knuthPlace1(DrawTree *tree, int i);

  void wsPlace1(DrawTree *tree, int depth, std::map<int, int> &nexts);

  void wsPlaceCentered1(DrawTree *tree, int depth, std::map<int, int> &nexts,
                        std::map<int, int> &offset);

  void wsPlaceCenteredAddMods(DrawTree *tree, int modsum);

  //---

  void checkOverlaps1(DrawTree *root, PositionMap &positionMap) const;

  void moveChildren(const DrawTree *parent, const DrawTree *startChild);

 private:
  double    x_             { 0.0 };
  double    y_             { 0.0 };
  double    r_             { 0.5 };
  Tree*     tree_          { nullptr };
  DrawTree* parent_        { nullptr };
  Children  children_;
  DrawTree* ancestor_      { nullptr };
  DrawTree* thread_        { nullptr };
  int       offset_        { 0 };
  int       change_        { 0 };
  int       shift_         { 0 };
  DrawTree* lmost_sibling_ { nullptr };
  int       number_        { 0 };
  double    x1_            { 0.0 };
  double    y1_            { 0.0 };
  double    x2_            { 0.0 };
  double    y2_            { 0.0 };
};

}

#endif
