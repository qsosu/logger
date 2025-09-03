#pragma once
#include <QLayout>
#include <QRect>
#include <QStyle>
#include <QWidgetItem>

class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;

    QSize sizeHint() const override;
    QSize minimumSize() const override;

    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int) const override;
    void setWrap(bool wrap) { m_wrap = wrap; }

protected:
    void setGeometry(const QRect &rect) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;
    void stretchLastInRow(const QList<QLayoutItem*> &lineItems, int extraWidth);
    int horizontalSpacing() const;
    int verticalSpacing() const;

    QList<QLayoutItem*> itemList;
    int m_hSpace;
    int m_vSpace;
    bool m_wrap = true;   // по умолчанию перенос включён
};
