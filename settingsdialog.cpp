#include "settingsdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QFont>
#include <QColor>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , settings("LionCPP", "IDE")
{
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    setWindowTitle("设置 - 小狮子C++ IDE");
    setModal(true);
    resize(600, 500);
    
    // 应用深色主题
    setDarkTheme();
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    tabWidget = new QTabWidget(this);
    setupEditorTab();
    setupCompilerTab();
    setupGeneralTab();
    
    mainLayout->addWidget(tabWidget);
    
    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    resetButton = new QPushButton("重置", this);
    cancelButton = new QPushButton("取消", this);
    okButton = new QPushButton("确定", this);
    
    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::onOkButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelButtonClicked);
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetButtonClicked);
}

void SettingsDialog::setupEditorTab()
{
    QWidget *editorTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(editorTab);
    
    // 字体设置组
    QGroupBox *fontGroup = new QGroupBox("字体设置", editorTab);
    QFormLayout *fontLayout = new QFormLayout(fontGroup);
    
    fontFamilyEdit = new QLineEdit(editorTab);
    fontSizeSpinBox = new QSpinBox(editorTab);
    fontSizeSpinBox->setRange(8, 72);
    fontSizeSpinBox->setValue(12);
    
    QPushButton *fontButton = new QPushButton("选择字体", editorTab);
    connect(fontButton, &QPushButton::clicked, this, &SettingsDialog::onFontButtonClicked);
    
    fontLayout->addRow("字体:", fontFamilyEdit);
    fontLayout->addRow("字号:", fontSizeSpinBox);
    fontLayout->addRow("", fontButton);
    
    // 编辑器选项组
    QGroupBox *optionsGroup = new QGroupBox("编辑器选项", editorTab);
    QFormLayout *optionsLayout = new QFormLayout(optionsGroup);
    
    lineNumberCheckBox = new QCheckBox("显示行号", editorTab);
    syntaxHighlightCheckBox = new QCheckBox("语法高亮", editorTab);
    autoIndentCheckBox = new QCheckBox("自动缩进", editorTab);
    showWhitespaceCheckBox = new QCheckBox("显示空白字符", editorTab);
    
    tabSizeSpinBox = new QSpinBox(editorTab);
    tabSizeSpinBox->setRange(1, 8);
    tabSizeSpinBox->setValue(4);
    
    optionsLayout->addRow(lineNumberCheckBox);
    optionsLayout->addRow(syntaxHighlightCheckBox);
    optionsLayout->addRow(autoIndentCheckBox);
    optionsLayout->addRow(showWhitespaceCheckBox);
    optionsLayout->addRow("制表符大小:", tabSizeSpinBox);
    
    // 颜色设置组
    QGroupBox *colorGroup = new QGroupBox("颜色设置", editorTab);
    QFormLayout *colorLayout = new QFormLayout(colorGroup);
    
    backgroundColorEdit = new QLineEdit(editorTab);
    textColorEdit = new QLineEdit(editorTab);
    keywordColorEdit = new QLineEdit(editorTab);
    commentColorEdit = new QLineEdit(editorTab);
    stringColorEdit = new QLineEdit(editorTab);
    
    QPushButton *bgColorButton = new QPushButton("选择", editorTab);
    QPushButton *textColorButton = new QPushButton("选择", editorTab);
    QPushButton *keywordColorButton = new QPushButton("选择", editorTab);
    QPushButton *commentColorButton = new QPushButton("选择", editorTab);
    QPushButton *stringColorButton = new QPushButton("选择", editorTab);
    
    connect(bgColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorButtonClicked);
    connect(textColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorButtonClicked);
    connect(keywordColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorButtonClicked);
    connect(commentColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorButtonClicked);
    connect(stringColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorButtonClicked);
    
    colorLayout->addRow("背景色:", backgroundColorEdit);
    colorLayout->addRow("", bgColorButton);
    colorLayout->addRow("文本色:", textColorEdit);
    colorLayout->addRow("", textColorButton);
    colorLayout->addRow("关键字色:", keywordColorEdit);
    colorLayout->addRow("", keywordColorButton);
    colorLayout->addRow("注释色:", commentColorEdit);
    colorLayout->addRow("", commentColorButton);
    colorLayout->addRow("字符串色:", stringColorEdit);
    colorLayout->addRow("", stringColorButton);
    
    layout->addWidget(fontGroup);
    layout->addWidget(optionsGroup);
    layout->addWidget(colorGroup);
    layout->addStretch();
    
    tabWidget->addTab(editorTab, "编辑器");
}

void SettingsDialog::setupCompilerTab()
{
    QWidget *compilerTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(compilerTab);
    
    // 编译器设置组
    QGroupBox *compilerGroup = new QGroupBox("编译器设置", compilerTab);
    QFormLayout *compilerLayout = new QFormLayout(compilerGroup);
    
    compilerPathEdit = new QLineEdit(compilerTab);
    cmakePathEdit = new QLineEdit(compilerTab);
    buildDirectoryEdit = new QLineEdit(compilerTab);
    buildDirectoryEdit->setText("build");
    
    compilerTypeComboBox = new QComboBox(compilerTab);
    compilerTypeComboBox->addItems({"GCC", "Clang", "MSVC"});
    
    QPushButton *compilerButton = new QPushButton("浏览", compilerTab);
    QPushButton *cmakeButton = new QPushButton("浏览", compilerTab);
    
    compilerLayout->addRow("编译器路径:", compilerPathEdit);
    compilerLayout->addRow("", compilerButton);
    compilerLayout->addRow("CMake路径:", cmakePathEdit);
    compilerLayout->addRow("", cmakeButton);
    compilerLayout->addRow("构建目录:", buildDirectoryEdit);
    compilerLayout->addRow("编译器类型:", compilerTypeComboBox);
    
    // 编译选项组
    QGroupBox *optionsGroup = new QGroupBox("编译选项", compilerTab);
    QFormLayout *optionsLayout = new QFormLayout(optionsGroup);
    
    autoSaveCheckBox = new QCheckBox("编译前自动保存", compilerTab);
    showOutputCheckBox = new QCheckBox("显示编译输出", compilerTab);
    
    optionsLayout->addRow(autoSaveCheckBox);
    optionsLayout->addRow(showOutputCheckBox);
    
    layout->addWidget(compilerGroup);
    layout->addWidget(optionsGroup);
    layout->addStretch();
    
    tabWidget->addTab(compilerTab, "编译器");
}

void SettingsDialog::setupGeneralTab()
{
    QWidget *generalTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(generalTab);
    
    // 通用选项组
    QGroupBox *optionsGroup = new QGroupBox("通用选项", generalTab);
    QFormLayout *optionsLayout = new QFormLayout(optionsGroup);
    
    autoBackupCheckBox = new QCheckBox("自动备份", generalTab);
    backupIntervalSpinBox = new QSpinBox(generalTab);
    backupIntervalSpinBox->setRange(1, 60);
    backupIntervalSpinBox->setValue(5);
    backupIntervalSpinBox->setSuffix(" 分钟");
    
    rememberLastProjectCheckBox = new QCheckBox("记住上次打开的项目", generalTab);
    checkForUpdatesCheckBox = new QCheckBox("检查更新", generalTab);
    
    optionsLayout->addRow(autoBackupCheckBox);
    optionsLayout->addRow("备份间隔:", backupIntervalSpinBox);
    optionsLayout->addRow(rememberLastProjectCheckBox);
    optionsLayout->addRow(checkForUpdatesCheckBox);
    
    layout->addWidget(optionsGroup);
    layout->addStretch();
    
    tabWidget->addTab(generalTab, "通用");
}

void SettingsDialog::loadSettings()
{
    // 编辑器设置
    fontFamilyEdit->setText(settings.value("editor/fontFamily", "Consolas").toString());
    fontSizeSpinBox->setValue(settings.value("editor/fontSize", 12).toInt());
    lineNumberCheckBox->setChecked(settings.value("editor/showLineNumbers", true).toBool());
    syntaxHighlightCheckBox->setChecked(settings.value("editor/syntaxHighlight", true).toBool());
    autoIndentCheckBox->setChecked(settings.value("editor/autoIndent", true).toBool());
    tabSizeSpinBox->setValue(settings.value("editor/tabSize", 4).toInt());
    showWhitespaceCheckBox->setChecked(settings.value("editor/showWhitespace", false).toBool());
    
    backgroundColorEdit->setText(settings.value("editor/backgroundColor", "#1e1e1e").toString());
    textColorEdit->setText(settings.value("editor/textColor", "#d4d4d4").toString());
    keywordColorEdit->setText(settings.value("editor/keywordColor", "#569cd6").toString());
    commentColorEdit->setText(settings.value("editor/commentColor", "#6a9955").toString());
    stringColorEdit->setText(settings.value("editor/stringColor", "#ce9178").toString());
    
    // 编译器设置
    compilerPathEdit->setText(settings.value("compiler/path", "g++").toString());
    cmakePathEdit->setText(settings.value("compiler/cmakePath", "cmake").toString());
    buildDirectoryEdit->setText(settings.value("compiler/buildDirectory", "build").toString());
    compilerTypeComboBox->setCurrentText(settings.value("compiler/type", "GCC").toString());
    autoSaveCheckBox->setChecked(settings.value("compiler/autoSave", true).toBool());
    showOutputCheckBox->setChecked(settings.value("compiler/showOutput", true).toBool());
    
    // 通用设置
    autoBackupCheckBox->setChecked(settings.value("general/autoBackup", true).toBool());
    backupIntervalSpinBox->setValue(settings.value("general/backupInterval", 5).toInt());
    rememberLastProjectCheckBox->setChecked(settings.value("general/rememberLastProject", true).toBool());
    checkForUpdatesCheckBox->setChecked(settings.value("general/checkForUpdates", true).toBool());
}

void SettingsDialog::saveSettings()
{
    // 编辑器设置
    settings.setValue("editor/fontFamily", fontFamilyEdit->text());
    settings.setValue("editor/fontSize", fontSizeSpinBox->value());
    settings.setValue("editor/showLineNumbers", lineNumberCheckBox->isChecked());
    settings.setValue("editor/syntaxHighlight", syntaxHighlightCheckBox->isChecked());
    settings.setValue("editor/autoIndent", autoIndentCheckBox->isChecked());
    settings.setValue("editor/tabSize", tabSizeSpinBox->value());
    settings.setValue("editor/showWhitespace", showWhitespaceCheckBox->isChecked());
    
    settings.setValue("editor/backgroundColor", backgroundColorEdit->text());
    settings.setValue("editor/textColor", textColorEdit->text());
    settings.setValue("editor/keywordColor", keywordColorEdit->text());
    settings.setValue("editor/commentColor", commentColorEdit->text());
    settings.setValue("editor/stringColor", stringColorEdit->text());
    
    // 编译器设置
    settings.setValue("compiler/path", compilerPathEdit->text());
    settings.setValue("compiler/cmakePath", cmakePathEdit->text());
    settings.setValue("compiler/buildDirectory", buildDirectoryEdit->text());
    settings.setValue("compiler/type", compilerTypeComboBox->currentText());
    settings.setValue("compiler/autoSave", autoSaveCheckBox->isChecked());
    settings.setValue("compiler/showOutput", showOutputCheckBox->isChecked());
    
    // 通用设置
    settings.setValue("general/autoBackup", autoBackupCheckBox->isChecked());
    settings.setValue("general/backupInterval", backupIntervalSpinBox->value());
    settings.setValue("general/rememberLastProject", rememberLastProjectCheckBox->isChecked());
    settings.setValue("general/checkForUpdates", checkForUpdatesCheckBox->isChecked());
    
    settings.sync();
}

void SettingsDialog::setDarkTheme()
{
    // 设置深色主题样式表
    QString darkStyle = R"(
        QDialog {
            background-color: #1e1e1e;
            color: #d4d4d4;
        }
        
        QTabWidget::pane {
            border: 1px solid #3e3e42;
            background-color: #1e1e1e;
        }
        
        QTabBar::tab {
            background-color: #2d2d30;
            color: #d4d4d4;
            padding: 8px 12px;
            border: 1px solid #3e3e42;
            border-bottom: none;
        }
        
        QTabBar::tab:selected {
            background-color: #1e1e1e;
        }
        
        QTabBar::tab:hover {
            background-color: #3e3e42;
        }
        
        QGroupBox {
            background-color: #2d2d30;
            color: #d4d4d4;
            border: 1px solid #3e3e42;
            margin-top: 10px;
            padding-top: 10px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        
        QLineEdit, QSpinBox, QComboBox {
            background-color: #3e3e42;
            color: #d4d4d4;
            border: 1px solid #3e3e42;
            padding: 4px;
        }
        
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 1px solid #007acc;
        }
        
        QCheckBox {
            color: #d4d4d4;
            spacing: 5px;
        }
        
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
        
        QCheckBox::indicator:unchecked {
            background-color: #3e3e42;
            border: 1px solid #3e3e42;
        }
        
        QCheckBox::indicator:checked {
            background-color: #007acc;
            border: 1px solid #007acc;
        }
        
        QPushButton {
            background-color: #3e3e42;
            color: #d4d4d4;
            border: 1px solid #3e3e42;
            padding: 6px 12px;
            border-radius: 3px;
        }
        
        QPushButton:hover {
            background-color: #4e4e52;
            border: 1px solid #4e4e52;
        }
        
        QPushButton:pressed {
            background-color: #2d2d30;
        }
        
        QLabel {
            color: #d4d4d4;
        }
    )";
    
    setStyleSheet(darkStyle);
}

void SettingsDialog::onFontButtonClicked()
{
    QFont currentFont(fontFamilyEdit->text(), fontSizeSpinBox->value());
    bool ok;
    QFont font = QFontDialog::getFont(&ok, currentFont, this, "选择字体");
    
    if (ok) {
        fontFamilyEdit->setText(font.family());
        fontSizeSpinBox->setValue(font.pointSize());
    }
}

void SettingsDialog::onColorButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QLineEdit *colorEdit = nullptr;
    
    // 简化颜色选择逻辑，直接使用对象名称来识别
    QString buttonText = button->text();
    if (buttonText == "选择") {
        // 根据按钮在布局中的位置来确定对应的颜色编辑框
        QWidget *parent = qobject_cast<QWidget*>(button->parent());
        if (parent) {
            QLayout *layout = parent->layout();
            if (layout) {
                int index = layout->indexOf(button);
                switch (index) {
                    case 1: colorEdit = backgroundColorEdit; break;
                    case 3: colorEdit = textColorEdit; break;
                    case 5: colorEdit = keywordColorEdit; break;
                    case 7: colorEdit = commentColorEdit; break;
                    case 9: colorEdit = stringColorEdit; break;
                }
            }
        }
    }
    
    if (colorEdit) {
        QColor currentColor(colorEdit->text());
        QColor color = QColorDialog::getColor(currentColor, this, "选择颜色");
        
        if (color.isValid()) {
            colorEdit->setText(color.name());
        }
    }
}

void SettingsDialog::onResetButtonClicked()
{
    int result = QMessageBox::question(this, "重置设置", 
                                     "确定要重置所有设置到默认值吗？",
                                     QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        settings.clear();
        loadSettings();
    }
}

void SettingsDialog::onOkButtonClicked()
{
    saveSettings();
    accept();
}

void SettingsDialog::onCancelButtonClicked()
{
    reject();
} 