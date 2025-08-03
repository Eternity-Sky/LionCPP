#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextEdit>
#include <QDir>
#include <QFileInfo>

class Compiler : public QObject
{
    Q_OBJECT

public:
    explicit Compiler(QObject *parent = nullptr);
    ~Compiler();

    void setOutputWidget(QTextEdit *widget);
    void setProjectPath(const QString &path);
    void setProjectName(const QString &name);
    
    void compile();
    void run();
    void clean();
    void build();
    
    bool isCompiling() const { return compiling; }
    bool isRunning() const { return running; }

signals:
    void compilationStarted();
    void compilationFinished(bool success, const QString &output);
    void runStarted();
    void runFinished(int exitCode, const QString &output);
    void buildStarted();
    void buildFinished(bool success, const QString &output);

private slots:
    void onCompilationFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onRunFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onBuildFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessOutput();
    void onProcessError();

private:
    void setupCMake();
    void setupBuildDirectory();
    QString findCompiler();
    void appendOutput(const QString &text);
    
    QProcess *compileProcess;
    QProcess *runProcess;
    QProcess *buildProcess;
    QTextEdit *outputWidget;
    QString projectPath;
    QString projectName;
    QString buildPath;
    bool compiling;
    bool running;
    QString currentOutput;
}; 