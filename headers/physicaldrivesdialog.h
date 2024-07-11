#ifndef PHYSICALDRIVESDIALOG_H
#define PHYSICALDRIVESDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class PhysicalDrivesDialog;  // Forward declaration
}

class PhysicalDrivesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PhysicalDrivesDialog(QWidget *parent = nullptr);
    ~PhysicalDrivesDialog();

    QString selectedDrive() const;

private slots:
    void on_openButton_clicked();

private:
    Ui::PhysicalDrivesDialog *ui;
    QStandardItemModel *model;
    QString formatSize(qint64 size) const;
    bool isRunningAsAdmin() const;
};

#endif // PHYSICALDRIVESDIALOG_H
