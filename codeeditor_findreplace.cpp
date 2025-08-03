#include "codeeditor.h"
#include <QTextDocument>
#include <QTextCursor>

void CodeEditor::findNext(const QString &text, bool matchCase, bool wholeWord) {
    if (text.isEmpty()) return;
    QTextDocument::FindFlags flags;
    if (matchCase) flags |= QTextDocument::FindCaseSensitively;
    if (wholeWord) flags |= QTextDocument::FindWholeWords;
    QTextCursor cur = textCursor();
    cur = document()->find(text, cur, flags);
    if (!cur.isNull()) {
        setTextCursor(cur);
    }
}

void CodeEditor::findPrevious(const QString &text, bool matchCase, bool wholeWord) {
    if (text.isEmpty()) return;
    QTextDocument::FindFlags flags(QTextDocument::FindBackward);
    if (matchCase) flags |= QTextDocument::FindCaseSensitively;
    if (wholeWord) flags |= QTextDocument::FindWholeWords;
    QTextCursor cur = textCursor();
    cur = document()->find(text, cur, flags);
    if (!cur.isNull()) {
        setTextCursor(cur);
    }
}

void CodeEditor::replaceOne(const QString &findText, const QString &replaceText, bool matchCase, bool wholeWord) {
    QTextCursor cur = textCursor();
    if (!cur.hasSelection()) {
        findNext(findText, matchCase, wholeWord);
        cur = textCursor();
        if (!cur.hasSelection()) return;
    }
    if (cur.selectedText() == findText || (!matchCase && cur.selectedText().compare(findText, Qt::CaseInsensitive) == 0)) {
        cur.insertText(replaceText);
    }
}

void CodeEditor::replaceAll(const QString &findText, const QString &replaceText, bool matchCase, bool wholeWord) {
    if (findText.isEmpty()) return;
    QTextDocument::FindFlags flags;
    if (matchCase) flags |= QTextDocument::FindCaseSensitively;
    if (wholeWord) flags |= QTextDocument::FindWholeWords;
    QTextCursor cur(document());
    cur.beginEditBlock();
    int count = 0;
    while (true) {
        cur = document()->find(findText, cur, flags);
        if (cur.isNull()) break;
        cur.insertText(replaceText);
        ++count;
    }
    cur.endEditBlock();
    // 可选：显示替换次数
    // emit statusMessage(QString("替换了 %1 个匹配项").arg(count));
}
