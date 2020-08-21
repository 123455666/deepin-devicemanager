#include "PageDetail.h"
#include "TextBrowser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPalette>
#include <QDebug>
#include <QScrollBar>
#include <QAction>

#include <DApplicationHelper>
#include <DApplication>
#include <DStyle>
#include <DMenu>
#include <QClipboard>
#include <DFontSizeManager>

#include "DeviceInfo.h"

#include "DeviceInfo.h"

DWIDGET_USE_NAMESPACE

DetailButton::DetailButton(const QString &txt)
    : DCommandLinkButton(txt)
{

}

void DetailButton::updateText()
{
    if (this->text() == tr("More")) {
        this->setText(tr("Collapse"));
    } else {
        this->setText(tr("More"));
    }
}

void DetailButton::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    painter.fillRect(rect, bgBrush);

    painter.restore();
    DCommandLinkButton::paintEvent(e);
}

DetailSeperator::DetailSeperator(DWidget *parent)
    : DWidget(parent)
{
    setFixedHeight(1);
}

void DetailSeperator::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 分割线两端到边框的边距
    int spacing = 20;

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 清除背景色颜色
    QBrush clearBrush(palette.color(cg, DPalette::Base));
    painter.fillRect(rect, clearBrush);

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    rect.setX(rect.x() + spacing);
    rect.setWidth(rect.width() - spacing);
    rect.setHeight(1);
//    rect.setY(rect.y() + height() - 2);
    QBrush bgBrush(palette.color(cg, DPalette::FrameShadowBorder));
    painter.fillRect(rect, bgBrush);

    painter.restore();
    DWidget::paintEvent(e);
}

ScrollAreaWidget::ScrollAreaWidget(DWidget *parent)
    : DWidget(parent)
{

}

void ScrollAreaWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 清除背景色颜色
    QBrush clearBrush(palette.color(cg, DPalette::Base));
    painter.fillRect(rect, clearBrush);

    painter.restore();
}

PageDetail::PageDetail(QWidget *parent)
    : Dtk::Widget::DWidget(parent)
    , mp_ScrollAreaLayout(new QVBoxLayout())
    , mp_ScrollArea(new QScrollArea(this))
{
    setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *hLayout = new QVBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);

    // 设置scrollarea的属性
    mp_ScrollArea->setWidgetResizable(true);
    mp_ScrollArea->setFrameShape(QFrame::NoFrame);
    mp_ScrollArea->setContentsMargins(0, 0, 0, 0);
    mp_ScrollAreaLayout->setContentsMargins(0, 0, 0, 0);
    mp_ScrollAreaLayout->setSpacing(0);


    // 设置ScrollArea里面的widget,这个widget是必须要的
    ScrollAreaWidget *widget = new ScrollAreaWidget(this);
    widget->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(mp_ScrollAreaLayout);
    mp_ScrollArea->setWidget(widget);
    hLayout->addWidget(mp_ScrollArea);
    setLayout(hLayout);
}

void PageDetail::showDeviceInfo(const QList<DeviceBaseInfo *> &lstInfo)
{
    // Clear widgets first
    clearWidget();

    // Create widgets for showing device info
    foreach (auto device, lstInfo) {
        if (!device) {continue;}
        TextBrowser *txtBrowser = new TextBrowser(this);
        txtBrowser->showDeviceInfo(device);
        connect(txtBrowser, &TextBrowser::refreshInfo, this, &PageDetail::slotRefreshInfo);
        connect(txtBrowser, &TextBrowser::exportInfo, this, &PageDetail::slotExportInfo);
        connect(txtBrowser, &TextBrowser::copyAllInfo, this, &PageDetail::slotCopyAllInfo);
        addWidgets(txtBrowser, device->enable());
        // 当添加到最后一个设备详细信息时，隐藏分隔符
        if (device == lstInfo.last()) {
            m_ListDetailSeperator[lstInfo.size() - 1]->setVisible(false);
        }
    }
    mp_ScrollAreaLayout->addStretch();
}

void PageDetail::showInfoOfNum(int index)
{
    if (index >= m_ListHlayout.size()
            || index >= m_ListTextBrowser.size()
            || index >= m_ListDetailButton.size()
            || index >= m_ListDetailSeperator.size()) {
        return;
    }
    int value = 0;
    for (int i = 0; i <= index - 1; i++) {
        value += m_ListTextBrowser[i]->height();
        value += m_ListDetailButton[i]->height();
        value += m_ListDetailSeperator[i]->height();
    }
    mp_ScrollArea->verticalScrollBar()->setValue(value);
}

bool PageDetail::enableDevice(int row, bool enable)
{
    if (m_ListTextBrowser.size() <= row) {
        return false;
    }

    // 设置 TextBrowser 可用
    TextBrowser *browser = m_ListTextBrowser.at(row);
    if (!browser) {
        return false;
    }
    if (!browser->setDeviceEnabled(enable)) {
        return false;
    }

    // 设置 展开收起按钮 可见和可用
//    DetailButton *button = m_ListDetailButton.at(row);
//    if (button) {
//        button->setVisible(enable);
//        button->setEnabled(enable);
//    }
    return true;
}

void PageDetail::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取系统默认的圆角半径
//    QStyleOptionFrame option;
//    initStyleOption(&option);
//    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
//    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);
    int radius = 8;

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QPainterPath paintPath;
    paintPath.addRoundedRect(rect, radius, radius);
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    painter.fillPath(paintPath, bgBrush);

    painter.restore();
    DWidget::paintEvent(e);
}

void PageDetail::addWidgets(TextBrowser *widget, bool enable)
{
    // 添加 textBrowser
    if (widget != nullptr && m_ListTextBrowser.size() != 0) {
        mp_ScrollAreaLayout->addSpacing(10);
    }

    mp_ScrollAreaLayout->addWidget(widget);

    // 添加按钮
    QHBoxLayout *vLayout = new QHBoxLayout();
    vLayout->addSpacing(20);
    DetailButton *button = new DetailButton(tr("More"));
    DFontSizeManager::instance()->bind(button, DFontSizeManager::T8);
    connect(button, &DetailButton::clicked, this, &PageDetail::slotBtnClicked);
    if (!enable) {
        button->setVisible(false);
    }
    vLayout->addWidget(button);
    if (!enable) {
        button->setVisible(false);
    }
//    vLayout->addStretch(-1);
    mp_ScrollAreaLayout->addLayout(vLayout);

    // 添加分割线
    mp_ScrollAreaLayout->addSpacing(10);

    DetailSeperator *seperator = new DetailSeperator(widget);
    mp_ScrollAreaLayout->addWidget(seperator);

    // **********************************
    m_ListTextBrowser.append(widget);
    m_ListHlayout.append(vLayout);
    m_ListDetailButton.append(button);
    m_ListDetailSeperator.append(seperator);
}

void PageDetail::clearWidget()
{
    foreach (auto widget, m_ListTextBrowser) {
        delete widget;
    }
    foreach (auto widget, m_ListHlayout) {
        delete widget;
    }
    foreach (auto widget, m_ListDetailButton) {
        delete widget;
    }
    foreach (auto widget, m_ListDetailSeperator) {
        delete widget;
    }

    m_ListTextBrowser.clear();
    m_ListHlayout.clear();
    m_ListDetailButton.clear();
    m_ListDetailSeperator.clear();

    // 删除最后的一个弹簧
    QLayoutItem *layoutItem = mp_ScrollAreaLayout->itemAt(0);
    mp_ScrollAreaLayout->removeItem(layoutItem);
    qDebug() << mp_ScrollAreaLayout->count();
    delete  layoutItem;
}

void PageDetail::slotBtnClicked()
{
    DetailButton *button = qobject_cast<DetailButton *>(sender());
    int index = 0;
    foreach (DetailButton *b, m_ListDetailButton) {
        if (button == b) {
            break;
        }
        index++;
    }

    // 改变按钮的状态，展开和收起的切换
    if (button) {
        button->updateText();
    }

    // 显示内容发生相应变化，也就是是否显示其它信息
    TextBrowser *browser = m_ListTextBrowser[index];
    if (browser) {
        browser->updateShowOtherInfo();
        browser->updateInfo();
    }
}

void PageDetail::slotRefreshInfo()
{
    emit refreshInfo();
}
void PageDetail::slotExportInfo()
{
    emit exportInfo();
}

void PageDetail::slotCopyAllInfo()
{
    QString str;
    foreach (TextBrowser *tb, m_ListTextBrowser) {
        if (tb) {
            str.append(tb->toPlainText());
        }
    }
    QClipboard *clipboard = DApplication::clipboard();
    clipboard->setText(str);
}
