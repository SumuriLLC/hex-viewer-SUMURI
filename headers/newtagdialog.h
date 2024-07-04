#ifndef NEWTAGDIALOG_H
#define NEWTAGDIALOG_H

#include <QDialog>

namespace Ui {
class NewTagDialog;
}

class NewTagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewTagDialog(QWidget *parent = nullptr);
    ~NewTagDialog();
    void setStartAddress(quint64 address);
    void setEndAddress(quint64 address);
    void setDescription(QString description);

signals:
    void tagSaved(quint64 offset, quint64 length, const QString &description, const QColor &color, const QString &type);

private slots:
    void openColorPicker();
    void onSaveButtonClicked();
    void updateHexLabels();

private:
    Ui::NewTagDialog *ui;
    QString selectedColor;
};

#endif // NEWTAGDIALOG_H
