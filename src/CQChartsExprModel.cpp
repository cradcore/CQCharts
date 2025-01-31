#include <CQChartsExprModel.h>
#include <CQChartsExprModelFn.h>
#include <CQChartsExprCmdValues.h>
#include <CQChartsExprTcl.h>
#include <CQChartsModelData.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelFilter.h>
#include <CQChartsModelUtil.h>
#include <CQChartsColumnType.h>
#include <CQChartsRand.h>
#include <CQChartsVariant.h>
#include <CQCharts.h>

#include <CQDataModel.h>
#include <CQModelUtil.h>
#include <CQPerfMonitor.h>

#include <QColor>
#include <iostream>

//------

CQChartsExprModel::
CQChartsExprModel(CQCharts *charts, CQChartsModelFilter *filter, QAbstractItemModel *model) :
 charts_(charts), filter_(filter), model_(model)
{
  assert(model);

  setObjectName("exprModel");

  qtcl_ = new CQChartsExprTcl(this);

  addBuiltinFunctions();

  setSourceModel(model);

  connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(dataChangedSlot(const QModelIndex &, const QModelIndex &)));
}

CQChartsExprModel::
~CQChartsExprModel()
{
  for (auto &tclCmd : tclCmds_)
    delete tclCmd;

  delete qtcl_;

  for (auto &extraColumn : extraColumns_)
    delete extraColumn;
}

void
CQChartsExprModel::
addBuiltinFunctions()
{
  // get/set model data (TODO: hierarchical)
  addFunction("column"   );
  addFunction("row"      );
  addFunction("cell"     );
  addFunction("setColumn");
  addFunction("setRow"   );
  addFunction("setCell"  );

  // get/set header data
  addFunction("header"   );
  addFunction("setHeader");

  // get/set column type
  addFunction("type"   );
  addFunction("setType");

  // map values
  addFunction("map"   );
  addFunction("remap" );
  addFunction("bucket");
  addFunction("norm"  );
  addFunction("scale" );

  // random
  addFunction("rand" );
  addFunction("rnorm");

  // string
  addFunction("match");

  // color
  addFunction("color");

  // time
  addFunction("timeval");

//addFunction("key");

  addFunction("concat");
}

CQTcl *
CQChartsExprModel::
qtcl() const
{
  return qtcl_;
}

void
CQChartsExprModel::
addFunction(const QString &name)
{
  assert(name.length());

  auto *fn = new CQChartsExprModelFn(this, name);

  tclCmds_.push_back(fn);

  fnNames_.insert(name);
}

bool
CQChartsExprModel::
isOrigColumn(int column) const
{
  int nc = columnCount();

  int numNonExtra = nc - numExtraColumns();

  return (column >= 0 && column < numNonExtra);
}

bool
CQChartsExprModel::
isExtraColumn(int column, int &ecolumn) const
{
  ecolumn = -1;

  int nc = columnCount();

  int numNonExtra = nc - numExtraColumns();

  if (column < numNonExtra)
    return false;

  ecolumn = column - numNonExtra;

  return (ecolumn >= 0 && ecolumn < numExtraColumns());
}

bool
CQChartsExprModel::
isReadOnly() const
{
  auto *dataModel = qobject_cast<CQDataModel *>(model_);

  return (dataModel ? dataModel->isReadOnly() : true);
}

void
CQChartsExprModel::
setReadOnly(bool b)
{
  auto *dataModel = qobject_cast<CQDataModel *>(model_);

  if (dataModel)
    dataModel->setReadOnly(b);
}

bool
CQChartsExprModel::
addExtraColumnExpr(const QString &exprStr, int &column)
{
  QString header, expr;

  if (! decodeExpression(exprStr, header, expr))
    return false;

  return addExtraColumnExpr(header, expr, column);
}

bool
CQChartsExprModel::
addExtraColumnExpr(const QString &header, const QString &expr, int &column)
{
  CQPerfTrace trace("CQChartsExprModel::addExtraColumnExpr");

  initCalc();

  //---

  nc_ = columnCount(QModelIndex());

  beginInsertColumns(QModelIndex(), nc_, nc_);

  //---

  // add new column
  extraColumns_.push_back(new ExtraColumn(expr, header));

  int ecolumn = numExtraColumns() - 1;

  //---

  // init calculated values in separate array
  auto *extraColumn = extraColumns_[size_t(ecolumn)];

  nr_ = rowCount();

  extraColumn->values.resize(size_t(nr_));

  for (int r = 0; r < nr_; ++r)
    extraColumn->values[size_t(r)] = QVariant();

  // calculate new values
  extraColumn->function = Function::ADD;

  calcExtraColumn(nc_, ecolumn);

  extraColumn->function = Function::EVAL;

  // remove extra calculated values
  extraColumn->values.clear();

  //---

  endInsertColumns();

  column = columnCount(QModelIndex()) - 1;

  return true;
}

bool
CQChartsExprModel::
addExtraColumnStrs(const QString &header, const QStringList &strs, int &column)
{
  CQPerfTrace trace("CQChartsExprModel::addExtraColumnStrs");

  initCalc();

  //---

  nc_ = columnCount(QModelIndex());

  beginInsertColumns(QModelIndex(), nc_, nc_);

  //---

  // add new column
  extraColumns_.push_back(new ExtraColumn("", header));

  int ecolumn = numExtraColumns() - 1;

  //---

  // init calculated values in separate array
  auto *extraColumn = extraColumns_[size_t(ecolumn)];

  nr_ = rowCount();

  extraColumn->values.resize(size_t(nr_));

  int ns = strs.size();

  for (int r = 0; r < std::min(ns, nr_); ++r)
    extraColumn->values[size_t(r)] = strs[r];

  extraColumn->function = Function::NONE;

  //---

  endInsertColumns();

  column = columnCount(QModelIndex()) - 1;

  return true;
}

bool
CQChartsExprModel::
removeExtraColumn(int column)
{
  int ecolumn;

  if (! isExtraColumn(column, ecolumn))
    return false;

  delete extraColumns_[size_t(ecolumn)];

  beginRemoveColumns(QModelIndex(), column, column);

  for (int i = ecolumn + 1; i < numExtraColumns(); ++i)
    extraColumns_[size_t(i - 1)] = extraColumns_[size_t(i)];

  extraColumns_.pop_back();

  endRemoveColumns();

  return true;
}

#if 0
bool
CQChartsExprModel::
assignColumn(int column, const QString &exprStr)
{
  QString header, expr;

  if (! decodeExpression(exprStr, header, expr))
    return false;

  return assignColumn(header, column, expr);
}
#endif

bool
CQChartsExprModel::
assignExtraColumn(int column, const QString &exprStr)
{
  QString header, expr;

  if (! decodeExpression(exprStr, header, expr))
    return false;

  return assignExtraColumn(header, column, expr);
}

bool
CQChartsExprModel::
assignColumn(const QString &header, int column, const QString &expr)
{
  CQPerfTrace trace("CQChartsExprModel::assignColumn");

  initCalc();

  nc_ = columnCount();

  // set new expression and ensure all column values calculated
  int numNonExtra = nc_ - numExtraColumns();

  if (column < 0 || column >= numNonExtra)
    return false;

  int numErrors = 0;

  int role = Qt::EditRole;

  nr_ = rowCount();

  for (int r = 0; r < nr_; ++r) {
    currentRow_ = r;
    currentCol_ = column;

    auto expr1 = replaceExprColumns(expr, currentRow_, currentCol_).trimmed();

    QVariant var;

    if (evaluateExpression(expr1, var)) {
      currentCol_ = column;

      auto ind = index(currentRow_, currentCol_, QModelIndex());

      setData(ind, var, role);
    }
    else
      ++numErrors;
  }

  if (header != "")
    setHeaderData(column, Qt::Horizontal, header);

  //---

  auto index1 = index(0      , column, QModelIndex());
  auto index2 = index(nr_ - 1, column, QModelIndex());

  emit dataChanged(index1, index2);

  return (numErrors == 0);
}

bool
CQChartsExprModel::
assignExtraColumn(const QString &header, int column, const QString &expr)
{
  CQPerfTrace trace("CQChartsExprModel::assignExtraColumn");

  int ecolumn;

  if (! isExtraColumn(column, ecolumn))
    return false;

  initCalc();

  nc_ = columnCount();

  // set new expression and ensure all column values calculated
  calcExtraColumn(column, ecolumn);

  // store calculated values in separate array
  auto *extraColumn = extraColumns_[size_t(ecolumn)];

  nr_ = rowCount();

  extraColumn->values.resize(size_t(nr_));

  for (int r = 0; r < nr_; ++r)
    extraColumn->values[size_t(r)] = extraColumn->variantMap[r];

  // set new expression and ensure all column values calculated
  if (header != "")
    extraColumn->header = header;

  extraColumn->expr = expr;

  extraColumn->variantMap.clear();

  extraColumn->function = Function::ASSIGN;

  calcExtraColumn(column, ecolumn);

  extraColumn->function = Function::EVAL;

  // remove extra calculated values
  extraColumn->values.clear();

  //---

  auto index1 = index(0      , column, QModelIndex());
  auto index2 = index(nr_ - 1, column, QModelIndex());

  emit dataChanged(index1, index2);

  return true;
}

bool
CQChartsExprModel::
calcColumn(int column, const QString &expr, Values &values, const NameValues &nameValues) const
{
  CQPerfTrace trace("CQChartsExprModel::calcColumn");

  initCalc();

  int numErrors = 0;

  for (const auto &nv : nameValues)
    qtcl_->createVar(nv.first, nv.second);

  for (int r = 0; r < nr_; ++r) {
    currentRow_ = r;
    currentCol_ = column;

    QVariant var;

    auto expr1 = replaceExprColumns(expr, currentRow_, currentCol_).trimmed();

    if (! evaluateExpression(expr1, var))
      ++numErrors;

    values.push_back(var);
  }

  return (numErrors == 0);
}

bool
CQChartsExprModel::
queryColumn(int column, const QString &expr, Rows &rows) const
{
  CQPerfTrace trace("CQChartsExprModel::queryColumn");

  initCalc();

  int numErrors = 0;

  for (int r = 0; r < nr_; ++r) {
    currentRow_ = r;
    currentCol_ = column;

    QVariant var;

    auto expr1 = replaceExprColumns(expr, currentRow_, currentCol_).trimmed();

    if (! evaluateExpression(expr1, var)) {
      ++numErrors;
      continue;
    }

    bool ok;
    if (CQChartsVariant::toBool(var, ok))
      rows.push_back(r);
  }

  return (numErrors == 0);
}

bool
CQChartsExprModel::
getExtraColumnDetails(int column, QString &header, QString &expr) const
{
  int ecolumn;

  if (! isExtraColumn(column, ecolumn))
    return false;

  const auto *extraColumn = extraColumns_[size_t(ecolumn)];

  header = extraColumn->header;
  expr   = extraColumn->expr;

  return true;
}

void
CQChartsExprModel::
initCalc() const
{
  auto *th = const_cast<CQChartsExprModel *>(this);

  th->initCalc();
}

void
CQChartsExprModel::
initCalc()
{
  std::unique_lock<std::mutex> lock(mutex_);

  nr_ = rowCount();
  nc_ = columnCount();

  columnNames_.clear();
  nameColumns_.clear();

  qtcl_->resetLastValue();
  qtcl_->resetColumns();

  //---

  // add user defined functions
  // TODO: only add if needed ? (traces)
  for (const auto &np : charts_->procs(CQCharts::ProcType::TCL)) {
    const auto &proc = np.second;

    qtcl_->defineProc(proc.name, proc.args, proc.body);
  }

  //---

  // add traces for column names
  for (int ic = 0; ic < nc_; ++ic) {
    CQChartsColumn c(ic);

    bool ok;

    auto name = CQChartsModelUtil::modelHHeaderString(this, c, ok);

    columnNames_[ic  ] = name;
    nameColumns_[name] = ic;

    qtcl_->setNameColumn(qtcl_->encodeColumnName(name), ic);
  }

  qtcl_->initVars();
}

bool
CQChartsExprModel::
calcExtraColumn(int column, int ecolumn)
{
  nr_ = rowCount();
  nc_ = columnCount();

  // ensure all values are evaluated
  int numErrors = 0;

  for (int r = 0; r < nr_; ++r) {
    currentRow_ = r;
    currentCol_ = column;

    bool rc = true;

    (void) getExtraColumnValue(r, column, ecolumn, rc);

    if (! rc)
      ++numErrors;
  }

  return (numErrors == 0);
}

bool
CQChartsExprModel::
processExpr(const QString &expr)
{
  CQPerfTrace trace("CQChartsExprModel::processExpr");

  initCalc();

  int numErrors = 0;

  auto expr1 = replaceExprColumns(expr, -1, -1);

  for (int r = 0; r < nr_; ++r) {
    currentRow_ = r;

    QVariant var;

    if (! evaluateExpression(expr1, var)) {
      ++numErrors;
      continue;
    }
  }

  return (numErrors == 0);
}

bool
CQChartsExprModel::
columnRange(int column, double &minVal, double &maxVal) const
{
  auto p = columnDatas_.find(column);

  if (p != columnDatas_.end()) {
    const auto &columnData = (*p).second;

    if (columnData.rmin) {
      minVal = *columnData.rmin;
      maxVal = *columnData.rmax;

      return true;
    }
  }

  //---

  auto *th = const_cast<CQChartsExprModel *>(this);

  return th->calcColumnRange(column, minVal, maxVal);
}

bool
CQChartsExprModel::
calcColumnRange(int column, double &minVal, double &maxVal)
{
  std::unique_lock<std::mutex> lock(mutex_);

  auto &columnData = columnDatas_[column];

  nr_ = rowCount();

  for (int r = 0; r < nr_; ++r) {
    auto ind = index(r, column, QModelIndex());

    auto var = data(ind, Qt::DisplayRole);

    bool ok;

    double value = CQChartsVariant::toReal(var, ok);
    if (! ok) continue;

    if (r == 0) {
      minVal = value;
      maxVal = value;
    }
    else {
      minVal = std::min(minVal, value);
      maxVal = std::max(maxVal, value);
    }
  }

  columnData.rmin = minVal;
  columnData.rmax = maxVal;

  return true;
}

bool
CQChartsExprModel::
columnRange(int column, long &minVal, long &maxVal) const
{
  auto p = columnDatas_.find(column);

  if (p != columnDatas_.end()) {
    const auto &columnData = (*p).second;

    if (columnData.imin) {
      minVal = *columnData.imin;
      maxVal = *columnData.imax;

      return true;
    }
  }

  //---

  auto *th = const_cast<CQChartsExprModel *>(this);

  return th->calcColumnRange(column, minVal, maxVal);
}

bool
CQChartsExprModel::
calcColumnRange(int column, long &minVal, long &maxVal)
{
  std::unique_lock<std::mutex> lock(mutex_);

  auto &columnData = columnDatas_[column];

  nr_ = rowCount();

  for (int r = 0; r < nr_; ++r) {
    auto ind = index(r, column, QModelIndex());

    auto var = data(ind, Qt::DisplayRole);

    bool ok;

    auto value = CQChartsVariant::toInt(var, ok);
    if (! ok) continue;

    if (r == 0) {
      minVal = value;
      maxVal = value;
    }
    else {
      minVal = std::min(minVal, value);
      maxVal = std::max(maxVal, value);
    }
  }

  columnData.imin = minVal;
  columnData.imax = maxVal;

  return true;
}

QModelIndex
CQChartsExprModel::
index(int row, int column, const QModelIndex &parent) const
{
  int nc = sourceModel()->columnCount(sourceModel()->index(row, column, mapToSource(parent)));

  if (column < nc)
    return mapFromSource(sourceModel()->index(row, column, mapToSource(parent)));

  return createIndex(row, column, nullptr);
}

QModelIndex
CQChartsExprModel::
parent(const QModelIndex &child) const
{
  return sourceModel()->parent(mapToSource(child));
}

int
CQChartsExprModel::
rowCount(const QModelIndex &parent) const
{
  return sourceModel()->rowCount(mapToSource(parent));
}

int
CQChartsExprModel::
columnCount(const QModelIndex &parent) const
{
  int nc = sourceModel()->columnCount(mapToSource(parent));

  return nc + numExtraColumns();
}

bool
CQChartsExprModel::
hasChildren(const QModelIndex &parent) const
{
  if (! parent.isValid())
    return true;

  return sourceModel()->hasChildren(mapToSource(parent));
}

QVariant
CQChartsExprModel::
data(const QModelIndex &index, int role) const
{
  if (! index.isValid())
    return QVariant();

  //---

  int nc = sourceModel()->columnCount(mapToSource(index));

  if (index.column() >= 0 && index.column() < nc) {
    auto sind = mapToSource(index);

    auto var = sourceModel()->data(sind, role);

    if (role == Qt::ToolTipRole && ! var.isValid())
      var = sourceModel()->data(sind, Qt::DisplayRole);

    return var;
  }

  //---

  QVariant var;

  bool rc = true;

  int column = index.column() - nc;

  if      (role == Qt::DisplayRole || role == Qt::EditRole) {
    currentRow_ = index.row();
    currentCol_ = index.column();

    var = getExtraColumnValue(currentRow_, currentCol_, column, rc);
  }
  else if (role == Qt::TextAlignmentRole) {
    const auto &extraColumn = this->extraColumn(column);

    if (extraColumn.typeData.type == CQBaseModelType::INTEGER ||
        extraColumn.typeData.type == CQBaseModelType::REAL)
      var = QVariant(Qt::AlignRight | Qt::AlignVCenter);
    else
      var = QVariant(Qt::AlignLeft | Qt::AlignVCenter);
  }
  else if (role == Qt::ToolTipRole) {
    currentRow_ = index.row();
    currentCol_ = index.column();

    var = getExtraColumnValue(currentRow_, currentCol_, column, rc);
  }

  return var;
}

bool
CQChartsExprModel::
decodeExpressionFn(const QString &exprStr, Function &function, long &column, QString &expr) const
{
  function = Function::EVAL;
  column   = -1;

  expr.clear();

  if (! exprStr.length())
    return true;

  if      (exprStr[0] == '+') {
    function = Function::ADD;

    expr = exprStr.mid(1).trimmed();
  }
  // delete column <n>
  else if (expr[0] == '-') {
    function = Function::DELETE;

    auto columnStr = exprStr.mid(1).trimmed();

    bool ok;

    column = CQChartsUtil::toInt(columnStr, ok);

    if (! ok)
      return false;
  }
  else if (expr[0] == '=') {
    function = Function::ASSIGN;

    auto columnExprStr = exprStr.mid(1).trimmed();

    int pos = columnExprStr.indexOf(':');

    if (pos < 0)
      return false;

    auto columnStr = columnExprStr.mid(0, pos).trimmed();

    bool ok;

    column = CQChartsUtil::toInt(columnStr, ok);

    if (! ok)
      return false;

    expr = expr.mid(1).trimmed();
  }
  else {
    function = Function::EVAL;
    expr     = exprStr;
  }

  return true;
}

bool
CQChartsExprModel::
decodeExpression(const QString &exprStr, QString &header, QString &expr) const
{
  expr = exprStr;

  int pos = expr.indexOf('=');

  if (pos >= 0) {
    header = expr.mid(0, pos).trimmed();
    expr   = expr.mid(pos + 1).trimmed();
  }

  return true;
}

QVariant
CQChartsExprModel::
getExtraColumnValue(int row, int column, int ecolumn, bool &rc) const
{
  const auto &extraColumn = this->extraColumn(ecolumn);

  //---

  if (extraColumn.function == Function::NONE) {
    return extraColumn.values[size_t(row)];
  }

  //---

  // if evaluating the expression for this row use cached values for referenced values
  if (extraColumn.evaluating.load()) {
    if (! extraColumn.values.empty())
      return extraColumn.values[size_t(row)];

    return QVariant();
  }

  //---

  auto p = extraColumn.variantMap.find(row);

  if (p != extraColumn.variantMap.end())
    return (*p).second;

  //---

  auto *th = const_cast<CQChartsExprModel *>(this);

  bool rc1 = true;

  auto var = th->calcExtraColumnValue(row, column, ecolumn, rc1);

  if (! rc1)
    rc = false;

  return var;
}

QVariant
CQChartsExprModel::
calcExtraColumnValue(int row, int column, int ecolumn, bool &rc)
{
  std::unique_lock<std::mutex> lock(mutex_);

  auto &extraColumn = this->extraColumn(ecolumn);

  extraColumn.evaluating = true;

  auto expr = extraColumn.expr;

  expr = replaceExprColumns(expr, row, column).trimmed();

  QVariant var;

  if (evaluateExpression(expr, var)) {
    std::string str;

    if      (CQChartsVariant::isReal(var)) {
      bool ok;
      double real = CQChartsVariant::toReal(var, ok);

      bool isInt = CQModelUtil::isInteger(real);

      if      (extraColumn.typeData.type == CQBaseModelType::NONE) {
        if (isInt)
          extraColumn.typeData.type = CQBaseModelType::INTEGER;
        else
          extraColumn.typeData.type = CQBaseModelType::REAL;
      }
      else if (extraColumn.typeData.type == CQBaseModelType::INTEGER) {
        if (! isInt)
          extraColumn.typeData.type = CQBaseModelType::REAL;
      }
      else if (extraColumn.typeData.type == CQBaseModelType::REAL) {
      }
    }
    else if (CQChartsVariant::isInt(var)) {
      if (extraColumn.typeData.type == CQBaseModelType::NONE)
        extraColumn.typeData.type = CQBaseModelType::INTEGER;
    }
    else if (CQChartsVariant::isBool(var)) {
      if (extraColumn.typeData.type == CQBaseModelType::NONE)
        extraColumn.typeData.type = CQBaseModelType::INTEGER;
    }
    else {
      if (extraColumn.typeData.type == CQBaseModelType::NONE)
        extraColumn.typeData.type = CQBaseModelType::STRING;
    }
  }
  else
    rc = false;

  auto p = extraColumn.variantMap.insert(extraColumn.variantMap.end(),
                                         VariantMap::value_type(row, var));

  if (extraColumn.function == Function::ADD) {
    if (! extraColumn.values.empty())
      extraColumn.values[size_t(row)] = var;
  }

  if (isDebug())
    std::cerr << "Set Row " << row << " Column " << column << " = " <<
                 var.toString().toStdString() << std::endl;

  extraColumn.evaluating = false;

  return (*p).second;
}

bool
CQChartsExprModel::
setData(const QModelIndex &index, const QVariant &value, int role)
{
  int nc = sourceModel()->columnCount(index);

  if (! index.isValid() || index.column() < nc)
    return sourceModel()->setData(mapToSource(index), value, role);

  int column = index.column() - nc;

  if (column >= numExtraColumns())
    return false;

  auto &extraColumn = this->extraColumn(column);

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    extraColumn.variantMap[index.row()] = value;

    emit dataChanged(index, index);

    return true;
  }

  return false;
}

QVariant
CQChartsExprModel::
headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal)
    return QAbstractProxyModel::headerData(section, orientation, role);

  //--

  int nc = sourceModel()->columnCount(sourceModel()->index(0, section, QModelIndex()));

  if (section < nc)
    return sourceModel()->headerData(section, orientation, role);

  //---

  int column = section - nc;

  if (column >= numExtraColumns())
    return QVariant();

  const auto &extraColumn = this->extraColumn(column);

  if      (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (extraColumn.header.length())
      return extraColumn.header;

    auto header = QString::number(section + 1);

    return header;
  }
  else if (role == Qt::ToolTipRole) {
    auto str = extraColumn.header;

    auto type = extraColumn.typeData.type;

    str += ":" + CQBaseModel::typeName(type);

    if (extraColumn.typeValues.length())
      str += ":" + extraColumn.typeValues;

    str += QString("\n(%1)").arg(extraColumn.expr);

    return str;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::Type)) {
    return CQBaseModel::typeToVariant(extraColumn.typeData.type);
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::BaseType)) {
    return CQBaseModel::typeToVariant(extraColumn.typeData.baseType);
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::TypeValues)) {
    return QVariant(extraColumn.typeValues);
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::Min)) {
    QVariant value;

    if (! extraColumn.typeData.nameValues.nameValue("min", value))
      return QVariant();

    return value;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::Max)) {
    QVariant value;

    if (! extraColumn.typeData.nameValues.nameValue("max", value))
      return QVariant();

    return value;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::Sorted)) {
    QVariant value;

    if (! extraColumn.typeData.nameValues.nameValue("sorted", value))
      return QVariant();

    return value;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::SortOrder)) {
    QVariant value;

    if (! extraColumn.typeData.nameValues.nameValue("sort_order", value))
      return QVariant();

    return value;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::HeaderType)) {
    return CQBaseModel::typeToVariant(extraColumn.typeData.headerType);
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::HeaderTypeValues)) {
    return QVariant(extraColumn.headerTypeValues);
  }

  return QVariant();
}

bool
CQChartsExprModel::
setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
  if (orientation != Qt::Horizontal)
    return QAbstractProxyModel::setHeaderData(section, orientation, value, role);

  //--

  int nc = sourceModel()->columnCount(sourceModel()->index(0, section, QModelIndex()));

  if (section < nc)
    return sourceModel()->setHeaderData(section, orientation, value, role);

  //---

  int column = section - nc;

  if (column >= numExtraColumns())
    return false;

  auto &extraColumn = this->extraColumn(column);

  if      (role == Qt::DisplayRole || role == Qt::EditRole) {
    extraColumn.header = value.toString();

    return true;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::Type)) {
    extraColumn.typeData.type = CQBaseModel::variantToType(value);

    return true;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::BaseType)) {
    extraColumn.typeData.baseType = CQBaseModel::variantToType(value);

    return true;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::TypeValues)) {
    extraColumn.typeValues = value.toString();

    return true;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::Min)) {
    extraColumn.typeData.nameValues.setNameValue("min", value);

    return true;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::Max)) {
    extraColumn.typeData.nameValues.setNameValue("max", value);

    return true;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::Sorted)) {
    extraColumn.typeData.nameValues.setNameValue("sorted", value);

    return true;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::SortOrder)) {
    extraColumn.typeData.nameValues.setNameValue("sort_order", value);

    return true;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::HeaderType)) {
    extraColumn.typeData.headerType = CQBaseModel::variantToType(value);

    return true;
  }
  else if (role == CQModelUtil::roleCast(CQBaseModelRole::HeaderTypeValues)) {
    extraColumn.headerTypeValues = value.toString();

    return true;
  }

  return false;
}

Qt::ItemFlags
CQChartsExprModel::
flags(const QModelIndex &index) const
{
  int nc = sourceModel()->columnCount(index);

  if (! index.isValid() || index.column() < nc)
    return sourceModel()->flags(mapToSource(index));

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (editable_)
    flags |= Qt::ItemIsEditable;

  return flags;
}

QModelIndex
CQChartsExprModel::
mapToSource(const QModelIndex &index) const
{
  if (! index.isValid())
    return QModelIndex();

  return model_->index(index.row(), index.column(), QModelIndex());
}

QModelIndex
CQChartsExprModel::
mapFromSource(const QModelIndex &index) const
{
  if (! index.isValid())
    return QModelIndex();

  return createIndex(index.row(), index.column(), index.internalPointer());
}

//------

QVariant
CQChartsExprModel::
processCmd(const QString &name, const Values &values)
{
  // get/set model data
  if      (name == "column"   ) return columnCmd   (values);
  else if (name == "row"      ) return rowCmd      (values);
  else if (name == "cell"     ) return cellCmd     (values);
  else if (name == "setColumn") return setColumnCmd(values);
  else if (name == "setRow"   ) return setRowCmd   (values);
  else if (name == "setCell"  ) return setCellCmd  (values);

  // get/set header data
  else if (name == "header"   ) return headerCmd   (values);
  else if (name == "setHeader") return setHeaderCmd(values);

  // get/set column type
  else if (name == "type"   ) return typeCmd   (values);
  else if (name == "setType") return setTypeCmd(values);

  // map values
  else if (name == "map"   ) return mapCmd   (values);
  else if (name == "remap" ) return remapCmd (values);
  else if (name == "bucket") return bucketCmd(values);
  else if (name == "norm"  ) return normCmd  (values);
  else if (name == "scale" ) return scaleCmd (values);

  // random
  else if (name == "rand" ) return randCmd (values);
  else if (name == "rnorm") return rnormCmd(values);

  // string
  else if (name == "match") return matchCmd(values);

  // color
  else if (name == "color") return colorCmd(values);

  // time
  else if (name == "timeval") return timevalCmd(values);

//else if (name == "key") return keyCmd(values);

  else if (name == "concat") return concatCmd(values);

  else return CQChartsVariant::fromBool(false);
}

//------

// get column value for current row:
//   column(), column(col), column(col, defVal) : get column value
QVariant
CQChartsExprModel::
columnCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  if (! cmdValues.hasValues())
    return currentCol();

  //---

  int row = currentRow();

  int col;

  if (! getColumnValue(cmdValues, col))
    return QVariant();

  //---

  if (! checkIndex(row, col)) {
    QString defStr;

    if (! cmdValues.getStr(defStr))
      return QVariant();

    bool ok; double r = CQChartsUtil::toReal(defStr, ok);

    if (ok)
      return CQChartsVariant::fromReal(r);

    return CQChartsVariant::fromString(defStr);
  }

  //---

  return getCmdData(row, col);
}

//---

// get row value for current column:
//   row(), row(row) : get row value
QVariant
CQChartsExprModel::
rowCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  if (! cmdValues.hasValues())
    return currentRow();

  //---

  int row;

  if (! getRowValue(cmdValues, row))
    return QVariant();

  int col = currentCol();

  //---

  if (! checkIndex(row, col)) {
    QString defStr;

    if (! cmdValues.getStr(defStr))
      return QVariant();

    bool ok; double r = CQChartsUtil::toReal(defStr, ok);

    if (ok)
      return CQChartsVariant::fromReal(r);

    return CQChartsVariant::fromString(defStr);
  }

  //---

  return getCmdData(row, col);
}

//---

// get cell (row/column) value:
//   cell(), cell(row, column), cell(row, column, defVal) : get cell value
QVariant
CQChartsExprModel::
cellCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() < 2) {
    // TODO: row and column string value ?
    return QVariant();
  }

  //---

  int row;

  if (! getRowValue(cmdValues, row))
    return QVariant();

  int col;

  if (! getColumnValue(cmdValues, col))
    return QVariant();

  //---

  if (! checkIndex(row, col)) {
    QString defStr;

    if (! cmdValues.getStr(defStr))
      return QVariant();

    bool ok; double r = CQChartsUtil::toReal(defStr, ok);

    if (ok)
      return CQChartsVariant::fromReal(r);

    return CQChartsVariant::fromString(defStr);
  }

  //---

  return getCmdData(row, col);
}

//---

// set column value (for current row):
//   setColumn(value), setColumn(col, value) : set column value
QVariant
CQChartsExprModel::
setColumnCmd(const Values &values)
{
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() < 1)
    return CQChartsVariant::fromBool(false);

  auto var = cmdValues.popValue(); // last value

  //---

  int row = currentRow();

  int col; (void) getColumnValue(cmdValues, col);

  //---

  if (! checkIndex(row, col))
    return CQChartsVariant::fromBool(false);

  //---

  bool b = setCmdData(row, col, var);

  return CQChartsVariant::fromBool(b);
}

//---

// set row value (for current column):
//   setRow(value), setRow(row, value), setRow(row, col, value) : set row value
QVariant
CQChartsExprModel::
setRowCmd(const Values &values)
{
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() < 1)
    return CQChartsVariant::fromBool(false);

  auto var = cmdValues.popValue(); // last value

  //---

  int col = currentCol();

  int row; (void) getRowValue(cmdValues, row);

  //---

  if (! checkIndex(row, col))
    return CQChartsVariant::fromBool(false);

  //---

  bool b = setCmdData(row, col, var);

  return CQChartsVariant::fromBool(b);
}

//---

// set cell value (for row/column):
//   setCell(value), setCell(row, col, value) : set row value
QVariant
CQChartsExprModel::
setCellCmd(const Values &values)
{
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() < 1)
    return CQChartsVariant::fromBool(false);

  auto var = cmdValues.popValue();

  //---

  int row; (void) getRowValue   (cmdValues, row);
  int col; (void) getColumnValue(cmdValues, col);

  //---

  if (! checkIndex(row, col))
    return CQChartsVariant::fromBool(false);

  //---

  bool b = setCmdData(row, col, var);

  return CQChartsVariant::fromBool(b);
}

//---

// get header value (for column):
//   header(), header(col)
QVariant
CQChartsExprModel::
headerCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  int col; (void) getColumnValue(cmdValues, col);

  //---

  if (! checkColumn(col))
    return QVariant();

  //---

  return headerData(col, Qt::Horizontal, Qt::DisplayRole);
}

//---

// set header value (for column):
//   setHeader(s), setHeader(col, s)
QVariant
CQChartsExprModel::
setHeaderCmd(const Values &values)
{
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() < 1)
    return CQChartsVariant::fromBool(false);

  auto var = cmdValues.popValue();

  //---

  int col; (void) getColumnValue(cmdValues, col);

  //---

  if (! checkColumn(col))
    return CQChartsVariant::fromBool(false);

  //---

  return setHeaderData(col, Qt::Horizontal, var, Qt::DisplayRole);
}

//---

// get type (for column):
//   type(), type(col)
QVariant
CQChartsExprModel::
typeCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  int col; (void) getColumnValue(cmdValues, col);

  //---

  if (! checkColumn(col))
    return QVariant();

  //---

  int role = CQModelUtil::roleCast(CQBaseModelRole::Type);

  auto var = headerData(col, Qt::Horizontal, role);

  bool ok;

  auto type = static_cast<CQBaseModelType>(CQChartsVariant::toInt(var, ok));
  if (! ok) return "";

  auto typeName = CQBaseModel::typeName(type);

  return CQChartsVariant::fromString(typeName);
}

//---

// set type (for column):
//   setType(s), setType(col, s)
QVariant
CQChartsExprModel::
setTypeCmd(const Values &values)
{
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() < 1)
    return CQChartsVariant::fromBool(false);

  auto var = cmdValues.popValue();

  //---

  int col; (void) getColumnValue(cmdValues, col);

  //---

  if (! checkColumn(col))
    return CQChartsVariant::fromBool(false);

  //---

  int role = CQModelUtil::roleCast(CQBaseModelRole::Type);

  auto type = CQBaseModel::nameType(var.toString());

  QVariant typeVar(CQModelUtil::typeCast(type));

  bool b = setHeaderData(col, Qt::Horizontal, typeVar, role);

  return CQChartsVariant::fromBool(b);
}

//---

// map row value to min/max
//   map(), map(max), map(min, max)
QVariant
CQChartsExprModel::
mapCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  int row = currentRow();

  double min = 0.0, max = 1.0;

  if (cmdValues.numValues() == 0)
    return CQChartsVariant::fromInt(row);

  if (cmdValues.numValues() == 1)
    (void) cmdValues.getReal(max);
  else {
    (void) cmdValues.getReal(min);
    (void) cmdValues.getReal(max);
  }

  //---

  // scale row number to 0->1
  double x = 0.0;

  int nr = rowCount();

  if (nr > 0)
    x = (1.0*row)/nr;

  // map 0->1 -> min->max
  double x1 = x*(max - min) + min;

  return CQChartsVariant::fromReal(x1);
}

//---

// remap column value range to min/max
//   remap()              - current column, 0.0, 1.0
//   remap(col)           - specified column, 0.0, 1.0
//   remap(col, max)      - specified column, 0.0, max
//   remap(col, min, max) - specified column, min, max
QVariant
CQChartsExprModel::
remapCmd(const Values &values)
{
  CQChartsExprCmdValues cmdValues(values);

  int row = currentRow();
  int col = currentCol();

  double r1 = 0.0, r2 = 1.0;

  if (cmdValues.numValues() >= 1) {
    (void) getColumnValue(cmdValues, col);

    if      (cmdValues.numValues() >= 3) {
      (void) cmdValues.getReal(r1);
      (void) cmdValues.getReal(r2);
    }
    else if (cmdValues.numValues() >= 2) {
      (void) cmdValues.getReal(r2);
    }
  }

  //---

  if (! this->checkIndex(row, col))
    return CQChartsVariant::fromReal(0.0);

  auto ind = this->index(row, col, QModelIndex());

  //---

  double rmin = 0.0, rmax = 1.0;

  getColumnRange(ind, rmin, rmax);

  //---

  bool ok;

  double r = CQChartsModelUtil::modelReal(this, ind, ok);
  if (! ok) return CQChartsVariant::fromReal(0.0);

  double rm = CMathUtil::map(r, rmin, rmax, r1, r2);

  return CQChartsVariant::fromReal(rm);
}

//---

// bucket value (for row/column)
//   bucket(col, delta), bucket(col, start, delta)
QVariant
CQChartsExprModel::
bucketCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() == 0)
    return CQChartsVariant::fromInt(0);

  //---

  int row = currentRow();

  int col; (void) getColumnValue(cmdValues, col);

  if (col < 0 || col >= columnCount())
    return CQChartsVariant::fromInt(0);

  //---

  if (! checkIndex(row, col))
    return QVariant();

  //---

  auto var = getCmdData(row, col);

  //---

  auto p = columnDatas_.find(col);

  if (p == columnDatas_.end())
    return CQChartsVariant::fromInt(0);

  const auto &columnData = (*p).second;

  //---

  int bucket = -1;

  const auto &bucketer = columnData.bucketer;

  if      (CQChartsVariant::isReal(var)) {
    bool ok;

    double value = CQChartsVariant::toReal(var, ok);

    double start = 0;
    double delta = 1;

    if      (cmdValues.numValues() == 2) {
      double minVal = 0.0, maxVal = 0.0;

      columnRange(col, minVal, maxVal);

      start = minVal;

      (void) cmdValues.getReal(delta);
    }
    else if (cmdValues.numValues() == 3) {
      (void) cmdValues.getReal(start);
      (void) cmdValues.getReal(delta);
    }
    else {
      return CQChartsVariant::fromInt(-1);
    }

    auto bucketer1 = bucketer;

    bucketer1.setType(CQBucketer::Type::REAL_RANGE);

    bucketer1.setRStart(start);
    bucketer1.setRDelta(delta);

    bucket = bucketer1.realBucket(value);
  }
  else if (CQChartsVariant::isInt(var)) {
    return var;
  }
  else {
    auto str = var.toString();

    bucket = bucketer.stringBucket(str);
  }

  return CQChartsVariant::fromInt(bucket);
}

//---

// normalized column value
//   norm(col), norm(col, scale)
QVariant
CQChartsExprModel::
normCmd(const Values &values) const
{
  // get column and optional scale
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() == 0)
    return CQChartsVariant::fromReal(0.0);

  //---

  int col; (void) getColumnValue(cmdValues, col);

  if (col < 0 || col >= columnCount())
    return CQChartsVariant::fromReal(0.0);

  double scale = 1.0;

  (void) cmdValues.getReal(scale);

  //---

  // check index valid and get value
  int row = currentRow();

  if (! checkIndex(row, col))
    return QVariant();

  auto var = getCmdData(row, col);

  if (! var.isValid())
    return QVariant();

  //---

  if      (CQChartsVariant::isReal(var)) {
    bool ok;

    double value = CQChartsVariant::toReal(var, ok);

    double minVal = 0, maxVal = 1;

    columnRange(col, minVal, maxVal);

    if (cmdValues.numValues() <= 3)
      (void) cmdValues.getReal(maxVal);
    else {
      (void) cmdValues.getReal(minVal);
      (void) cmdValues.getReal(maxVal);
    }

    double d = maxVal - minVal;
    double s = 0.0;

    if (d > 0.0)
      s = scale*(value - minVal)/d;

    return CQChartsVariant::fromReal(s);
  }
  else if (CQChartsVariant::isInt(var)) {
    bool ok;

    auto value = CQChartsVariant::toInt(var, ok);

    long minVal = 0, maxVal = 1;

    columnRange(col, minVal, maxVal);

    if (cmdValues.numValues() <= 3)
      (void) cmdValues.getInt(maxVal);
    else {
      (void) cmdValues.getInt(minVal);
      (void) cmdValues.getInt(maxVal);
    }

    long   d = maxVal - minVal;
    double s = 0.0;

    if (d > 0)
      s = scale*double(value - minVal)/double(d);

    return CQChartsVariant::fromReal(s);
  }
  else {
    return CQChartsVariant::fromReal(0.0);
  }
}

//---

// scaled column value
//   scale(col, center=true, scale=true)
QVariant
CQChartsExprModel::
scaleCmd(const Values &values) const
{
  // get column and optional center, scale
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() == 0)
    return CQChartsVariant::fromReal(0.0);

  int col; (void) getColumnValue(cmdValues, col);

  if (col < 0 || col >= columnCount())
    return CQChartsVariant::fromReal(0.0);

  bool center = true;
  bool scale  = true;

  (void) cmdValues.getBool(center);
  (void) cmdValues.getBool(scale );

  //---

  // check index valid and get value
  int row = currentRow();

  if (! checkIndex(row, col))
    return CQChartsVariant::fromReal(0.0);

  auto var = getCmdData(row, col);

  if (! var.isValid())
    return CQChartsVariant::fromReal(0.0);

  bool ok;

  double value = CQChartsVariant::toReal(var, ok);

  //---

  auto ind = this->index(row, col, QModelIndex());

  CQChartsColumn column(ind.column());

  auto *modelData = getModelData();
  if (! modelData) return CQChartsVariant::fromReal(0.0);

  auto *details = modelData->details();
  assert(details);

  auto *columnDetails = details->columnDetails(column);
  assert(columnDetails);

  auto meanVar   = columnDetails->meanValue  (/*useNaN*/false);
  auto stddevVar = columnDetails->stdDevValue(/*useNaN*/false);

  if (! meanVar.isValid() || ! stddevVar.isValid())
    return CQChartsVariant::fromReal(0.0);

  //---

  double mean   = CQChartsVariant::toReal(meanVar  , ok);
  double stddev = CQChartsVariant::toReal(stddevVar, ok);

  double value1 = (stddev > 0.0 ? (value - mean)/stddev : 0.0);

  return CQChartsVariant::fromReal(value1);
}

//---

// random value
//   rand(min=0, max=1)
QVariant
CQChartsExprModel::
randCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  double min = 0.0;
  double max = 1.0;

  (void) cmdValues.getReal(min);
  (void) cmdValues.getReal(max);

  CQChartsRand::RealInRange rand(min, max);

  double r = rand.gen();

  return CQChartsVariant::fromReal(r);
}

//---

// random normalized value
//   rnorm(mean=0, stddev=1)
QVariant
CQChartsExprModel::
rnormCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  double mean   = 0.0;
  double stddev = 1.0;

  (void) cmdValues.getReal(mean);
  (void) cmdValues.getReal(stddev);

  CQChartsRand::NormalRealInRange rand(mean, stddev);

  double r = rand.gen();

  return CQChartsVariant::fromReal(r);
}

//---

#if 0
// key from values:
//   key(str1, str2, ...)
QVariant
CQChartsExprModel::
keyCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  QString s;

  QString key;

  while (cmdValues.getStr(s)) {
    if (s.size())
      key += ":" + s;
    else
      key = s;
  }

  return CQChartsVariant::fromString(key);
}
#endif

//---

// concat values
//   concat(str1, str2, ...)
QVariant
CQChartsExprModel::
concatCmd(const Values &values) const
{
  QString str;

  for (std::size_t i = 0; i < values.size(); ++i)
    str += values[i].toString();

  return CQChartsVariant::fromString(str);
}

//---

// match string to regexp
//   match(name, regexp)
QVariant
CQChartsExprModel::
matchCmd(const Values &values) const
{
  if (values.size() == 2) {
    auto str     = values[0].toString();
    auto pattern = values[1].toString();

    QRegExp regexp(pattern);

    return regexp.exactMatch(str);
  }

  return QVariant();
}

//---

// string to color
//   color(name)
QVariant
CQChartsExprModel::
colorCmd(const Values &values) const
{
  if (values.size() == 1) {
    auto c = CQChartsUtil::stringToColor(values[0].toString());

    return CQChartsVariant::fromColor(c);
  }

  return QVariant();
}

//---

// time value from column value
//   timeval(fmt)      - timeval fmt for current column
//   timeval(col, fmt) - timeval fmt for specified column
QVariant
CQChartsExprModel::
timevalCmd(const Values &values) const
{
  CQChartsExprCmdValues cmdValues(values);

  if (cmdValues.numValues() < 1)
    return QVariant();

  auto fmt = cmdValues.popValue().toString(); // last value

  //---

  int row = currentRow();

  int col; (void) getColumnValue(cmdValues, col);

  //---

  if (! checkIndex(row, col))
    return QVariant();

  //---

  auto *th = const_cast<CQChartsExprModel *>(this);

  auto ind = index(row, col, QModelIndex());

  bool ok;

  auto var = CQChartsModelUtil::modelValue(th, ind, Qt::EditRole, ok);
  if (! ok) return QVariant();

  CQChartsColumn column(col);

  bool converted;

  auto var1 = CQChartsModelUtil::columnUserData(charts_, th, column, var, converted);

  if (var1.isValid())
    var = var1;

  double t = CQChartsVariant::toReal(var1, ok);
  if (! ok) return QVariant();

  return CQChartsUtil::timeToString(fmt, t);
}

//---

bool
CQChartsExprModel::
getColumnValue(CQChartsExprCmdValues &cmdValues, int &col) const
{
  col = 0;

  long col1 = currentCol();

  if (! cmdValues.getInt(col1)) {
    QString name;

    if (! cmdValues.getStr(name))
      return false;

    auto p = nameColumns_.find(name);
    if (p == nameColumns_.end()) return false;

    col1 = (*p).second;
  }

  col = int(col1);

  return true;
}

bool
CQChartsExprModel::
getRowValue(CQChartsExprCmdValues &cmdValues, int &row) const
{
  row = 0;

  long row1 = currentRow();

  if (! cmdValues.getInt(row1))
    return false;

  row = int(row1);

  return true;
}

//---

QVariant
CQChartsExprModel::
getCmdData(int row, int col) const
{
  QModelIndex parent; // TODO

  auto ind = this->index(row, col, parent);

  auto var = this->data(ind, Qt::EditRole);

  if (! var.isValid())
    var = this->data(ind, Qt::DisplayRole);

  return var;
}

bool
CQChartsExprModel::
setCmdData(int row, int col, const QVariant &var)
{
  QModelIndex parent; // TODO

  auto ind = this->index(row, col, parent);

  bool b = this->setData(ind, var, Qt::EditRole);

  if (! b)
    b = this->setData(ind, var, Qt::DisplayRole);

  return b;
}

//---

bool
CQChartsExprModel::
evaluateExpression(const QString &expr, QVariant &value) const
{
  if (expr.length() == 0)
    return false;

  qtcl_->setRow   (currentRow());
  qtcl_->setColumn(currentCol());

  return qtcl_->evaluateExpression(expr, value, isDebug());
}

bool
CQChartsExprModel::
checkColumn(int col) const
{
  if (col < 0 || col >= columnCount()) return false;

  return true;
}

bool
CQChartsExprModel::
checkIndex(int row, int col) const
{
  if (row < 0 || row >= rowCount   ()) return false;
  if (col < 0 || col >= columnCount()) return false;

  return true;
}

QString
CQChartsExprModel::
replaceExprColumns(const QString &expr, int row, int column) const
{
  auto ind = this->index(row, column, QModelIndex());

  if (! ind.isValid())
    ind = this->index(row, 0, QModelIndex());

  return CQChartsModelUtil::replaceModelExprVars(expr, this, ind, nr_, nc_);
}

//---

bool
CQChartsExprModel::
getColumnRange(const QModelIndex &ind, double &rmin, double &rmax)
{
  CQChartsColumn column(ind.column());

  auto *modelData = getModelData();

  //---

  CQChartsModelTypeData columnTypeData;

  if (modelData) {
    auto *details = modelData->details();
    assert(details);

    auto *columnDetails = details->columnDetails(column);
    assert(columnDetails);

    columnTypeData.type       = columnDetails->type();
    columnTypeData.baseType   = columnDetails->baseType();
    columnTypeData.nameValues = columnDetails->nameValues();
  }
  else {
    if (! CQChartsModelUtil::columnValueType(charts_, this, column, columnTypeData))
      return false;
  }

  //---

  auto *columnTypeMgr = charts_->columnTypeMgr();

  const auto *typeData = columnTypeMgr->getType(columnTypeData.type);

  const auto *rtypeData = dynamic_cast<const CQChartsColumnRealType    *>(typeData);
  const auto *itypeData = dynamic_cast<const CQChartsColumnIntegerType *>(typeData);

  if      (rtypeData) {
    if (! rtypeData->rmin(columnTypeData.nameValues, rmin)) {
      if (modelData) {
        bool ok;

        rmin = CQChartsVariant::toReal(
          modelData->details()->columnDetails(CQChartsColumn(ind.column()))->minValue(), ok);
      }
    }

    if (! rtypeData->rmax(columnTypeData.nameValues, rmax)) {
      if (modelData) {
        bool ok;

        rmax = CQChartsVariant::toReal(
          modelData->details()->columnDetails(CQChartsColumn(ind.column()))->maxValue(), ok);
      }
    }
  }
  else if (itypeData) {
    long imin = 0, imax = 0;

    if (! itypeData->imin(columnTypeData.nameValues, imin)) {
      if (modelData) {
        bool ok;

        imin = CQChartsVariant::toInt(
          modelData->details()->columnDetails(CQChartsColumn(ind.column()))->minValue(), ok);
      }
    }

    if (! itypeData->imax(columnTypeData.nameValues, imax)) {
      if (modelData) {
        bool ok;

        imax = CQChartsVariant::toInt(
          modelData->details()->columnDetails(CQChartsColumn(ind.column()))->maxValue(), ok);
      }
    }

    rmin = double(imin);
    rmax = double(imax);
  }
  else {
    return false;
  }

  return true;
}

//---

CQChartsModelData *
CQChartsExprModel::
getModelData() const
{
  return charts_->getModelData(this->filter_);
}

//---

void
CQChartsExprModel::
dataChangedSlot(const QModelIndex &from, const QModelIndex &to)
{
  emit dataChanged(mapFromSource(from), mapFromSource(to));
}
