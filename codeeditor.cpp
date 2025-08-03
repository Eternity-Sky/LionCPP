#include "codeeditor.h"
#include "line_number_area.h"
#include <QPainter>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextLayout>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QApplication>
#include <QCompleter>
#include <QStringListModel>
#include <QPainterPath>

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , lineNumberArea(new LineNumberArea(this))
    , c(nullptr)
{
    qDebug() << "[CodeEditor] begin";
    qDebug() << "[CodeEditor] lineNumberArea created";
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    qDebug() << "[CodeEditor] connect blockCountChanged";
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    qDebug() << "[CodeEditor] connect updateRequest";
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    
    // 连接行号区域信号
    qDebug() << "[CodeEditor] connect cursorPositionChanged";
    connect(lineNumberArea, &LineNumberArea::setBreakpoint, this, &CodeEditor::onSetBreakpoint);
    qDebug() << "[CodeEditor] connect setBreakpoint";
    connect(lineNumberArea, &LineNumberArea::clearBreakpoint, this, &CodeEditor::onClearBreakpoint);
    qDebug() << "[CodeEditor] connect clearBreakpoint";
    connect(lineNumberArea, &LineNumberArea::runToLine, this, &CodeEditor::onRunToLine);
    qDebug() << "[CodeEditor] connect runToLine";

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    qDebug() << "[CodeEditor] highlightCurrentLine";

    // 设置字体
    QFont font("Consolas", 12);
    qDebug() << "[CodeEditor] before setFont";
    font.setFixedPitch(true);
    setFont(font);
    qDebug() << "[CodeEditor] setFont";
    
    // 设置深色主题的文本颜色
    QPalette palette = this->palette();
    qDebug() << "[CodeEditor] before setPalette";
    palette.setColor(QPalette::Base, QColor("#1e1e1e"));
    palette.setColor(QPalette::Text, QColor("#ffffff"));  // 改为纯白色，确保可见
    palette.setColor(QPalette::WindowText, QColor("#ffffff"));
    palette.setColor(QPalette::BrightText, QColor("#ffffff"));  // 添加亮文本色
    setPalette(palette);
    
    // 强制设置样式表确保文本颜色
    setStyleSheet("QPlainTextEdit { color: #ffffff; background-color: #1e1e1e; }");
    qDebug() << "[CodeEditor] setPalette and StyleSheet";

    // 设置制表符宽度
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
    qDebug() << "[CodeEditor] setTabStopDistance";

    // 设置语法高亮
    new CppHighlighter(document());
    qDebug() << "[CodeEditor] CppHighlighter created";

    // 设置代码补全
    // 初始化空的 QCompleter，词表后续动态设置
    qDebug() << "[CodeEditor] before completer";
    c = new QCompleter(QStringList(), this);
    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    connect(c, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);
    connect(document(), &QTextDocument::contentsChanged, this, [](){
        qDebug() << "[CodeEditor] document contentsChanged";
    });
}

void CodeEditor::setPlainText(const QString &text)
{
    qDebug() << "[CodeEditor] setPlainText called, len:" << text.length();
    QPlainTextEdit::setPlainText(text);
    qDebug() << "[CodeEditor] after QPlainTextEdit::setPlainText";
}

// 访问器方法实现（只保留一份）
QTextBlock CodeEditor::getFirstVisibleBlock() const
{
    return QPlainTextEdit::firstVisibleBlock();
}

QRectF CodeEditor::getBlockBoundingGeometry(const QTextBlock &block) const
{
    return QPlainTextEdit::blockBoundingGeometry(block);
}

QPointF CodeEditor::getContentOffset() const
{
    return QPlainTextEdit::contentOffset();
}

QRectF CodeEditor::getBlockBoundingRect(const QTextBlock &block) const
{
    return QPlainTextEdit::blockBoundingRect(block);
}

QTextCursor CodeEditor::getCursorForPosition(const QPoint &pos) const
{
    return QPlainTextEdit::cursorForPosition(pos);
}

CodeEditor::~CodeEditor()
{
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // 计算行号区域宽度：左边距 + 数字宽度 + 右边距
    int leftMargin = 4;  // 左侧边距（减小）
    int rightMargin = 6; // 右侧边距，确保与编辑器内容有足够间隙
    int digitWidth = fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    
    return leftMargin + digitWidth + rightMargin;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    int lineAreaWidth = lineNumberAreaWidth();
    // 设置行号区域的位置，确保不遮挡编辑器内容
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineAreaWidth, cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        // 使用更明显的当前行高亮颜色
        QColor lineColor = QColor("#2d2d30"); // 深灰色背景

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
    
    // 通知行号区域高亮当前行
    int currentLine = textCursor().blockNumber();
    lineNumberArea->setCurrentLine(currentLine);
}

void CodeEditor::onSetBreakpoint(int line)
{
    // 设置断点的实现
    // 这里可以添加断点管理逻辑
    qDebug() << "Set breakpoint at line:" << line;
}

void CodeEditor::onClearBreakpoint(int line)
{
    // 清除断点的实现
    // 这里可以添加断点管理逻辑
    qDebug() << "Clear breakpoint at line:" << line;
}

void CodeEditor::onRunToLine(int line)
{
    // 运行到指定行的实现
    // 这里可以添加调试器控制逻辑
    qDebug() << "Run to line:" << line;
}

void CodeEditor::insertCompletion(const QString &completion)
{
    if (c->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString CodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CodeEditor::focusInEvent(QFocusEvent *e)
{
    if (c)
        c->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void CodeEditor::wheelEvent(QWheelEvent *event)
{
    // 检查是否按下了Ctrl键
    if (event->modifiers() & Qt::ControlModifier) {
        // 阻止事件继续传播
        event->accept();
        
        // 根据滚轮方向调整字体大小
        int delta = event->angleDelta().y();
        if (delta > 0) {
            changeFontSize(1);  // 放大
        } else if (delta < 0) {
            changeFontSize(-1); // 缩小
        }
    } else {
        // 正常滚轮行为
        QPlainTextEdit::wheelEvent(event);
    }
}

void CodeEditor::changeFontSize(int delta)
{
    QFont f = font();
    f.setPointSize(f.pointSize() + delta);
    setFont(f);
    // 更新行号区域宽度
    updateLineNumberAreaWidth(0);
}

void CodeEditor::onFind(const QString &text, QTextDocument::FindFlags flags)
{
    qDebug() << "[CodeEditor] onFind, text:" << text << ", flags:" << flags;
    if (text.isEmpty()) return;
    QTextCursor cur = document()->find(text, textCursor(), flags);
    if (!cur.isNull()) {
        setTextCursor(cur);
    }
}

void CodeEditor::setCompleter(QCompleter *completer)
{
    if (c)
        c->disconnect(this);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    qDebug() << "[CodeEditor] before connect QCompleter::activated";
    connect(c, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);
    qDebug() << "[CodeEditor] after connect QCompleter::activated";
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    qDebug() << "[CodeEditor] keyPressEvent, key:" << e->key() << ", modifiers:" << e->modifiers();
    
    // 处理字体大小调整快捷键
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->key() == Qt::Key_Plus || e->key() == Qt::Key_Equal) {
            changeFontSize(1);  // 放大字体
            e->accept();
            return;
        } else if (e->key() == Qt::Key_Minus) {
            changeFontSize(-1); // 缩小字体
            e->accept();
            return;
        }
    }
    
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }

    const bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+SPACE
    if ((c == nullptr || !isShortcut) && !e->text().isEmpty())
        qDebug() << "[CodeEditor] before QPlainTextEdit::setPlainText";
    QPlainTextEdit::keyPressEvent(e);
    qDebug() << "[CodeEditor] after QPlainTextEdit::setPlainText";

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (c == nullptr || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    // --- 动态扫描文档所有单词，生成补全词表 ---
    QSet<QString> wordSet;
    QRegularExpression wordRegex("\\b\\w{3,}\\b"); // 至少3字母的单词
    QTextDocument *doc = document();
    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        QString text = block.text();
        QRegularExpressionMatchIterator i = wordRegex.globalMatch(text);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            wordSet.insert(match.captured());
        }
    }
    QStringList words = wordSet.values();
    words.sort(Qt::CaseInsensitive);
    qDebug() << "[CodeEditor] before deleteLater";
    static_cast<QAbstractItemModel*>(c->model())->deleteLater();
    qDebug() << "[CodeEditor] after deleteLater";
    qDebug() << "[CodeEditor] before setModel";
    c->setModel(new QStringListModel(words, c));
    qDebug() << "[CodeEditor] after setModel";
    // --- END 动态补全 ---

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr); // popup it up!
}

void CodeEditor::onReplace(const QString &findText, const QString &replaceText, QTextDocument::FindFlags flags)
{
    QTextCursor cur = textCursor();
    if (!cur.hasSelection()) {
        onFind(findText, flags);
        cur = textCursor();
        if (!cur.hasSelection()) return;
    }
    if (cur.selectedText() == findText || (flags.testFlag(QTextDocument::FindCaseSensitively) ? cur.selectedText() == findText : cur.selectedText().compare(findText, Qt::CaseInsensitive) == 0)) {
        cur.insertText(replaceText);
    }
}

void CodeEditor::onReplaceAll(const QString &findText, const QString &replaceText, QTextDocument::FindFlags flags)
{
    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::Start);
    setTextCursor(cur);
    
    int count = 0;
    while (true) {
        QTextCursor found = document()->find(findText, cur, flags);
        if (found.isNull()) break;
        
        found.insertText(replaceText);
        cur = found;
        count++;
    }
    
    qDebug() << "Replaced" << count << "occurrences";
}

// C++语法高亮器实现
CppHighlighter::CppHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    qDebug() << "[CppHighlighter] constructed";
    
    HighlightingRule rule;

    // 关键字格式
    keywordFormat.setForeground(QColor(86, 156, 214));
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bauto\\b"), QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bcase\\b"),
        QStringLiteral("\\bchar\\b"), QStringLiteral("\\bconst\\b"), QStringLiteral("\\bcontinue\\b"),
        QStringLiteral("\\bdefault\\b"), QStringLiteral("\\bdelete\\b"), QStringLiteral("\\bdo\\b"),
        QStringLiteral("\\bdouble\\b"), QStringLiteral("\\belse\\b"), QStringLiteral("\\benum\\b"),
        QStringLiteral("\\bextern\\b"), QStringLiteral("\\bfloat\\b"), QStringLiteral("\\bfor\\b"),
        QStringLiteral("\\bfriend\\b"), QStringLiteral("\\bgoto\\b"), QStringLiteral("\\bif\\b"),
        QStringLiteral("\\binline\\b"), QStringLiteral("\\bint\\b"), QStringLiteral("\\blong\\b"),
        QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\bnew\\b"), QStringLiteral("\\boperator\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
        QStringLiteral("\\bregister\\b"), QStringLiteral("\\breturn\\b"), QStringLiteral("\\bshort\\b"),
        QStringLiteral("\\bsigned\\b"), QStringLiteral("\\bsizeof\\b"), QStringLiteral("\\bstatic\\b"),
        QStringLiteral("\\bstruct\\b"), QStringLiteral("\\bswitch\\b"), QStringLiteral("\\btemplate\\b"),
        QStringLiteral("\\bthis\\b"), QStringLiteral("\\bthrow\\b"), QStringLiteral("\\btry\\b"),
        QStringLiteral("\\btypedef\\b"), QStringLiteral("\\btypename\\b"), QStringLiteral("\\bunion\\b"),
        QStringLiteral("\\bunsigned\\b"), QStringLiteral("\\busing\\b"), QStringLiteral("\\bvirtual\\b"),
        QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bwhile\\b"),
        QStringLiteral("\\bclass\\b"), QStringLiteral("\\btrue\\b"), QStringLiteral("\\bfalse\\b")
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 类名格式
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(QColor(78, 201, 176));
    rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
    rule.format = classFormat;
    highlightingRules.append(rule);

    // 单行注释格式
    singleLineCommentFormat.setForeground(QColor(106, 153, 85));
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 多行注释格式
    multiLineCommentFormat.setForeground(QColor(106, 153, 85));

    // 字符串格式
    quotationFormat.setForeground(QColor(206, 145, 120));
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 函数格式
    functionFormat.setForeground(QColor(220, 220, 170));
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z_][A-Za-z0-9_]*\\s*(?=\\()"));
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 数字格式
    numberFormat.setForeground(QColor(181, 206, 168));
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
    commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}

void CppHighlighter::highlightBlock(const QString &text)
{
    qDebug() << "[CppHighlighter] highlightBlock, text length:" << text.length();
    
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

void CppHighlighter::updateColors(const QColor &keywordColor, const QColor &commentColor, const QColor &stringColor)
{
    // 更新关键字颜色
    keywordFormat.setForeground(keywordColor);
    
    // 更新注释颜色
    singleLineCommentFormat.setForeground(commentColor);
    multiLineCommentFormat.setForeground(commentColor);
    
    // 更新字符串颜色
    quotationFormat.setForeground(stringColor);
    
    // 重新构建高亮规则
    highlightingRules.clear();
    
    // 重新添加关键字规则
    HighlightingRule rule;
    const QString keywordPatterns[] = {
        QStringLiteral("\\bauto\\b"), QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bcase\\b"),
        QStringLiteral("\\bchar\\b"), QStringLiteral("\\bconst\\b"), QStringLiteral("\\bcontinue\\b"),
        QStringLiteral("\\bdefault\\b"), QStringLiteral("\\bdelete\\b"), QStringLiteral("\\bdo\\b"),
        QStringLiteral("\\bdouble\\b"), QStringLiteral("\\belse\\b"), QStringLiteral("\\benum\\b"),
        QStringLiteral("\\bextern\\b"), QStringLiteral("\\bfloat\\b"), QStringLiteral("\\bfor\\b"),
        QStringLiteral("\\bfriend\\b"), QStringLiteral("\\bgoto\\b"), QStringLiteral("\\bif\\b"),
        QStringLiteral("\\binline\\b"), QStringLiteral("\\bint\\b"), QStringLiteral("\\blong\\b"),
        QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\bnew\\b"), QStringLiteral("\\boperator\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
        QStringLiteral("\\bregister\\b"), QStringLiteral("\\breturn\\b"), QStringLiteral("\\bshort\\b"),
        QStringLiteral("\\bsigned\\b"), QStringLiteral("\\bsizeof\\b"), QStringLiteral("\\bstatic\\b"),
        QStringLiteral("\\bstruct\\b"), QStringLiteral("\\bswitch\\b"), QStringLiteral("\\btemplate\\b"),
        QStringLiteral("\\bthis\\b"), QStringLiteral("\\bthrow\\b"), QStringLiteral("\\btry\\b"),
        QStringLiteral("\\btypedef\\b"), QStringLiteral("\\btypename\\b"), QStringLiteral("\\bunion\\b"),
        QStringLiteral("\\bunsigned\\b"), QStringLiteral("\\busing\\b"), QStringLiteral("\\bvirtual\\b"),
        QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bwhile\\b"),
        QStringLiteral("\\bclass\\b"), QStringLiteral("\\btrue\\b"), QStringLiteral("\\bfalse\\b")
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
    
    // 重新添加其他规则
    rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
    rule.format = classFormat;
    highlightingRules.append(rule);
    
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
    
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z_][A-Za-z0-9_]*\\s*(?=\\()"));
    rule.format = functionFormat;
    highlightingRules.append(rule);
    
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
    
    // 触发重新高亮
    rehighlight();
}

// ====== 虚函数 event、paintEvent 实现 ======
bool CodeEditor::event(QEvent *e)
{
    // 可根据需要处理自定义事件，否则交给基类
    return QPlainTextEdit::event(e);
}

void CodeEditor::paintEvent(QPaintEvent *event)
{
    QPlainTextEdit::paintEvent(event);
    // 可选: 添加自定义绘制逻辑
}