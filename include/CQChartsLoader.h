#ifndef CQChartsLoader_H
#define CQChartsLoader_H

#include <CQChartsFileType.h>
#include <CQChartsColumn.h>
#include <QVariant>
#include <vector>

class CQCharts;
class CQChartsFilterModel;
class CQChartsModelFilter;

struct CQChartsInputData;

class CQTcl;

class QAbstractItemModel;

/*!
 * \brief model loader class
 * \ingroup Charts
 */
class CQChartsLoader {
 public:
  using InputData   = CQChartsInputData;
  using ModelFilter = CQChartsModelFilter;
  using FileType    = CQChartsFileType;
  using FilterModel = CQChartsFilterModel;

 public:
  CQChartsLoader(CQCharts *charts);

  void setQtcl(CQTcl *qtcl);

  QAbstractItemModel *loadFile(const QString &filename, FileType type,
                               const InputData &inputData, bool &hierarchical);

  FilterModel *loadCsv (const QString &filename, const InputData &inputData);
  FilterModel *loadTsv (const QString &filename, const InputData &inputData);
  FilterModel *loadJson(const QString &filename, const InputData &inputData);
  FilterModel *loadData(const QString &filename, const InputData &inputData);

  FilterModel *createExprModel(int n);

  FilterModel *createVarsModel(const InputData &inputData);

  FilterModel *createTclModel(const InputData &inputData);

  //---

  using Columns = std::vector<CQChartsColumn>;

  struct CorrelationData {
    CorrelationData() { }

    bool    flip { false };
    Columns columns;
  };

  FilterModel *createCorrelationModel(QAbstractItemModel *model,
                                      const CorrelationData &correlationData=CorrelationData());

 private:
  void setFilter(ModelFilter *model, const InputData &inputData);

 private:
  CQCharts* charts_ { nullptr };
  CQTcl*    qtcl_   { nullptr };
};

#endif
