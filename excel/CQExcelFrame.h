#ifndef CQExcelFrame_H
#define CQExcelFrame_H

#include <QFrame>
#include <QModelIndex>

class QLabel;
class QLineEdit;

namespace CQExcel {

class View;
class Model;

class Frame : public QFrame {
  Q_OBJECT

 public:
  struct LoadOptions {
    bool comment_header      { false };
    bool first_line_header   { false };
    bool first_column_header { false };

    LoadOptions() { }
  };

 public:
  Frame(QWidget *parent=nullptr);

  View *view() const { return view_; }

  QLabel *nameLabel() const { return nameLabel_; }

  QLineEdit *functionEdit() const { return functionEdit_; }

  bool loadCsv(const QString &filename, const LoadOptions &options=LoadOptions());

  void setModel(QAbstractItemModel *model);

  QSize sizeHint() const override;

 private slots:
  void functionSlot();

  void loadSlot();
  void saveSlot();

  void currentIndexSlot();

  void selectionSlot();

 private:
  View*      view_         { nullptr };
  QLabel*    nameLabel_    { nullptr };
  QLineEdit* functionEdit_ { nullptr };
};

}

#endif
