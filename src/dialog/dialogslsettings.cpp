#include "dialogslsettings.h"
#include "ui_dialogslsettings.h"
#include "defines.h"
#include "settings.h"
#include "src/pch.h"

DialogSLSettings::DialogSLSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSLSettings)
{
    ui->setupUi(this);
    setFixedSize(400,190);
}

DialogSLSettings::~DialogSLSettings()
{
    delete ui;
}

void DialogSLSettings::initVar()
{
    QString s;
    s=Config.value("slconfig/geturl",SERVERLIST_URL_DEFAULTGET).toString();
    ui->lineEditDownload->setText(s);
    s=Config.value("slconfig/regurl",SERVERLIST_URL_DEFAULTREG).toString();
    ui->lineEditReg->setText(s);
    bool b=Config.value("slconfig/autogetinfo",true).toBool();
    ui->checkBoxAutoGetInfo->setChecked(b);
    b=Config.value("slconfig/short",true).toBool();
    ui->checkBoxShort->setChecked(b);
}

void DialogSLSettings::on_pushButtonCancel_clicked()
{
    this->close();
}

void DialogSLSettings::on_pushButtonOK_clicked()
{
    QString s;
    s=ui->lineEditDownload->text();
    if(!s.endsWith('/'))
    {
        s.append('/');
    }
    Config.setValue("slconfig/geturl",s);
    s=ui->lineEditReg->text();
    Config.setValue("slconfig/regurl",s);
    Config.setValue("slconfig/autogetinfo",ui->checkBoxAutoGetInfo->isChecked());
    Config.setValue("slconfig/short",ui->checkBoxShort->isChecked());
    this->close();
}

void DialogSLSettings::on_pushButtonDefualt_clicked()
{
    ui->lineEditDownload->setText(SERVERLIST_URL_DEFAULTGET);
    ui->lineEditReg->setText(SERVERLIST_URL_DEFAULTREG);
    ui->checkBoxAutoGetInfo->setChecked(true);
    ui->checkBoxShort->setChecked(true);
}
