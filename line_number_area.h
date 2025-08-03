#ifndef LINE_NUMBER_AREA_H
#define LINE_NUMBER_AREA_H

#include <QWidget>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QTextBlock>
#include <QPaintEvent>
#include <QSize>
#include <QPoint>
#include <QTextCursor>

#include "codeeditor.h"
class CodeEditor;

class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    explicit LineNumberArea(CodeEditor *editor);
    QSize sizeHint() const override;
    void setCurrentLine(int line);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    CodeEditor *codeEditor;
    int currentLine;
    
private slots:
    void onSetBreakpoint();
    void onClearBreakpoint();
    void onRunToLine();

signals:
    void setBreakpoint(int line);
    void clearBreakpoint(int line);
    void runToLine(int line);
};

#endif // LINE_NUMBER_AREA_H
