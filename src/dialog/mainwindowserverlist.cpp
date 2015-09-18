#include "mainwindowserverlist.h"
#include "ui_mainwindowserverlist.h"

#include "settings.h"
#include "engine.h"
#include "dialogslsettings.h"
#include "defines.h"
#include "protocol.h"
#include "connectiondialog.h"
#include "package.h"

MainWindowServerList::MainWindowServerList(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowServerList)
{
    dialogSLSettings=NULL;
    socketCount=0;
    parentDialog=static_cast<ConnectionDialog*>(parent);

    ui->setupUi(this);

    labelMessage=new QLabel(statusBar());
    labelMessage->setGeometry(1,1,1000,20);
    labelMessage->show();

    ui->tableWidgetServerList->setSelectionBehavior(QAbstractItemView::SelectRows);
    QStringList sl;
    sl << tr("地址") << tr("延时") << tr("版本") << tr("模式") << tr("服务器名") << tr("特殊") << tr("当前人数") << tr("扩展包");
    ui->tableWidgetServerList->setHorizontalHeaderLabels(sl);

    serverList=new CServerList(this);

    QAction *actionChoose,*actionGetInfo;
    actionChoose=new QAction(&rcMenu);
    actionGetInfo=new QAction(&rcMenu);
	actionGetInfo->setText(tr("获取简略信息"));
	actionChoose->setText(tr("选定"));
    rcMenu.addAction(actionGetInfo);
    rcMenu.addAction(actionChoose);
    connect(ui->tableWidgetServerList,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(handleCustomMenu(QPoint)));
    connect(actionChoose,SIGNAL(triggered()),this,SLOT(handleChoose()));
    connect(actionGetInfo,SIGNAL(triggered()),this,SLOT(handleGetInfo()));

    ui->tableWidgetServerList->setColumnWidth(0,140);
    ui->tableWidgetServerList->setColumnWidth(1,60);
    ui->tableWidgetServerList->setColumnWidth(2,150);
    ui->tableWidgetServerList->setColumnWidth(3,150);
    ui->tableWidgetServerList->setColumnWidth(4,434);
    ui->tableWidgetServerList->setColumnWidth(5,150);
    ui->tableWidgetServerList->setColumnWidth(6,80);
    ui->tableWidgetServerList->setColumnWidth(7,800);
}

MainWindowServerList::~MainWindowServerList()
{
    delete ui;
}

void MainWindowServerList::handleChoose()
{
    QList<QTableWidgetItem *> si;
    si=ui->tableWidgetServerList->selectedItems();
    if(si.size())
        on_tableWidgetServerList_cellDoubleClicked(si[0]->row(),0);
}

void MainWindowServerList::handleGetInfo()
{
    QList<QTableWidgetItem *> si;
    si=ui->tableWidgetServerList->selectedItems();
    if(si.size())
    {
        QString s=ui->tableWidgetServerList->item(si[0]->row(),1)->text();
        if(s=="")
        {
            CSLSocketHandle* handle;
            handle=(CSLSocketHandle*)ui->tableWidgetServerList->item(si[0]->row(),0)->data(Qt::UserRole).value<void*>();
            socketCount++;
            handle->getInfo();
        }
    }
}

void MainWindowServerList::handleCustomMenu(QPoint)
{
    QList<QTableWidgetItem *> si;
    si=ui->tableWidgetServerList->selectedItems();
    if(si.size())
        rcMenu.exec(QCursor::pos());
}

void MainWindowServerList::initWindow()
{
    if(!ui->tableWidgetServerList->rowCount())
    {
        serverList->requestList();
    }
    this->showMaximized();
}

void MainWindowServerList::addAddress(int r, QHostAddress &address, quint16 port)
{
    QTableWidgetItem *item,*first;
    QString s=address.toString()+":"+QString::number(port);
    first=new QTableWidgetItem(s);
    CSLSocketHandle* handle=new CSLSocketHandle(address,port,first,this);
    first->setData(Qt::UserRole,qVariantFromValue((void*)handle));
    ui->tableWidgetServerList->setItem(r,0,first);

    for(int i=1;i<8;i++)
    {
        item=new QTableWidgetItem("");
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetServerList->setItem(r,i,item);
    }
    bool b=Config.value("slconfig/autogetinfo",true).toBool();
    if(b&&socketCount<5)
    {
        socketCount++;
        handle->getInfo();
    }
}

void MainWindowServerList::clearServers()
{
    int l=ui->tableWidgetServerList->rowCount();
    QTableWidgetItem* item;
    CSLSocketHandle* handle;
    for(int i=0;i<l;i++)
    {
        item=ui->tableWidgetServerList->item(i,0);
        handle=(CSLSocketHandle*)item->data(Qt::UserRole).value<void*>();
        if(handle)
            delete handle;
    }
    ui->tableWidgetServerList->clearContents();
    ui->tableWidgetServerList->setRowCount(0);
}

void MainWindowServerList::socketFinished()
{
    if(!socketCount) return;
    bool b=Config.value("slconfig/autogetinfo",true).toBool();
    if(b)
    {
        QTableWidgetItem* item;
        CSLSocketHandle* handle;
        int i,l;
        l=ui->tableWidgetServerList->rowCount();
        for(i=0;i<l;i++)
        {
            item=ui->tableWidgetServerList->item(i,1);
            if(item->text()=="")
            {
                item=ui->tableWidgetServerList->item(i,0);
                handle=(CSLSocketHandle*)item->data(Qt::UserRole).value<void*>();
                handle->getInfo();
                return;
            }
        }
    }
    socketCount--;
}

void MainWindowServerList::on_actionGetMore_triggered()
{
    labelMessage->clear();
    serverList->requestList();
}

void MainWindowServerList::on_actionRefresh_triggered()
{
    socketCount=0;
    labelMessage->clear();
    serverList->refresh();
}

void MainWindowServerList::on_actionAdvance_triggered()
{
    if(!dialogSLSettings)
    {
        dialogSLSettings=new DialogSLSettings(this);
    }
    dialogSLSettings->initVar();
    dialogSLSettings->show();
}

void MainWindowServerList::on_tableWidgetServerList_cellDoubleClicked(int row, int)
{
    QString s;
    s=ui->tableWidgetServerList->item(row,0)->text();
    parentDialog->setAddress(s);
    this->close();
}

void MainWindowServerList::showMessage(const QString &s)
{
    labelMessage->setText(s);
}

CServerList::CServerList(MainWindowServerList *msl)
{
    this->msl=msl;
    initVar();
}

CServerList::~CServerList()
{

}

void CServerList::initVar()
{
    firstload=true;
    stop=false;
    loading=false;
    networkReply=NULL;
    leftServers.clear();
}

void CServerList::replyFinished()
{
    quint32 addr;
    quint16 port;
    int i,j,k;
    bool b;
    QByteArray ba=networkReply->readAll();
    if(ba.size()<2||(ba.size()-2)%10!=0)
        goto aa;
    memcpy(&port,ba.data(),2);
    port=qFromLittleEndian(port);
    if(port!=SERVERLIST_VERSION_SERVERLIST)
    {
aa:
#ifdef QT_DEBUG
        QFile file("serverlist.log");
        file.open(QIODevice::WriteOnly);
        QTextStream out(&file);
        out << ba;
#endif
        tryTimes--;
        if(tryTimes>0)
        {
            networkReply->deleteLater();
            sendRequest();
            return;
        }
        msl->showMessage(tr("列表服务器返回的数据与本程序不兼容，如果您使用的是最新的版本，那么可能是服务器出现了故障。"));
        stop=true;
    }
    else
    {
        msl->showMessage(tr("服务器列表载入成功"));
        i=ba.size()-2;
        j=msl->ui->tableWidgetServerList->rowCount();
        if(i>=200)
        {
            k=j+20;
            msl->ui->tableWidgetServerList->setRowCount(k);
        }
        else if(i>=10)
        {
            k=j+i/10;
            msl->ui->tableWidgetServerList->setRowCount(k);
        }
        i=0;
        b=false;
        int pos=2;
        while(pos<ba.size())
        {
            memcpy(&addr,ba.data()+pos,4);
            addr=qFromLittleEndian(addr);
            memcpy(&port,ba.data()+pos+4,2);
            port=qFromLittleEndian(port);
            addAddress(j+i,addr,port);
            i++;
            pos+=10;
            if(i==20)
            {
                b=true;
                break;
            }
        }
        if(!b)
        {
            stop=true;
        }
        else if(!firstload)
        {
            while(pos<ba.size())
            {
                memcpy(&addr,ba.data()+pos,4);
                addr=qFromLittleEndian(addr);
                memcpy(&port,ba.data()+pos+4,2);
                port=qFromLittleEndian(port);
                leftServers.append(QPair<quint32,quint16>(addr,port));
            }
            if(!leftServers.size())
            {
                stop=true;
            }
        }
        if(firstload) firstload=false;
    }
    loading=false;
    networkReply->deleteLater();
    networkReply=NULL;
}

void CServerList::sendRequest()
{
    QString s=Config.value("slconfig/geturl",SERVERLIST_URL_DEFAULTGET).toString();
    s+="servers";
    networkReply=msl->networkam.get(QNetworkRequest(QUrl(s)));
    connect(networkReply,&QNetworkReply::finished,this,&CServerList::replyFinished);
}

void CServerList::requestList()
{
    if(loading)
    {
        return;
    }
    if(stop)
    {
		msl->showMessage(tr("无法获得更多服务器。"));
        return;
    }
    msl->showMessage(tr("正在加载中……"));
    if(firstload)
    {
        tryTimes=3;
        lastTime=QDateTime::currentDateTime();
        sendRequest();
        loading=true;
    }
    else
    {
        int l=leftServers.size();
        if(l)
        {
            QPair<quint32,quint16> pair;
            if(l>20) l=20;
            int j=msl->ui->tableWidgetServerList->rowCount();
            if(l)
            {
                msl->ui->tableWidgetServerList->setRowCount(j+l);
            }
            for(int i=0;i<l;i++)
            {
                pair=leftServers.takeFirst();
                addAddress(j+i,pair.first,pair.second);
            }
            if(!leftServers.size())
            {
                stop=true;
            }
        }
        else
        {
            QString s=Config.value("slconfig/geturl",SERVERLIST_URL_DEFAULTGET).toString();
            s+="full";
            networkReply=msl->networkam.get(QNetworkRequest(QUrl(s)));
            connect(networkReply,&QNetworkReply::finished,this,&CServerList::replyFinished);
            loading=true;
        }
    }
}

void CServerList::addAddress(int j, quint32 add, quint16 port)
{
    QHostAddress ha;
    ha.setAddress(add);
    msl->addAddress(j,ha,port);
}

void CServerList::refresh()
{
    if(lastTime.secsTo(QDateTime::currentDateTime())<5)
    {
		msl->showMessage(tr("离上次获取列表的时间不足5秒钟，请稍后再刷新。"));
    }
    else
    {
        if(networkReply)
        {
            networkReply->deleteLater();
            networkReply=NULL;
        }
        initVar();
        msl->clearServers();
        requestList();
    }
}

CSLSocketHandle::CSLSocketHandle(QHostAddress &address, quint16 port, QTableWidgetItem* item, MainWindowServerList *msl)
{
    socket=NULL;
    this->hostaddress=address;
    this->port=port;
    this->item=item;
    this->msl=msl;
    connect(&timer,&QTimer::timeout,this,&CSLSocketHandle::handleTimeout);
}

CSLSocketHandle::~CSLSocketHandle()
{
    if(socket)
    {
        socket->deleteLater();
        int i=item->row();
        QTableWidgetItem *ti;
        ti=msl->ui->tableWidgetServerList->item(i,1);
        if(ti->text()!="")
        {
			ti->setText(tr("无法连接"));
            ti->setTextColor(QColor(255,0,0));
        }
    }
    item->setData(Qt::UserRole,0);
    msl->socketFinished();
}

void CSLSocketHandle::getInfo()
{
    socket=new QTcpSocket;
    connect(socket,&QTcpSocket::connected,this,&CSLSocketHandle::handleConnected);
    connect(socket,&QTcpSocket::readyRead,this,&CSLSocketHandle::handleRead);
    connect(socket,&QTcpSocket::disconnected,this,&CSLSocketHandle::handleDisconnected);
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(handleError(QAbstractSocket::SocketError)));
    lastTime=QDateTime::currentDateTime();
    int i=item->row();
    QTableWidgetItem *ti;
    ti=msl->ui->tableWidgetServerList->item(i,1);
	ti->setText(tr("连接中"));
    ti->setTextColor(QColor(0,0,0));
    socket->connectToHost(hostaddress,port);
    timer.start(10000);
}

void CSLSocketHandle::handleError(QAbstractSocket::SocketError)
{
    this->deleteLater();
}

void CSLSocketHandle::handleTimeout()
{
    if(lastTime.msecsTo(QDateTime::currentDateTime())>=10000)
    {
        this->deleteLater();
    }
}

void CSLSocketHandle::handleConnected()
{
    qint64 ms=lastTime.msecsTo(QDateTime::currentDateTime());
    lastTime=QDateTime::currentDateTime();
    int i=item->row();
    QTableWidgetItem *ti;
    ti=msl->ui->tableWidgetServerList->item(i,1);
    ti->setText(QString::number(ms)+"ms");
    if(ms<100)
    {
        ti->setTextColor(QColor(0,255,0));
    }
    else if(ms<1000)
    {
        ti->setTextColor(QColor(255,255,0));
    }
    else
    {
        ti->setTextColor(QColor(255,0,0));
    }
    timer.start(10000);
}

void CSLSocketHandle::handleDisconnected()
{
    this->deleteLater();
}

void CSLSocketHandle::handleRead()
{
    if(!socket->canReadLine()) return;
aa:
    QByteArray ba=socket->readLine();
    QSanProtocol::Packet packet;
    if(!packet.parse(ba))
    {
        infoError();
        this->deleteLater();
        return;
    }
    int i=item->row();
    QTableWidgetItem *ti;
    QTableWidget* tw=msl->ui->tableWidgetServerList;
    if(packet.getCommandType()==QSanProtocol::S_COMMAND_CHECK_VERSION)
    {
        ti=tw->item(i,2);
        ti->setText(packet.getMessageBody().toString());
        ti->setTextColor(QColor(0,0,0));
        if(socket->canReadLine())
            goto aa;
        else
            return;
    }
    else if(packet.getCommandType()==QSanProtocol::S_COMMAND_SETUP)
    {
        QByteArrayList bal;
        bal=packet.getMessageBody().toByteArray().split(':');
        if(bal.size()<6)
        {
            infoError();
            this->deleteLater();
            return;
        }
        ti=tw->item(i,0);
        bool isOfficial;
        if(ti->text().split(':')[0]==SERVERLIST_OFFICIALSERVER)
            isOfficial=true;
        else
            isOfficial=false;
        QString s=QString::fromUtf8(QByteArray::fromBase64(bal[0]));
        ti=tw->item(i,4);
        if(isOfficial) {
            ti->setTextColor(QColor(255,153,0));
            s.prepend("【官方服务器】 ");
        }
        else
            ti->setTextColor(QColor(0,0,0));
        ti->setText(s);
        s=Sanguosha->getModeName(QString::fromUtf8(bal[1]));
        ti=tw->item(i,3);
        ti->setText(s);
        s.clear();
        if(bal[5].contains('C'))
			s.append(tr("作弊 "));
        if(bal[5].contains('S'))
			s.append(tr("双将 "));
        if(bal[5].contains('T'))
			s.append(tr("同将 "));
        if(bal[5].contains('B'))
			s.append(tr("暗将 "));
        if(bal[5].contains('H'))
			s.append(tr("国战 "));
        if(bal[5].contains('M'))
			s.append(tr("禁聊 "));
        if(bal[5].contains('A'))
            s.append(tr("AI "));
        ti=tw->item(i,5);
        ti->setText(s);
		if (bal.size() >= 7)
		{
			s = QString::fromUtf8(bal[6]);
			ti = tw->item(i, 6);
			ti->setText(s);
		}

        QString spackage;
        QString banp=QString::fromUtf8(bal[4]);
        QStringList ban_packages = banp.split('+');
        QList<const Package *> packages = Sanguosha->findChildren<const Package *>();
        foreach (const Package *package, packages) {
            if (package->inherits("Scenario"))
                continue;

            QString package_name = package->objectName();
            if (!ban_packages.contains(package_name))
            {
                if(package->getType()==Package::GeneralPack)
                {
                    s=Sanguosha->translate(package_name);
                    if(Config.value("slconfig/short",true).toBool())
                        s=shortPackageName(s);
                    spackage+=s+" ";
                }
            }
        }
        ti=tw->item(i,7);
        ti->setText(spackage);
        socket->deleteLater();
        socket=NULL;
    }
    else
    {
        infoError();
    }
    this->deleteLater();
}

QString CSLSocketHandle::shortPackageName(const QString &n)
{
    QString s;
    if(n=="标准版")
        s="标";
    else if(n=="风包")
        s="风";
    else if(n=="火包")
        s="火";
    else if(n=="林包")
        s="林";
    else if(n=="山包")
        s="山";
    else if(n=="一将成名")
        s="将1";
    else if(n=="一将成名2012")
        s="将2";
    else if(n=="一将成名2013")
        s="将3";
    else if(n=="一将成名2014")
        s="将4";
    else if(n=="铜雀台")
        s="铜";
    else if(n=="OL专属")
        s="OL";
    else if(n=="台湾一将成名")
        s="台";
    else if(n=="桌游志贴纸")
        s="桌";
    else if(n=="国战身份局")
        s="国";
    else if(n=="国战-阵包")
        s="阵";
    else if(n=="一将成名")
        s="将1";
    else if(n=="台版SP")
        s="台SP";
    else if(n=="翼包")
        s="翼";
    else if(n=="国战-势包")
        s="势";
    else if(n=="国战SP")
        s="国SP";
    else if(n=="界限突破-SP")
        s="界";
    else if(n=="怀旧-标准")
        s="旧标";
    else if(n=="怀旧-风")
        s="旧风";
    else if(n=="怀旧-一将")
        s="旧将";
    else if(n=="怀旧-一将2")
        s="旧将2";
    else if(n=="怀旧-一将3")
        s="旧将3";
    else
        s=n;
    return s;
}

void CSLSocketHandle::infoError()
{
    int i=item->row();
    QTableWidgetItem *ti;
    QTableWidget* tw=msl->ui->tableWidgetServerList;
    ti=tw->item(i,2);
	ti->setText(tr("未知版本"));
    ti->setTextColor(QColor(255,0,0));
    socket->deleteLater();
    socket=NULL;
}
