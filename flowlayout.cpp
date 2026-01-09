#include "flowlayout.h"
#include <QWidget>


FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}
//------------------------------------------------------------------------------------------------------------------------------------------

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
    delete item;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void FlowLayout::addItem(QLayoutItem *item)
{
    itemList.append(item);
}
//------------------------------------------------------------------------------------------------------------------------------------------

int FlowLayout::count() const
{
    return itemList.size();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return itemList.value(index);
}
//------------------------------------------------------------------------------------------------------------------------------------------

QLayoutItem *FlowLayout::takeAt(int index)
{
    return index >= 0 && index < itemList.size() ? itemList.takeAt(index) : nullptr;
}
//------------------------------------------------------------------------------------------------------------------------------------------

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) return -1;

    if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout *>(parent)->spacing();
}
//------------------------------------------------------------------------------------------------------------------------------------------

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int x = rect.x();
    int y = rect.y();
    int lineHeight = 0;

    for (QLayoutItem *item : itemList) {
        QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        int spaceY = verticalSpacing();
        QSize itemSize = item->sizeHint();

        if (m_wrap) {
            if (x + itemSize.width() > rect.right() && lineHeight > 0) {
                x = rect.x();
                y += lineHeight + spaceY;
                lineHeight = 0;
            }
        }

        if (!testOnly) {
            item->setGeometry(QRect(QPoint(x, y), itemSize));
        }

        x += itemSize.width() + spaceX;
        lineHeight = qMax(lineHeight, itemSize.height());
    }
    return y + lineHeight - rect.y();
}
//------------------------------------------------------------------------------------------------------------------------------------------

int FlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);

    int x = rect.x();
    int y = rect.y();
    int lineHeight = 0;

    QList<QLayoutItem*> lineItems;

    for (int i = 0; i < itemList.size(); ++i) {
        QLayoutItem *item = itemList.at(i);
        QWidget *wid = item->widget();
        QSize itemSize = item->sizeHint();

        if (x + itemSize.width() > rect.right() && !lineItems.isEmpty()) {
            // переносим на новую строку
            stretchLastInRow(lineItems, rect.right() - x + rect.x());
            lineItems.clear();

            x = rect.x();
            y += lineHeight + spacing();
            lineHeight = 0;
        }

        item->setGeometry(QRect(QPoint(x, y), itemSize));
        x += itemSize.width() + spacing();
        lineHeight = qMax(lineHeight, itemSize.height());
        lineItems << item;
    }

    // последняя строка
    if (!lineItems.isEmpty()) {
        stretchLastInRow(lineItems, rect.right() - x + rect.x());
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void FlowLayout::stretchLastInRow(const QList<QLayoutItem*> &lineItems, int extraWidth)
{
    if (lineItems.isEmpty())
        return;

    QLayoutItem *last = lineItems.last();
    QRect geom = last->geometry();
    geom.setWidth(geom.width() + extraWidth);  // растягиваем на остаток
    last->setGeometry(geom);
}
//------------------------------------------------------------------------------------------------------------------------------------------

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (QLayoutItem *item : itemList)
        size = size.expandedTo(item->minimumSize());

    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    size += QSize(left + right, top + bottom);
    return size;
}
//------------------------------------------------------------------------------------------------------------------------------------------

int FlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0) {
        return m_hSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

int FlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0) {
        return m_vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
