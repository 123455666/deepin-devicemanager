// 项目自身文件
#include "DeviceInfo.h"
#include "commondefine.h"
#include"DeviceManager.h"

#include <DApplication>

#include <QDebug>
#include <QProcess>

DWIDGET_USE_NAMESPACE

DeviceBaseInfo::DeviceBaseInfo(QObject *parent)
    : QObject(parent)
    , m_UniqueID("")
    , m_SerialID("")
    , m_SysPath("")
    , m_HardwareClass("")
    , m_HwinfoToLshw("")
    , m_Enable(true)
    , m_CanEnable(false)
    , m_CanUninstall(false)
    , m_Available(true)
    , m_Index(0)
    , m_forcedDisplay(false)
{
}

DeviceBaseInfo::~DeviceBaseInfo()
{

}

const QList<QPair<QString, QString>> &DeviceBaseInfo::getOtherAttribs()
{
    // 获取其他设备信息列表
    m_LstOtherInfo.clear();
    loadOtherDeviceInfo();
    return m_LstOtherInfo;
}

const QList<QPair<QString, QString> > &DeviceBaseInfo::getBaseAttribs()
{
    // 获取基本信息列表
    m_LstBaseInfo.clear();
    loadBaseDeviceInfo();
    return m_LstBaseInfo;
}

const QStringList &DeviceBaseInfo::getTableHeader()
{
    // 获取表头
    if (m_TableHeader.size() == 0) {
        loadTableHeader();
        m_TableHeader.append(m_CanEnable ? "yes" : "no");
    }

    return m_TableHeader;
}

const QStringList &DeviceBaseInfo::getTableData()
{
    // 获取表格数据
    m_TableData.clear();
    loadTableData();
    return m_TableData;
}

QString DeviceBaseInfo::subTitle()
{
    return QString("");
}

bool DeviceBaseInfo::isValueValid(QString &value)
{
    // 判断属性值是否有效
    if (value.isEmpty())
        return false;

    if (value == QObject::tr("Unknown"))
        return false;

    if (value == QString("Unknown"))
        return false;

    if (value.compare(QString("N/A"), Qt::CaseInsensitive) == 0)
        return false;

    if (value == QString(""))
        return false;

    if (value.compare(QString("Null"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("none"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Not Provided"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Not Specified"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Default string"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Unspecified"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Not Present"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("<OUT OF SPEC>"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Other"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("TBD"), Qt::CaseInsensitive) == 0)
        return false;

    return true;
}

void DeviceBaseInfo::setForcedDisplay(const bool &flag)
{
    m_forcedDisplay = flag;
}

void DeviceBaseInfo::toHtmlString(QDomDocument &doc)
{
    // 设备信息转为Html
    baseInfoToHTML(doc, m_LstBaseInfo);
    baseInfoToHTML(doc, m_LstOtherInfo);
}

void DeviceBaseInfo::baseInfoToHTML(QDomDocument &doc, QList<QPair<QString, QString> > &infoLst)
{
    // 设备信息转为HTML
    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
    table.setAttribute("cellpadding", "3");

    // 添加HTML表格内容
    foreach (auto info, infoLst) {
        if (isValueValid(info.second)) {
            QDomElement tr = doc.createElement("tr");

            // 第一列
            QDomElement td = doc.createElement("td");
            td.setAttribute("width", "15%");
            td.setAttribute("style", "text-align:left;");

            QDomText nameText = doc.createTextNode(info.first + ": ");
            td.appendChild(nameText);
            tr.appendChild(td);

            // 第二列
            QDomElement td2 = doc.createElement("td");
            td2.setAttribute("width", "85%");

            QDomText valueText;
            valueText = doc.createTextNode(info.second);
            td2.appendChild(valueText);

            tr.appendChild(td2);

            table.appendChild(tr);
        }

        doc.appendChild(table);
    }
}

void DeviceBaseInfo::subTitleToHTML(QDomDocument &doc)
{
    // 子标题转为HTML格式
    if (false == this->subTitle().isEmpty()) {
        QDomElement h3 = doc.createElement("h3");
        QDomText valueText = doc.createTextNode(this->subTitle());
        h3.appendChild(valueText);
        doc.appendChild(h3);
    }
}

void DeviceBaseInfo::toDocString(Docx::Document &doc)
{
    // 设备信息转为doc
    baseInfoToDoc(doc, m_LstBaseInfo);
    baseInfoToDoc(doc, m_LstOtherInfo);
}

void DeviceBaseInfo::baseInfoToDoc(Docx::Document &doc, QList<QPair<QString, QString> > &infoLst)
{
    // 设备信息保存为Doc
    foreach (auto item, infoLst) {
        QString value = item.second;

        // 判断属性值是否有效
        if (false == isValueValid(value))
            continue;

        // 添加doc段落
        QString line = item.first + ":  " + item.second;
        doc.addParagraph(line);
    }
}

void DeviceBaseInfo::toXlsxString(QXlsx::Document &xlsx, QXlsx::Format &boldFont)
{
    // 设备信息转为xlxs表格
    baseInfoToXlsx(xlsx, boldFont, m_LstBaseInfo);
    baseInfoToXlsx(xlsx, boldFont, m_LstOtherInfo);
}

void DeviceBaseInfo::baseInfoToXlsx(QXlsx::Document &xlsx, QXlsx::Format &boldFont, QList<QPair<QString, QString> > &infoLst)
{
    // 设置表格内容字体不加粗,字号10
    boldFont.setFontBold(false);
    boldFont.setFontSize(10);

    foreach (auto item, infoLst) {
        QString value = item.second;

        // 判断属性值是否有效
        if (false == isValueValid(value))
            continue;

        // 获取行数
        int _row = DeviceManager::instance()->currentXlsRow();
        xlsx.write(_row, 1, item.first, boldFont);
        xlsx.write(_row, 2, item.second, boldFont);
    }
}

void DeviceBaseInfo::toTxtString(QTextStream &out)
{
    // 设备信息转为txt
    baseInfoToTxt(out, m_LstBaseInfo);
    baseInfoToTxt(out, m_LstOtherInfo);
}

void DeviceBaseInfo::baseInfoToTxt(QTextStream &out, QList<QPair<QString, QString> > &infoLst)
{
    foreach (auto item, infoLst) {
        QString value = item.second;

        // 判断属性值是否有效
        if (false == isValueValid(value))
            continue;

        // 设置第一列占21个字符
        out.setFieldWidth(21);
        out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
        out << item.first + ": ";
        out.setFieldWidth(0);
        out << item.second;
        out << "\n";
    }
}

void DeviceBaseInfo::tableInfoToTxt(QTextStream &out)
{
    // 获取表格内容
    getTableData();

    // 判断是否有表格内容
    if (m_TableData.size() < 1)
        return;

    // 设置占位宽度
    QString text = m_TableData[0];
    out.setFieldWidth(int(text.size() * 1.5));
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignRight);

    foreach (auto item, m_TableData) {
        out.setFieldWidth(28);
        out << item;
    }

    out.setFieldWidth(0);
    out << "\n";
}

void DeviceBaseInfo::tableHeaderToTxt(QTextStream &out)
{
    // 获取表头
    getTableHeader();

    // 判断是否有表头
    if (m_TableHeader.size() < 1)
        return;

    // 设置占位宽度
    QString text = m_TableHeader[0];
    out.setFieldWidth(int(text.size() * 1.5));
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);

    out << "\n";
    for (int col = 0; col < m_TableHeader.size() - 1; ++col) {
        out.setFieldWidth(30);
        out << m_TableHeader[col];
    }
    out.setFieldWidth(0);
    out << "\n";
}

void DeviceBaseInfo::tableInfoToHtml(QFile &html)
{
    // 获取表格内容
    getTableData();

    // 判断是否有表格内容
    if (m_TableData.size() < 1)
        return;

    // 写表格内容
    foreach (auto item, m_TableData) {
        html.write(QString("<td style=\"width:200px;text-align:left;\">" + item + "</td>").toUtf8().data());
    }

    html.write("</tr>\n");
}

void DeviceBaseInfo::tableHeaderToHtml(QFile &html)
{
    // 获取表头信息
    getTableHeader();

    // 判断是否有表头
    if (m_TableHeader.size() < 1)
        return;

    html.write("<thead><tr>\n");

    // 写表头内容
    for (int col = 0; col < m_TableHeader.size() - 1; ++col)
        html.write(QString("<th style=\"width:200px;text-align:left; white-space:pre;\">" + m_TableHeader[col] + "</th>").toUtf8().data());

    html.write("</tr></thead>\n");
}

void DeviceBaseInfo::tableInfoToDoc(Docx::Table *tab, int &row)
{
    // 表格信息保存为Doc
    if (nullptr == tab)
        return;

    // 获取表格数据
    getTableData();

    if (m_TableData.size() < 1)
        return;

    // 添加doc表格
    for (int col = 0; col < m_TableData.size(); ++col) {
        auto cel = tab->cell(row, col);
        cel->addText(m_TableData[col]);
    }
}

void DeviceBaseInfo::tableHeaderToDoc(Docx::Table *tab)
{
    // 表头保存为doc
    getTableHeader();

    if (m_TableHeader.size() < 1)
        return;

    // 添加表头信息
    for (int col = 0; col < m_TableHeader.size() - 1; ++col)  {
        tab->addColumn();
        auto cel = tab->cell(0, col);
        cel->addText(m_TableHeader[col]);
    }
}

void DeviceBaseInfo::tableInfoToXlsx(QXlsx::Document &xlsx)
{
    // 获取表格信息
    getTableData();

    if (m_TableData.size() < 1)
        return;

    // 添加表格信息
    int curRow = DeviceManager::instance()->currentXlsRow();
    for (int col = 0; col < m_TableData.size(); ++col)
        xlsx.write(curRow, col + 1, m_TableData[col]);
}

void DeviceBaseInfo::tableHeaderToXlsx(QXlsx::Document &xlsx)
{
    // 获取表头
    getTableHeader();

    if (m_TableHeader.size() < 1)
        return;

    // 添加表头信息
    int curRow = DeviceManager::instance()->currentXlsRow();
    for (int col = 0; col < m_TableHeader.size() - 1; ++col) {
        QXlsx::Format boldFont;
        boldFont.setFontSize(10);
        boldFont.setFontBold(true);
        xlsx.write(curRow, col + 1, m_TableHeader[col], boldFont);
    }
}

void DeviceBaseInfo::setOtherDeviceInfo(const QString &key, const QString &value)
{
    m_MapOtherInfo[key] = value;
}

EnableDeviceStatus DeviceBaseInfo::setEnable(bool)
{
    return EDS_Success;
}

bool DeviceBaseInfo::enable()
{
    return m_Enable;
}

bool DeviceBaseInfo::available()
{
    if (driver().isEmpty()) {
        m_Available = false;
    }
    return m_forcedDisplay ? m_forcedDisplay : m_Available;
}

bool DeviceBaseInfo::driverIsKernelIn(const QString &driver)
{
    // 驱动为空情况:
    // 1. 驱动被卸载了 此时驱动属于核外驱动
    // 2. ps/2 笔记本触摸板 暂无法获取驱动 此时当成核内驱动处理
    // 3. 但是由于判断是否是ps/2或者笔记本触摸板在子类判断(无需放在基类)，因此此处为空时先返回false，而在子类(DeviceInput)调用后判断是否是ps/2鼠标
    if (driver.isEmpty()) {
        return false;
    }

    // 英伟达驱动无法获取驱动模块，但是不属于核内驱动
    if ("nvidia" == driver) {
        return false;
    }

    QString info = "";
    QProcess process;

    // 判断lsmod是否能查询
    process.start("sh", QStringList() << "-c" << QString("modinfo %1 | grep 'filename:'").arg(driver));
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();
    return info.isEmpty();
}

void DeviceBaseInfo::setCanEnale(bool can)
{
    m_CanEnable = can;
}

bool DeviceBaseInfo::canEnable()
{
    return m_CanEnable;
}

void DeviceBaseInfo::setEnableValue(bool e)
{
    m_Enable = e;
}

bool DeviceBaseInfo::canUninstall()
{
    return m_CanUninstall;
}

void DeviceBaseInfo::setCanUninstall(bool can)
{
    m_CanUninstall = can;
}

void DeviceBaseInfo::setHardwareClass(const QString &hclass)
{
    m_HardwareClass = hclass;
}

const QString &DeviceBaseInfo::hardwareClass() const
{
    return m_HardwareClass;
}

const QString &DeviceBaseInfo::uniqueID() const
{
    return m_UniqueID;
}

const QString &DeviceBaseInfo::sysPath() const
{
    return m_SysPath;
}

const QString DeviceBaseInfo::getVendorOrModelId(const QString &sysPath, bool flag)
{
    // 从文件中获取制造商ID信息
    QFile vendorFile;
    QString strVendorFile("/vendor");
    QString strDeviceFile("/device");
    QString strSysFSLink = sysPath;

    if (sysPath.contains("usb")) {
        strVendorFile = "/idVendor";
        strDeviceFile = "/idProduct";
        if (!QFile::exists("/sys" + strSysFSLink + strVendorFile)) {
            strSysFSLink = strSysFSLink.mid(0, sysPath.lastIndexOf('/'));
        }
    }

    if (flag) {
        vendorFile.setFileName(QString("/sys") + strSysFSLink + strVendorFile);
    } else {
        vendorFile.setFileName(QString("/sys") + strSysFSLink + strDeviceFile);
    }

    if (false == vendorFile.open(QIODevice::ReadOnly))
        return QString();

    QString vendor = vendorFile.readAll();
    vendorFile.close();

    return vendor;
}

const QString DeviceBaseInfo::getDriverVersion()
{
    QProcess process;
    process.start("modinfo " + driver()  + "| grep version");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();

    foreach (QString out, output.split("\n")) {
        QStringList item = out.split(":");
        if ("version" == item[0].trimmed()) {
            return item[1];
        }
    }

    return "";
}

const QString DeviceBaseInfo::getOverviewInfo()
{
    return QString("");
}

void DeviceBaseInfo::loadTableHeader()
{
    // 添加表头信息
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
    m_TableHeader.append(tr("Model"));
}

void DeviceBaseInfo::addFilterKey(const QString &key)
{
    // 添加可显示设备属性
    m_FilterKey.insert(key);
}

void DeviceBaseInfo::getOtherMapInfo(const QMap<QString, QString> &mapInfo)
{
    // 获取其他设备信息
    QMap<QString, QString>::const_iterator it = mapInfo.begin();
    for (; it != mapInfo.end(); ++it) {
        QString k = DApplication::translate("QObject", it.key().trimmed().toStdString().data());

        // 可显示设备属性中存在该属性
        if (m_FilterKey.find(k) != m_FilterKey.end())
            m_MapOtherInfo.insert(k, it.value().trimmed());
    }
}

void DeviceBaseInfo::addBaseDeviceInfo(const QString &key, const QString &value)
{
    // 添加基础设备信息
    if (!value.isEmpty())
        m_LstBaseInfo.append(QPair<QString, QString>(key, value));
}

void DeviceBaseInfo::addOtherDeviceInfo(const QString &key, const QString &value)
{
    // 添加其他设备信息
    if (!value.isEmpty())
        m_LstOtherInfo.insert(0, QPair<QString, QString>(key, value));
}

void DeviceBaseInfo::setAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite)
{
    // map中存在该属性
    if (mapInfo.find(key) == mapInfo.end())
        return;

    // 属性值不能为空
    if (mapInfo[key] == "")
        return;

    // overwrite 为true直接覆盖
    if (overwrite) {
        variable = mapInfo[key].trimmed();
    } else {

        // overwrite 为false,如果当前属性值为空或unknown时可覆盖
        if (variable.isEmpty())
            variable = mapInfo[key].trimmed();

        if (variable.contains("Unknown", Qt::CaseInsensitive))
            variable = mapInfo[key].trimmed();
    }
}

void DeviceBaseInfo::mapInfoToList()
{
    // m_MapOtherInfo --> m_LstOtherInfo
    // QMap内容转为QList存储
    auto iter = m_MapOtherInfo.begin();

    for (; iter != m_MapOtherInfo.end(); ++iter) {
        if (isValueValid(iter.value()))
            m_LstOtherInfo.append(QPair<QString, QString>(iter.key(), iter.value()));
    }
}

void DeviceBaseInfo::setHwinfoLshwKey(const QMap<QString, QString> &mapInfo)
{
    // 网卡使用物理地址作为匹配值
    if (mapInfo.find("HW Address") != mapInfo.end()) {
        m_HwinfoToLshw = mapInfo["HW Address"];
        return;
    }

    // 非usb总线设备直接使用 SysFS BusID
    if (mapInfo.find("SysFS ID") != mapInfo.end()
            && mapInfo.find("SysFS BusID") != mapInfo.end()
            && !mapInfo["SysFS ID"].contains("usb")) {
        m_HwinfoToLshw = mapInfo["SysFS BusID"];
        return;
    }

    // usb总线设备
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() != 2) {
        return;
    }
    QStringList chs = words[0].split("-");
    if (chs.size() != 2) {
        return;
    }

    // 1-1.3
    // 1-3
    QStringList nums = QStringList() << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "a" << "b" << "c" << "d" << "e" << "f" << "g" << "h" << "i" << "j";
    QRegExp reg("([0-9a-zA-Z])-([0-9a-zA-Z]\\.[0-9a-zA-Z])");
    if (reg.exactMatch(words[0])) {
        m_HwinfoToLshw = QString("usb@%1:%2").arg(reg.cap(1)).arg(reg.cap(2));
    } else {
        int first = chs[0].toInt();
        int second = chs[1].toInt();
        m_HwinfoToLshw = QString("usb@%1:%2").arg(nums.at(first)).arg(nums.at(second));
    }
}

bool DeviceBaseInfo::matchToLshw(const QMap<QString, QString> &mapInfo)
{
    // 网卡设备与序列号匹配上
    if (mapInfo.find("logical name") != mapInfo.end() && mapInfo.find("serial") != mapInfo.end()) {
        if (m_HwinfoToLshw == mapInfo["serial"]) {
            return true;
        }
    }

    if (mapInfo.find("bus info") == mapInfo.end()) {
        return false;
    }
    // 非usb设备
    if (mapInfo["bus info"].startsWith("pci")) {
        QStringList words = mapInfo["bus info"].split("@");
        if (2 == words.size() && words[1] == m_HwinfoToLshw) {
            return true;
        }
    }

    // USB 设备
    if (m_HwinfoToLshw == mapInfo["bus info"]) {
        return true;
    }
    return false;
}

void DeviceBaseInfo::setsysFStoHwinfoKey(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("VID_PID") != mapInfo.end() ) {
        m_sysFSToHwinfo = mapInfo["VID_PID"];
        return;
    }
}

bool DeviceBaseInfo::sysFSmatchToHwinfo(const QMap<QString, QString> &mapInfo)
{
    // VID_PID 匹配上
    if (mapInfo.find("VID_PID") != mapInfo.end() ) {
        if (m_sysFSToHwinfo == mapInfo["VID_PID"]) {
            return true;
        }
    }
    return false;
}

const  QString DeviceBaseInfo::get_string(const QString &sysPathfile)
{
    // 从文件中获取D信息
    QFile file(sysPathfile);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QString info = file.readAll();
    file.close();
    return info;
}

