#ifndef MUTISELECTTREEVIEW_H
#define MUTISELECTTREEVIEW_H

#include <QObject>
#include <QTreeView>
#include <QMouseEvent>

class MutiSelectTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit MutiSelectTreeView(QWidget *parent = nullptr);
    QList<qint32> selectedRows();
protected:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
signals:
private:
    QList<qint32> m_selectedRows;

};

#endif // MUTISELECTTREEVIEW_H
