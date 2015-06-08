var url=require('url');
var net=require('net');
var http=require('http');
var util = require("util");
var fs=require('fs');

var logs=new Array(4);
var serverList=new Array();
var version=1;
var official=[['115.159.24.202',9527],['115.159.24.202',9529]];//官服
var logFile;
var lastDate;
init();

var server=http.createServer(httpRequest);
server.listen(80,'127.0.0.1');//监听端口，只接受IPV4

function httpRequest(request,response)
{
	logRequest(request,response);
	if(request.method!='GET')
	{
		response.end();
		return;
	}
	
	var rp=url.parse(request.url);
	switch(rp.pathname)
	{
	case '/reg':
		regServer(request,response);
		break;
	case '/servers':
		getServers(response,false);
		break;
	case '/full':
		getServers(response,true);
		break;
	default:
		response.end();
	}
}

function init()
{
	version=binaryPort(version);
	var os=new Array();
	for(var i=0;i<official.length;i++)
	{
		os[i]=new Array(2);
		os[i][0]=binaryIP(official[i][0]);
		os[i][1]=binaryPort(official[i][1]);
	}
	official=os;
	for(var i=0;i<4;i++)
	{
		logs[i]=0;
	}
	
	try	{
		fs.mkdirSync('./log');
	} catch(e){
	}
	
	lastDate=today();
	try	{
		logFile=fs.createWriteStream('./log/access_'+today()+'.log',{flags:'a'});
	} catch (e)	{
		logFile=false;
	}
}

function today()
{
	var today = new Date();
	var dd = today.getDate();
	var mm = today.getMonth()+1; //January is 0!
	var yyyy = today.getFullYear();

	if(dd<10) {
		dd='0'+dd
	} 

	if(mm<10) {
		mm='0'+mm
	} 

	today = yyyy+"_"+mm+"_"+dd;
	return today;
}

function binaryIP(ip)
{
	var address=new Buffer(4);
	var array=ip.split('.');
	for(var j=0;j<4;j++)
	{
		address.writeUIntLE(array[3-j],j,1);
	}
	return address;
}

function binaryPort(port)
{
	var bp=new Buffer(2);
	bp.writeUIntLE(port,0,2);
	return bp;
}

function regServer(request,response)
{
	var rp=url.parse(request.url,true);
	var port=parseInt(rp.query['p']);
	if(isNaN(port) || port<=0 || port>65535)
	{
		response.end();
		return;
	}
	var ip=response.socket.remoteAddress;
	//console.log(util.inspect(ip));
	var bport=binaryPort(port);
	var bip=binaryIP(ip);
	
	response.writeHead(200,{
		'Content-Length':1,
		'Content-Type':'text/plain'
	});
	
	for(var i in official)
	{
		if(official[i][0].compare(bip)==0 && official[i][1].compare(bport)==0)
		{
			response.end('0');
			return;
		}
	}
	
	var needtest=true;
	for(var i in serverList)
	{
		if(serverList[i][0].compare(bip)==0 && serverList[i][1].compare(bport)==0)
		{
			needtest=false;
			break;
		}
	}
	
	if(needtest)
	{
		var socket=net.createConnection({host:ip,port:port},function(){
			socket.end();
			addToServerList(bip,bport);
			response.end('0');
			logs[0]++;
			outputLog();
		});
		socket.setTimeout(10000,function(){
			regFail(socket,response);
		});
		socket.on('error',function(err){
			regFail(socket,response);
		});
	}
	else
	{
		addToServerList(bip,bport);
		response.end('0');
	}
}

function regFail(socket,response)
{
	socket.destroy();
	response.end('1');
	logs[1]++;
	outputLog();
}

function addToServerList(ip,port)
{
	var ctime=new Date().getTime();
	for(var i=0;i<serverList.length;i++)
	{
		if(ctime-serverList[i][2]>3600000)
		{
			serverList.splice(i,serverList.length-i);
			break;
		}
		else if(serverList[i][0].compare(ip)==0 && serverList[i][1].compare(port)==0)
		{
			serverList.splice(i,1);
			i--;
		}
	}
	serverList.unshift(new Array(ip,port,ctime));
}

function getServers(response,isFull)
{
	var barray=new Array();
	var output;
	barray.push(version);
	var emptyBuffer=new Buffer(4);
	if(!isFull)
	{
		logs[2]++;
		var count=20;
		for(var i in official)
		{
			barray.push(official[i][0],official[i][1],emptyBuffer);
			count--;
			if(!count)
				break;
		}
		if(count)
		{
			for(var i in serverList)
			{
				barray.push(serverList[i][0],serverList[i][1],emptyBuffer);
				count--;
				if(!count)
					break;
			}
		}
		output=Buffer.concat(barray,(20-count)*10+2);
	}
	else
	{
		logs[3]++;
		var count=0;
		var start=20-official.length;
		if(start<0)
		{
			start=0;
			for(var i=20;i<official.length;i++)
			{
				barray.push(official[i][0],official[i][1],emptyBuffer);
				count++;
			}
		}
		for(var i=start;i<serverList.length;i++)
		{
			barray.push(serverList[i][0],serverList[i][1],emptyBuffer);
			count++;
		}
		output=Buffer.concat(barray,count*10+2);
	}
	response.writeHead(200,{
		'Content-Length':output.length,
		'Content-Type':'application/octet-stream'
	});
	response.end(output);
	outputLog();
}

function logRequest(request,response)
{
	if(!logFile)
		return;
	if(lastDate!=today())
	{
		lastDate=today();
		try	{
			logFile.end();
			logFile=fs.createWriteStream('./log/access_'+today()+'.log',{flags:'a'});
		} catch (e)	{
			logFile=false;
			return;
		}
	}
	
	var cdate=new Date();
	var str=response.socket.remoteAddress+" "
	str+=cdate.getHours()+":"+cdate.getMinutes()+":"+cdate.getSeconds()+" ";
	str+=request.method+" ";
	str+=request.url+" ";
	str+=request.httpVersion+" ";
	str+=util.inspect(request.headers);
	logFile.write(str+"\n");
}

function outputLog()
{
	console.log("Reg Success: "+logs[0]+" Reg Failed: "+logs[1]+" servers: "+logs[2]+" full: "+logs[3]);
}