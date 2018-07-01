#include "delegate.h"

Delegate::Delegate(QObject *parent) :
    QItemDelegate(parent) {}

QWidget *Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    QLineEdit *editor = new QLineEdit(parent);
    connect(editor, &QLineEdit::editingFinished, this, &Delegate::commitAndCloseEditor);
    return editor;
}

void Delegate::commitAndCloseEditor()
{
    QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

void Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);
    edit->setText(index.model()->data(index, Qt::EditRole).toString());
    return;
}

void Delegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);
    if (edit) {
        model->setData(index, edit->text());
        return;
    }
}
