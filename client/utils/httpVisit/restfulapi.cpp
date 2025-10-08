#include "restfulapi.h"
#include "QDebug"
#include "qreplytimeout.h"
#include <QTimer>
#include <QHttpMultiPart>

QMap<QNetworkReply *, int> replyTypeMap;
QNetworkAccessManager RestFulApi::m_accessManager(0);
RestFulApi::RestFulApi()
{

}

QNetworkReply * RestFulApi::visitUrl(QString url,VisitType visitType,ReplyType replyType,QString hearderInfo,QByteArray byte, bool outLog, int timeout,QNetworkRequest::Priority priority)
{
    if(outLog)
    {
        qDebug() << "ReplyType: " << int(replyType) << "====URL: " << url;
    }

    if(m_accessManager.networkAccessible() == QNetworkAccessManager::NotAccessible)
    {
        m_accessManager.setNetworkAccessible(QNetworkAccessManager::Accessible);
    }

    QNetworkRequest request_registered;//设置函数头
    QUrl serviceUrl(url);
    request_registered.setUrl(serviceUrl);
    // 设置SSL认证方式
    QSslConfiguration sslconfig;
    sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    //传输层安全协议，一般设为TlsV1_0 通用性高，具体要看服务器配置；
    sslconfig.setProtocol(QSsl::TlsV1_2);
    //配置SSL
    request_registered.setSslConfiguration(sslconfig);
    //设置请求头
    request_registered.setHeader(QNetworkRequest::ContentTypeHeader, hearderInfo);
    request_registered.setPriority(priority);

    QNetworkReply* replyResult = nullptr;
    //application/json，向数据库传输参数
    if(nullptr != byte)
    {
        //发起POST请求
        if(visitType == VisitType::POST)
        {
            replyResult = m_accessManager.post(request_registered,byte);
            replyTypeMap.insert(replyResult,replyType);
        }
        //发起POST请求
        else if(visitType == VisitType::PUT)
        {
            replyResult = m_accessManager.put(request_registered,byte);
            request_registered.setUrl(request_registered.url().toString());
            replyTypeMap.insert(replyResult,replyType);
        }
        else if(visitType == VisitType::DELETE)
        {
            replyResult = m_accessManager.sendCustomRequest(request_registered,"DELETE",byte);
            request_registered.setUrl(request_registered.url().toString());
            replyTypeMap.insert(replyResult,replyType);
        }
    } else {
        //发起POST请求
        if(visitType == VisitType::POST)
        {
            replyResult = m_accessManager.post(request_registered,m_postData.toString(QUrl::FullyEncoded).toUtf8());
            replyTypeMap.insert(replyResult,replyType);
        }
        else if(visitType == VisitType::GET)
        {
            QString queryString;
            int count=0;
            for(auto each:m_postData.queryItems())
            {
                if(count==0)
                {
                    queryString+="?";
                }
                else
                {
                    queryString+="&";
                }
                 queryString+=(each.first+"="+each.second);
                 count++;
            }
            request_registered.setUrl(QUrl(request_registered.url().toString()+queryString));
            auto replyResult=m_accessManager.get(request_registered);
            replyTypeMap.insert(replyResult,replyType);
            return replyResult;
        }
        else if(visitType == VisitType::DELETE)
        {
            QString queryString;
            int count=0;
            for(auto each:m_postData.queryItems())
            {
                if(count==0)
                {
                    queryString+="?";
                }
                else
                {
                    queryString+="&";
                }
                 queryString+=(each.first+"="+each.second);
                 count++;
            }
            request_registered.setUrl(QUrl(request_registered.url().toString()+queryString));
            auto replyResult=m_accessManager.deleteResource(request_registered);
            replyTypeMap.insert(replyResult,replyType);
            return replyResult;
        }
    }

    QReplyTimeout *pTimeout = new QReplyTimeout(replyResult, timeout);
    // 超时进一步处理
    connect(pTimeout, &QReplyTimeout::timeout, [=]() {
        qDebug() << url << "网络超时......";
        //pTimeout->deleteLater();
        replyResult->abort();
        //replyResult->deleteLater();
    });
    return replyResult;
}

QNetworkReply *RestFulApi::visitUrlByFormData(QString url, int replyType, QMap<QString, QString> textMap, QMap<QString, QString> fileMap, int timeout)
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QMap<QString, QString>::iterator it1;
    QMap<QString, QString>::iterator it2;

    QVector<QFile*> qFiles;
    for(it2 = fileMap.begin(); it2 != fileMap.end(); ++it2){
        QHttpPart filePart;
        //有多个文件时不能直接使用QFile file（path），for代码块结束的时候就会被析构，导致数据无法发送，程序crash
        //单个文件可以不用for遍历，直接QFile file（path）本接口结束时才会析构
        QFile *file = new QFile(it2.value());
        file->open(QFile::ReadOnly);
        qFiles.push_back(file);

        filePart.setBodyDevice(file);
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));//这里我们上传的是图片
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + it2.key() + "\"; filename=\""+file->fileName()+"\""));
        multiPart->append(filePart);

    }

    QHttpPart dataPartTemp;

    for(it1 = textMap.begin(); it1 != textMap.end(); ++it1)
    {
        QHttpPart dataPart;
        //dataPart.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/plain"));
        QString header = "form-data; name=\"" + it1.key() + "\"";
        QString body = it1.value().toUtf8();
        dataPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant(header));
        dataPart.setBody(it1.value().toUtf8());
        multiPart->append(dataPart);
        multiPart->setBoundary("----WebKitFormBoundary8XBbCfyRYNRimVsH");
     }

     //
    QNetworkRequest request_registered;
    // 设置SSL认证方式
    QSslConfiguration sslconfig;
    sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    //传输层安全协议，一般设为TlsV1_0 通用性高，具体要看服务器配置；
    sslconfig.setProtocol(QSsl::TlsV1_0);
    //配置SSL
    request_registered.setSslConfiguration(sslconfig);
    request_registered.setHeader(QNetworkRequest::ContentTypeHeader, QString("multipart/form-data; boundary=----WebKitFormBoundary8XBbCfyRYNRimVsH"));
    request_registered.setUrl(QUrl(url));
    QNetworkReply* replyResult = nullptr;
    replyResult = m_accessManager.post(request_registered, multiPart);
    replyTypeMap.insert(replyResult,replyType);

    //防止内存泄漏
    multiPart->setParent(replyResult);
    for(auto file : qFiles)
    {
        file->setParent(replyResult);
    }

    return replyResult;
}

void RestFulApi::downloadFileFromUrl(QByteArray byte, QString strFilePath)
{
    QFile file;
    file.setFileName(strFilePath);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(byte);
        file.close();
    }
}

bool RestFulApi::replyResultCheck(QJsonObject obj, QNetworkReply *networkReply, bool outLog)
{
    networkReply->deleteLater();//合适的时候释放

    if(outLog || obj.value("code").toInt() != 200)
    {
        qDebug() << "ReplyType: " << replyTypeMap.value(networkReply) <<
                    "====code: " + QString::number(obj.value("code").toInt()) +
                    "====msg: " + obj.value("message").toString();
    }

    if(obj.value("code").toInt() != 200)
        qDebug() << "错误data: " << obj.value("data").toString();

    if(obj.value("code").toInt() == 200)
        return true;

    return false;
}

