#include "line_number_area.h"
#include <QPainter>
#include <QTextBlock>
#include <QMenu>
#include <QContextMenuEvent>
#include <QCursor>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDebug>

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor)
    , codeEditor(editor)
    , currentLine(-1)
{
    setMouseTracking(true);
    setFixedWidth(30);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    
    // 确保编辑器已正确初始化
    if (!codeEditor) {
        qWarning() << "LineNumberArea: Invalid editor passed to constructor";
    }
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::setCurrentLine(int line)
{
    currentLine = line;
    update();
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    if (!codeEditor) {
        QWidget::paintEvent(event);
        return;
    }

    QPainter painter(this);
    // 使用深色主题背景色
    painter.fillRect(rect(), QColor(45, 45, 45));  // 深灰色背景
    painter.setPen(QColor(180, 180, 180));  // 浅灰色文字
    painter.setFont(codeEditor->font());

    // 获取第一个可见的文本块
    QTextBlock block = codeEditor->document()->firstBlock();
    int blockNumber = block.blockNumber();
    
    // 获取视口位置信息
    qreal top = codeEditor->getBlockBoundingGeometry(block).translated(codeEditor->getContentOffset()).top();
    qreal bottom = top + codeEditor->getBlockBoundingRect(block).height();
    
    // 获取可见区域
    QRectF visibleRect = rect();
    visibleRect.adjust(0, -codeEditor->getContentOffset().y(), 0, -codeEditor->getContentOffset().y() + height());

    while (block.isValid() && top <= visibleRect.bottom()) {
        if (block.isVisible() && bottom >= visibleRect.top()) {
            QString number = QString::number(blockNumber + 1);
            QRectF blockRect(0, top, width(), bottom - top);

            // 绘制当前行高亮
            if (blockNumber == currentLine) {
                painter.fillRect(blockRect, QColor(70, 70, 100));  // 深色当前行高亮
                painter.setPen(QColor(255, 255, 255));  // 白色文字
            } else {
                painter.setPen(QColor(150, 150, 150));  // 浅灰色文字
            }

            // 绘制行号
            painter.drawText(blockRect, Qt::AlignRight | Qt::AlignVCenter, number);
        }

        block = block.next();
        if (!block.isValid())
            break;
            
        top = bottom;
        bottom = top + codeEditor->getBlockBoundingRect(block).height();
        ++blockNumber;
    }
}

void LineNumberArea::contextMenuEvent(QContextMenuEvent *event)
{
    if (!codeEditor) {
        event->ignore();
        return;
    }

    // 获取点击位置对应的行号
    QPoint pos = event->pos();
    int line = -1;
    
    // 获取文档中的第一个可见块
    QTextBlock block = codeEditor->document()->firstBlock();
    qreal top = codeEditor->getBlockBoundingGeometry(block).translated(codeEditor->getContentOffset()).top();
    qreal bottom = top + codeEditor->getBlockBoundingRect(block).height();
    
    // 查找点击位置对应的行号
    while (block.isValid() && top <= pos.y()) {
        if (block.isVisible() && pos.y() >= top && pos.y() < bottom) {
            line = block.blockNumber();
            break;
        }
        
        block = block.next();
        if (!block.isValid())
            break;
            
        top = bottom;
        bottom = top + codeEditor->getBlockBoundingRect(block).height();
    }
    
    // 如果找到有效行号，显示上下文菜单
    if (line >= 0) {
        QMenu menu(this);
        menu.setStyleSheet("QMenu { background-color: #f0f0f0; border: 1px solid #ccc; }"
                         "QMenu::item { padding: 5px 20px; }"
                         "QMenu::item:selected { background-color: #cce8ff; }");
        
        // 使用文本代替图标，避免资源文件问题
        QAction *setBreakpointAction = menu.addAction("设置断点 (F9)");
        QAction *clearBreakpointAction = menu.addAction("清除断点 (Shift+F9)");
        menu.addSeparator();
        QAction *runToLineAction = menu.addAction("运行到此行 (Ctrl+F10)");
        
        // 显示菜单并处理用户选择
        QAction *selectedAction = menu.exec(mapToGlobal(pos));
        if (selectedAction) {
            if (selectedAction == setBreakpointAction) {
                emit setBreakpoint(line + 1);  // 转换为1-based行号
            } else if (selectedAction == clearBreakpointAction) {
                emit clearBreakpoint(line + 1);
            } else if (selectedAction == runToLineAction) {
                emit runToLine(line + 1);
            }
        }
    }
}

void LineNumberArea::onSetBreakpoint()
{
    if (!codeEditor)
        return;
        
    // 获取当前光标位置对应的行号
    QTextCursor cursor = codeEditor->textCursor();
    int line = cursor.blockNumber();
    emit setBreakpoint(line + 1);  // 转换为1-based行号
}

void LineNumberArea::onClearBreakpoint()
{
    if (!codeEditor)
        return;
        
    // 获取当前光标位置对应的行号
    QTextCursor cursor = codeEditor->textCursor();
    int line = cursor.blockNumber();
    emit clearBreakpoint(line + 1);  // 转换为1-based行号
}

void LineNumberArea::onRunToLine()
{
    if (!codeEditor)
        return;
        
    // 获取当前光标位置对应的行号
    QTextCursor cursor = codeEditor->textCursor();
    int line = cursor.blockNumber();
    emit runToLine(line + 1);  // 转换为1-based行号
}
