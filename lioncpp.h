#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QTreeView>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <QCloseEvent>
#include <QApplication>
#include <QMenu>
#include <QToolButton>
#include <QLabel>
#include <QProgressBar>
#include <QProcess>

#include "codeeditor.h"
#include "projectmanager.h"
#include "compiler.h"
#include "settingsdialog.h"
#include "findreplacedialog.h"
#include "welcomedialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class LionCPP;
}
QT_END_NAMESPACE

class LionCPP : public QMainWindow
{
    Q_OBJECT

public:
    LionCPP(QWidget *parent = nullptr);
    ~LionCPP();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件菜单
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    void onCloseFile();
    void onExit();
    
    // 编辑菜单
    void onUndo();
    void onRedo();
    void onCut();
    void onCopy();
    void onPaste();
    void onFind();
    void onReplace();
    
    // 运行菜单
    void onCompile();
    void onRun();
    void onCompileAndRun();
    void onStop();
    
    // 工具菜单
    void onSettings();
    void onAbout();
    
    // 编译和运行
    void onCompilationStarted();
    void onCompilationFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onRunStarted();
    void onRunFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupDockWidgets();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void updateWindowTitle();
    void updateActions();
    CodeEditor* getCurrentEditor();
    void openFileInEditor(const QString &filePath);
    bool saveCurrentFile();
    void closeCurrentFile();
    bool saveFileIfModified();
    void compileCurrentFile();
    void runCurrentFile();
    void runInExternalTerminal(const QString &executablePath);
    void runInBuiltinTerminal(const QString &executablePath);
    void showWelcomeDialog();
    void onWelcomeNewFile();
    void onWelcomeOpenFile();
    void updateTabTitle(CodeEditor *editor);
    void setDarkTheme();
    void applyEditorSettings();
    void applyEditorSettingsToEditor(CodeEditor *editor);
    
    // 编译器扫描和配置
    void scanCompilers();
    void showCompilerSetupDialog();
    QString detectSystemCompiler();
    bool isFirstRun();
    
    Ui::LionCPP *ui;
    
    // UI组件
    QSplitter *mainSplitter;
    QTabWidget *editorTabWidget;
    QTextEdit *outputWidget;
    QTreeView *projectTreeView;
    QDockWidget *outputDock;
    
    // 菜单和工具栏
    QMenuBar *mainMenuBar;
    QToolBar *mainToolBar;
    QStatusBar *mainStatusBar;
    
    // 状态栏组件
    QLabel *statusLabel;
    QLabel *lineColumnLabel;
    
    // 菜单
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *runMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;
    
    // 动作
    QAction *newFileAction;
    QAction *openFileAction;
    QAction *saveFileAction;
    QAction *saveAsFileAction;
    QAction *closeFileAction;
    QAction *exitAction;
    
    QAction *undoAction;
    QAction *redoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *findAction;
    QAction *replaceAction;
    
    QAction *compileAction;
    QAction *runAction;
    QAction *compileAndRunAction;
    QAction *stopAction;
    
    QAction *settingsAction;
    QAction *aboutAction;
    
    // 核心组件
    SettingsDialog *settingsDialog;
    QProcess *compileProcess;
    QProcess *runProcess;
    
    // 状态
    QString currentFilePath;
    bool isCompiling;
    bool isRunning;
    
    // 设置
    QSettings settings;
};
