#include "findreplacedialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

FindReplaceDialog::FindReplaceDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("查找/替换"));
    setModal(false);
    auto *mainLayout = new QVBoxLayout(this);
    
    auto *findLayout = new QHBoxLayout();
    findLayout->addWidget(new QLabel(tr("查找内容:")));
    findLineEdit = new QLineEdit(this);
    findLayout->addWidget(findLineEdit);
    mainLayout->addLayout(findLayout);

    auto *replaceLayout = new QHBoxLayout();
    replaceLayout->addWidget(new QLabel(tr("替换为:")));
    replaceLineEdit = new QLineEdit(this);
    replaceLayout->addWidget(replaceLineEdit);
    mainLayout->addLayout(replaceLayout);

    auto *optionsLayout = new QHBoxLayout();
    caseCheckBox = new QCheckBox(tr("区分大小写"), this);
    wholeWordCheckBox = new QCheckBox(tr("全词匹配"), this);
    optionsLayout->addWidget(caseCheckBox);
    optionsLayout->addWidget(wholeWordCheckBox);
    mainLayout->addLayout(optionsLayout);

    auto *buttonLayout = new QHBoxLayout();
    findNextButton = new QPushButton(tr("查找下一个"), this);
    findPrevButton = new QPushButton(tr("查找上一个"), this);
    replaceButton = new QPushButton(tr("替换"), this);
    replaceAllButton = new QPushButton(tr("全部替换"), this);
    buttonLayout->addWidget(findNextButton);
    buttonLayout->addWidget(findPrevButton);
    buttonLayout->addWidget(replaceButton);
    buttonLayout->addWidget(replaceAllButton);
    mainLayout->addLayout(buttonLayout);

    connect(findNextButton, &QPushButton::clicked, this, [this]() {
        emit findNext(findText(), matchCase(), wholeWord());
    });
    connect(findPrevButton, &QPushButton::clicked, this, [this]() {
        emit findPrevious(findText(), matchCase(), wholeWord());
    });
    connect(replaceButton, &QPushButton::clicked, this, [this]() {
        emit replaceOne(findText(), replaceText(), matchCase(), wholeWord());
    });
    connect(replaceAllButton, &QPushButton::clicked, this, [this]() {
        emit replaceAll(findText(), replaceText(), matchCase(), wholeWord());
    });
}

QString FindReplaceDialog::findText() const {
    return findLineEdit->text();
}
QString FindReplaceDialog::replaceText() const {
    return replaceLineEdit->text();
}
bool FindReplaceDialog::matchCase() const {
    return caseCheckBox->isChecked();
}
bool FindReplaceDialog::wholeWord() const {
    return wholeWordCheckBox->isChecked();
}
