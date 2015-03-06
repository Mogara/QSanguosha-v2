#ifndef _BUBBLE_CHAT_BOX_H
#define _BUBBLE_CHAT_BOX_H

#include <QGraphicsObject>
#include <QTimer>
#include <QTextOption>
#include <QTextDocument>
#include <QGraphicsTextItem>

class QPropertyAnimation;

class BubbleChatBox : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit BubbleChatBox(const QRect &area, QGraphicsItem *parent = 0);
    ~BubbleChatBox();

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QPainterPath shape() const;

    void setText(const QString &text);
    void setArea(const QRect &newArea);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    class BubbleChatLabel : public QGraphicsTextItem
    {
    public:
        explicit BubbleChatLabel(QGraphicsItem *parent = 0);
        virtual QRectF boundingRect() const;
        void setBoundingRect(const QRectF &newRect);
        void setAlignment(Qt::Alignment alignment);
        void setWrapMode(QTextOption::WrapMode wrap);

    private:
        QRectF rect;
        QTextDocument *doc;

    };

    void updatePos();

    QPixmap backgroundPixmap;
    QRectF rect;
    QRect area;
    QTimer timer;
    BubbleChatLabel *chatLabel;

    QPropertyAnimation *appearAndDisappear;

private slots:
    void clear();
};

#endif // _BUBBLE_CHAT_BOX_H
