#include "qtupnpportmapping.h"

#ifdef _MSC_VER
#include <windows.h>
#include <Iphlpapi.h>
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")
#endif // _MSC_VER

QtUpnpPortMapping::QtUpnpPortMapping(QObject *parent) : QObject(parent)
{
    noNewRoot=false;
    state=UPNPPORTMAPPING_STATE_INIT;
    udpSocket=new QUdpSocket(this);
    udpSocket->bind();
    QString s;
    s="M-SEARCH * HTTP/1.1\r\n"
            "Host: 239.255.255.250:1900\r\n"
            "Man: \"ssdp:discover\"\r\n"
            "Mx: 5\r\n"
            "ST: upnp:rootdevice\r\n\r\n";
    connect(udpSocket,&QUdpSocket::readyRead,this,&QtUpnpPortMapping::handleUdpRead);
    QByteArray ba;
    ba=s.toUtf8();
    //QHostAddress addr("239.255.255.250");
    udpSocket->writeDatagram(ba,getDefaultGateway(),1900);
    QTimer::singleShot(6000,this,&QtUpnpPortMapping::handleTimeout);
}

QtUpnpPortMapping::~QtUpnpPortMapping()
{
    qDeleteAll(pendingRootDevices);
    qDeleteAll(successedRootDevices);
    if(udpSocket) udpSocket->deleteLater();
}

void QtUpnpPortMapping::handleTimeout()
{
    udpSocket->deleteLater();
    udpSocket=NULL;
    if(pendingRootDevices.isEmpty())
    {
        noNewRoot=true;
        pendingAdd.clear();
        pendingDelete.clear();
        if(successedRootDevices.isEmpty())
        {
            state=UPNPPORTMAPPING_STATE_FAILED;
            emit finished();
        }
    }
}

void QtUpnpPortMapping::addPortMapping(quint16 externalPort, quint16 internalPort, const QString &description, bool tcp)
{
    if(state==UPNPPORTMAPPING_STATE_FAILED)
    {
        emit finished();
        return;
    }
    SAddPortMapping sa;
    sa.description=description;
    sa.externalPort=externalPort;
    sa.internalPort=internalPort;
    sa.tcp=tcp;
    if(!noNewRoot) pendingAdd.append(sa);
    if(!successedRootDevices.isEmpty())
    {
        QList<SAddPortMapping> list;
        list.append(sa);
        foreach (QtUpnpPortMappingSocket* handle, successedRootDevices) {
            handle->doAdd(list);
        }
    }
}

void QtUpnpPortMapping::deletePortMapping(quint16 externalPort, bool tcp)
{
    if(state==UPNPPORTMAPPING_STATE_FAILED)
    {
        emit finished();
        return;
    }
    SDeletePortMapping sd;
    sd.externalPort=externalPort;
    sd.tcp=tcp;
    if(!noNewRoot) pendingDelete.append(sd);
    if(!successedRootDevices.isEmpty())
    {
        QList<SDeletePortMapping> list;
        list.append(sd);
        foreach (QtUpnpPortMappingSocket* handle, successedRootDevices) {
            handle->doDelete(list);
        }
    }
}

#ifdef _MSC_VER
QHostAddress QtUpnpPortMapping::winGetDefaultGateway()
{
	QHostAddress defaultAddress("239.255.255.250");
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		return defaultAddress;
	}
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			return defaultAddress;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			unsigned long gaddr = inet_addr(pAdapter->GatewayList.IpAddress.String);
            gaddr=ntohl(gaddr);
			bool addressRight = true;
			if (gaddr&&gaddr != INADDR_NONE) {
				unsigned long u = 0xff;
				for (int i = 0; i < 4; i++) {
                    if ((gaddr&u) == u) {
						addressRight = false;
						break;
					}
					u <<= 8;
				}
			}
			else
				addressRight = false;
			if (addressRight) {
                defaultAddress.setAddress(gaddr);
                break;
            }
			pAdapter = pAdapter->Next;
		}
	}
	if (pAdapterInfo)
		FREE(pAdapterInfo);
	return defaultAddress;
}
#endif // _MSC_VER

void QtUpnpPortMapping::handleUdpRead()
{
    QByteArray ba;
    while(udpSocket->hasPendingDatagrams())
    {
        ba.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 port;
        udpSocket->readDatagram(ba.data(),ba.size(),&sender,&port);
        if(!ba.startsWith("HTTP/1.1 200 OK\r\n")) continue;
        int i=ba.indexOf("\r\nLOCATION: ");
        if(i==-1) continue;
        i+=12;
        int j=ba.indexOf("\r\n",i);
        if(j==-1) continue;
        QByteArray location;
        location=ba.mid(i,j-i);
        if(location.length()<10||!location.startsWith("http://")) continue;
        location=location.mid(7);
        j=location.indexOf('/');
        if(j==-1) continue;
        i=location.indexOf(':');
        QString host;
        host=host.fromUtf8(location.mid(0,j));
        if(i==-1)
        {
            port=80;
        }
        else
        {
            bool ok;
            int k=location.mid(i+1,j-i-1).toInt(&ok);
            if(!ok||k<0||k>65535) continue;
            port=k;
        }
        QString info;
        info=location.mid(j);
        addRoot(host,port,sender,info);
    }
}

void QtUpnpPortMapping::addRoot(QString host, quint16 port, QHostAddress address, QString infoURL)
{
    foreach (QtUpnpPortMappingSocket* handle, pendingRootDevices) {
        if(handle->host==host)
            return;
    }
    foreach (QtUpnpPortMappingSocket* handle, successedRootDevices) {
        if(handle->host==host)
            return;
    }
    QtUpnpPortMappingSocket *newHandle=new QtUpnpPortMappingSocket(address,port,host,infoURL,this);
    pendingRootDevices.append(newHandle);
}

QHostAddress QtUpnpPortMapping::getDefaultGateway()
{
#ifdef _MSC_VER
	return winGetDefaultGateway();
#else
	return QHostAddress("239.255.255.250");
#endif // _MSC_VER
}

void QtUpnpPortMapping::rootFailed(QtUpnpPortMappingSocket *handle)
{
    pendingRootDevices.removeOne(handle);
    handle->deleteLater();
    if(!udpSocket&&state==UPNPPORTMAPPING_STATE_INIT&&pendingRootDevices.isEmpty())
    {
        state=UPNPPORTMAPPING_STATE_FAILED;
        emit finished();
    }
}

void QtUpnpPortMapping::rootOK(QtUpnpPortMappingSocket *handle)
{
    pendingRootDevices.removeOne(handle);
    state=UPNPPORTMAPPING_STATE_READY;
    successedRootDevices.append(handle);
    if(!pendingAdd.isEmpty())
    {
        handle->doAdd(pendingAdd);
    }
    if(!pendingDelete.isEmpty())
    {
        handle->doDelete(pendingDelete);
    }
}

void QtUpnpPortMapping::emitFinished()
{
    emit finished();
}

QtUpnpPortMappingSocket::QtUpnpPortMappingSocket(QHostAddress address, quint16 port, QString host, QString infoURL,
                                                 QtUpnpPortMapping *parent) : QObject(parent)
{
    this->address=address;
    this->port=port;
    this->host=host;
    this->parent=parent;
    this->infoURL=infoURL;
    socket=new QTcpSocket();
    connect(socket,&QTcpSocket::connected,this,&QtUpnpPortMappingSocket::handleConnected);
    connect(socket,&QTcpSocket::readyRead,this,&QtUpnpPortMappingSocket::handleRead);
    connect(socket,&QTcpSocket::disconnected,this,&QtUpnpPortMappingSocket::handleDisconnected);
    socket->connectToHost(address,port);
    connect(&timer,&QTimer::timeout,this,&QtUpnpPortMappingSocket::handleTimeout);
    timer.setSingleShot(true);
    timer.start(5000);
    state=0;
    contentLength=0;
}

QtUpnpPortMappingSocket::~QtUpnpPortMappingSocket()
{
    socket->deleteLater();
}

void QtUpnpPortMappingSocket::handleDisconnected()
{
    if(!sendList.isEmpty())
    {
        socket->connectToHost(address,port);
    }
}

void QtUpnpPortMappingSocket::handleConnected()
{
    QString s;
    QByteArray ba;
    if(state==0)
    {
        timer.stop();
        s="GET "+infoURL+" HTTP/1.1\r\n"
            "HOST: "+host+"\r\n"
            "ACCEPT-LANGUAGE: en\r\n\r\n";
        ba=s.toUtf8();
        socket->write(ba);
        timer.start(5000);
        state=1;
        localIP=socket->localAddress().toString();
    }
    else
    {
        if(!sendList.isEmpty())
        {
            ba=sendList.takeFirst();
            socket->write(ba);
        }
    }
}

void QtUpnpPortMappingSocket::handleRead()
{
    int i;
    QString s;
    QByteArray ba;
    switch (state) {
    case 1:
        if(socket->bytesAvailable()<17) return;
        ba.resize(17);
        ba=socket->read(17);
        if(!ba.startsWith("HTTP/1.1 200 OK\r\n"))
        {
            parent->rootFailed(this);
            return;
        }
        state=2;
    case 2:
    case 3:
        ba.resize(1024);
aa:
        i=socket->readLine(ba.data(),1024);
        if(i<=0) return;
        remain.append(ba.left(i));
        if(remain[remain.length()-1]!='\n')//没读完整行，等下一次SIGNAL再继续读
        {
            return;
        }
        if(state==3)
        {
            if(remain.length()==2&&remain.startsWith("\r\n"))//HTTP头部分结束了
            {
                state=4;
                remain.clear();
                goto bb;
            }
            else
            {
                remain.clear();
                goto aa;
            }
        }
        if(remain.length()<=2)//行太短
        {
            parent->rootFailed(this);
            return;
        }
        if(remain.length()>18&&remain.startsWith("CONTENT-LENGTH: "))
        {
            s=s.fromLocal8Bit(remain.mid(16,remain.length()-2));
            bool ok;
            contentLength=s.toInt(&ok);
            if(!contentLength)
            {
                parent->rootFailed(this);
                return;
            }
            state=3;
        }
        remain.clear();
        goto aa;
    case 4:
bb:
        if(socket->bytesAvailable()<contentLength) return;
        ba=socket->read(contentLength);
    {
        QXmlStreamReader reader;
        reader.addData(ba);
        int sta=0;
        while(!reader.atEnd())
        {
            i=reader.readNext();
            switch(sta)
            {
            case 0:
                if(i==QXmlStreamReader::StartElement&&reader.name()=="service")
                {
                    sta=1;
                }
                break;
            case 1:
                if(i==QXmlStreamReader::StartElement&&reader.name()=="serviceType")
                {
                    s=reader.readElementText();
                    if(s.contains(":WANIPConnection:"))
                    {
                        id=s;
                        sta=2;
                    }
                }
                if(i==QXmlStreamReader::EndElement&&reader.name()=="service")
                {
                    sta=0;
                }
                break;
            case 2:
                if(i==QXmlStreamReader::StartElement&&reader.name()=="controlURL")
                {
                    controlURL=reader.readElementText();
                    parent->rootOK(this);
                    timer.stop();
                    state=5;
                    return;
                }
                if(i==QXmlStreamReader::EndElement&&reader.name()=="service")
                {
                    parent->rootFailed(this);
                    return;
                }
                break;
            }
        }
    }
        break;
    default:
        socket->close();
        parent->emitFinished();
        if(sendList.size())
            socket->connectToHost(address,port);
        break;
    }
}

void QtUpnpPortMappingSocket::handleTimeout()
{
    parent->rootFailed(this);
}

void QtUpnpPortMappingSocket::doAdd(QList<SAddPortMapping> &list)
{
    QString s;
    QString header;
    QByteArray ba;
    int l;
    foreach (SAddPortMapping sa, list) {
        s="<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
                      "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>";
        s+="<u:AddPortMapping xmlns:u=\""+id+"\"><NewRemoteHost></NewRemoteHost><NewExternalPort>"
                +s.number(sa.externalPort)+"</NewExternalPort><NewProtocol>"
                +(sa.tcp?"TCP":"UDP")+"</NewProtocol><NewInternalPort>"
                +s.number(sa.internalPort)+"</NewInternalPort><NewInternalClient>"
                +localIP+"</NewInternalClient><NewEnabled>1</NewEnabled><NewPortMappingDescription>"
                +sa.description+"</NewPortMappingDescription><NewLeaseDuration>0</NewLeaseDuration></u:AddPortMapping>";
        s+="</s:Body></s:Envelope>";
        ba=s.toUtf8();
        l=ba.length();
        header="POST "+controlURL+" HTTP/1.1\r\n"
                                  "HOST: "+host+"\r\n"
                                  "CONTENT-LENGTH: "+s.number(l)+"\r\n"
                                  "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
                                  "SOAPACTION: \""+id+"#AddPortMapping\"\r\n\r\n";
        ba.prepend(header.toUtf8());
        sendList.append(ba);
    }
    if(!socket->isOpen())
        socket->connectToHost(address,port);
}

void QtUpnpPortMappingSocket::doDelete(QList<SDeletePortMapping> &list)
{
    QString s;
    QByteArray ba;
    int l;
    QString header;
    foreach (SDeletePortMapping sa, list) {
        s="<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
                      "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>";
        s+="<u:DeletePortMapping xmlns:u=\""+id+"\"><NewRemoteHost></NewRemoteHost><NewExternalPort>"
                +s.number(sa.externalPort)+"</NewExternalPort><NewProtocol>"
                +(sa.tcp?"TCP":"UDP")+"</NewProtocol></u:DeletePortMapping>";
        s+="</s:Body></s:Envelope>";
        ba=s.toUtf8();
        l=ba.length();
        header="POST "+controlURL+" HTTP/1.1\r\n"
                                  "HOST: "+host+"\r\n"
                                  "CONTENT-LENGTH: "+s.number(l)+"\r\n"
                                  "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
                                  "SOAPACTION: \""+id+"#DeletePortMapping\"\r\n\r\n";
        ba.prepend(header.toUtf8());
        sendList.append(ba);
    }
    if(!socket->isOpen())
        socket->connectToHost(address,port);
}
