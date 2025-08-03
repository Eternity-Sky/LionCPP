#include "compiler.h"
#include <QDebug>
#include <QStandardPaths>
#include <QApplication>
#include <QMessageBox>

Compiler::Compiler(QObject *parent)
    : QObject(parent)
    , compileProcess(nullptr)
    , runProcess(nullptr)
    , buildProcess(nullptr)
    , outputWidget(nullptr)
    , compiling(false)
    , running(false)
{
    compileProcess = new QProcess(this);
    runProcess = new QProcess(this);
    buildProcess = new QProcess(this);
    
    connect(compileProcess, &QProcess::finished,
            this, &Compiler::onCompilationFinished);
    connect(compileProcess, &QProcess::readyReadStandardOutput,
            this, &Compiler::onProcessOutput);
    connect(compileProcess, &QProcess::readyReadStandardError,
            this, &Compiler::onProcessError);
    
    connect(runProcess, &QProcess::finished,
            this, &Compiler::onRunFinished);
    connect(runProcess, &QProcess::readyReadStandardOutput,
            this, &Compiler::onProcessOutput);
    connect(runProcess, &QProcess::readyReadStandardError,
            this, &Compiler::onProcessError);
    
    connect(buildProcess, &QProcess::finished,
            this, &Compiler::onBuildFinished);
    connect(buildProcess, &QProcess::readyReadStandardOutput,
            this, &Compiler::onProcessOutput);
    connect(buildProcess, &QProcess::readyReadStandardError,
            this, &Compiler::onProcessError);
}

Compiler::~Compiler()
{
    if (compileProcess && compileProcess->state() == QProcess::Running) {
        compileProcess->terminate();
        compileProcess->waitForFinished(3000);
    }
    if (runProcess && runProcess->state() == QProcess::Running) {
        runProcess->terminate();
        runProcess->waitForFinished(3000);
    }
    if (buildProcess && buildProcess->state() == QProcess::Running) {
        buildProcess->terminate();
        buildProcess->waitForFinished(3000);
    }
}

void Compiler::setOutputWidget(QTextEdit *widget)
{
    outputWidget = widget;
}

void Compiler::setProjectPath(const QString &path)
{
    projectPath = path;
    buildPath = path + "/build";
}

void Compiler::setProjectName(const QString &name)
{
    projectName = name;
}

void Compiler::compile()
{
    if (compiling) return;
    
    compiling = true;
    currentOutput.clear();
    appendOutput("开始编译项目: " + projectName + "\n");
    
    emit compilationStarted();
    
    setupBuildDirectory();
    setupCMake();
    
    QStringList arguments;
    arguments << "--build" << buildPath;
    arguments << "--config" << "Debug";
    
    compileProcess->setWorkingDirectory(projectPath);
    compileProcess->start("cmake", arguments);
}

void Compiler::run()
{
    if (running) return;
    
    QString executablePath = buildPath + "/" + projectName;
    if (!QFile::exists(executablePath)) {
        appendOutput("错误: 可执行文件不存在，请先编译项目\n");
        return;
    }
    
    running = true;
    currentOutput.clear();
    appendOutput("开始运行程序: " + projectName + "\n");
    
    emit runStarted();
    
    runProcess->setWorkingDirectory(buildPath);
    runProcess->start(executablePath);
}

void Compiler::clean()
{
    if (compiling) return;
    
    QDir buildDir(buildPath);
    if (buildDir.exists()) {
        buildDir.removeRecursively();
        appendOutput("清理构建目录完成\n");
    }
}

void Compiler::build()
{
    if (compiling) return;
    
    compiling = true;
    currentOutput.clear();
    appendOutput("开始构建项目: " + projectName + "\n");
    
    emit buildStarted();
    
    setupBuildDirectory();
    setupCMake();
    
    QStringList arguments;
    arguments << "--build" << buildPath;
    arguments << "--config" << "Debug";
    
    buildProcess->setWorkingDirectory(projectPath);
    buildProcess->start("cmake", arguments);
}

void Compiler::setupCMake()
{
    if (!QFile::exists(buildPath + "/CMakeCache.txt")) {
        QStringList arguments;
        arguments << "-S" << projectPath;
        arguments << "-B" << buildPath;
        arguments << "-DCMAKE_BUILD_TYPE=Debug";
        
        QProcess cmakeProcess;
        cmakeProcess.setWorkingDirectory(projectPath);
        cmakeProcess.start("cmake", arguments);
        cmakeProcess.waitForFinished();
        
        if (cmakeProcess.exitCode() != 0) {
            appendOutput("CMake配置失败: " + cmakeProcess.readAllStandardError() + "\n");
        } else {
            appendOutput("CMake配置成功\n");
        }
    }
}

void Compiler::setupBuildDirectory()
{
    QDir buildDir(buildPath);
    if (!buildDir.exists()) {
        buildDir.mkpath(".");
    }
}

QString Compiler::findCompiler()
{
    QStringList compilers = {"g++", "clang++", "gcc", "clang"};
    
    for (const QString &compiler : compilers) {
        QProcess process;
        process.start("which", QStringList() << compiler);
        process.waitForFinished();
        
        if (process.exitCode() == 0) {
            return compiler;
        }
    }
    
    return "g++"; // 默认编译器
}

void Compiler::appendOutput(const QString &text)
{
    currentOutput += text;
    if (outputWidget) {
        outputWidget->append(text);
        outputWidget->ensureCursorVisible();
    }
}

void Compiler::onCompilationFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    
    compiling = false;
    
    if (exitCode == 0) {
        appendOutput("编译成功完成!\n");
        emit compilationFinished(true, currentOutput);
    } else {
        appendOutput("编译失败，退出代码: " + QString::number(exitCode) + "\n");
        emit compilationFinished(false, currentOutput);
    }
}

void Compiler::onRunFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    
    running = false;
    
    appendOutput("程序运行完成，退出代码: " + QString::number(exitCode) + "\n");
    emit runFinished(exitCode, currentOutput);
}

void Compiler::onBuildFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    
    compiling = false;
    
    if (exitCode == 0) {
        appendOutput("构建成功完成!\n");
        emit buildFinished(true, currentOutput);
    } else {
        appendOutput("构建失败，退出代码: " + QString::number(exitCode) + "\n");
        emit buildFinished(false, currentOutput);
    }
}

void Compiler::onProcessOutput()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (process) {
        QString output = QString::fromUtf8(process->readAllStandardOutput());
        appendOutput(output);
    }
}

void Compiler::onProcessError()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (process) {
        QString error = QString::fromUtf8(process->readAllStandardError());
        appendOutput("错误: " + error);
    }
} 