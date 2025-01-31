#include <CQChartsLoader.h>
#include <CQChartsInputData.h>
#include <CQChartsFileType.h>
#include <CQChartsModelExprMatch.h>
#include <CQChartsFilterModel.h>
#include <CQChartsVarsModel.h>
#include <CQChartsTclModel.h>
#include <CQChartsCorrelationModel.h>
#include <CQChartsExprDataModel.h>
#include <CQChartsModelUtil.h>
#include <CQChartsColumnType.h>
#include <CQChartsFile.h>
#include <CQCharts.h>
#include <CQChartsVariant.h>

#include <CQCsvModel.h>
#include <CQTsvModel.h>
#include <CQGnuDataModel.h>
#include <CQJsonModel.h>
#ifdef CQCHARTS_EXCEL
#include <CQExcelModel.h>
#endif

#include <CQPerfMonitor.h>
#include <CQTclUtil.h>
#include <CMathCorrelation.h>

CQChartsLoader::
CQChartsLoader(CQCharts *charts) :
 charts_(charts)
{
}

void
CQChartsLoader::
setQtcl(CQTcl *qtcl)
{
  qtcl_ = qtcl;
}

QAbstractItemModel *
CQChartsLoader::
loadFile(const CQChartsFile &file, CQChartsFileType type, const InputData &inputData,
         bool &hierarchical)
{
  CQPerfTrace trace("CQChartsLoader::loadFile");

  hierarchical = false;

  if      (type == CQChartsFileType::CSV) {
    auto *csv = loadCsv(file, inputData);

    if (! csv) {
      charts_->errorMsg("Failed to load '" + file.resolve() + "'");
      return nullptr;
    }

    return csv;
  }
  else if (type == CQChartsFileType::TSV) {
    auto *tsv = loadTsv(file, inputData);

    if (! tsv) {
      charts_->errorMsg("Failed to load '" + file.resolve() + "'");
      return nullptr;
    }

    return tsv;
  }
  else if (type == CQChartsFileType::JSON) {
    auto *json = loadJson(file, inputData);

    if (! json) {
      charts_->errorMsg("Failed to load '" + file.resolve() + "'");
      return nullptr;
    }

    auto *jsonModel = qobject_cast<CQJsonModel *>(json->baseModel());
    assert(jsonModel);

    hierarchical = jsonModel->isHierarchical();

    return json;
  }
  else if (type == CQChartsFileType::DATA) {
    auto *data = loadData(file, inputData);

    if (! data) {
      charts_->errorMsg("Failed to load '" + file.resolve() + "'");
      return nullptr;
    }

    return data;
  }
  else if (type == CQChartsFileType::EXPR) {
    auto *model = createExprModel(inputData.numRows);

    if (! model) {
      charts_->errorMsg("Failed to load '" + file.resolve() + "'");
      return nullptr;
    }

    return model;
  }
  else if (type == CQChartsFileType::VARS) {
    auto *model = createVarsModel(inputData);

    if (! model) {
      charts_->errorMsg("Failed to load '" + file.resolve() + "'");
      return nullptr;
    }

    return model;
  }
  else if (type == CQChartsFileType::TCL) {
    auto *model = createTclModel(inputData);

    if (! model) {
      charts_->errorMsg("Failed to load '" + file.resolve() + "'");
      return nullptr;
    }

    return model;
  }
  else {
    charts_->errorMsg("Bad file type specified '" +
                      CQChartsFileTypeUtil::fileTypeToString(type) + "'");
    return nullptr;
  }
}

CQChartsFilterModel *
CQChartsLoader::
loadCsv(const CQChartsFile &file, const InputData &inputData)
{
  CQPerfTrace trace("CQChartsLoader::loadCsv");

  auto *csvModel = new CQCsvModel;

  csvModel->setCommentHeader    (inputData.commentHeader);
  csvModel->setFirstLineHeader  (inputData.firstLineHeader);
  csvModel->setFirstColumnHeader(inputData.firstColumnHeader);

  if (inputData.separator.length())
    csvModel->setSeparator(inputData.separator[0].toLatin1());

  if (inputData.maxRows > 0)
    csvModel->setMaxRows(inputData.maxRows);

  if (inputData.columns.length() > 0)
    csvModel->setColumns(inputData.columns);

  if (! csvModel->load(file.resolve())) {
    delete csvModel;
    return nullptr;
  }

  //---

  bool isExprModel = true;

  auto *dataModel = dynamic_cast<CQDataModel *>(csvModel);

#ifdef CQCHARTS_EXCEL
  if (inputData.spreadsheet) {
    auto *spreadsheetModel = new CQExcel::Model;

    spreadsheetModel->copyModel(dataModel);

    dataModel = spreadsheetModel;

    isExprModel = false;
  }
#endif

  auto *filterModel = new CQChartsFilterModel(charts_, dataModel, isExprModel);
  filterModel->setObjectName("csvFilterModel");

  setFilter(filterModel, inputData);

  return filterModel;
}

CQChartsFilterModel *
CQChartsLoader::
loadTsv(const CQChartsFile &file, const InputData &inputData)
{
  CQPerfTrace trace("CQChartsLoader::loadTsv");

  auto *tsvModel = new CQTsvModel;

  auto *tsv = new CQChartsFilterModel(charts_, tsvModel);
  tsv->setObjectName("tsvFilterModel");

  tsvModel->setCommentHeader    (inputData.commentHeader);
  tsvModel->setFirstLineHeader  (inputData.firstLineHeader);
  tsvModel->setFirstColumnHeader(inputData.firstColumnHeader);

  if (inputData.columns.length() > 0)
    tsvModel->setColumns(inputData.columns);

  if (! tsvModel->load(file.resolve())) {
    delete tsv;
    return nullptr;
  }

  //---

  setFilter(tsv, inputData);

  return tsv;
}

CQChartsFilterModel *
CQChartsLoader::
loadJson(const CQChartsFile &file, const InputData &inputData)
{
  CQPerfTrace trace("CQChartsLoader::loadJson");

  auto *jsonModel = new CQJsonModel;

  jsonModel->setAllowSingleQuote(true);

  auto *json = new CQChartsFilterModel(charts_, jsonModel, /*exprModel*/false);
  json->setObjectName("jsonFilterModel");

  if (! jsonModel->load(file.resolve())) {
    delete json;
    return nullptr;
  }

  //---

  setFilter(json, inputData);

  return json;
}

CQChartsFilterModel *
CQChartsLoader::
loadData(const CQChartsFile &file, const InputData &inputData)
{
  CQPerfTrace trace("CQChartsLoader::loadData");

  auto *dataModel = new CQGnuDataModel;

  auto *data = new CQChartsFilterModel(charts_, dataModel);
  data->setObjectName("dataFilterModel");

  dataModel->setCommentHeader    (inputData.commentHeader);
  dataModel->setFirstLineHeader  (inputData.firstLineHeader);
  dataModel->setFirstColumnHeader(inputData.firstColumnHeader);

  if (inputData.columns.length() > 0)
    dataModel->setColumns(inputData.columns);

  if (! dataModel->load(file.resolve())) {
    delete data;
    return nullptr;
  }

  //---

  setFilter(data, inputData);

  return data;
}

CQChartsFilterModel *
CQChartsLoader::
createExprModel(int n)
{
  CQPerfTrace trace("CQChartsLoader::createExprModel");

  auto *dataModel = new CQChartsExprDataModel(n);

  QModelIndex parent;

  for (int r = 0; r < n; ++r) {
    auto ind = dataModel->index(r, 0, parent);

    dataModel->setData(ind, CQChartsVariant::fromInt(r));
  }

  auto *data = new CQChartsFilterModel(charts_, dataModel);
  data->setObjectName("exprFilterModel");

  //---

  //setFilter(data, inputData);

  return data;
}

CQChartsFilterModel *
CQChartsLoader::
createVarsModel(const InputData &inputData)
{
  CQPerfTrace trace("CQChartsLoader::createVarsModel");

  using ColumnValues = std::vector<QVariant>;
  using VarColumns   = std::vector<ColumnValues>;

  VarColumns varColumns;

  //---

  QStringList varNames;

  for (const auto &var : inputData.vars)
    varNames << var.toString();

  //---

  int nr = -1;

  int nv = varNames.length();

  if (nv == 1) {
    auto varName = varNames[0];

    ColumnValues columnValues;

    if (qtcl_)
      columnValues = qtcl_->getListVar(varName);

    if (! inputData.transpose) {
      auto nc = columnValues.size();

      for (size_t c = 0; c < nc; ++c) {
        ColumnValues columnValues1;

        if (CQChartsVariant::isList(columnValues[c])) {
          auto rowVars = columnValues[c].toList();

          for (int i = 0; i < rowVars.length(); ++i)
            columnValues1.push_back(rowVars[i]);
        }
        else {
          columnValues1.push_back(columnValues[c]);
        }

        auto nv1 = columnValues1.size();

        if (nr < 0)
          nr = int(nv1);
        else
          nr = std::min(nr, int(nv1));

        varColumns.push_back(columnValues1);
      }
    }
    else {
      using IndColumnValues = std::map<int, ColumnValues>;

      IndColumnValues indColumnValues;

      nr = int(columnValues.size());

      for (uint r = 0; r < uint(nr); ++r) {
        if (CQChartsVariant::isList(columnValues[r])) {
          auto columnVars = columnValues[r].toList();

          for (int c = 0; c < columnVars.length(); ++c)
            indColumnValues[c].push_back(columnVars[c]);
        }
        else {
          indColumnValues[0].push_back(columnValues[r]);
        }
      }

      nr = 0;

      for (auto &i : indColumnValues) {
        auto &columnValues = i.second;

        nr = std::max(nr, int(columnValues.size()));
      }

      for (auto &i : indColumnValues) {
        auto &columnValues = i.second;

        while (int(columnValues.size()) < nr)
          columnValues.emplace_back();
      }

      for (auto &i : indColumnValues) {
        const auto &columnValues = i.second;

        varColumns.push_back(columnValues);
      }
    }
  }
  else {
    for (int i = 0; i < nv; ++i) {
      auto varName = varNames[i];

      ColumnValues columnValues;

      if (qtcl_)
        columnValues = qtcl_->getListVar(varName);

      auto nv1 = columnValues.size();

      if (nr < 0)
        nr = int(nv1);
      else
        nr = std::min(nr, int(nv1));

      varColumns.push_back(columnValues);
    }
  }

  if (nr < 0)
    return nullptr;

  //---

  int ic = 0, ir = 0;

  if (inputData.firstColumnHeader) ++ic;
  if (inputData.firstLineHeader  ) ++ir;

  //---

  auto nc = varColumns.size();

  int nc1 = int(nc) - ic;
  int nr1 = int(nr) - ir;

  if (nc1 < 0 || nr1 < 0)
    return nullptr;

  auto *varsModel = new CQChartsVarsModel(nc1, nr1);

  auto *filterModel = new CQChartsFilterModel(charts_, varsModel);
  filterModel->setObjectName("varsFilterModel");

  QModelIndex parent;

  if (inputData.firstColumnHeader) {
    const auto &columnValues = varColumns[0];

    for (uint r = uint(ir); r < uint(nr); ++r)
      varsModel->setHeaderData(int(r - uint(ir)), Qt::Vertical, columnValues[r]);
  }

  if (inputData.firstLineHeader) {
    for (uint c = uint(ic); c < uint(nc); ++c) {
      const auto &columnValues = varColumns[c];

      varsModel->setHeaderData(int(c - uint(ic)), Qt::Horizontal, columnValues[0]);
    }
  }

  for (uint c = uint(ic); c < nc; ++c) {
    const auto &columnValues = varColumns[c];

    for (uint r = uint(ir); r < uint(nr); ++r) {
      auto ind = varsModel->index(int(r - uint(ir)), int(c - uint(ic)), parent);

      varsModel->setData(ind, columnValues[r]);
    }
  }

  varsModel->setTranspose        (inputData.transpose);
  varsModel->setFirstColumnHeader(inputData.firstColumnHeader);
  varsModel->setFirstLineHeader  (inputData.firstLineHeader);

  varsModel->setVarNames(varNames);

  //---

  //setFilter(filterModel, inputData);

  return filterModel;
}

CQChartsFilterModel *
CQChartsLoader::
createTclModel(const InputData &inputData)
{
  CQPerfTrace trace("CQChartsLoader::createTclModel");

  int nr = 0, nc = 0;

  using ColumnStrs = std::vector<QStringList>;

  ColumnStrs columnStrs;

  if (! inputData.transpose) {
    // var per column
    for (const auto &var : inputData.vars) {
      auto strs = var.toStringList();

      nr = std::max(nr, strs.length());
    }

    nc = int(inputData.vars.size());

    for (const auto &var : inputData.vars) {
      auto strs = var.toStringList();

      while (strs.length() < nr)
        strs << "";

      columnStrs.push_back(strs);
    }
  }
  else {
    // var per row
    for (const auto &var : inputData.vars) {
      auto strs = var.toStringList();

      nc = std::max(nc, strs.length());
    }

    nr = int(inputData.vars.size());

    columnStrs.resize(size_t(nc));

    for (const auto &var : inputData.vars) {
      auto strs = var.toStringList();

      while (strs.length() < nc)
        strs << "";

      for (uint i = 0; i < uint(nc); ++i)
        columnStrs[i].push_back(strs[int(i)]);
    }
  }

  //---

  int ic = 0, ir = 0;

  if (inputData.firstColumnHeader) ++ic;
  if (inputData.firstLineHeader  ) ++ir;

  int nc1 = nc - ic;
  int nr1 = nr - ir;

  if (nc1 < 0 || nr1 < 0)
    return nullptr;

  //---

  auto *tclModel = new CQChartsTclModel(nc1, nr1);

  auto *filterModel = new CQChartsFilterModel(charts_, tclModel);
  filterModel->setObjectName("tclFilterModel");

  QModelIndex parent;

  for (uint c = uint(ic); c < uint(nc); ++c) {
    const auto &strs = columnStrs[c];

    for (uint r = uint(ir); r < uint(nr); ++r) {
      auto ind = tclModel->index(int(r - uint(ir)), int(c - uint(ic)), parent);

      tclModel->setData(ind, strs[int(r)]);
    }
  }

  //---

  if (inputData.firstColumnHeader) {
    const auto &strs = columnStrs[0];

    for (uint r = uint(ir); r < uint(nr); ++r)
      tclModel->setHeaderData(int(r - uint(ir)), Qt::Vertical, strs[int(r)]);
  }

  if (inputData.firstLineHeader) {
    for (uint c = uint(ic); c < uint(nc); ++c) {
      const auto &strs = columnStrs[c];

      tclModel->setHeaderData(int(c - uint(ic)), Qt::Horizontal, strs[0]);
    }
  }

  //---

  //setFilter(filterModel, inputData);

  return filterModel;
}

CQChartsFilterModel *
CQChartsLoader::
createCorrelationModel(QAbstractItemModel *model, const CorrelationData &correlationData)
{
  CQPerfTrace trace("CQChartsLoader::createCorrelationModel");

  int nr = model->rowCount   ();
  int nc = model->columnCount();

  int nv = 0;

  auto *columnTypeMgr = charts_->columnTypeMgr();

  using ColumnValues = std::vector<CMathCorrelation::Values>;
  using ColumnNames  = std::vector<QString>;
  using RMinMax      = CQChartsGeom::RMinMax;
  using ColumnMinMax = std::vector<RMinMax>;

  ColumnValues columnValues;
  ColumnNames  columnNames;
  ColumnMinMax columnMinMax;

  using ColumnSet = std::set<CQChartsColumn>;

  ColumnSet inputColumnSet;

  for (auto &c : correlationData.columns)
    inputColumnSet.insert(c);

  ColumnSet columnSet;

  if (! correlationData.flip) {
    for (int ic = 0; ic < nc; ++ic) {
      CQChartsColumn c(ic);

      if (! inputColumnSet.empty() && inputColumnSet.find(c) == inputColumnSet.end())
        continue;

      CQChartsModelTypeData typeData;

      if (! columnTypeMgr->getModelColumnType(model, c, typeData))
        typeData.type = CQBaseModelType::STRING;

      if (typeData.type == CQBaseModelType::INTEGER || typeData.type == CQBaseModelType::REAL)
        columnSet.insert(c);
    }

    //---

    auto nc1 = columnSet.size();

    nv = int(nc1);

    columnValues.resize(size_t(nv));
    columnNames .resize(size_t(nv));
    columnMinMax.resize(size_t(nv));

    //---

    size_t ic = 0;

    for (const auto &c : columnSet) {
      bool ok;
      columnNames[ic] = CQChartsModelUtil::columnToString(model, c, ok);

      auto &values = columnValues[ic];
      auto &minMax = columnMinMax[ic];

      values.resize(size_t(nr));

      for (int ir = 0; ir < nr; ++ir) {
        auto ind = model->index(ir, c.column(), QModelIndex());

        bool ok;

        double v = CQChartsModelUtil::modelReal(model, ind, ok);

        values[size_t(ir)] = v;

        minMax.add(v);
      }

      ++ic;
    }
  }
  else {
    nv = nr;

    columnValues.resize(size_t(nv));
    columnNames .resize(size_t(nv));
    columnMinMax.resize(size_t(nv));

    //---

    for (size_t ir = 0; ir < size_t(nr); ++ir) {
      bool ok;

      columnNames[ir] = CQChartsModelUtil::modelVHeaderString(model, int(ir), ok);

      auto &values = columnValues[ir];
      auto &minMax = columnMinMax[ir];

      values.resize(size_t(nc));

      for (size_t ic = 0; ic < size_t(nc); ++ic) {
        auto ind = model->index(int(ir), int(ic), QModelIndex());

        bool ok;

        double v = CQChartsModelUtil::modelReal(model, ind, ok);

        values[ic] = v;

        minMax.add(v);
      }
    }
  }

  //---

  // calc off diagonal values
  using ColumnReal          = std::map<int, double>;
  using ColumnColumnReal    = std::map<int, ColumnReal>;
  using ColumnPoints        = std::map<int, CQChartsCorrelationModel::Points>;
  using ColumnColumnPoints  = std::map<int, ColumnPoints>;
  using DevData             = std::pair<double, double>;
  using ColumnDevData       = std::map<int, DevData>;
  using ColumnColumnDevData = std::map<int, ColumnDevData>;
  using ColumnDensity       = std::map<int, CQChartsDensity *>;

  ColumnReal          columnSumSq;
  ColumnColumnReal    columnsCorr;
  ColumnColumnPoints  columnsPoints;
  ColumnColumnDevData columnsDevData;
  ColumnDensity       columnDensity;

  for (int ic1 = 0; ic1 < nv; ++ic1) {
    CQChartsColumn c1(ic1);

    auto &values1 = columnValues[size_t(ic1)];

    for (int ic2 = ic1; ic2 < nv; ++ic2) {
      if (ic1 != ic2) {
        CQChartsColumn c2(ic2);

        auto &values2 = columnValues[size_t(ic2)];

        double corr = CMathCorrelation::calc(values1, values2);

        columnsCorr[ic1][ic2] = corr;
        columnsCorr[ic2][ic1] = corr;

        columnSumSq[ic1] += corr*corr;
        columnSumSq[ic2] += corr*corr;

        CQChartsCorrelationModel::Points points1, points2;

        assert(values1.size() == values2.size());

        auto nv1 = values1.size();

        for (size_t j = 0; j < size_t(nv1); ++j) {
          points1.emplace_back(values1[j], values2[j]);
          points2.emplace_back(values2[j], values1[j]);
        }

        columnsPoints[ic1][ic2] = points1;
        columnsPoints[ic2][ic1] = points2;

        double stddev1 = CMathCorrelation::stddev(values1);
        double stddev2 = CMathCorrelation::stddev(values2);

        columnsDevData[ic1][ic2] = DevData(stddev1, stddev2);
        columnsDevData[ic2][ic1] = DevData(stddev2, stddev1);
      }
      else {
        CQChartsCorrelationModel::Points points1;

        auto nv1 = values1.size();

        CQChartsDensity::XVals xvals;

        for (size_t j = 0; j < size_t(nv1); ++j)
          xvals.push_back(values1[j]);

        if (! columnDensity[ic1])
          columnDensity[ic1] = new CQChartsDensity;

        columnDensity[ic1]->setXVals(xvals);
      }
    }
  }

  //---

  // sort by sum of squares
  using ColumnNums = std::vector<int>;
  using SumColumns = std::map<double, ColumnNums>;

  SumColumns sumColumns;

  for (const auto &pc : columnSumSq) {
    sumColumns[pc.second].push_back(pc.first);
  }

  ColumnNums sortedColumns;

  for (const auto &ps : sumColumns)
    for (const auto &c : ps.second)
      sortedColumns.push_back(c);

  assert(int(sortedColumns.size()) == nv);

  //---

  // create model
  auto *correlationModel = new CQChartsCorrelationModel(nv);

  auto *filterModel = new CQChartsFilterModel(charts_, correlationModel);
  filterModel->setObjectName("correlationFilterModel");

  for (int ic = 0; ic < nv; ++ic) {
    CQChartsColumn c(ic);

    (void) columnTypeMgr->setModelColumnType(correlationModel, c, CQBaseModelType::REAL);
  }

  //---

  // set header values and diagonal values
  for (int ic = 0; ic < nv; ++ic) {
    int ics = sortedColumns[size_t(ic)];

    CQChartsColumn c(ic);

    auto columnName = columnNames[size_t(ics)];

    CQChartsModelUtil::setModelHeaderValue(correlationModel, ic, Qt::Horizontal,
                                           columnName, Qt::DisplayRole);
    CQChartsModelUtil::setModelHeaderValue(correlationModel, ic, Qt::Vertical  ,
                                           columnName, Qt::DisplayRole);

    CQChartsModelUtil::setModelValue(correlationModel, ic, c, QModelIndex(), 1.0);
  }

  //---

  // set off diagonal values
  for (int ic1 = 0; ic1 < nv; ++ic1) {
    int ic1s = sortedColumns[size_t(ic1)];

    CQChartsColumn c1(ic1);

    auto &minMax = columnMinMax[size_t(ic1s)];

    correlationModel->setMinMax(ic1, minMax);

    for (int ic2 = ic1; ic2 < nv; ++ic2) {
      if (ic1 != ic2) {
        int ic2s = sortedColumns[size_t(ic2)];

        CQChartsColumn c2(ic2);

        double corr = columnsCorr[ic1s][ic2s];

        CQChartsModelUtil::setModelValue(correlationModel, ic1, c2, QModelIndex(), corr);
        CQChartsModelUtil::setModelValue(correlationModel, ic2, c1, QModelIndex(), corr);

        correlationModel->setPoints(ic1, ic2, columnsPoints[ic1s][ic2s]);
        correlationModel->setPoints(ic2, ic1, columnsPoints[ic2s][ic1s]);

        correlationModel->setDevData(ic1, ic2,
          columnsDevData[ic1s][ic2s].first, columnsDevData[ic1s][ic2s].second);
        correlationModel->setDevData(ic2, ic1,
          columnsDevData[ic2s][ic1s].first, columnsDevData[ic2s][ic1s].second);
      }
      else {
        CQChartsModelUtil::setModelValue(correlationModel, ic1, c1, QModelIndex(), 1.0);
        CQChartsModelUtil::setModelValue(correlationModel, ic1, c1, QModelIndex(), 1.0); // Dup OK

        correlationModel->setDensity(ic1, columnDensity[ic1s]);

        correlationModel->setDevData(ic1, ic1, 0.0, 0.0);
      }
    }
  }

  //---

  //setFilter(filterModel, inputData);

  return filterModel;
}

void
CQChartsLoader::
setFilter(ModelFilter *model, const InputData &inputData)
{
  if (! inputData.filter.length())
    return;

  if      (inputData.filterType == CQChartsModelFilterData::Type::EXPRESSION)
    model->setExpressionFilter(inputData.filter);
  else if (inputData.filterType == CQChartsModelFilterData::Type::REGEXP)
    model->setRegExpFilter(inputData.filter);
  else if (inputData.filterType == CQChartsModelFilterData::Type::WILDCARD)
    model->setWildcardFilter(inputData.filter);
  else if (inputData.filterType == CQChartsModelFilterData::Type::SIMPLE)
    model->setSimpleFilter(inputData.filter);
}
