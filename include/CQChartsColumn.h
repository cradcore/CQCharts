#ifndef CQChartsColumn_H
#define CQChartsColumn_H

#include <cassert>
#include <QString>
#include <vector>
#include <iostream>

class CQChartsColumn {
 public:
  enum class Type {
    NONE,
    DATA,
    VHEADER,
    GROUP,
    EXPR,
    DATA_INDEX
  };

 public:
  static void registerMetaType();

 public:
  CQChartsColumn() = default;
  CQChartsColumn(int column, int role=-1); // data
  CQChartsColumn(Type type, int column, const QString &s, int role=-1); // explicit
  CQChartsColumn(const QString &s); // parsed

  CQChartsColumn(const CQChartsColumn &rhs);

 ~CQChartsColumn();

  CQChartsColumn &operator=(const CQChartsColumn &rhs);

  //--

  bool isValid() const { return type_ != Type::NONE; }

  Type type() const { return type_; }

  //--

  bool hasColumn() const {
    return ((type_ == Type::DATA || type_ == Type::DATA_INDEX) && column_ >= 0);
  }

  int column() const { return (hasColumn() ? column_ : -1); }

  //--

  bool hasRole() const {
    return ((type_ == Type::DATA || type_ == Type::DATA_INDEX) && role_ >= 0);
  }

  int role() const { return (hasRole() ? role_ : -1); }

  int role(int defRole) const { return (hasRole() ? role_ : defRole); }

  //--

  bool hasExpr() const { return (type_ == Type::EXPR && expr_); }

  QString expr() const { return QString(hasExpr() ? expr_ : ""); }

  //--

  bool hasIndex() const { return (type_ == Type::DATA_INDEX && expr_); }

  QString index() const { return QString(hasIndex() ? expr_ : ""); }

  //--

  bool isGroup() const { return (type_ == Type::GROUP); }

  //--

  bool setValue(const QString &str);

  bool isMapped() const { return mapped_; }
  void setMapped(bool b) { mapped_ = b; }

  double mapMin() const { return mapMin_; }
  void setMapMin(double r) { mapMin_ = r; }

  double mapMax() const { return mapMax_; }
  void setMapMax(double r) { mapMax_ = r; }

  //---

  QString toString() const;

  void fromString(const QString &s);

  //---

  static int cmp(const CQChartsColumn &lhs, const CQChartsColumn &rhs);

  friend bool operator==(const CQChartsColumn &lhs, const CQChartsColumn &rhs) {
    return cmp(lhs, rhs) == 0;
  }

  friend bool operator!=(const CQChartsColumn &lhs, const CQChartsColumn &rhs) {
    return ! operator==(lhs, rhs);
  }

  friend bool operator<(const CQChartsColumn &lhs, const CQChartsColumn &rhs) {
    return cmp(lhs, rhs) < 0;
  }

  //---

  void print(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os, const CQChartsColumn &l) {
    l.print(os);

    return os;
  }

  //---

 public:
  using Columns = std::vector<CQChartsColumn>;

  static bool stringToColumns(const QString &str, Columns &columns);

  static QString columnsToString(const Columns &columns);

 private:
  bool decodeString(const QString &str, Type &type, int &column, int &role, QString &expr);

 private:
  Type   type_   { Type::NONE };
  int    column_ { -1 };
  int    role_   { -1 };
  char*  expr_   { nullptr };
  bool   mapped_ { false };
  double mapMin_ { 0.0 };
  double mapMax_ { 1.0 };
};

//---

// manage list of columns or single column
class CQChartsColumns {
 public:
  using Columns = std::vector<CQChartsColumn>;

 public:
  CQChartsColumns() { }

  CQChartsColumns(const CQChartsColumn &c) {
    setColumn(c);
  }

  // get single column
  const CQChartsColumn &column() const { return column_; }

  // set single column (multiple columns empty if column invalid)
  void setColumn(const CQChartsColumn &c) {
    column_ = c;

    columns_.clear();

    if (column_.isValid())
      columns_.push_back(column_);
  }

  // get multiple columns
  const Columns &columns() const { return columns_; }

  // set multiple columns (single column is invalid if columns empty)
  void setColumns(const Columns &columns) {
    columns_ = columns;

    if (! columns_.empty())
      column_ = columns_[0];
    else
      column_ = CQChartsColumn();
  }

  QString columnsStr() const {
    return CQChartsColumn::columnsToString(columns_);
  }

  bool setColumnsStr(const QString &s) {
    Columns cols;

    if (! CQChartsColumn::stringToColumns(s, cols))
      return false;

    setColumns(cols);

    return true;
  }

  // return number of columns (minumum is one as single invalid counts)
  int count() const {
    if (columns_.empty())
      return 1;

    return columns_.size();
  }

  const CQChartsColumn &getColumn(int i) const {
    if (! columns_.empty()) {
      assert(i >= 0 && i < int(columns_.size()));

      return columns_[i];
    }

    assert(i == 0);

    return column_;
  }

  friend bool operator==(const CQChartsColumns &lhs, const CQChartsColumns &rhs) {
    int nl = lhs.columns_.size();
    int nr = rhs.columns_.size();

    if (nl != nr)
      return false;

    if (! nl)
      return (lhs.column_ == rhs.column_);

    for (int i = 0; i < nl; ++i) {
      if (lhs.columns_[i] != rhs.columns_[i])
        return false;
    }

    return true;
  }

  friend bool operator!=(const CQChartsColumns &lhs, const CQChartsColumns &rhs) {
    return ! operator==(lhs, rhs);
  }

 private:
  CQChartsColumn column_;  // single column
  Columns        columns_; // multiple columns
};

//---

#include <QModelIndex>

struct CQChartsModelIndex {
  int            row { -1 };
  CQChartsColumn column;
  QModelIndex    parent;

  CQChartsModelIndex() = default;

  CQChartsModelIndex(int row, const CQChartsColumn &column,
                     const QModelIndex &parent=QModelIndex()) :
   row(row), column(column), parent(parent) {
  }
};

//---

#include <CQUtilMeta.h>

CQUTIL_DCL_META_TYPE(CQChartsColumn)

#endif
