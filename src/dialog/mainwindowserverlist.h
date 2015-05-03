#ifndef MAINWINDOWSERVERLIST_H
#define MAINWINDOWSERVERLIST_H

#include "src/pch.h"

class DialogSLSettings;
class MainWindowServerList;
class ConnectionDialog;

class CServerList:public QObject
{
    Q_OBJECT
public:
    CServerList(MainWindowServerList *msl);
    ~CServerList();
    void requestList();
    void refresh();

    QLinkedList<QPair<quint32,quint16> > leftServers;
	
private slots:
    void replyFinished();
private:
    bool firstload,stop,loading;
    void addAddress(int,quint32,quint16);
    void initVar();
    void sendRequest();
    int tryTimes;
    MainWindowServerList *msl;
    QDateTime lastTime;
    QNetworkReply* networkReply;
};

class CSLSocketHandle:public QObject
{
    Q_OBJECT
public:
    CSLSocketHandle(QHostAddress &address,quint16 port,QTableWidgetItem*,MainWindowServerList *msl);
    ~CSLSocketHandle();

    void getInfo();
private slots:
    void handleConnected();
    void handleRead();
    void handleDisconnected();
    void handleTimeout();
    void handleError(QAbstractSocket::SocketError);
private:
    QTcpSocket* socket;
    QDateTime lastTime;
    QHostAddress hostaddress;
    quint16 port;
    QTableWidgetItem *item;
    QTimer timer;
    MainWindowServerList *msl;
    QByteArray lrData;
    void infoError();
    QString shortPackageName(const QString &);
};

namespace Ui {
class MainWindowServerList;
}

class MainWindowServerList : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowServerList(QWidget *parent = 0);
    ~MainWindowServerList();
    void initWindow();
    void addAddress(int r,QHostAddress &address,quint16 port);
    void socketFinished();
    void clearServers();
    void showMessage(const QString &);
    Ui::MainWindowServerList *ui;
    CServerList *serverList;
    DialogSLSettings* dialogSLSettings;
    int socketCount;
    QMenu rcMenu;
    QNetworkAccessManager networkam;

private slots:
    void handleCustomMenu(QPoint);
    void handleGetInfo();
    void handleChoose();
    void on_actionGetMore_triggered();

    void on_actionRefresh_triggered();

    void on_actionAdvance_triggered();

    void on_tableWidgetServerList_cellDoubleClicked(int row, int);

private:
    ConnectionDialog *parentDialog;
    QLabel *labelMessage;
};

#endif // MAINWINDOWSERVERLIST_H
