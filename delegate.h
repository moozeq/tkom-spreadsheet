#ifndef DELEGATE_H
#define DELEGATE_H

#include <QObject>
#include <QItemDelegate>
#include <QModelIndex>
#include <QSize>
#include <QLineEdit>

class Delegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit Delegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
public slots:
    void commitAndCloseEditor();
};

#endif // DELEGATE_H
