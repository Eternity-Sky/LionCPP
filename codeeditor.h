#pragma once

#include <QPlainTextEdit>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QCompleter>
#include <QStringListModel>
#include <QSet>
#include <QRegularExpression>
#include <QPainter>
#include <QTextBlock>

// 前向声明
class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    void setPlainText(const QString &text);
    ~CodeEditor();

    // 行号区域相关
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;
    
    // 代码补全相关
    void setCompleter(QCompleter *completer);
    QCompleter *completer() const;
    
    // 访问器方法 - 用于LineNumberArea访问受保护的QPlainTextEdit成员
    QTextBlock getFirstVisibleBlock() const;
    QRectF getBlockBoundingGeometry(const QTextBlock &block) const;
    QPointF getContentOffset() const;
    QRectF getBlockBoundingRect(const QTextBlock &block) const;
    QTextCursor getCursorForPosition(const QPoint &pos) const;

protected:
    // 事件处理
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    bool event(QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void wheelEvent(QWheelEvent *event) override;

public slots:
    // 查找/替换相关槽
    void findNext(const QString &text, bool matchCase, bool wholeWord);
    void findPrevious(const QString &text, bool matchCase, bool wholeWord);
    void replaceOne(const QString &findText, const QString &replaceText, bool matchCase, bool wholeWord);
    void replaceAll(const QString &findText, const QString &replaceText, bool matchCase, bool wholeWord);

public slots:
    // 行号区域更新
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    
    // 代码补全
    void insertCompletion(const QString &completion);
    
    // 断点相关槽函数
    void onSetBreakpoint(int line);
    void onClearBreakpoint(int line);
    void onRunToLine(int line);

    // 查找/替换相关槽函数
    void onFind(const QString &text, QTextDocument::FindFlags flags);
    void onReplace(const QString &findText, const QString &replaceText, QTextDocument::FindFlags flags);
    void onReplaceAll(const QString &findText, const QString &replaceText, QTextDocument::FindFlags flags);

private:
    // UI组件
    LineNumberArea *lineNumberArea;
    QCompleter *c = nullptr;
    QString textUnderCursor() const;
    void changeFontSize(int delta);
    

};

class CppHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    CppHighlighter(QTextDocument *parent = nullptr);
    void updateColors(const QColor &keywordColor, const QColor &commentColor, const QColor &stringColor);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat numberFormat;
}; 