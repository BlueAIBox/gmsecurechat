/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QWidget *leftPanel;
    QVBoxLayout *leftLayout;
    QGroupBox *loginGroup;
    QVBoxLayout *loginGroupLayout;
    QLineEdit *nickInput;
    QLineEdit *manualIpInput;
    QPushButton *loginBtn;
    QGroupBox *onlineGroup;
    QVBoxLayout *onlineGroupLayout;
    QListWidget *userList;
    QWidget *centerPanel;
    QVBoxLayout *centerLayout;
    QGroupBox *chatGroup;
    QVBoxLayout *chatGroupLayout;
    QTextEdit *chatDisplay;
    QGroupBox *inputGroup;
    QHBoxLayout *inputGroupLayout;
    QTextEdit *msgInput;
    QVBoxLayout *btnLayout;
    QPushButton *sendMsgBtn;
    QPushButton *sendFileBtn;
    QWidget *rightPanel;
    QVBoxLayout *rightLayout;
    QGroupBox *netLogGroup;
    QVBoxLayout *netLogGroupLayout;
    QTextEdit *netLog;
    QGroupBox *cryptoLogGroup;
    QVBoxLayout *cryptoLogGroupLayout;
    QTextEdit *cryptoLog;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1000, 680);
        MainWindow->setMinimumSize(QSize(900, 600));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setSpacing(8);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(8, 8, 8, 8);
        leftPanel = new QWidget(centralwidget);
        leftPanel->setObjectName(QString::fromUtf8("leftPanel"));
        leftPanel->setMinimumSize(QSize(200, 0));
        leftPanel->setMaximumSize(QSize(240, 16777215));
        leftLayout = new QVBoxLayout(leftPanel);
        leftLayout->setSpacing(6);
        leftLayout->setObjectName(QString::fromUtf8("leftLayout"));
        leftLayout->setContentsMargins(4, 4, 4, 4);
        loginGroup = new QGroupBox(leftPanel);
        loginGroup->setObjectName(QString::fromUtf8("loginGroup"));
        loginGroupLayout = new QVBoxLayout(loginGroup);
        loginGroupLayout->setSpacing(6);
        loginGroupLayout->setObjectName(QString::fromUtf8("loginGroupLayout"));
        nickInput = new QLineEdit(loginGroup);
        nickInput->setObjectName(QString::fromUtf8("nickInput"));
        nickInput->setMaxLength(20);

        loginGroupLayout->addWidget(nickInput);

        manualIpInput = new QLineEdit(loginGroup);
        manualIpInput->setObjectName(QString::fromUtf8("manualIpInput"));

        loginGroupLayout->addWidget(manualIpInput);

        loginBtn = new QPushButton(loginGroup);
        loginBtn->setObjectName(QString::fromUtf8("loginBtn"));
        loginBtn->setMinimumSize(QSize(0, 32));
        loginBtn->setCursor(QCursor(Qt::PointingHandCursor));

        loginGroupLayout->addWidget(loginBtn);


        leftLayout->addWidget(loginGroup);

        onlineGroup = new QGroupBox(leftPanel);
        onlineGroup->setObjectName(QString::fromUtf8("onlineGroup"));
        onlineGroupLayout = new QVBoxLayout(onlineGroup);
        onlineGroupLayout->setSpacing(2);
        onlineGroupLayout->setObjectName(QString::fromUtf8("onlineGroupLayout"));
        onlineGroupLayout->setContentsMargins(2, 2, 2, 2);
        userList = new QListWidget(onlineGroup);
        userList->setObjectName(QString::fromUtf8("userList"));

        onlineGroupLayout->addWidget(userList);


        leftLayout->addWidget(onlineGroup);


        horizontalLayout->addWidget(leftPanel);

        centerPanel = new QWidget(centralwidget);
        centerPanel->setObjectName(QString::fromUtf8("centerPanel"));
        centerLayout = new QVBoxLayout(centerPanel);
        centerLayout->setSpacing(6);
        centerLayout->setObjectName(QString::fromUtf8("centerLayout"));
        centerLayout->setContentsMargins(4, 4, 4, 4);
        chatGroup = new QGroupBox(centerPanel);
        chatGroup->setObjectName(QString::fromUtf8("chatGroup"));
        chatGroupLayout = new QVBoxLayout(chatGroup);
        chatGroupLayout->setSpacing(4);
        chatGroupLayout->setObjectName(QString::fromUtf8("chatGroupLayout"));
        chatGroupLayout->setContentsMargins(2, 2, 2, 2);
        chatDisplay = new QTextEdit(chatGroup);
        chatDisplay->setObjectName(QString::fromUtf8("chatDisplay"));
        chatDisplay->setReadOnly(true);
        chatDisplay->setFrameShape(QFrame::NoFrame);

        chatGroupLayout->addWidget(chatDisplay);


        centerLayout->addWidget(chatGroup);

        inputGroup = new QGroupBox(centerPanel);
        inputGroup->setObjectName(QString::fromUtf8("inputGroup"));
        inputGroupLayout = new QHBoxLayout(inputGroup);
        inputGroupLayout->setSpacing(6);
        inputGroupLayout->setObjectName(QString::fromUtf8("inputGroupLayout"));
        msgInput = new QTextEdit(inputGroup);
        msgInput->setObjectName(QString::fromUtf8("msgInput"));
        msgInput->setMaximumSize(QSize(16777215, 72));
        msgInput->setFrameShape(QFrame::NoFrame);
        msgInput->setAcceptRichText(false);

        inputGroupLayout->addWidget(msgInput);

        btnLayout = new QVBoxLayout();
        btnLayout->setSpacing(4);
        btnLayout->setObjectName(QString::fromUtf8("btnLayout"));
        sendMsgBtn = new QPushButton(inputGroup);
        sendMsgBtn->setObjectName(QString::fromUtf8("sendMsgBtn"));
        sendMsgBtn->setMinimumSize(QSize(90, 32));
        sendMsgBtn->setCursor(QCursor(Qt::PointingHandCursor));

        btnLayout->addWidget(sendMsgBtn);

        sendFileBtn = new QPushButton(inputGroup);
        sendFileBtn->setObjectName(QString::fromUtf8("sendFileBtn"));
        sendFileBtn->setMinimumSize(QSize(90, 32));
        sendFileBtn->setCursor(QCursor(Qt::PointingHandCursor));

        btnLayout->addWidget(sendFileBtn);


        inputGroupLayout->addLayout(btnLayout);


        centerLayout->addWidget(inputGroup);


        horizontalLayout->addWidget(centerPanel);

        rightPanel = new QWidget(centralwidget);
        rightPanel->setObjectName(QString::fromUtf8("rightPanel"));
        rightPanel->setMinimumSize(QSize(260, 0));
        rightPanel->setMaximumSize(QSize(320, 16777215));
        rightLayout = new QVBoxLayout(rightPanel);
        rightLayout->setSpacing(6);
        rightLayout->setObjectName(QString::fromUtf8("rightLayout"));
        rightLayout->setContentsMargins(4, 4, 4, 4);
        netLogGroup = new QGroupBox(rightPanel);
        netLogGroup->setObjectName(QString::fromUtf8("netLogGroup"));
        netLogGroupLayout = new QVBoxLayout(netLogGroup);
        netLogGroupLayout->setSpacing(0);
        netLogGroupLayout->setObjectName(QString::fromUtf8("netLogGroupLayout"));
        netLogGroupLayout->setContentsMargins(2, 2, 2, 2);
        netLog = new QTextEdit(netLogGroup);
        netLog->setObjectName(QString::fromUtf8("netLog"));
        netLog->setReadOnly(true);
        netLog->setFrameShape(QFrame::NoFrame);

        netLogGroupLayout->addWidget(netLog);


        rightLayout->addWidget(netLogGroup);

        cryptoLogGroup = new QGroupBox(rightPanel);
        cryptoLogGroup->setObjectName(QString::fromUtf8("cryptoLogGroup"));
        cryptoLogGroupLayout = new QVBoxLayout(cryptoLogGroup);
        cryptoLogGroupLayout->setSpacing(0);
        cryptoLogGroupLayout->setObjectName(QString::fromUtf8("cryptoLogGroupLayout"));
        cryptoLogGroupLayout->setContentsMargins(2, 2, 2, 2);
        cryptoLog = new QTextEdit(cryptoLogGroup);
        cryptoLog->setObjectName(QString::fromUtf8("cryptoLog"));
        cryptoLog->setReadOnly(true);
        cryptoLog->setFrameShape(QFrame::NoFrame);

        cryptoLogGroupLayout->addWidget(cryptoLog);


        rightLayout->addWidget(cryptoLogGroup);


        horizontalLayout->addWidget(rightPanel);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1000, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        statusbar->setSizeGripEnabled(true);
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "GM Secure Chat \342\200\224 \345\233\275\345\257\206\345\256\211\345\205\250\351\200\232\344\277\241", nullptr));
        loginGroup->setTitle(QCoreApplication::translate("MainWindow", "\350\272\253\344\273\275\350\256\244\350\257\201", nullptr));
        nickInput->setPlaceholderText(QCoreApplication::translate("MainWindow", "\350\276\223\345\205\245\346\230\265\347\247\260", nullptr));
        manualIpInput->setPlaceholderText(QCoreApplication::translate("MainWindow", "\346\211\213\345\212\250\346\214\207\345\256\232\345\257\271\346\226\271IP (\345\217\257\351\200\211\357\274\214\347\224\250\344\272\216\345\271\277\346\222\255\345\217\221\347\216\260\345\244\261\350\264\245\346\227\266)", nullptr));
        loginBtn->setText(QCoreApplication::translate("MainWindow", "\347\231\273\345\275\225\344\270\212\347\272\277", nullptr));
        onlineGroup->setTitle(QCoreApplication::translate("MainWindow", "\345\234\250\347\272\277\347\224\250\346\210\267", nullptr));
        chatGroup->setTitle(QCoreApplication::translate("MainWindow", "\345\256\211\345\205\250\344\274\232\350\257\235", nullptr));
        chatDisplay->setPlaceholderText(QCoreApplication::translate("MainWindow", "\344\274\232\350\257\235\345\206\205\345\256\271\345\260\206\345\234\250\346\255\244\346\230\276\347\244\272...", nullptr));
        inputGroup->setTitle(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201\346\266\210\346\201\257", nullptr));
        msgInput->setPlaceholderText(QCoreApplication::translate("MainWindow", "\350\276\223\345\205\245\346\266\210\346\201\257\357\274\214\346\214\211\345\217\221\351\200\201\346\214\211\351\222\256\345\217\221\351\200\201...", nullptr));
        sendMsgBtn->setText(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201\346\266\210\346\201\257", nullptr));
        sendFileBtn->setText(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201\346\226\207\344\273\266", nullptr));
        netLogGroup->setTitle(QCoreApplication::translate("MainWindow", "\347\275\221\347\273\234\346\227\245\345\277\227", nullptr));
        netLog->setPlaceholderText(QCoreApplication::translate("MainWindow", "\347\275\221\347\273\234\344\272\213\344\273\266\346\227\245\345\277\227", nullptr));
        cryptoLogGroup->setTitle(QCoreApplication::translate("MainWindow", "\345\212\240\345\257\206\350\277\275\350\270\252", nullptr));
        cryptoLog->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\212\240\350\247\243\345\257\206\350\277\207\347\250\213\350\277\275\350\270\252", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
