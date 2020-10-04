#ifndef CQChartsCmds_H
#define CQChartsCmds_H

#include <CQChartsCmdBase.h>
#include <CQChartsCmdsSlot.h>

#include <CQCharts.h>
#include <CQChartsInitData.h>
#include <CQChartsGeom.h>

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QPointer>

class CQChartsWindow;
class CQChartsView;
class CQChartsPlot;
class CQChartsGradientPalette;
class CQChartsColumn;
class CQChartsModelData;
class CQChartsPlotType;
class CQChartsAnnotation;

class CQChartsCmds;

class CQTcl;

class QAbstractItemModel;
class QItemSelectionModel;

//---

/*!
 * \brief Charts Tcl Commands
 * \ingroup Charts
 */
class CQChartsCmds : public QObject {
  Q_OBJECT

 public:
  using Vars   = std::vector<QVariant>;
  using ModelP = QSharedPointer<QAbstractItemModel>;
  using ViewP  = QPointer<CQChartsView>;
  using Plots  = std::vector<CQChartsPlot *>;

 private:
  using OptReal = boost::optional<double>;

 public:
  CQChartsCmds(CQCharts *charts);
 ~CQChartsCmds();

  CQCharts *charts() const { return charts_; }

  CQChartsCmdBase *cmdBase() { return cmdBase_; }

  void addCommands();

  void addCommand(const QString &name, CQChartsCmdProc *proc);

  static QString fixTypeName(const QString &typeName);

  //void setViewProperties(CQChartsView *view, const QString &properties);
  //void setPlotProperties(CQChartsPlot *plot, const QString &properties);

  bool setAnnotationProperties(CQChartsAnnotation *annotation, const QString &properties);

  //---

  bool loadFileModel(const QString &filename, CQChartsFileType type,
                     const CQChartsInputData &inputData);

  //---

  bool sortModel(ModelP &model, const QString &args);
  bool sortModel(ModelP &model, int column, Qt::SortOrder order);

  //---

  CQChartsPlot *createPlot(CQChartsView *view, const ModelP &model,
                           CQChartsPlotType *type, bool reuse);

  bool initPlot(CQChartsPlot *plot, const CQChartsNameValueData &nameValueData,
                const CQChartsGeom::BBox &bbox);

  //---

  CQChartsModelData *getModelDataOrCurrent(const QString &id);

  CQChartsModelData *getModelData(const QString &id);

  //---

  CQChartsView *getView(bool reuse=true);

  CQChartsView *addView();

  //---

  QStringList modelArgValues() const;
  QStringList viewArgValues() const;
  QStringList plotArgValues(CQChartsView *view) const;
  QStringList plotTypeArgValues() const;
  QStringList annotationArgValues(CQChartsView *view, CQChartsPlot *plot) const;
  QStringList roleArgValues() const;

  //---

  static bool stringToColumn(QAbstractItemModel *model, const QString &str,
                             CQChartsColumn &column);

 public:
  bool createChartsCorrelationModelCmd(CQChartsCmdArgs &args);
  bool createChartsFoldedModelCmd     (CQChartsCmdArgs &args);
  bool createChartsBucketModelCmd     (CQChartsCmdArgs &args);
  bool createChartsSubsetModelCmd     (CQChartsCmdArgs &args);
  bool createChartsTransposeModelCmd  (CQChartsCmdArgs &args);
  bool createChartsCollapseModelCmd   (CQChartsCmdArgs &args);
  bool createChartsPivotModelCmd      (CQChartsCmdArgs &args);
  bool createChartsStatsModelCmd      (CQChartsCmdArgs &args);
  bool createChartsDataModelCmd       (CQChartsCmdArgs &args);

  bool addChartsKeyItemCmd(CQChartsCmdArgs &args);

  bool printChartsImageCmd(CQChartsCmdArgs &args);
  bool writeChartsDataCmd (CQChartsCmdArgs &args);

  bool showChartsLoadModelDlgCmd   (CQChartsCmdArgs &args);
  bool showChartsManageModelsDlgCmd(CQChartsCmdArgs &args);
  bool showChartsCreatePlotDlgCmd  (CQChartsCmdArgs &args);
  bool showChartsTextDlgCmd        (CQChartsCmdArgs &args);
  bool showChartsHelpDlgCmd        (CQChartsCmdArgs &args);

  bool testEditCmd(CQChartsCmdArgs &args);

  //---

  QAbstractItemModel *loadFile(const QString &filename, CQChartsFileType type,
                               const CQChartsInputData &inputData, bool &hierarchical);

  QAbstractItemModel *loadCsv (const QString &filename, const CQChartsInputData &inputData);
  QAbstractItemModel *loadTsv (const QString &filename, const CQChartsInputData &inputData);
  QAbstractItemModel *loadJson(const QString &filename, bool &hierarchical);
  QAbstractItemModel *loadData(const QString &filename, const CQChartsInputData &inputData);

  //---

  bool getViewPlotArg(CQChartsCmdArgs &argv, CQChartsView* &view, CQChartsPlot* &plot);

  CQChartsView *getViewByName(const QString &viewName) const;

  bool getPlotsByName(CQChartsView *view, const Vars &plotNames, Plots &plot) const;

  CQChartsPlot *getOptPlotByName(CQChartsView *view, const QString &name) const;

  CQChartsPlot *getPlotByName(CQChartsView *view, const QString &name) const;

  CQChartsAnnotation *getAnnotationByName(const QString &name) const;

  //---

  bool setAnnotationArgProperties(CQChartsCmdArgs &argv, CQChartsAnnotation *annotation);

  //---

  QStringList stringToNamedColumns(const QString &str) const;

 private:
  CQCharts*        charts_  { nullptr };
  CQChartsCmdBase* cmdBase_ { nullptr };
};

//---

/*!
 * \brief Charts Tcl Command
 * \ingroup Charts
 */
#define CQCHARTS_DEF_PROC_CMD(NAME, PROC) \
class CQCharts##NAME##Cmd : public CQChartsCmdProc { \
 public: \
  CQCharts##NAME##Cmd(CQChartsCmds *cmds) : CQChartsCmdProc(cmds->cmdBase()), cmds_(cmds) { } \
\
  bool exec(CQChartsCmdArgs &args) override { return cmds_->PROC(args); } \
\
 private: \
  CQChartsCmds* cmds_ { nullptr }; \
};

#define CQCHARTS_DEF_CMD(NAME) \
class CQCharts##NAME##Cmd : public CQChartsCmdProc { \
 public: \
  CQCharts##NAME##Cmd(CQChartsCmds *cmds) : CQChartsCmdProc(cmds->cmdBase()), cmds_(cmds) { } \
\
  bool exec(CQChartsCmdArgs &args) override; \
\
  void addArgs(CQChartsCmdArgs &args) override; \
\
  QStringList getArgValues(const QString &arg, \
                           const NameValueMap &nameValueMap=NameValueMap()) override; \
\
  CQChartsCmds *cmds() const { return cmds_; } \
\
  CQCharts *charts() const { return cmds_->charts(); } \
\
 private: \
  CQChartsCmds* cmds_ { nullptr }; \
};

//---

CQCHARTS_DEF_CMD(LoadChartsModel)
CQCHARTS_DEF_CMD(ProcessChartsModel)

CQCHARTS_DEF_CMD(DefineChartsProc)

CQCHARTS_DEF_CMD(SortChartsModel)
CQCHARTS_DEF_CMD(FoldChartsModel)
CQCHARTS_DEF_CMD(FilterChartsModel)
CQCHARTS_DEF_CMD(FlattenChartsModel)
CQCHARTS_DEF_CMD(CopyChartsModel)
CQCHARTS_DEF_CMD(JoinChartsModel)
CQCHARTS_DEF_CMD(GroupChartsModel)
CQCHARTS_DEF_CMD(WriteChartsModel)

CQCHARTS_DEF_CMD(RemoveChartsModel)

//---

CQCHARTS_DEF_PROC_CMD(CreateChartsCorrelationModel, createChartsCorrelationModelCmd)
CQCHARTS_DEF_PROC_CMD(CreateChartsFoldedModel     , createChartsFoldedModelCmd)
CQCHARTS_DEF_PROC_CMD(CreateChartsBucketModel     , createChartsBucketModelCmd)
CQCHARTS_DEF_PROC_CMD(CreateChartsSubsetModel     , createChartsSubsetModelCmd)
CQCHARTS_DEF_PROC_CMD(CreateChartsTransposeModel  , createChartsTransposeModelCmd)
CQCHARTS_DEF_CMD     (CreateChartsSummaryModel)
CQCHARTS_DEF_PROC_CMD(CreateChartsCollapseModel   , createChartsCollapseModelCmd)
CQCHARTS_DEF_PROC_CMD(CreateChartsPivotModel      , createChartsPivotModelCmd)
CQCHARTS_DEF_PROC_CMD(CreateChartsStatsModel      , createChartsStatsModelCmd)
CQCHARTS_DEF_PROC_CMD(CreateChartsDataModel       , createChartsDataModelCmd)

//---

CQCHARTS_DEF_CMD(ExportChartsModel)

//---

CQCHARTS_DEF_CMD(GroupChartsPlots)
CQCHARTS_DEF_CMD(PlaceChartsPlots)

CQCHARTS_DEF_CMD(MeasureChartsText)
CQCHARTS_DEF_CMD(EncodeChartsText)

CQCHARTS_DEF_CMD(CreateChartsView)
CQCHARTS_DEF_CMD(RemoveChartsView)

CQCHARTS_DEF_CMD(CreateChartsPlot)
CQCHARTS_DEF_CMD(RemoveChartsPlot)

CQCHARTS_DEF_CMD(GetChartsProperty)
CQCHARTS_DEF_CMD(SetChartsProperty)

CQCHARTS_DEF_CMD(GetChartsData)
CQCHARTS_DEF_CMD(SetChartsData)

CQCHARTS_DEF_CMD(CreateChartsPalette)
CQCHARTS_DEF_CMD(GetChartsPalette)
CQCHARTS_DEF_CMD(SetChartsPalette)

//---

CQCHARTS_DEF_CMD(CreateChartsAnnotationGroup)
CQCHARTS_DEF_CMD(CreateChartsArrowAnnotation)
CQCHARTS_DEF_CMD(CreateChartsAxisAnnotation)
CQCHARTS_DEF_CMD(CreateChartsButtonAnnotation)
CQCHARTS_DEF_CMD(CreateChartsEllipseAnnotation)
CQCHARTS_DEF_CMD(CreateChartsImageAnnotation)
CQCHARTS_DEF_CMD(CreateChartsKeyAnnotation)
CQCHARTS_DEF_CMD(CreateChartsPieSliceAnnotation)
CQCHARTS_DEF_CMD(CreateChartsPointAnnotation)
CQCHARTS_DEF_CMD(CreateChartsPointSetAnnotation)
CQCHARTS_DEF_CMD(CreateChartsPolygonAnnotation)
CQCHARTS_DEF_CMD(CreateChartsPolylineAnnotation)
CQCHARTS_DEF_CMD(CreateChartsRectangleAnnotation)
CQCHARTS_DEF_CMD(CreateChartsTextAnnotation)
CQCHARTS_DEF_CMD(CreateChartsValueSetAnnotation)
CQCHARTS_DEF_CMD(CreateChartsWidgetAnnotation)
CQCHARTS_DEF_CMD(RemoveChartsAnnotation)

//---

CQCHARTS_DEF_PROC_CMD(AddChartsKeyItem, addChartsKeyItemCmd)

//---

CQCHARTS_DEF_CMD(ConnectChartsSignal)

//---

CQCHARTS_DEF_PROC_CMD(PrintChartsImage, printChartsImageCmd)
CQCHARTS_DEF_PROC_CMD(WriteChartsData , writeChartsDataCmd)

//---

CQCHARTS_DEF_PROC_CMD(ShowChartsLoadModelDlg   , showChartsLoadModelDlgCmd)
CQCHARTS_DEF_PROC_CMD(ShowChartsManageModelsDlg, showChartsManageModelsDlgCmd)
CQCHARTS_DEF_PROC_CMD(ShowChartsCreatePlotDlg  , showChartsCreatePlotDlgCmd)
CQCHARTS_DEF_PROC_CMD(ShowChartsTextDlg        , showChartsTextDlgCmd)
CQCHARTS_DEF_PROC_CMD(ShowChartsHelpDlg        , showChartsHelpDlgCmd)

//---

CQCHARTS_DEF_PROC_CMD(TestEdit, testEditCmd)

//---

#endif
