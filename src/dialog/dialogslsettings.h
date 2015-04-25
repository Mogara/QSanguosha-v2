#ifndef DIALOGSLSETTINGS_H
#define DIALOGSLSETTINGS_H

#include <QDialog>


namespace Ui {
class DialogSLSettings;
}

class DialogSLSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSLSettings(QWidget *parent = 0);
    ~DialogSLSettings();
    void initVar();

private slots:
    void on_pushButtonCancel_clicked();

    void on_pushButtonOK_clicked();

    void on_pushButtonDefualt_clicked();

private:
    Ui::DialogSLSettings *ui;
};

#endif // DIALOGSLSETTINGS_H
