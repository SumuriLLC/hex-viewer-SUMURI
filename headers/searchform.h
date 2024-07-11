#ifndef SEARCHFORM_H
#define SEARCHFORM_H

#include <QDialog>

namespace Ui {
class searchform;
}

class searchform : public QDialog
{
    Q_OBJECT

public:
    explicit searchform(QWidget *parent = nullptr);
    ~searchform();

    QString getSearchPattern() const;
    QString getSearchType() const;
    QPushButton* getSearchButton() const;

private:
    Ui::searchform *ui;
};

#endif // SEARCHFORM_H
