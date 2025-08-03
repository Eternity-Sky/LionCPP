#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(QObject *parent = nullptr);
    ~ProjectManager();

    void setProjectTree(QTreeWidget *tree);
    void openProject(const QString &path);
    void createNewProject(const QString &name, const QString &path);
    void addFileToProject(const QString &filePath);
    void removeFileFromProject(const QString &filePath);
    QString getProjectPath() const { return projectPath; }
    QString getProjectName() const { return projectName; }
    QStringList getSourceFiles() const { return sourceFiles; }
    QStringList getHeaderFiles() const { return headerFiles; }

signals:
    void projectOpened(const QString &projectPath);
    void projectClosed();
    void fileAdded(const QString &filePath);
    void fileRemoved(const QString &filePath);

private slots:
    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onTreeContextMenu(const QPoint &pos);
    void onAddNewFile();
    void onAddExistingFile();
    void onRemoveFile();
    void onOpenFile();
    void onRenameFile();

private:
    void setupProjectTree();
    void loadProjectFiles();
    void saveProjectFile();
    void createCMakeLists();
    void createMainCpp();
    QTreeWidgetItem* createFileItem(const QString &filePath, const QString &fileName);
    void updateProjectTree();

    QTreeWidget *projectTree;
    QString projectPath;
    QString projectName;
    QStringList sourceFiles;
    QStringList headerFiles;
    QStringList otherFiles;
}; 