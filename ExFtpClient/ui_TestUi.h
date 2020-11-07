/********************************************************************************
** Form generated from reading UI file 'TestUi.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTUI_H
#define UI_TESTUI_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include "MutiSelectTreeView.h"

QT_BEGIN_NAMESPACE

class Ui_TestUi
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_home;
    QLineEdit *lineEdit_path;
    QPushButton *pushButto_refresh;
    QLineEdit *lineEdit_search;
    MutiSelectTreeView *treeView;
    QProgressBar *progressBar;

    void setupUi(QDialog *TestUi)
    {
        if (TestUi->objectName().isEmpty())
            TestUi->setObjectName(QString::fromUtf8("TestUi"));
        TestUi->resize(823, 529);
        verticalLayout = new QVBoxLayout(TestUi);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton_home = new QPushButton(TestUi);
        pushButton_home->setObjectName(QString::fromUtf8("pushButton_home"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton_home->sizePolicy().hasHeightForWidth());
        pushButton_home->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(pushButton_home);

        lineEdit_path = new QLineEdit(TestUi);
        lineEdit_path->setObjectName(QString::fromUtf8("lineEdit_path"));
        lineEdit_path->setEnabled(false);
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lineEdit_path->sizePolicy().hasHeightForWidth());
        lineEdit_path->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(lineEdit_path);

        pushButto_refresh = new QPushButton(TestUi);
        pushButto_refresh->setObjectName(QString::fromUtf8("pushButto_refresh"));
        sizePolicy.setHeightForWidth(pushButto_refresh->sizePolicy().hasHeightForWidth());
        pushButto_refresh->setSizePolicy(sizePolicy);
        pushButto_refresh->setMinimumSize(QSize(24, 24));
        pushButto_refresh->setStyleSheet(QString::fromUtf8(""));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/img/ftp/sync.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pushButto_refresh->setIcon(icon);
        pushButto_refresh->setAutoDefault(false);
        pushButto_refresh->setFlat(true);

        horizontalLayout->addWidget(pushButto_refresh);

        lineEdit_search = new QLineEdit(TestUi);
        lineEdit_search->setObjectName(QString::fromUtf8("lineEdit_search"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(lineEdit_search->sizePolicy().hasHeightForWidth());
        lineEdit_search->setSizePolicy(sizePolicy2);
        lineEdit_search->setMaximumSize(QSize(517, 16777215));
        lineEdit_search->setAcceptDrops(true);
        lineEdit_search->setClearButtonEnabled(true);

        horizontalLayout->addWidget(lineEdit_search);


        verticalLayout->addLayout(horizontalLayout);

        treeView = new MutiSelectTreeView(TestUi);
        treeView->setObjectName(QString::fromUtf8("treeView"));

        verticalLayout->addWidget(treeView);

        progressBar = new QProgressBar(TestUi);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(24);

        verticalLayout->addWidget(progressBar);


        retranslateUi(TestUi);

        QMetaObject::connectSlotsByName(TestUi);
    } // setupUi

    void retranslateUi(QDialog *TestUi)
    {
        TestUi->setWindowTitle(QCoreApplication::translate("TestUi", "TestUi", nullptr));
        pushButton_home->setText(QCoreApplication::translate("TestUi", "home", nullptr));
        lineEdit_path->setText(QCoreApplication::translate("TestUi", "/", nullptr));
#if QT_CONFIG(tooltip)
        pushButto_refresh->setToolTip(QCoreApplication::translate("TestUi", "\345\210\267\346\226\260", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButto_refresh->setText(QCoreApplication::translate("TestUi", "refresh", nullptr));
#if QT_CONFIG(tooltip)
        lineEdit_search->setToolTip(QCoreApplication::translate("TestUi", "\346\220\234\347\264\242", nullptr));
#endif // QT_CONFIG(tooltip)
        lineEdit_search->setText(QString());
        lineEdit_search->setPlaceholderText(QCoreApplication::translate("TestUi", "\346\220\234\347\264\242", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TestUi: public Ui_TestUi {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTUI_H
