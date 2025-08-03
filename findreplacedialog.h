#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QCheckBox;
QT_END_NAMESPACE

class FindReplaceDialog : public QDialog {
    Q_OBJECT
public:
    explicit FindReplaceDialog(QWidget *parent = nullptr);
    QString findText() const;
    QString replaceText() const;
    bool matchCase() const;
    bool wholeWord() const;

signals:
    void findNext(const QString &text, bool matchCase, bool wholeWord);
    void findPrevious(const QString &text, bool matchCase, bool wholeWord);
    void replaceOne(const QString &findText, const QString &replaceText, bool matchCase, bool wholeWord);
    void replaceAll(const QString &findText, const QString &replaceText, bool matchCase, bool wholeWord);

private:
    QLineEdit *findLineEdit;
    QLineEdit *replaceLineEdit;
    QPushButton *findNextButton;
    QPushButton *findPrevButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
    QCheckBox *caseCheckBox;
    QCheckBox *wholeWordCheckBox;
};

#endif // FINDREPLACEDIALOG_H
