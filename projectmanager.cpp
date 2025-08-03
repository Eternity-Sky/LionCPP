#include "projectmanager.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ProjectManager::ProjectManager(QObject *parent)
    : QObject(parent)
    , projectTree(nullptr)
{
}

ProjectManager::~ProjectManager()
{
}

void ProjectManager::setProjectTree(QTreeWidget *tree)
{
    projectTree = tree;
    setupProjectTree();
}

void ProjectManager::setupProjectTree()
{
    if (!projectTree) return;

    projectTree->setHeaderLabel("项目文件");
    projectTree->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(projectTree, &QTreeWidget::itemDoubleClicked,
            this, &ProjectManager::onTreeItemDoubleClicked);
    connect(projectTree, &QTreeWidget::customContextMenuRequested,
            this, &ProjectManager::onTreeContextMenu);
}

void ProjectManager::openProject(const QString &path)
{
    projectPath = path;
    QFileInfo fileInfo(path);
    projectName = fileInfo.baseName();
    
    loadProjectFiles();
    updateProjectTree();
    
    emit projectOpened(path);
}

void ProjectManager::createNewProject(const QString &name, const QString &path)
{
    projectName = name;
    projectPath = path;
    
    // 创建项目目录
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // 创建项目文件
    sourceFiles.clear();
    headerFiles.clear();
    otherFiles.clear();
    
    // 创建默认文件
    createMainCpp();
    createCMakeLists();
    
    // 保存项目文件
    saveProjectFile();
    updateProjectTree();
    
    emit projectOpened(path);
}

void ProjectManager::loadProjectFiles()
{
    sourceFiles.clear();
    headerFiles.clear();
    otherFiles.clear();
    
    QDir projectDir(projectPath);
    QStringList filters;
    filters << "*.cpp" << "*.c" << "*.cc" << "*.cxx";
    sourceFiles = projectDir.entryList(filters, QDir::Files);
    
    filters.clear();
    filters << "*.h" << "*.hpp" << "*.hh" << "*.hxx";
    headerFiles = projectDir.entryList(filters, QDir::Files);
    
    // 加载项目配置文件
    QString projectFile = projectPath + "/" + projectName + ".lionproj";
    if (QFile::exists(projectFile)) {
        QFile file(projectFile);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonObject obj = doc.object();
            
            QJsonArray sources = obj["sourceFiles"].toArray();
            for (const QJsonValue &value : sources) {
                sourceFiles.append(value.toString());
            }
            
            QJsonArray headers = obj["headerFiles"].toArray();
            for (const QJsonValue &value : headers) {
                headerFiles.append(value.toString());
            }
        }
    }
}

void ProjectManager::saveProjectFile()
{
    QJsonObject obj;
    obj["projectName"] = projectName;
    obj["projectPath"] = projectPath;
    
    QJsonArray sources;
    for (const QString &file : sourceFiles) {
        sources.append(file);
    }
    obj["sourceFiles"] = sources;
    
    QJsonArray headers;
    for (const QString &file : headerFiles) {
        headers.append(file);
    }
    obj["headerFiles"] = headers;
    
    QJsonDocument doc(obj);
    QString projectFile = projectPath + "/" + projectName + ".lionproj";
    QFile file(projectFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

void ProjectManager::createCMakeLists()
{
    QString cmakeContent = QString(
        "cmake_minimum_required(VERSION 3.14)\n"
        "project(%1 VERSION 0.1 LANGUAGES CXX)\n\n"
        "set(CMAKE_AUTOUIC ON)\n"
        "set(CMAKE_AUTOMOC ON)\n"
        "set(CMAKE_AUTORCC ON)\n\n"
        "set(CMAKE_CXX_STANDARD 17)\n"
        "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
        "set(CMAKE_CXX_EXTENSIONS OFF)\n\n"
        "set(CMAKE_INCLUDE_CURRENT_DIR ON)\n\n"
        "find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)\n"
        "find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)\n\n"
        "set(PROJECT_SOURCES\n"
        "    main.cpp\n"
        ")\n\n"
        "if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)\n"
        "    qt_add_executable(%1\n"
        "        MANUAL_FINALIZATION\n"
        "        ${PROJECT_SOURCES}\n"
        "    )\n"
        "else()\n"
        "    add_executable(%1\n"
        "        ${PROJECT_SOURCES}\n"
        "    )\n"
        "endif()\n\n"
        "target_link_libraries(%1 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)\n\n"
        "if(QT_VERSION_MAJOR EQUAL 6)\n"
        "    qt_finalize_executable(%1)\n"
        "endif()\n"
    ).arg(projectName);
    
    QString cmakePath = projectPath + "/CMakeLists.txt";
    QFile file(cmakePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << cmakeContent;
    }
}

void ProjectManager::createMainCpp()
{
    QString mainContent = QString(
        "#include <QApplication>\n"
        "#include <QMainWindow>\n"
        "#include <QLabel>\n\n"
        "int main(int argc, char *argv[])\n"
        "{\n"
        "    QApplication app(argc, argv);\n\n"
        "    QMainWindow window;\n"
        "    window.setWindowTitle(\"%1\");\n"
        "    window.resize(800, 600);\n\n"
        "    QLabel *label = new QLabel(\"欢迎使用小狮子C++ IDE!\");\n"
        "    label->setAlignment(Qt::AlignCenter);\n"
        "    window.setCentralWidget(label);\n\n"
        "    window.show();\n"
        "    return app.exec();\n"
        "}\n"
    ).arg(projectName);
    
    QString mainPath = projectPath + "/main.cpp";
    QFile file(mainPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << mainContent;
    }
    
    sourceFiles.append("main.cpp");
}

void ProjectManager::addFileToProject(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "cpp" || extension == "c" || extension == "cc" || extension == "cxx") {
        if (!sourceFiles.contains(fileInfo.fileName())) {
            sourceFiles.append(fileInfo.fileName());
        }
    } else if (extension == "h" || extension == "hpp" || extension == "hh" || extension == "hxx") {
        if (!headerFiles.contains(fileInfo.fileName())) {
            headerFiles.append(fileInfo.fileName());
        }
    } else {
        if (!otherFiles.contains(fileInfo.fileName())) {
            otherFiles.append(fileInfo.fileName());
        }
    }
    
    saveProjectFile();
    updateProjectTree();
    emit fileAdded(filePath);
}

void ProjectManager::removeFileFromProject(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    
    sourceFiles.removeAll(fileName);
    headerFiles.removeAll(fileName);
    otherFiles.removeAll(fileName);
    
    saveProjectFile();
    updateProjectTree();
    emit fileRemoved(filePath);
}

void ProjectManager::updateProjectTree()
{
    if (!projectTree) return;
    
    projectTree->clear();
    
    // 添加源文件
    if (!sourceFiles.isEmpty()) {
        QTreeWidgetItem *sourceItem = new QTreeWidgetItem(projectTree);
        sourceItem->setText(0, "源文件");
        sourceItem->setIcon(0, QIcon(":/icons/source.png"));
        
        for (const QString &file : sourceFiles) {
            createFileItem(projectPath + "/" + file, file);
        }
    }
    
    // 添加头文件
    if (!headerFiles.isEmpty()) {
        QTreeWidgetItem *headerItem = new QTreeWidgetItem(projectTree);
        headerItem->setText(0, "头文件");
        headerItem->setIcon(0, QIcon(":/icons/header.png"));
        
        for (const QString &file : headerFiles) {
            createFileItem(projectPath + "/" + file, file);
        }
    }
    
    // 添加其他文件
    if (!otherFiles.isEmpty()) {
        QTreeWidgetItem *otherItem = new QTreeWidgetItem(projectTree);
        otherItem->setText(0, "其他文件");
        otherItem->setIcon(0, QIcon(":/icons/file.png"));
        
        for (const QString &file : otherFiles) {
            createFileItem(projectPath + "/" + file, file);
        }
    }
    
    projectTree->expandAll();
}

QTreeWidgetItem* ProjectManager::createFileItem(const QString &filePath, const QString &fileName)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(projectTree);
    item->setText(0, fileName);
    item->setData(0, Qt::UserRole, filePath);
    
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "cpp" || extension == "c" || extension == "cc" || extension == "cxx") {
        item->setIcon(0, QIcon(":/icons/cpp.png"));
    } else if (extension == "h" || extension == "hpp" || extension == "hh" || extension == "hxx") {
        item->setIcon(0, QIcon(":/icons/h.png"));
    } else {
        item->setIcon(0, QIcon(":/icons/file.png"));
    }
    
    return item;
}

void ProjectManager::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    
    if (item->childCount() == 0) { // 文件项
        QString filePath = item->data(0, Qt::UserRole).toString();
        if (!filePath.isEmpty()) {
            emit fileAdded(filePath);
        }
    }
}

void ProjectManager::onTreeContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = projectTree->itemAt(pos);
    if (!item) return;
    
    QMenu menu;
    
    if (item->childCount() == 0) { // 文件项
        menu.addAction("打开文件", this, &ProjectManager::onOpenFile);
        menu.addAction("重命名", this, &ProjectManager::onRenameFile);
        menu.addSeparator();
        menu.addAction("从项目中移除", this, &ProjectManager::onRemoveFile);
    } else { // 文件夹项
        menu.addAction("添加新文件", this, &ProjectManager::onAddNewFile);
        menu.addAction("添加现有文件", this, &ProjectManager::onAddExistingFile);
    }
    
    menu.exec(projectTree->mapToGlobal(pos));
}

void ProjectManager::onAddNewFile()
{
    QString fileName = QInputDialog::getText(nullptr, "新建文件", "文件名:");
    if (fileName.isEmpty()) return;
    
    QString filePath = projectPath + "/" + fileName;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        addFileToProject(filePath);
    }
}

void ProjectManager::onAddExistingFile()
{
    QString filePath = QFileDialog::getOpenFileName(nullptr, "选择文件", projectPath);
    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        QString newPath = projectPath + "/" + fileInfo.fileName();
        
        if (QFile::copy(filePath, newPath)) {
            addFileToProject(newPath);
        }
    }
}

void ProjectManager::onRemoveFile()
{
    QTreeWidgetItem *item = projectTree->currentItem();
    if (!item) return;
    
    QString filePath = item->data(0, Qt::UserRole).toString();
    if (!filePath.isEmpty()) {
        removeFileFromProject(filePath);
    }
}

void ProjectManager::onOpenFile()
{
    QTreeWidgetItem *item = projectTree->currentItem();
    if (!item) return;
    
    QString filePath = item->data(0, Qt::UserRole).toString();
    if (!filePath.isEmpty()) {
        emit fileAdded(filePath);
    }
}

void ProjectManager::onRenameFile()
{
    QTreeWidgetItem *item = projectTree->currentItem();
    if (!item) return;
    
    QString oldPath = item->data(0, Qt::UserRole).toString();
    if (oldPath.isEmpty()) return;
    
    QFileInfo fileInfo(oldPath);
    QString newName = QInputDialog::getText(nullptr, "重命名文件", "新文件名:", QLineEdit::Normal, fileInfo.fileName());
    if (newName.isEmpty() || newName == fileInfo.fileName()) return;
    
    QString newPath = projectPath + "/" + newName;
    QFile file(oldPath);
    if (file.rename(newPath)) {
        removeFileFromProject(oldPath);
        addFileToProject(newPath);
    }
} 