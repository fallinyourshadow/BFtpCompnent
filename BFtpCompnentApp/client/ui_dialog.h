/********************************************************************************
** Form generated from reading UI file 'dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_H
#define UI_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButtonHome;
    QLabel *labelPath_2;
    QLineEdit *lineEditPath;
    QPushButton *pushButtonRefresh;
    QLineEdit *lineEdit;
    QTreeWidget *treeWidgetFtp;
    QScrollArea *scrollArea;
    QWidget *widget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *layout1;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QString::fromUtf8("Dialog"));
        Dialog->resize(836, 475);
        Dialog->setAcceptDrops(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/flat/flatico/cloud.ico"), QSize(), QIcon::Normal, QIcon::Off);
        Dialog->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(Dialog);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(6, 6, 6, 6);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButtonHome = new QPushButton(Dialog);
        pushButtonHome->setObjectName(QString::fromUtf8("pushButtonHome"));
        pushButtonHome->setMinimumSize(QSize(24, 24));
        pushButtonHome->setAutoFillBackground(true);
        pushButtonHome->setStyleSheet(QString::fromUtf8(""));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/img/ftp/home.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pushButtonHome->setIcon(icon1);
        pushButtonHome->setAutoDefault(false);
        pushButtonHome->setFlat(true);

        horizontalLayout->addWidget(pushButtonHome);

        labelPath_2 = new QLabel(Dialog);
        labelPath_2->setObjectName(QString::fromUtf8("labelPath_2"));

        horizontalLayout->addWidget(labelPath_2);

        lineEditPath = new QLineEdit(Dialog);
        lineEditPath->setObjectName(QString::fromUtf8("lineEditPath"));
        lineEditPath->setEnabled(true);
        lineEditPath->setFrame(false);

        horizontalLayout->addWidget(lineEditPath);

        pushButtonRefresh = new QPushButton(Dialog);
        pushButtonRefresh->setObjectName(QString::fromUtf8("pushButtonRefresh"));
        pushButtonRefresh->setMinimumSize(QSize(24, 24));
        pushButtonRefresh->setStyleSheet(QString::fromUtf8(""));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/img/ftp/sync.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pushButtonRefresh->setIcon(icon2);
        pushButtonRefresh->setAutoDefault(false);
        pushButtonRefresh->setFlat(true);

        horizontalLayout->addWidget(pushButtonRefresh);

        lineEdit = new QLineEdit(Dialog);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(9);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lineEdit->sizePolicy().hasHeightForWidth());
        lineEdit->setSizePolicy(sizePolicy);
        lineEdit->setAcceptDrops(true);
        lineEdit->setClearButtonEnabled(true);

        horizontalLayout->addWidget(lineEdit);

        horizontalLayout->setStretch(2, 20);

        verticalLayout->addLayout(horizontalLayout);

        treeWidgetFtp = new QTreeWidget(Dialog);
        treeWidgetFtp->setObjectName(QString::fromUtf8("treeWidgetFtp"));
        treeWidgetFtp->setFocusPolicy(Qt::NoFocus);
        treeWidgetFtp->setContextMenuPolicy(Qt::CustomContextMenu);
        treeWidgetFtp->setFrameShape(QFrame::StyledPanel);
        treeWidgetFtp->setLineWidth(0);
        treeWidgetFtp->setMidLineWidth(0);
        treeWidgetFtp->setDragEnabled(true);
        treeWidgetFtp->setDragDropMode(QAbstractItemView::DropOnly);
        treeWidgetFtp->setDefaultDropAction(Qt::CopyAction);
        treeWidgetFtp->setSelectionMode(QAbstractItemView::ExtendedSelection);

        verticalLayout->addWidget(treeWidgetFtp);

        scrollArea = new QScrollArea(Dialog);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setMinimumSize(QSize(0, 20));
        scrollArea->setMaximumSize(QSize(16777215, 200));
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setLineWidth(0);
        scrollArea->setWidgetResizable(true);
        widget = new QWidget();
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(0, 0, 824, 39));
        verticalLayout_2 = new QVBoxLayout(widget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        layout1 = new QVBoxLayout();
        layout1->setSpacing(6);
        layout1->setObjectName(QString::fromUtf8("layout1"));

        verticalLayout_2->addLayout(layout1);

        scrollArea->setWidget(widget);

        verticalLayout->addWidget(scrollArea);

        verticalLayout->setStretch(1, 10);
        verticalLayout->setStretch(2, 1);

        retranslateUi(Dialog);

        pushButtonHome->setDefault(false);


        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", nullptr));
#ifndef QT_NO_TOOLTIP
        pushButtonHome->setToolTip(QApplication::translate("Dialog", "\346\210\221\347\232\204\344\272\221\347\233\230", nullptr));
#endif // QT_NO_TOOLTIP
        pushButtonHome->setText(QApplication::translate("Dialog", "home", nullptr));
        labelPath_2->setText(QApplication::translate("Dialog", "\357\274\232", nullptr));
#ifndef QT_NO_TOOLTIP
        lineEditPath->setToolTip(QApplication::translate("Dialog", "\345\275\223\345\211\215\350\267\257\345\276\204", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        pushButtonRefresh->setToolTip(QApplication::translate("Dialog", "\345\210\267\346\226\260", nullptr));
#endif // QT_NO_TOOLTIP
        pushButtonRefresh->setText(QApplication::translate("Dialog", "refresh", nullptr));
#ifndef QT_NO_TOOLTIP
        lineEdit->setToolTip(QApplication::translate("Dialog", "\346\220\234\347\264\242", nullptr));
#endif // QT_NO_TOOLTIP
        lineEdit->setText(QString());
        lineEdit->setPlaceholderText(QApplication::translate("Dialog", "\346\220\234\347\264\242", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeWidgetFtp->headerItem();
        ___qtreewidgetitem->setText(4, QApplication::translate("Dialog", "\345\244\247\345\260\217", nullptr));
        ___qtreewidgetitem->setText(3, QApplication::translate("Dialog", "\346\235\203\351\231\220", nullptr));
        ___qtreewidgetitem->setText(2, QApplication::translate("Dialog", "\347\261\273\345\236\213", nullptr));
        ___qtreewidgetitem->setText(1, QApplication::translate("Dialog", "\344\277\256\346\224\271\346\227\245\346\234\237", nullptr));
        ___qtreewidgetitem->setText(0, QApplication::translate("Dialog", "\346\226\207\344\273\266\345\220\215", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_H
