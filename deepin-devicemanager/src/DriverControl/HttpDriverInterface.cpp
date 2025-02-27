#include "HttpDriverInterface.h"
#include "commonfunction.h"
#include "commontools.h"

#include <QJsonDocument>
#include <QtNetwork>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <DSysInfo>

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<HttpDriverInterface *> HttpDriverInterface::s_Instance;
std::mutex HttpDriverInterface::m_mutex;

HttpDriverInterface::HttpDriverInterface(QObject *parent) : QObject(parent)
{
}

HttpDriverInterface::~HttpDriverInterface()
{

}

QString HttpDriverInterface::getRequestJson(QString strUrl)
{
    strJsonDriverInfo = "";
    const QUrl newUrl = QUrl::fromUserInput(strUrl);
    qInfo() << "strUrl : " << newUrl;

    QNetworkRequest request(newUrl);
    QNetworkAccessManager qnam;
    QNetworkReply *reply = qnam.get(request);

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();

    strJsonDriverInfo = reply->readAll();
    //! [networkreply-error-handling-1]
    QNetworkReply::NetworkError error = reply->error();

    reply->reset();
    reply->deleteLater();
    if (error != QNetworkReply::NoError) {
        return "network error";
    }
    return strJsonDriverInfo;
}

void HttpDriverInterface::getRequest(DriverInfo *driverInfo)
{
    QString strJson;
    switch (driverInfo->type()) {
    case DR_Printer:
        strJson = getRequestPrinter(driverInfo->vendorName(), driverInfo->modelName()); break;
    //case DR_Camera:
    case DR_Scaner:
//        strJson = getRequestCamera(driverInfo->modelName());
//        break;
    case DR_Sound:
    case DR_Gpu:
    case DR_Network:
    case DR_WiFi:
        strJson = getRequestBoard(driverInfo->vendorId(), driverInfo->modelId()); break;
    default:
        break;
    }

    if (strJson.contains("network error")) {
        emit sigRequestFinished(false, "network error");
    } else {
        checkDriverInfo(strJson, driverInfo);
        qInfo() << "m_VendorId:" << driverInfo->m_VendorId;
        qInfo() << "m_ModelId:" << driverInfo->m_ModelId;
        qInfo() << "m_Packages:" << driverInfo->m_Packages;
        qInfo() << "m_DebVersion:" << driverInfo->m_DebVersion;
        qInfo() << "m_Status:" << driverInfo->m_Status;
    }
}

QString HttpDriverInterface::getRequestBoard(QString strManufacturer, QString strModels, int iClassP, int iClass)
{
    QString arch = Common::getArchStore();
    QString strUrl = CommonTools::getUrl() + "?arch=" + arch;
    QString build = getOsBuild();
    if (! build.isEmpty())
        strUrl += "&system=" + build;

    if (!strManufacturer.isEmpty()) {
        strUrl += "&deb_manufacturer=" + strManufacturer;
    }
    if (!strModels.isEmpty()) {
        strUrl += "&product=" + strModels;
    }
    if (0 < iClassP) {
        strUrl += "&class_p=" + QString(iClassP);
    }
    if (0 < iClass) {
        strUrl += "&class=" + QString(iClass);
    }
    return getRequestJson(strUrl);
}

QString HttpDriverInterface::getRequestPrinter(QString strDebManufacturer, QString strDesc)
{
    QString arch = Common::getArchStore();
    QString strUrl = CommonTools::getUrl() + "?arch=" + arch;
    int iType = DTK_CORE_NAMESPACE::DSysInfo::uosType();
    int iEditionType = DTK_CORE_NAMESPACE::DSysInfo::uosEditionType();
    strUrl += "&system=" + QString::number(iType) + '-' + QString::number(iEditionType);

    if (!strDebManufacturer.isEmpty()) {
        if (strDebManufacturer == "HP" || strDebManufacturer == "Hewlett-Packard") {
            strDebManufacturer = "HP";
        }
        strUrl += "&deb_manufacturer=" + strDebManufacturer;
    }
    if (!strDesc.isEmpty()) {
        strUrl += "&desc=" + strDesc;
    }
    return getRequestJson(strUrl);
}

QString HttpDriverInterface::getRequestCamera(QString strDesc)
{
    QString arch = Common::getArchStore();
    QString strUrl = CommonTools::getUrl() + "?arch=" + arch;
    int iType = DTK_CORE_NAMESPACE::DSysInfo::uosType();
    int iEditionType = DTK_CORE_NAMESPACE::DSysInfo::uosEditionType();
    strUrl += "&system=" + QString::number(iType) + '-' + QString::number(iEditionType);

    if (!strDesc.isEmpty()) {
        strUrl += "&desc=" + strDesc;
    }
    return getRequestJson(strUrl);
}

void HttpDriverInterface::checkDriverInfo(QString strJson, DriverInfo *driverInfo)
{
    if (strJson.isEmpty()) {
        return;
    }

    QList<RepoDriverInfo> lstDriverInfo;
    if (! convertJsonToDeviceList(strJson, lstDriverInfo))
        return ;

    if (lstDriverInfo.size() == 0) {
        return;
    }

    // 找到最优等级
    int max = 0;
    foreach (const RepoDriverInfo &info, lstDriverInfo) {
        if (max < info.iLevel) {
            max = info.iLevel;
        }
    }

    // 找到最优选择
    int index = 0;
    int res_out = 0;
    for (int i = 0; i < lstDriverInfo.size(); i++) {
        if (max == lstDriverInfo[i].iLevel) {
            // 选中第一个最优等级的index
            int res = packageInstall(lstDriverInfo[i].strPackages, lstDriverInfo[i].strDebVersion);
            if (res > 0) {
                res_out = res;
                index = i;
            }
        }
    }

    // 找到最优选择后，设置状态，最新、可安装、可更新
    driverInfo->m_DebVersion  = lstDriverInfo[index].strDebVersion;
    driverInfo->m_Packages = lstDriverInfo[index].strPackages;
    driverInfo->m_Size = lstDriverInfo[index].strSize;
    driverInfo->m_Byte = lstDriverInfo[index].bytes;

    if (2 == res_out) {
        // 此时说明最优版本已经安装
        driverInfo->m_Status = ST_DRIVER_IS_NEW;
    } else if (1 == res_out) {
        // 此时安装了最优包的不同版本
        driverInfo->m_Status = ST_CAN_UPDATE;
    } else {
        // 此时没有安装最优推荐包(包括其他版本)
        if (driverInfo->driverName().isEmpty() && driverInfo->type() != DR_Printer) {
            driverInfo->m_Status = ST_NOT_INSTALL;
        } else {
            // 此时安装了其他驱动
            driverInfo->m_Status = ST_CAN_UPDATE;
        }
    }
}

int HttpDriverInterface::packageInstall(const QString &package_name, const QString &version)
{
    // 0:没有包 1:版本不一致 2:版本一致
    QProcess process;
    QStringList options;
    options << "-c" << "apt policy " + package_name;
    process.start("/bin/bash", options);
    process.waitForFinished(-1);
    QStringList infoList = QString(process.readAllStandardOutput()).split("\n");

    if (infoList.size() <= 2 || infoList[1].contains("（") || infoList[1].contains("("))
        return 0;
    if (infoList[1].contains(version))
        return 2;
    return 1;
}

QString HttpDriverInterface::getOsBuild()
{
    QFile file("/etc/os-version");
    if (!file.open(QIODevice::ReadOnly))
        return "";
    QString info = file.readAll().data();
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        if (line.startsWith("OsBuild")) {
            QStringList words = line.split("=");
            if (2 == words.size()) {
                return words[1].trimmed();
            }
        }
    }
    return "";
}

bool HttpDriverInterface::convertJsonToDeviceList(QString strJson, QList<RepoDriverInfo> &lstDriverInfo)
{
    QJsonArray ja;
    QJsonArray jappds;
    QJsonArray jamodel;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(strJson.toLocal8Bit(), &json_error));

    lstDriverInfo.clear();
    if (strJson.isEmpty() || json_error.error != QJsonParseError::NoError) {
        return false;
    }
    if ("success" != jsonDoc.object().value("msg").toString()) {
        return false;
    }
    ja = jsonDoc.object().value("data").toObject().value("list").toArray();
    if (ja.size() < 1) {
        return false;
    }

    QJsonObject _jsonObj;
    QJsonObject _jsonObjppds;
    for (int i = 0; i < ja.size(); i++) {
        _jsonObj = ja.at(i).toObject();
        RepoDriverInfo _driverinfo;
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (_jsonObj.contains("arch")) {
            _driverinfo.strArch = _jsonObj.value("arch").toString();
        }
        if (_jsonObj.contains("manufacturer")) {
            _driverinfo.strManufacturer = _jsonObj.value("manufacturer").toString();
        }
        if (_jsonObj.contains("deb_manufacturer")) {
            _driverinfo.strDebManufacturer = _jsonObj.value("deb_manufacturer").toString();
        }
        if (_jsonObj.contains("version")) {
            _driverinfo.strVersion = _jsonObj.value("version").toString();
        }
        if (_jsonObj.contains("deb_version")) {
            _driverinfo.strDebVersion = _jsonObj.value("deb_version").toString();
        }
        if (_jsonObj.contains("packages")) {
            _driverinfo.strPackages = _jsonObj.value("packages").toString();
        }
        if (_jsonObj.contains("class_p")) {
            _driverinfo.strClass_p = _jsonObj.value("class_p").toString();
        }
        if (_jsonObj.contains("class")) {
            _driverinfo.strClass = _jsonObj.value("class").toString();
        }
        if (_jsonObj.contains("models")) {
            //_driverinfo.strModels = ;
            jamodel = _jsonObj.value("models").toArray();
            for (int j = 0; j < jamodel.size(); j++) {
                _driverinfo.strModels.push_back(jamodel.at(j).toString());
            }
        }
        if (_jsonObj.contains("products")) {
            _driverinfo.strProducts = _jsonObj.value("products").toString();
        }
        if (_jsonObj.contains("deb")) {
            _driverinfo.strDeb = _jsonObj.value("deb").toString();
        }
        if (_jsonObj.contains("level")) {
            _driverinfo.iLevel = _jsonObj.value("level").toInt();
        }
        if (_jsonObj.contains("system")) {
            _driverinfo.strSystem = _jsonObj.value("system").toString();
        }
        if (_jsonObj.contains("desc")) {
            _driverinfo.strDesc = _jsonObj.value("desc").toString();
        }
        if (_jsonObj.contains("adaptation")) {
            _driverinfo.strAdaptation = _jsonObj.value("adaptation").toString();
        }
        if (_jsonObj.contains("source")) {
            _driverinfo.strSource = _jsonObj.value("source").toString();
        }
        if (_jsonObj.contains("download_url")) {
            _driverinfo.strDownloadUrl = _jsonObj.value("download_url").toString();
        }
        if (_jsonObj.contains("size")) {
            double size = _jsonObj.value("size").toInt();
            _driverinfo.bytes = qint64(size);
            if (size < 1024 * 1024) {
                _driverinfo.strSize = QString::number(size / 1024, 'f', 2) + "KB";
            } else if (size < 1024 * 1024 * 1024) {
                _driverinfo.strSize = QString::number(size / 1024 / 1024, 'f', 2) + "MB";
            } else {
                _driverinfo.strSize = QString::number(size / 1024 / 1024 / 1024, 'f', 2) + "GB";
            }
        }
        if (_jsonObj.contains("ppds")) {
            jappds = _jsonObj.value("ppds").toArray();
            strPpds _ppds;
            for (int j = 0; j < jappds.size(); j++) {
                _jsonObjppds = jappds.at(j).toObject();
                if (_jsonObjppds.contains("desc")) {
                    _ppds.strDesc = _jsonObjppds.value("desc").toString();
                }
                if (_jsonObjppds.contains("manufacturer")) {
                    _ppds.strManufacturer = _jsonObjppds.value("manufacturer").toString();
                }
                if (_jsonObjppds.contains("source")) {
                    _ppds.strSource = _jsonObjppds.value("source").toString();
                }
                _driverinfo.lstPpds.push_back(_ppds);
            }
        }

        lstDriverInfo.push_back(_driverinfo);
    }

    return true;
}

