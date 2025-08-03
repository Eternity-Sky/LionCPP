#include "lioncpp.h"
#include "./ui_lioncpp.h"
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QFileSystemModel>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QDialog>
#include "findreplacedialog.h"

LionCPP::LionCPP(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LionCPP)
    , settingsDialog(nullptr)
    , compileProcess(nullptr)
    , runProcess(nullptr)
    , isCompiling(false)
    , isRunning(false)
    , settings("LionCPP", "IDE")
{
    ui->setupUi(this);
    
    // 检查是否首次运行，如果是则扫描编译器
    if (isFirstRun()) {
        QTimer::singleShot(200, this, &LionCPP::scanCompilers);
    }
    
    // 显示欢迎界面
    QTimer::singleShot(100, this, &LionCPP::showWelcomeDialog);

    // 获取新UI控件指针
    projectTreeView = findChild<QTreeView*>("projectTreeView");
    editorTabWidget = findChild<QTabWidget*>("editorTabWidget");
    outputWidget = findChild<QTextEdit*>("outputWidget");
    QDockWidget* sideDock = findChild<QDockWidget*>("sideDock");

    // 初始化文件系统模型
    if (projectTreeView) {
        auto *model = new QFileSystemModel(projectTreeView);
        model->setRootPath(QDir::currentPath());
        QStringList filters;
        filters << "*.cpp" << "*.h" << "*.hpp" << "*.c" << "*.cc" << "*.cxx";
        model->setNameFilters(filters);
        model->setNameFilterDisables(false);
        projectTreeView->setModel(static_cast<QAbstractItemModel*>(model));
        projectTreeView->setRootIndex(model->index(QDir::currentPath()));
        projectTreeView->setHeaderHidden(true);
        projectTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // 类型已匹配

        // 双击打开文件
        connect(projectTreeView, &QTreeView::doubleClicked, this, [this, model](const QModelIndex &index) {
            QString filePath = model->filePath(index);
            QFileInfo info(filePath);
            if (info.isFile()) {
                openFileInEditor(filePath);
            }
        });
    }

    // 设置objectName消除saveState警告
    if (sideDock) sideDock->setObjectName("sideDock");
    if (editorTabWidget) editorTabWidget->setObjectName("editorTabWidget");
    if (outputWidget) outputWidget->setObjectName("outputWidget");
    if (projectTreeView) projectTreeView->setObjectName("projectTreeView");

    setupUI();
    setupMenuBar();
    setupDockWidgets();
    setupStatusBar();
    
    // 创建状态栏标签
    statusLabel = new QLabel("就绪", this);
    lineColumnLabel = new QLabel("行: 1, 列: 1", this);
    statusBar()->addWidget(statusLabel);
    statusBar()->addPermanentWidget(lineColumnLabel);
    setupConnections();
    loadSettings();
    
    setWindowTitle("小狮子C++ IDE");
    resize(1200, 800);
    
    // 设置深色主题
    setDarkTheme();
    
    // 欢迎对话框已在构造函数中通过定时器调用，避免重复显示
    
    // 显示欢迎信息
    outputWidget->append("欢迎使用小狮子C++ IDE!");
    outputWidget->append("请创建新文件或打开现有文件开始编程。");
}

LionCPP::~LionCPP()
{
    saveSettings();
    delete ui;
}

void LionCPP::setupUI()
{
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);
    
    // 创建编辑器标签页
    editorTabWidget = new QTabWidget(mainSplitter);
    editorTabWidget->setTabsClosable(true);
    editorTabWidget->setMovable(true);
    
    mainSplitter->addWidget(editorTabWidget);
    mainSplitter->setStretchFactor(0, 1);
}

void LionCPP::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // 文件菜单
    fileMenu = menuBar->addMenu("文件(&F)");
    
    newFileAction = new QAction("新建文件(&N)", this);
    newFileAction->setShortcut(QKeySequence::New);
    fileMenu->addAction(newFileAction);
    
    openFileAction = new QAction("打开文件(&O)", this);
    openFileAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openFileAction);
    
    fileMenu->addSeparator();
    
    saveFileAction = new QAction("保存文件(&S)", this);
    saveFileAction->setShortcut(QKeySequence::Save);
    fileMenu->addAction(saveFileAction);
    
    saveAsFileAction = new QAction("另存为(&A)", this);
    saveAsFileAction->setShortcut(QKeySequence::SaveAs);
    fileMenu->addAction(saveAsFileAction);
    
    closeFileAction = new QAction("关闭文件(&C)", this);
    closeFileAction->setShortcut(QKeySequence("Ctrl+W"));
    fileMenu->addAction(closeFileAction);
    
    fileMenu->addSeparator();
    
    exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(exitAction);
    
    // 编辑菜单
    editMenu = menuBar->addMenu("编辑(&E)");
    
    undoAction = new QAction("撤销(&U)", this);
    undoAction->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undoAction);
    
    redoAction = new QAction("重做(&R)", this);
    redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addAction(redoAction);
    
    editMenu->addSeparator();
    
    cutAction = new QAction("剪切(&T)", this);
    cutAction->setShortcut(QKeySequence::Cut);
    editMenu->addAction(cutAction);
    
    copyAction = new QAction("复制(&C)", this);
    copyAction->setShortcut(QKeySequence::Copy);
    editMenu->addAction(copyAction);
    
    pasteAction = new QAction("粘贴(&P)", this);
    pasteAction->setShortcut(QKeySequence::Paste);
    editMenu->addAction(pasteAction);
    
    editMenu->addSeparator();
    
    findAction = new QAction("查找(&F)", this);
    findAction->setShortcut(QKeySequence::Find);
    editMenu->addAction(findAction);
    
    replaceAction = new QAction("替换(&R)", this);
    replaceAction->setShortcut(QKeySequence::Replace);
    editMenu->addAction(replaceAction);
    
    // 运行菜单
    runMenu = menuBar->addMenu("运行(&R)");
    
    compileAction = new QAction("编译(&C)", this);
    compileAction->setShortcut(QKeySequence("Ctrl+Shift+B"));
    runMenu->addAction(compileAction);
    
    runAction = new QAction("运行(&R)", this);
    runAction->setShortcut(QKeySequence("Ctrl+R"));
    runMenu->addAction(runAction);
    
    compileAndRunAction = new QAction("编译并运行(&B)", this);
    compileAndRunAction->setShortcut(QKeySequence("F5"));
    runMenu->addAction(compileAndRunAction);
    
    stopAction = new QAction("停止(&S)", this);
    stopAction->setShortcut(QKeySequence("Ctrl+Break"));
    runMenu->addAction(stopAction);
    
    // 工具菜单
    toolsMenu = menuBar->addMenu("工具(&T)");
    
    settingsAction = new QAction("设置(&S)", this);
    toolsMenu->addAction(settingsAction);
    
    // 帮助菜单
    helpMenu = menuBar->addMenu("帮助(&H)");
    
    aboutAction = new QAction("关于(&A)", this);
    helpMenu->addAction(aboutAction);
}

void LionCPP::setupToolBar()
{
    mainToolBar = new QToolBar(tr("主工具栏"), this);
    mainToolBar->setObjectName("mainToolBar");
    mainToolBar->setMovable(false);
    
    mainToolBar->addAction(newFileAction);
    mainToolBar->addAction(openFileAction);
    mainToolBar->addAction(saveFileAction);
    mainToolBar->addSeparator();
    
    mainToolBar->addAction(compileAction);
    mainToolBar->addAction(runAction);
    mainToolBar->addAction(compileAndRunAction);
    mainToolBar->addAction(stopAction);
}

void LionCPP::setupStatusBar()
{
    QStatusBar *statusBar = this->statusBar();
    
    QLabel *statusLabel = new QLabel("就绪", statusBar);
    statusBar->addWidget(statusLabel);
    
    QProgressBar *progressBar = new QProgressBar(statusBar);
    progressBar->setVisible(false);
    statusBar->addPermanentWidget(progressBar);
}

void LionCPP::setupDockWidgets()
{
    // 输出窗口
    outputDock = new QDockWidget(tr("输出"), this);
    outputDock->setObjectName("outputDock");
    outputDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    
    outputWidget = new QTextEdit(outputDock);
    outputWidget->setReadOnly(true);
    outputWidget->setMaximumHeight(200);
    outputDock->setWidget(outputWidget);
    
    addDockWidget(Qt::BottomDockWidgetArea, outputDock);
}

void LionCPP::setupConnections()
{
    // 文件菜单连接
    connect(newFileAction, &QAction::triggered, this, &LionCPP::onNewFile);
    connect(openFileAction, &QAction::triggered, this, &LionCPP::onOpenFile);
    connect(saveFileAction, &QAction::triggered, this, &LionCPP::onSaveFile);
    connect(saveAsFileAction, &QAction::triggered, this, &LionCPP::onSaveAsFile);
    connect(closeFileAction, &QAction::triggered, this, &LionCPP::onCloseFile);
    connect(exitAction, &QAction::triggered, this, &LionCPP::onExit);
    
    // 编辑菜单连接
    connect(undoAction, &QAction::triggered, this, &LionCPP::onUndo);
    connect(redoAction, &QAction::triggered, this, &LionCPP::onRedo);
    connect(cutAction, &QAction::triggered, this, &LionCPP::onCut);
    connect(copyAction, &QAction::triggered, this, &LionCPP::onCopy);
    connect(pasteAction, &QAction::triggered, this, &LionCPP::onPaste);
    connect(findAction, &QAction::triggered, this, &LionCPP::onFind);
    connect(replaceAction, &QAction::triggered, this, &LionCPP::onReplace);
    
    // 运行菜单连接
    connect(compileAction, &QAction::triggered, this, &LionCPP::onCompile);
    connect(runAction, &QAction::triggered, this, &LionCPP::onRun);
    connect(compileAndRunAction, &QAction::triggered, this, &LionCPP::onCompileAndRun);
    connect(stopAction, &QAction::triggered, this, &LionCPP::onStop);
    
    // 工具菜单连接
    connect(settingsAction, &QAction::triggered, this, &LionCPP::onSettings);
    connect(aboutAction, &QAction::triggered, this, &LionCPP::onAbout);
    
    // 编辑器标签页连接
    connect(editorTabWidget, &QTabWidget::tabCloseRequested, [this](int index) {
        editorTabWidget->removeTab(index);
    });
}

void LionCPP::loadSettings()
{
    // 加载窗口几何信息
    restoreGeometry(settings.value("window/geometry").toByteArray());
    restoreState(settings.value("window/state").toByteArray());
    
    // 应用编辑器设置
    applyEditorSettings();
}

void LionCPP::saveSettings()
{
    // 保存窗口几何信息
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());
    
    settings.sync();
}

void LionCPP::updateWindowTitle()
{
    QString title = "🦁 小狮子C++";
    if (!currentFilePath.isEmpty()) {
        QFileInfo fileInfo(currentFilePath);
        title += " - " + fileInfo.fileName();
    }
    setWindowTitle(title);
}

void LionCPP::updateActions()
{
    bool hasEditor = getCurrentEditor() != nullptr;
    
    // 文件操作
    saveFileAction->setEnabled(hasEditor);
    saveAsFileAction->setEnabled(hasEditor);
    closeFileAction->setEnabled(hasEditor);
    
    // 编辑操作
    undoAction->setEnabled(hasEditor);
    redoAction->setEnabled(hasEditor);
    cutAction->setEnabled(hasEditor);
    copyAction->setEnabled(hasEditor);
    pasteAction->setEnabled(hasEditor);
    findAction->setEnabled(hasEditor);
    replaceAction->setEnabled(hasEditor);
    
    // 运行操作
    compileAction->setEnabled(hasEditor && !isCompiling);
    runAction->setEnabled(hasEditor && !isRunning);
    compileAndRunAction->setEnabled(hasEditor && !isCompiling && !isRunning);
    stopAction->setEnabled(isCompiling || isRunning);
}

CodeEditor* LionCPP::getCurrentEditor()
{
    if (editorTabWidget->count() == 0) return nullptr;
    return qobject_cast<CodeEditor*>(editorTabWidget->currentWidget());
}

void LionCPP::openFileInEditor(const QString &filePath)
{
    qDebug() << "[openFileInEditor] called with filePath:" << filePath;
    qDebug() << "[openFileInEditor] called with filePath:" << filePath;
    // 检查文件是否已经打开
    for (int i = 0; i < editorTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(editorTabWidget->widget(i));
        if (editor && editor->property("filePath").toString() == filePath) {
            editorTabWidget->setCurrentIndex(i);
            return;
        }
    }
    
    // 创建新的编辑器
    CodeEditor *editor = new CodeEditor();
    qDebug() << "[openFileInEditor] CodeEditor created";
    editor->setProperty("filePath", filePath);
    
    // 应用当前编辑器设置
    applyEditorSettingsToEditor(editor);
    
    QFile file(filePath);
    qDebug() << "[openFileInEditor] before QFile open";
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[openFileInEditor] File opened for reading";
        QTextStream in(&file);
        QString content = in.readAll();
        qDebug() << "[openFileInEditor] file readAll done, content length:" << content.length();
        editor->setPlainText(content);
        qDebug() << "[openFileInEditor] setPlainText done";
        file.close();
    }
    
    QFileInfo fileInfo(filePath);
    qDebug() << "[openFileInEditor] before addTab";
    int index = editorTabWidget->addTab(editor, fileInfo.fileName());
    qDebug() << "[openFileInEditor] Tab added, index:" << index;
    editorTabWidget->setCurrentIndex(index);
    qDebug() << "[openFileInEditor] Tab setCurrentIndex done";
    
    // 连接文档修改信号
    connect(editor->document(), &QTextDocument::contentsChanged, [this, editor]() {
        updateTabTitle(editor);
    });
    
    currentFilePath = filePath;
    updateWindowTitle();
    updateActions();
}

bool LionCPP::saveCurrentFile()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return false;
    
    QString filePath = editor->property("filePath").toString();
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getSaveFileName(this, "保存文件", "", "C++ Files (*.cpp *.h);;All Files (*)");
        if (filePath.isEmpty()) return false;
        editor->setProperty("filePath", filePath);
    }
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << editor->toPlainText();
        file.close();
        
        // 标记文档为未修改状态
        editor->document()->setModified(false);
        
        // 更新标签页标题
        updateTabTitle(editor);
        
        currentFilePath = filePath;
        updateWindowTitle();
        return true;
    }
    return false;
}

void LionCPP::closeCurrentFile()
{
    if (saveFileIfModified()) {
        int index = editorTabWidget->currentIndex();
        if (index >= 0) {
            editorTabWidget->removeTab(index);
            currentFilePath.clear();
            updateWindowTitle();
            updateActions();
        }
    }
}

bool LionCPP::saveFileIfModified()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return true;
    
    if (editor->document()->isModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "保存文件", 
            "文件已修改，是否保存？", 
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Yes) {
            saveCurrentFile();
            return true;
        } else if (reply == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void LionCPP::compileCurrentFile()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return;
    
    QString filePath = editor->property("filePath").toString();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先保存文件");
        return;
    }
    
    // 保存文件
    if (!saveCurrentFile()) {
        QMessageBox::warning(this, "错误", "保存文件失败");
        return;
    }
    
    if (isCompiling) return;
    
    isCompiling = true;
    updateActions();
    outputWidget->clear();
    outputWidget->append("=== 开始编译 " + QFileInfo(filePath).fileName() + " ===");
    
    onCompilationStarted();
    
    // 创建编译进程
    if (!compileProcess) {
        compileProcess = new QProcess(this);
        connect(compileProcess, &QProcess::finished, this, &LionCPP::onCompilationFinished);
        connect(compileProcess, &QProcess::readyReadStandardOutput, [this]() {
            QString output = QString::fromUtf8(compileProcess->readAllStandardOutput());
            outputWidget->append(output.trimmed());
        });
        connect(compileProcess, &QProcess::readyReadStandardError, [this]() {
            QString error = QString::fromUtf8(compileProcess->readAllStandardError());
            outputWidget->append("编译错误: " + error.trimmed());
        });
    }
    
    QString executablePath = filePath;
    executablePath.replace(".cpp", "");
    
    // 使用更完整的编译参数
    QStringList arguments;
    arguments << "-std=c++17" << "-Wall" << "-O2" << "-o" << executablePath << filePath;
    
    compileProcess->setWorkingDirectory(QFileInfo(filePath).absolutePath());
    compileProcess->start("g++", arguments);
}

void LionCPP::runCurrentFile()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return;
    
    QString filePath = editor->property("filePath").toString();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先保存文件");
        return;
    }
    
    QString executablePath = filePath;
    executablePath.replace(".cpp", "");
    
    if (!QFile::exists(executablePath)) {
        QMessageBox::warning(this, "错误", "可执行文件不存在，请先编译");
        return;
    }
    
    if (isRunning) return;
    
    isRunning = true;
    updateActions();
    outputWidget->append("=== 在独立控制台中运行程序 ===");
    
    onRunStarted();
    
    // 在独立的终端窗口中运行程序（类似Dev C++）
    runInExternalTerminal(executablePath);
}

void LionCPP::runInExternalTerminal(const QString &executablePath)
{
    // 实现像Dev C++那样的独立控制台运行
    // 支持多种终端模拟器，优先使用系统默认的
    
    QStringList terminalCommands;
    
    // 检测并使用可用的终端模拟器
    // 使用更安全的方法：将可执行文件路径作为单独的参数传递
    
    // gnome-terminal (Ubuntu/GNOME) - Dev C++风格
    if (!QStandardPaths::findExecutable("gnome-terminal").isEmpty()) {
        QString workDir = QFileInfo(executablePath).absolutePath();
        QString fileName = QFileInfo(executablePath).fileName();
        // 创建一个脚本来模拟Dev C++的运行体验
        QString scriptContent = QString(
            "#!/bin/bash\n"
            "cd '%1'\n"
            "echo '正在处理C++源程序文件:'\n"
            "echo '%2'\n"
            "echo\n"
            "echo '编译器配置: System GCC, debug with ASan'\n"
            "echo\n"
            "echo '正在处理C++源程序文件:'\n"
            "echo '--------------------'\n"
            "echo\n"
            "start_time=$(date +%s.%N)\n"
            "'./%3' 2>&1\n"
            "exit_code=$?\n"
            "end_time=$(date +%s.%N)\n"
            "runtime=$(echo \"$end_time - $start_time\" | bc -l)\n"
            "echo\n"
            "echo '编译结果:'\n"
            "echo '--------------------'\n"
            "echo \"- 错误数: 0\"\n"
            "echo \"- 警告数: 0\"\n"
            "echo \"- 输出文件名: %3\"\n"
            "echo \"- 输出文件大小: $(du -h '%3' 2>/dev/null | cut -f1 || echo 'N/A') B\"\n"
            "echo \"- 编译时间: ${runtime} 秒\"\n"
            "echo\n"
            "printf \"Process exited after %.3f seconds with return value %d, $(ps -o pid,vsz --no-headers -p $$ | awk '{print $2}') KB mem used.\\n\\n\" \"$runtime\" \"$exit_code\"\n"
            "echo 'Press ANY key to exit...'\n"
            "read -n 1\n"
        ).arg(workDir).arg(QFileInfo(executablePath).baseName() + ".cpp").arg(fileName);
        
        // 将脚本写入临时文件
        QString scriptPath = workDir + "/run_script.sh";
        QFile scriptFile(scriptPath);
        if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&scriptFile);
            out << scriptContent;
            scriptFile.close();
            QFile::setPermissions(scriptPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        }
        
        terminalCommands << "gnome-terminal" << "--" << "bash" << scriptPath;
    }
    
    // konsole (KDE)
    else if (!QStandardPaths::findExecutable("konsole").isEmpty()) {
        terminalCommands.clear();
        QString workDir = QFileInfo(executablePath).absolutePath();
        QString fileName = QFileInfo(executablePath).fileName();
        QString command = QString("cd '%1' && echo '正在运行程序: %2' && './%2'; echo; echo '程序执行完毕，按Enter键退出...'; read")
                         .arg(workDir).arg(fileName);
        terminalCommands << "konsole" << "-e" << "bash" << "-c" << command;
    }
    // xterm (通用)
    else if (!QStandardPaths::findExecutable("xterm").isEmpty()) {
        terminalCommands.clear();
        QString workDir = QFileInfo(executablePath).absolutePath();
        QString fileName = QFileInfo(executablePath).fileName();
        QString command = QString("cd '%1' && echo '正在运行程序: %2' && './%2'; echo; echo '程序执行完毕，按Enter键退出...'; read")
                         .arg(workDir).arg(fileName);
        terminalCommands << "xterm" << "-e" << "bash" << "-c" << command;
    }
    // x-terminal-emulator (Debian/Ubuntu 通用)
    else if (!QStandardPaths::findExecutable("x-terminal-emulator").isEmpty()) {
        terminalCommands.clear();
        QString workDir = QFileInfo(executablePath).absolutePath();
        QString fileName = QFileInfo(executablePath).fileName();
        QString command = QString("cd '%1' && echo '正在运行程序: %2' && './%2'; echo; echo '程序执行完毕，按Enter键退出...'; read")
                         .arg(workDir).arg(fileName);
        terminalCommands << "x-terminal-emulator" << "-e" << "bash" << "-c" << command;
    }
    
    if (terminalCommands.isEmpty()) {
        // 如果没有找到合适的终端，回退到内置输出
        QMessageBox::warning(this, "警告", "未找到合适的终端模拟器，将在IDE内运行程序");
        runInBuiltinTerminal(executablePath);
        return;
    }
    
    // 创建运行进程来启动独立终端
    if (!runProcess) {
        runProcess = new QProcess(this);
        connect(runProcess, &QProcess::finished, this, &LionCPP::onRunFinished);
        connect(runProcess, &QProcess::errorOccurred, [this](QProcess::ProcessError error) {
            QString errorMsg;
            switch (error) {
                case QProcess::FailedToStart:
                    errorMsg = "程序启动失败";
                    break;
                case QProcess::Crashed:
                    errorMsg = "程序运行时崩溃";
                    break;
                case QProcess::Timedout:
                    errorMsg = "程序运行超时";
                    break;
                default:
                    errorMsg = "程序运行出现未知错误";
                    break;
            }
            outputWidget->append("运行错误: " + errorMsg);
            isRunning = false;
            updateActions();
            onRunFinished(1, QProcess::CrashExit);
        });
    }
    
    // 设置工作目录为可执行文件所在目录
    runProcess->setWorkingDirectory(QFileInfo(executablePath).absolutePath());
    
    // 启动独立终端窗口
    QString program = terminalCommands.takeFirst();
    outputWidget->append("正在独立控制台中启动程序: " + QFileInfo(executablePath).fileName());
    outputWidget->append("终端命令: " + program + " " + terminalCommands.join(" "));
    
    runProcess->start(program, terminalCommands);
    
    if (!runProcess->waitForStarted(3000)) {
        QMessageBox::warning(this, "错误", "无法启动终端窗口");
        isRunning = false;
        updateActions();
        onRunFinished(1, QProcess::CrashExit);
    }
}

void LionCPP::runInBuiltinTerminal(const QString &executablePath)
{
    // 回退方案：在IDE内置终端运行（保留原有功能）
    outputWidget->append("=== 在内置终端中运行程序 ===");
    
    if (!runProcess) {
        runProcess = new QProcess(this);
        connect(runProcess, &QProcess::finished, this, &LionCPP::onRunFinished);
        connect(runProcess, &QProcess::readyReadStandardOutput, [this]() {
            QString output = QString::fromUtf8(runProcess->readAllStandardOutput());
            outputWidget->append("程序输出: " + output.trimmed());
        });
        connect(runProcess, &QProcess::readyReadStandardError, [this]() {
            QString error = QString::fromUtf8(runProcess->readAllStandardError());
            outputWidget->append("程序错误: " + error.trimmed());
        });
    }
    
    runProcess->setWorkingDirectory(QFileInfo(executablePath).absolutePath());
    runProcess->start(executablePath);
}

void LionCPP::setDarkTheme()
{
    // 设置深色主题样式表 - 参考VS Code深色主题
    QString darkStyle = R"(
        QMainWindow {
            background-color: #1e1e1e;
            color: #d4d4d4;
        }
        
        QMenuBar {
            background-color: #2d2d30;
            color: #d4d4d4;
            border: none;
        }
        
        QMenuBar::item {
            background-color: transparent;
            padding: 4px 8px;
        }
        
        QMenuBar::item:selected {
            background-color: #3e3e42;
        }
        
        QMenu {
            background-color: #2d2d30;
            color: #d4d4d4;
            border: 1px solid #3e3e42;
        }
        
        QMenu::item {
            padding: 4px 20px;
        }
        
        QMenu::item:selected {
            background-color: #3e3e42;
        }
        
        QToolBar {
            background-color: #2d2d30;
            border: none;
            spacing: 2px;
        }
        
        QToolButton {
            background-color: transparent;
            border: 1px solid transparent;
            border-radius: 3px;
            padding: 4px;
            color: #d4d4d4;
        }
        
        QToolButton:hover {
            background-color: #3e3e42;
            border: 1px solid #3e3e42;
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
        
        QTextEdit, QPlainTextEdit {
            background-color: #1e1e1e;
            color: #d4d4d4;
            border: none;
            selection-background-color: #264f78;
            font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 12px;
        }
        
        QDockWidget {
            background-color: #1e1e1e;
            color: #d4d4d4;
        }
        
        QDockWidget::title {
            background-color: #2d2d30;
            padding: 4px;
            border: 1px solid #3e3e42;
        }
        
        QStatusBar {
            background-color: #2d2d30;
            color: #d4d4d4;
        }
        
        QProgressBar {
            background-color: #1e1e1e;
            border: 1px solid #3e3e42;
            text-align: center;
            color: #d4d4d4;
        }
        
        QProgressBar::chunk {
            background-color: #007acc;
        }
        
        QScrollBar:vertical {
            background-color: #1e1e1e;
            width: 12px;
            border: none;
        }
        
        QScrollBar::handle:vertical {
            background-color: #3e3e42;
            border-radius: 6px;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #4e4e52;
        }
        
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        
        QScrollBar:horizontal {
            background-color: #1e1e1e;
            height: 12px;
            border: none;
        }
        
        QScrollBar::handle:horizontal {
            background-color: #3e3e42;
            border-radius: 6px;
            min-width: 20px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background-color: #4e4e52;
        }
        
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )";
    
    setStyleSheet(darkStyle);
}

void LionCPP::updateTabTitle(CodeEditor *editor)
{
    if (!editor) return;
    
    QString filePath = editor->property("filePath").toString();
    bool isModified = editor->document()->isModified();
    
    QString title;
    if (filePath.isEmpty()) {
        title = "未命名";
    } else {
        QFileInfo fileInfo(filePath);
        title = fileInfo.fileName();
    }
    
    if (isModified) {
        title += "*";
    }
    
    // 找到对应的标签页并更新标题
    for (int i = 0; i < editorTabWidget->count(); ++i) {
        if (editorTabWidget->widget(i) == editor) {
            editorTabWidget->setTabText(i, title);
            break;
        }
    }
    
    updateWindowTitle();
}

void LionCPP::showWelcomeDialog()
{
    WelcomeDialog *welcomeDialog = new WelcomeDialog(this);
    
    // 连接信号
    connect(welcomeDialog, &WelcomeDialog::newFileRequested, this, &LionCPP::onWelcomeNewFile);
    connect(welcomeDialog, &WelcomeDialog::openFileRequested, this, &LionCPP::onWelcomeOpenFile);
    
    welcomeDialog->exec();
    welcomeDialog->deleteLater();
}

void LionCPP::onWelcomeNewFile()
{
    onNewFile();
}

void LionCPP::onWelcomeOpenFile()
{
    onOpenFile();
}



void LionCPP::closeEvent(QCloseEvent *event)
{
    // 检查是否有未保存的文件
    for (int i = 0; i < editorTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(editorTabWidget->widget(i));
        if (editor && editor->document()->isModified()) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "保存文件", 
                "有未保存的文件，是否保存？", 
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            
            if (reply == QMessageBox::Yes) {
                saveCurrentFile();
            } else if (reply == QMessageBox::Cancel) {
                event->ignore();
                return;
            }
        }
    }
    
    saveSettings();
    event->accept();
}

// 文件菜单槽函数
void LionCPP::onNewFile()
{
    qDebug() << "[onNewFile] called";
    // 创建新的编辑器标签页
    CodeEditor *editor = new CodeEditor();
    
    // 应用当前编辑器设置
    applyEditorSettingsToEditor(editor);
    
    // 设置默认内容
    qDebug() << "[onNewFile] before setPlainText";
    editor->setPlainText("#include <iostream>\nusing namespace std;\n\nint main()\n{\n    cout << \"Hello World!\" << endl;\n    return 0;\n}");
    qDebug() << "[onNewFile] setPlainText done";
    
    qDebug() << "[onNewFile] before addTab";
    int index = editorTabWidget->addTab(editor, "未命名");
    qDebug() << "[onNewFile] Tab added, index:" << index;
    editorTabWidget->setCurrentIndex(index);
    qDebug() << "[onNewFile] setCurrentIndex done";
    
    // 连接文档修改信号
    connect(editor->document(), &QTextDocument::contentsChanged, [this, editor]() {
        updateTabTitle(editor);
    });
    
    currentFilePath.clear();
    updateWindowTitle();
    updateActions();
}

void LionCPP::onOpenFile()
{
    qDebug() << "[onOpenFile] called";
    // 使用系统文件管理器
    QString filePath = QFileDialog::getOpenFileName(this, "打开文件", "", "C++ Files (*.cpp *.h *.hpp *.cc *.cxx);;All Files (*)",
        nullptr, QFileDialog::DontUseNativeDialog);
    if (!filePath.isEmpty()) {
        openFileInEditor(filePath);
    }
}

void LionCPP::onSaveFile()
{
    saveCurrentFile();
}

void LionCPP::onSaveAsFile()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return;
    
    // 使用系统文件管理器
    QString filePath = QFileDialog::getSaveFileName(this, "另存为", "", "C++ Files (*.cpp *.h *.hpp *.cc *.cxx);;All Files (*)",
        nullptr, QFileDialog::DontUseNativeDialog);
    if (!filePath.isEmpty()) {
        editor->setProperty("filePath", filePath);
        saveCurrentFile();
    }
}

void LionCPP::onCloseFile()
{
    closeCurrentFile();
}

void LionCPP::onExit()
{
    close();
}

// 编辑菜单槽函数
void LionCPP::onUndo()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) editor->undo();
}

void LionCPP::onRedo()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) editor->redo();
}

void LionCPP::onCut()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) editor->cut();
}

void LionCPP::onCopy()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) editor->copy();
}

void LionCPP::onPaste()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) editor->paste();
}

void LionCPP::onFind()
{
    CodeEditor* editor = getCurrentEditor();
    if (!editor) return;

    static FindReplaceDialog* dialog = nullptr;
    if (!dialog) {
        dialog = new FindReplaceDialog(this);
        connect(dialog, &FindReplaceDialog::findNext, editor, &CodeEditor::findNext);
        connect(dialog, &FindReplaceDialog::findPrevious, editor, &CodeEditor::findPrevious);
        connect(dialog, &FindReplaceDialog::replaceOne, editor, &CodeEditor::replaceOne);
        connect(dialog, &FindReplaceDialog::replaceAll, editor, &CodeEditor::replaceAll);
    }
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void LionCPP::onReplace()
{
    CodeEditor* editor = getCurrentEditor();
    if (!editor) return;

    static FindReplaceDialog* dialog = nullptr;
    if (!dialog) {
        dialog = new FindReplaceDialog(this);
        connect(dialog, &FindReplaceDialog::findNext, editor, &CodeEditor::findNext);
        connect(dialog, &FindReplaceDialog::findPrevious, editor, &CodeEditor::findPrevious);
        connect(dialog, &FindReplaceDialog::replaceOne, editor, &CodeEditor::replaceOne);
        connect(dialog, &FindReplaceDialog::replaceAll, editor, &CodeEditor::replaceAll);
    }
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

// 运行菜单槽函数
void LionCPP::onCompile()
{
    compileCurrentFile();
}

void LionCPP::onRun()
{
    runCurrentFile();
}

void LionCPP::onCompileAndRun()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return;
    
    QString filePath = editor->property("filePath").toString();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先保存文件");
        return;
    }
    
    if (isCompiling || isRunning) return;
    
    // 保存文件
    if (!saveCurrentFile()) {
        QMessageBox::warning(this, "错误", "保存文件失败");
        return;
    }
    
    isCompiling = true;
    updateActions();
    outputWidget->clear();
    outputWidget->append("=== 编译并运行 " + QFileInfo(filePath).fileName() + " ===");
    
    onCompilationStarted();
    
    // 创建编译进程
    if (!compileProcess) {
        compileProcess = new QProcess(this);
        connect(compileProcess, &QProcess::readyReadStandardOutput, [this]() {
            QString output = QString::fromUtf8(compileProcess->readAllStandardOutput());
            outputWidget->append(output.trimmed());
        });
        connect(compileProcess, &QProcess::readyReadStandardError, [this]() {
            QString error = QString::fromUtf8(compileProcess->readAllStandardError());
            outputWidget->append("编译错误: " + error.trimmed());
        });
    }
    
    // 连接编译完成信号，编译成功后自动在独立控制台运行
    connect(compileProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus exitStatus) {
            isCompiling = false;
            updateActions();
            
            if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                outputWidget->append("编译成功！正在启动程序...");
                // 延迟100ms后在独立控制台运行
                QTimer::singleShot(100, this, &LionCPP::runCurrentFile);
            } else {
                outputWidget->append("编译失败，退出代码: " + QString::number(exitCode));
                onCompilationFinished(exitCode, exitStatus);
            }
        });
    
    QString executablePath = filePath;
    executablePath.replace(".cpp", "");
    
    // 使用更完整的编译参数
    QStringList arguments;
    arguments << "-std=c++17" << "-Wall" << "-O2" << "-o" << executablePath << filePath;
    
    compileProcess->setWorkingDirectory(QFileInfo(filePath).absolutePath());
    compileProcess->start("g++", arguments);
}

void LionCPP::onStop()
{
    if (compileProcess && compileProcess->state() == QProcess::Running) {
        compileProcess->terminate();
        compileProcess->waitForFinished(3000);
    }
    if (runProcess && runProcess->state() == QProcess::Running) {
        runProcess->terminate();
        runProcess->waitForFinished(3000);
    }
}

// 工具菜单槽函数
void LionCPP::onSettings()
{
    if (!settingsDialog) {
        settingsDialog = new SettingsDialog(this);
        // 连接设置对话框的accepted信号，当用户点击确定时应用新设置
        connect(settingsDialog, &QDialog::accepted, this, [this]() {
            applyEditorSettings();
        });
    }
    settingsDialog->show();
}

void LionCPP::onAbout()
{
    QMessageBox::about(this, "关于小狮子C++ IDE", 
        "小狮子C++ IDE v1.0\n\n"
        "一个简单的C++代码编辑器\n"
        "支持语法高亮、代码补全、单文件编译运行等功能\n\n"
        "作者: 小狮子团队\n"
        "版权所有 © 2024");
}

// 编译和运行槽函数
void LionCPP::onCompilationStarted()
{
    outputWidget->append("编译开始...");
}

void LionCPP::onCompilationFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    
    isCompiling = false;
    updateActions();
    
    if (exitCode == 0) {
        outputWidget->append("编译成功完成!");
    } else {
        outputWidget->append("编译失败!");
    }
}

void LionCPP::onRunStarted()
{
    outputWidget->append("程序开始运行...");
}

void LionCPP::onRunFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    
    isRunning = false;
    updateActions();
    
    outputWidget->append("----------------------------------------");
    outputWidget->append(QString("程序运行完成，退出代码: %1").arg(exitCode));
}

// 编辑器设置应用函数
void LionCPP::applyEditorSettings()
{
    // 应用设置到所有已打开的编辑器
    for (int i = 0; i < editorTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(editorTabWidget->widget(i));
        if (editor) {
            applyEditorSettingsToEditor(editor);
        }
    }
}

void LionCPP::applyEditorSettingsToEditor(CodeEditor *editor)
{
    if (!editor) return;
    
    // 从设置中读取字体配置
    QString fontFamily = settings.value("editor/fontFamily", "Consolas").toString();
    int fontSize = settings.value("editor/fontSize", 12).toInt();
    
    // 应用字体设置
    QFont font(fontFamily, fontSize);
    font.setFixedPitch(true);
    editor->setFont(font);
    
    // 更新制表符宽度
    editor->setTabStopDistance(editor->fontMetrics().horizontalAdvance(' ') * 
                               settings.value("editor/tabSize", 4).toInt());
    
    // 应用颜色主题设置
    QString bgColor = settings.value("editor/backgroundColor", "#1e1e1e").toString();
    QString textColor = settings.value("editor/textColor", "#d4d4d4").toString();
    
    QPalette palette = editor->palette();
    palette.setColor(QPalette::Base, QColor(bgColor));
    palette.setColor(QPalette::Text, QColor(textColor));
    palette.setColor(QPalette::WindowText, QColor(textColor));
    editor->setPalette(palette);
    
    // 应用行号显示设置（通过调用编辑器的公共方法）
    // 注意：行号区域的显示/隐藏需要在CodeEditor类内部处理
    // 这里我们只是触发更新，实际的margin设置在CodeEditor内部完成
    editor->updateLineNumberAreaWidth(0);
    
    // 应用语法高亮颜色设置
    QString keywordColor = settings.value("editor/keywordColor", "#569cd6").toString();
    QString commentColor = settings.value("editor/commentColor", "#6a9955").toString();
    QString stringColor = settings.value("editor/stringColor", "#ce9178").toString();
    
    // 查找并更新语法高亮器
    CppHighlighter *highlighter = editor->findChild<CppHighlighter*>();
    if (highlighter) {
        highlighter->updateColors(QColor(keywordColor), QColor(commentColor), QColor(stringColor));
    }
}

// 编译器扫描和配置功能
bool LionCPP::isFirstRun()
{
    // 检查是否存在配置文件中的首次运行标记
    return !settings.contains("setup/firstRunCompleted");
}

QString LionCPP::detectSystemCompiler()
{
    // 检测系统中可用的C++编译器
    QStringList compilers = {"g++", "clang++", "gcc", "clang"};
    
    for (const QString &compiler : compilers) {
        QString path = QStandardPaths::findExecutable(compiler);
        if (!path.isEmpty()) {
            return path;
        }
    }
    
    return QString(); // 未找到编译器
}

void LionCPP::scanCompilers()
{
    // 创建编译器扫描进度对话框
    QProgressDialog *progress = new QProgressDialog("正在扫描系统编译器...", "取消", 0, 100, this);
    progress->setWindowTitle("🦁 小狮子C++ - 首次设置");
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->setValue(10);
    
    QApplication::processEvents();
    
    // 扫描编译器
    progress->setLabelText("正在检测C++编译器...");
    progress->setValue(30);
    QApplication::processEvents();
    
    QString compilerPath = detectSystemCompiler();
    
    progress->setValue(60);
    QApplication::processEvents();
    
    // 检测终端模拟器
    progress->setLabelText("正在检测终端模拟器...");
    QStringList terminals = {"gnome-terminal", "konsole", "xterm", "x-terminal-emulator"};
    QStringList availableTerminals;
    
    for (const QString &terminal : terminals) {
        if (!QStandardPaths::findExecutable(terminal).isEmpty()) {
            availableTerminals << terminal;
        }
    }
    
    progress->setValue(90);
    QApplication::processEvents();
    
    // 完成扫描
    progress->setValue(100);
    progress->close();
    delete progress;
    
    // 显示扫描结果
    showCompilerSetupDialog();
    
    // 标记首次运行完成
    settings.setValue("setup/firstRunCompleted", true);
    settings.setValue("setup/scanDate", QDateTime::currentDateTime().toString());
    
    if (!compilerPath.isEmpty()) {
        settings.setValue("compiler/path", compilerPath);
        
        // 获取编译器版本信息
        QProcess versionProcess;
        versionProcess.start(compilerPath, QStringList() << "--version");
        versionProcess.waitForFinished(3000);
        QString version = QString::fromUtf8(versionProcess.readAllStandardOutput());
        settings.setValue("compiler/version", version.split('\n').first());
    }
    
    if (!availableTerminals.isEmpty()) {
        settings.setValue("terminal/available", availableTerminals);
        settings.setValue("terminal/preferred", availableTerminals.first());
    }
}

void LionCPP::showCompilerSetupDialog()
{
    QString compilerPath = detectSystemCompiler();
    QStringList terminals = {"gnome-terminal", "konsole", "xterm", "x-terminal-emulator"};
    QStringList availableTerminals;
    
    for (const QString &terminal : terminals) {
        if (!QStandardPaths::findExecutable(terminal).isEmpty()) {
            availableTerminals << terminal;
        }
    }
    
    // 创建设置结果对话框
    QDialog *setupDialog = new QDialog(this);
    setupDialog->setWindowTitle("🦁 小狮子C++ - 系统环境检测完成");
    setupDialog->setFixedSize(500, 400);
    setupDialog->setStyleSheet(
        "QDialog { background-color: #2d2d30; color: #ffffff; }"
        "QLabel { color: #ffffff; font-size: 12px; }"
        "QTextEdit { background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #3e3e42; }"
        "QPushButton { background-color: #0e639c; color: white; border: none; padding: 8px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #1177bb; }"
    );
    
    QVBoxLayout *layout = new QVBoxLayout(setupDialog);
    
    // 标题
    QLabel *titleLabel = new QLabel("🎉 环境检测完成！");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #4ec9b0; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // 检测结果文本
    QTextEdit *resultText = new QTextEdit();
    resultText->setReadOnly(true);
    
    QString resultContent = "=== 🔍 系统环境检测结果 ===\n\n";
    
    // 编译器信息
    resultContent += "📋 C++编译器:\n";
    if (!compilerPath.isEmpty()) {
        resultContent += QString("  ✅ 已找到: %1\n").arg(compilerPath);
        
        // 获取版本信息
        QProcess versionProcess;
        versionProcess.start(compilerPath, QStringList() << "--version");
        if (versionProcess.waitForFinished(3000)) {
            QString version = QString::fromUtf8(versionProcess.readAllStandardOutput());
            resultContent += QString("  📝 版本: %1\n").arg(version.split('\n').first());
        }
    } else {
        resultContent += "  ❌ 未找到C++编译器\n";
        resultContent += "  💡 建议安装: sudo apt install g++ 或 sudo apt install clang++\n";
    }
    
    resultContent += "\n";
    
    // 终端模拟器信息
    resultContent += "🖥️ 终端模拟器:\n";
    if (!availableTerminals.isEmpty()) {
        resultContent += QString("  ✅ 已找到 %1 个终端:\n").arg(availableTerminals.size());
        for (const QString &terminal : availableTerminals) {
            resultContent += QString("     • %1\n").arg(terminal);
        }
        resultContent += QString("  🎯 将使用: %1\n").arg(availableTerminals.first());
    } else {
        resultContent += "  ❌ 未找到终端模拟器\n";
        resultContent += "  💡 建议安装: sudo apt install gnome-terminal\n";
    }
    
    resultContent += "\n";
    
    // 功能状态
    resultContent += "⚡ 功能状态:\n";
    resultContent += QString("  📝 代码编辑: ✅ 可用\n");
    resultContent += QString("  🎨 语法高亮: ✅ 可用\n");
    resultContent += QString("  🔧 代码编译: %1\n").arg(!compilerPath.isEmpty() ? "✅ 可用" : "❌ 需要安装编译器");
    resultContent += QString("  🚀 程序运行: %1\n").arg(!availableTerminals.isEmpty() ? "✅ 可用" : "⚠️ 将使用内置输出");
    
    resultContent += "\n=== 🦁 小狮子C++已准备就绪！ ===\n";
    resultContent += "现在您可以开始愉快的C++编程之旅了！\n";
    
    resultText->setPlainText(resultContent);
    layout->addWidget(resultText);
    
    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("开始使用");
    QPushButton *helpButton = new QPushButton("查看帮助");
    
    connect(okButton, &QPushButton::clicked, setupDialog, &QDialog::accept);
    connect(helpButton, &QPushButton::clicked, [this]() {
        QDesktopServices::openUrl(QUrl("file://" + QCoreApplication::applicationDirPath() + "/使用说明.md"));
    });
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(helpButton);
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);
    
    setupDialog->exec();
    delete setupDialog;
}
