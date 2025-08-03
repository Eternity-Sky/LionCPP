#pragma once

#include <QDialog>
#include <QSettings>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QFontDialog>
#include <QColorDialog>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void loadSettings();
    void saveSettings();

private slots:
    void onFontButtonClicked();
    void onColorButtonClicked();
    void onResetButtonClicked();
    void onOkButtonClicked();
    void onCancelButtonClicked();

private:
    void setupUI();
    void setupEditorTab();
    void setupCompilerTab();
    void setupGeneralTab();
    void setDarkTheme();
    
    QTabWidget *tabWidget;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QPushButton *resetButton;
    
    // 编辑器设置
    QLineEdit *fontFamilyEdit;
    QSpinBox *fontSizeSpinBox;
    QCheckBox *lineNumberCheckBox;
    QCheckBox *syntaxHighlightCheckBox;
    QCheckBox *autoIndentCheckBox;
    QSpinBox *tabSizeSpinBox;
    QCheckBox *showWhitespaceCheckBox;
    QLineEdit *backgroundColorEdit;
    QLineEdit *textColorEdit;
    QLineEdit *keywordColorEdit;
    QLineEdit *commentColorEdit;
    QLineEdit *stringColorEdit;
    
    // 编译器设置
    QLineEdit *compilerPathEdit;
    QLineEdit *cmakePathEdit;
    QLineEdit *buildDirectoryEdit;
    QComboBox *compilerTypeComboBox;
    QCheckBox *autoSaveCheckBox;
    QCheckBox *showOutputCheckBox;
    
    // 通用设置
    QCheckBox *autoBackupCheckBox;
    QSpinBox *backupIntervalSpinBox;
    QCheckBox *rememberLastProjectCheckBox;
    QCheckBox *checkForUpdatesCheckBox;
    
    QSettings settings;
}; 