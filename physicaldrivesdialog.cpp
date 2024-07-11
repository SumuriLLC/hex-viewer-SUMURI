#include "headers/physicaldrivesdialog.h"
#include "ui_physicaldrivesdialog.h"
#include <windows.h>
#include <QDebug>

PhysicalDrivesDialog::PhysicalDrivesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PhysicalDrivesDialog),
    model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Set up the model for the table view
    model->setHorizontalHeaderLabels({"Device", "Type", "Size"});
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Check if running as Administrator
    if (!isRunningAsAdmin()) {
        ui->openButton->setEnabled(false);
        ui->adminWarningLabel->setVisible(true);

    } else {
        ui->adminWarningLabel->setVisible(false);

        connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {
            ui->openButton->setEnabled(ui->tableView->selectionModel()->hasSelection());
        });
    }

    // Populate the model with physical drives information
    for (int i = 0; i < 10; ++i) { // Adjust the range as needed
        QString drivePath = QString("\\\\.\\PhysicalDrive%1").arg(i);
        HANDLE hDevice = CreateFile(drivePath.toStdWString().c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

        if (hDevice == INVALID_HANDLE_VALUE) {
            break; // Stop when no more drives are found
        }

        // Get drive size
        DISK_GEOMETRY_EX diskGeometry;
        DWORD bytesReturned;
        if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, NULL)) {
            QString size = formatSize(diskGeometry.DiskSize.QuadPart);
            QList<QStandardItem *> items;
            items.append(new QStandardItem(drivePath));
            items.append(new QStandardItem("Physical Device"));
            items.append(new QStandardItem(size));
            model->appendRow(items);
        }

        CloseHandle(hDevice);
    }



    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

}

PhysicalDrivesDialog::~PhysicalDrivesDialog()
{
    delete ui;
}

QString PhysicalDrivesDialog::selectedDrive() const
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();
    if (!selectedRows.isEmpty()) {
        return model->item(selectedRows.first().row(), 0)->text();
    }
    return QString();
}

void PhysicalDrivesDialog::on_openButton_clicked()
{
    accept();
}

QString PhysicalDrivesDialog::formatSize(qint64 size) const
{
    double kb = 1024.0;
    double mb = kb * 1024;
    double gb = mb * 1024;
    double tb = gb * 1024;

    if (size < kb) return QString::number(size) + " B";
    else if (size < mb) return QString::number(size / kb, 'f', 2) + " KB";
    else if (size < gb) return QString::number(size / mb, 'f', 2) + " MB";
    else if (size < tb) return QString::number(size / gb, 'f', 2) + " GB";
    else return QString::number(size / tb, 'f', 2) + " TB";
}

bool PhysicalDrivesDialog::isRunningAsAdmin() const
{
    BOOL isElevated = FALSE;
    HANDLE token = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD size;
        if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
            isElevated = elevation.TokenIsElevated;
        }
    }
    if (token) {
        CloseHandle(token);
    }
    return isElevated;
}

