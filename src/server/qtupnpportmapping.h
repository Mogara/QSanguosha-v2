#ifndef QTUPNPPORTMAPPING_H
#define QTUPNPPORTMAPPING_H

#define UPNPPORTMAPPING_STATE_INIT 0
#define UPNPPORTMAPPING_STATE_READY 1
#define UPNPPORTMAPPING_STATE_FAILED 2

struct SAddPortMapping
{
    quint16 externalPort;
    quint16 internalPort;
    QString description;
    bool tcp;
};

struct SDeletePortMapping
{
    quint16 externalPort;
    bool tcp;
};

class QtUpnpPortMapping;

class QtUpnpPortMappingSocket : public QObject
{
    Q_OBJECT
public:
    explicit QtUpnpPortMappingSocket(QHostAddress address,quint16 port,QString host,QString infoURL,QtUpnpPortMapping *parent = 0);
    ~QtUpnpPortMappingSocket();
    void doAdd(QList<SAddPortMapping> &);
    void doDelete(QList<SDeletePortMapping> &);
    QString host;
signals:

public slots:
private:
    QTcpSocket *socket;
    QString controlURL;
    QString infoURL;
    QHostAddress address;
    quint16 port;
    QtUpnpPortMapping* parent;
    QTimer timer;
    int state;
    QByteArray remain;
    int contentLength;
    QString id;
    QString localIP;
    QList<QByteArray> sendList;
private slots:
    void handleConnected();
    void handleRead();
    void handleTimeout();
    void handleDisconnected();
};

class QtUpnpPortMapping:public QObject
{
    Q_OBJECT
public:
    explicit QtUpnpPortMapping(QObject *parent = 0);
    ~QtUpnpPortMapping();
    void addPortMapping(quint16 externalPort,quint16 internalPort,const QString &description="",bool tcp=true);
    void deletePortMapping(quint16 externalPort,bool tcp=true);
    void rootFailed(QtUpnpPortMappingSocket*);
    void rootOK(QtUpnpPortMappingSocket*);
    void emitFinished();
    QHostAddress getDefaultGateway();
signals:
    void finished();
private:
    QUdpSocket *udpSocket;
    QList<QtUpnpPortMappingSocket*> pendingRootDevices,successedRootDevices;
    int state;
    void addRoot(QString host,quint16 port,QHostAddress address,QString infoURL);
    QList<SAddPortMapping> pendingAdd;
    QList<SDeletePortMapping> pendingDelete;
    bool noNewRoot;
#ifdef _MSC_VER
	QHostAddress winGetDefaultGateway();
#endif // _MSC_VER

private slots:
    void handleUdpRead();
    void handleTimeout();
};

#endif // QTUPNPPORTMAPPING_H
