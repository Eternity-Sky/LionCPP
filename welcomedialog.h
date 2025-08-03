#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QFont>
#include <QFrame>
#include <QSpacerItem>

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *parent = nullptr);
    ~WelcomeDialog();

private slots:
    void onNewFile();
    void onOpenFile();
    void onAbout();

private:
    void setupUI();
    void setupConnections();
    void setDarkTheme();
    
    // UI 组件
    QVBoxLayout *mainLayout;
    QHBoxLayout *contentLayout;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    
    QLabel *logoLabel;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QLabel *versionLabel;
    QLabel *authorLabel;
    
    QPushButton *newFileBtn;
    QPushButton *openFileBtn;
    QPushButton *aboutBtn;
    
    QFrame *separatorLine;
    QSpacerItem *spacer;

signals:
    void newFileRequested();
    void openFileRequested();
};
