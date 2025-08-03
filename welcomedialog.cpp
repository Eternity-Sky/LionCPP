#include "welcomedialog.h"
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

WelcomeDialog::WelcomeDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setupConnections();
    setDarkTheme();
    
    setWindowTitle("🦁 小狮子C++ - 欢迎");
    setFixedSize(700, 500);  // 增加窗口大小
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
}

WelcomeDialog::~WelcomeDialog()
{
}

void WelcomeDialog::setupUI()
{
    // 主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(25);  // 增加间距
    mainLayout->setContentsMargins(40, 40, 40, 40);  // 增加边距
    
    // 内容布局
    contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(50);  // 增加左右间距
    
    // 左侧布局 - Logo和标题
    leftLayout = new QVBoxLayout();
    leftLayout->setAlignment(Qt::AlignCenter);
    leftLayout->setSpacing(15);  // 设置左侧元素间距
    
    // Logo
    logoLabel = new QLabel();
    logoLabel->setText("🦁");
    logoLabel->setAlignment(Qt::AlignCenter);
    QFont logoFont;
    logoFont.setPointSize(72);
    logoLabel->setFont(logoFont);
    logoLabel->setStyleSheet("color: #FFA500; margin: 20px;");
    
    // 标题
    titleLabel = new QLabel("小狮子C++");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #FFFFFF; margin: 10px;");
    
    // 副标题
    subtitleLabel = new QLabel("极简C++ IDE");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    QFont subtitleFont;
    subtitleFont.setPointSize(14);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setStyleSheet("color: #CCCCCC; margin: 5px;");
    
    // 版本信息
    versionLabel = new QLabel("版本 1.0.0");
    versionLabel->setAlignment(Qt::AlignCenter);
    QFont versionFont;
    versionFont.setPointSize(10);
    versionLabel->setFont(versionFont);
    versionLabel->setStyleSheet("color: #888888; margin: 5px;");
    
    // 作者信息
    authorLabel = new QLabel("作者: Eternity-Sky\nGitHub: Eternity-Sky\nB站: WIN_CTRL");
    authorLabel->setAlignment(Qt::AlignCenter);
    QFont authorFont;
    authorFont.setPointSize(11);  // 稍微增大字体
    authorLabel->setFont(authorFont);
    authorLabel->setStyleSheet("color: #CCCCCC; margin: 15px; line-height: 1.8;");  // 优化颜色和行间距
    authorLabel->setWordWrap(true);  // 允许换行
    authorLabel->setMinimumHeight(80);  // 设置最小高度
    
    leftLayout->addWidget(logoLabel);
    leftLayout->addWidget(titleLabel);
    leftLayout->addWidget(subtitleLabel);
    leftLayout->addWidget(versionLabel);
    leftLayout->addWidget(authorLabel);
    leftLayout->addStretch();
    
    // 分隔线
    separatorLine = new QFrame();
    separatorLine->setFrameShape(QFrame::VLine);
    separatorLine->setFrameShadow(QFrame::Sunken);
    separatorLine->setStyleSheet("color: #444444;");
    
    // 右侧布局 - 操作按钮
    rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignTop);
    rightLayout->setSpacing(15);
    
    // 按钮样式
    QString buttonStyle = 
        "QPushButton {"
        "    background-color: #3C3C3C;"
        "    border: 1px solid #555555;"
        "    border-radius: 8px;"
        "    color: #FFFFFF;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    padding: 12px 24px;"
        "    text-align: left;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4A4A4A;"
        "    border-color: #777777;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #2A2A2A;"
        "}";
    
    // 新建文件按钮
    newFileBtn = new QPushButton("📄 新建文件");
    newFileBtn->setStyleSheet(buttonStyle);
    newFileBtn->setMinimumHeight(50);
    
    // 打开文件按钮
    openFileBtn = new QPushButton("📂 打开文件");
    openFileBtn->setStyleSheet(buttonStyle);
    openFileBtn->setMinimumHeight(50);
    

    
    // 关于按钮
    aboutBtn = new QPushButton("ℹ️ 关于");
    aboutBtn->setStyleSheet(buttonStyle);
    aboutBtn->setMinimumHeight(50);
    
    rightLayout->addWidget(newFileBtn);
    rightLayout->addWidget(openFileBtn);
    rightLayout->addSpacing(30);  // 增加间距
    rightLayout->addWidget(aboutBtn);
    rightLayout->addStretch();
    
    // 组装布局
    contentLayout->addLayout(leftLayout, 1);
    contentLayout->addWidget(separatorLine);
    contentLayout->addLayout(rightLayout, 1);
    
    mainLayout->addLayout(contentLayout);
}

void WelcomeDialog::setupConnections()
{
    connect(newFileBtn, &QPushButton::clicked, this, &WelcomeDialog::onNewFile);
    connect(openFileBtn, &QPushButton::clicked, this, &WelcomeDialog::onOpenFile);
    connect(aboutBtn, &QPushButton::clicked, this, &WelcomeDialog::onAbout);
}

void WelcomeDialog::setDarkTheme()
{
    setStyleSheet(
        "QDialog {"
        "    background-color: #2B2B2B;"
        "    color: #FFFFFF;"
        "}"
    );
}

void WelcomeDialog::onNewFile()
{
    emit newFileRequested();
    accept();
}

void WelcomeDialog::onOpenFile()
{
    emit openFileRequested();
    accept();
}



void WelcomeDialog::onAbout()
{
    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle("关于小狮子C++");
    aboutBox.setStyleSheet(
        "QMessageBox {"
        "    background-color: #2B2B2B;"
        "    color: #FFFFFF;"
        "    font-size: 14px;"
        "}"
        "QMessageBox QLabel {"
        "    color: #FFFFFF;"
        "    font-size: 14px;"
        "    padding: 10px;"
        "}"
        "QMessageBox QPushButton {"
        "    background-color: #4A90E2;"
        "    border: 1px solid #357ABD;"
        "    border-radius: 6px;"
        "    color: #FFFFFF;"
        "    font-weight: bold;"
        "    padding: 10px 20px;"
        "    min-width: 80px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "    background-color: #357ABD;"
        "    border-color: #2968A3;"
        "}"
        "QMessageBox QPushButton:pressed {"
        "    background-color: #2968A3;"
        "}"
    );
    
    aboutBox.setText(
        "<div style='text-align: center; padding: 20px;'>"
        "<h2 style='color: #FFA500; margin-bottom: 20px;'>🦁 小狮子C++</h2>"
        "<p style='font-size: 16px; margin: 10px 0;'><b style='color: #4A90E2;'>版本:</b> <span style='color: #FFFFFF;'>1.0.0</span></p>"
        "<p style='font-size: 16px; margin: 10px 0;'><b style='color: #4A90E2;'>作者:</b> <span style='color: #FFFFFF;'>Eternity-Sky</span></p>"
        "<p style='font-size: 16px; margin: 10px 0;'><b style='color: #4A90E2;'>GitHub:</b> <a href='https://github.com/Eternity-Sky' style='color: #58A6FF; text-decoration: none;'>Eternity-Sky</a></p>"
        "<p style='font-size: 16px; margin: 10px 0;'><b style='color: #4A90E2;'>B站:</b> <a href='https://space.bilibili.com/WIN_CTRL' style='color: #FB7299; text-decoration: none;'>WIN_CTRL</a></p>"
        "<br>"
        "<p style='font-size: 15px; color: #E0E0E0; margin: 15px 0;'>一个专为C++开发者打造的极简IDE</p>"
        "<p style='font-size: 15px; color: #E0E0E0; margin: 15px 0;'>轻量 • 高效 • 现代化</p>"
        "<br>"
        "<p style='color: #999999; font-size: 13px; margin-top: 20px;'>基于Qt 6.9构建 | 使用C++17标准</p>"
        "</div>"
    );
    
    aboutBox.exec();
}
