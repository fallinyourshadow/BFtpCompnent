#include "MutiSelectTreeView.h"
#include <QDebug>

MutiSelectTreeView::MutiSelectTreeView(QWidget *parent) : QTreeView(parent)
{
    connect(this,&MutiSelectTreeView::doubleClicked,[=](){
        m_selectedRows.clear();
        selectionModel()->clear();
    });
}

QList<qint32> MutiSelectTreeView::selectedRows()
{
    return m_selectedRows;
}

void MutiSelectTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(!deselected.isEmpty())//如果有取消选中的就清空
    {
        QModelIndexList list =  deselected.indexes();
        foreach(QModelIndex v, list)
        {
            m_selectedRows.removeOne(v.row());
        }
    }
    if(!selected.isEmpty())//如果有选中的就添加
    {
        qint32  selectedtRow = selected.indexes().first().row();
        if(!m_selectedRows.contains(selectedtRow))
        {
            m_selectedRows.append(selectedtRow);
        }
    }
    QTreeView::selectionChanged(selected,deselected);
}
