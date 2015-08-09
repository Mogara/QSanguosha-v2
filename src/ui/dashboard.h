#ifndef _DASHBOARD_H
#define _DASHBOARD_H

//#include "qsan-selectable-item.h"
//#include "qsanbutton.h"
//#include "carditem.h"
#include "player.h"
#include "skin-bank.h"
//#include "skill.h"
#include "protocol.h"
//#include "timed-progressbar.h"
#include "generic-cardcontainer-ui.h"
//#include "pixmapanimation.h"
//#include "sprite.h"
//#include "util.h"

class Card;
class CardItem;
class EffectAnimation;
class ViewAsSkill;
class FilterSkill;
class PixmapAnimation;

class Dashboard : public PlayerCardContainer
{
    Q_OBJECT
    Q_ENUMS(SortType)

public:
    enum SortType
    {
        ByType, BySuit, ByNumber
    };

    Dashboard(QGraphicsPixmapItem *button_widget);
    virtual QRectF boundingRect() const;
    void setWidth(int width);
    int getMiddleWidth();
    inline QRectF getAvatarArea()
    {
        QRectF rect;
        rect.setSize(_dlayout->m_avatarArea.size());
        QPointF topLeft = mapFromItem(_getAvatarParent(), _dlayout->m_avatarArea.topLeft());
        rect.moveTopLeft(topLeft);
        return rect;
    }

    void hideControlButtons();
    void showControlButtons();
    virtual void showProgressBar(QSanProtocol::Countdown countdown);

    QSanSkillButton *removeSkillButton(const QString &skillName);
    QSanSkillButton *addSkillButton(const QString &skillName);
    bool isAvatarUnderMouse();

    void highlightEquip(QString skillName, bool hightlight);

    void setTrust(bool trust);
    virtual void killPlayer();
    virtual void revivePlayer();
    void selectCard(const QString &pattern, bool forward = true, bool multiple = false);
    void selectEquip(int position);
    void selectOnlyCard(bool need_only = false);
    void useSelected();
    const Card *getSelected() const;
    void unselectAll(const CardItem *except = NULL);
    void hideAvatar();

    void disableAllCards();
    void enableCards();
    void enableAllCards();

    void adjustCards(bool playAnimation = true);

    virtual QGraphicsItem *getMouseClickReceiver();

    QList<CardItem *> removeCardItems(const QList<int> &card_ids, Player::Place place);
    virtual QList<CardItem *> cloneCardItems(QList<int> card_ids);

    // pending operations
    void startPending(const ViewAsSkill *skill);
    void stopPending();
    void updatePending();
    void clearPendings();

    inline void addPending(CardItem *item)
    {
        pendings << item;
    }
    inline QList<CardItem *> getPendings() const
    {
        return pendings;
    }
    inline bool hasHandCard(CardItem *item) const
    {
        return m_handCards.contains(item);
    }

    const ViewAsSkill *currentSkill() const;
    const Card *pendingCard() const;

    void expandPileCards(const QString &pile_name);
    void retractPileCards(const QString &pile_name);
    void retractAllSkillPileCards();
    inline QStringList getPileExpanded() const
    {
        return _m_pile_expanded.keys();
    }

    void selectCard(CardItem *item, bool isSelected);

    int getButtonWidgetWidth() const;
    int getTextureWidth() const;

    int width();
    int height();

    virtual void repaintAll();
    int middleFrameAndRightFrameHeightDiff() const {
        return m_middleFrameAndRightFrameHeightDiff;
    }
    void showNullificationButton();
    void hideNullificationButton();

    static const int S_PENDING_OFFSET_Y = -25;

    inline void updateSkillButton()
    {
        if (_m_skillDock)
            _m_skillDock->update();
    }

    inline QRectF getAvatarAreaSceneBoundingRect() const
    {
        return _m_rightFrame->sceneBoundingRect();
    }

public slots:
    virtual void updateAvatar();

    void sortCards();
    void beginSorting();
    void changeShefuState();
    void reverseSelection();
    void cancelNullification();
    void setShefuState();
    void skillButtonActivated();
    void skillButtonDeactivated();
    void selectAll();
    void controlNullificationButton(bool show);

protected:
    void _createExtraButtons();
    virtual void _adjustComponentZValues(bool killed = false);
    virtual void addHandCards(QList<CardItem *> &cards);
    virtual QList<CardItem *> removeHandCards(const QList<int> &cardIds);

    // initialization of _m_layout is compulsory for children classes.
    inline virtual QGraphicsItem *_getEquipParent()
    {
        return _m_leftFrame;
    }
    inline virtual QGraphicsItem *_getDelayedTrickParent()
    {
        return _m_leftFrame;
    }
    inline virtual QGraphicsItem *_getAvatarParent()
    {
        return _m_rightFrame;
    }
    inline virtual QGraphicsItem *_getMarkParent()
    {
        return _m_floatingArea;
    }
    inline virtual QGraphicsItem *_getPhaseParent()
    {
        return _m_floatingArea;
    }
    inline virtual QGraphicsItem *_getRoleComboBoxParent()
    {
        return _m_rightFrame;
    }
    inline virtual QGraphicsItem *_getPileParent()
    {
        return _m_rightFrame;
    }
    inline virtual QGraphicsItem *_getProgressBarParent()
    {
        return _m_floatingArea;
    }
    inline virtual QGraphicsItem *_getFocusFrameParent()
    {
        return _m_rightFrame;
    }
    inline virtual QGraphicsItem *_getDeathIconParent()
    {
        return _m_middleFrame;
    }
    inline virtual QString getResourceKeyName()
    {
        return QSanRoomSkin::S_SKIN_KEY_DASHBOARD;
    }

    bool _addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void _addHandCard(CardItem *card_item, bool prepend = false, const QString &footnote = QString());
    void _adjustCards();
    void _adjustCards(const QList<CardItem *> &list, int y);

    int _m_width;
    // sync objects
    QMutex m_mutex;
    QMutex m_mutexEnableCards;

    QSanButton *m_btnReverseSelection;
    QSanButton *m_btnSortHandcard;
    QSanButton *m_btnNoNullification;
    QSanButton *m_btnShefu;
    QGraphicsPixmapItem *_m_leftFrame, *_m_middleFrame, *_m_rightFrame;
    // we can not draw bg directly _m_rightFrame because then it will always be
    // under avatar (since it's avatar's parent).
    //QGraphicsPixmapItem *_m_rightFrameBg;
    QGraphicsPixmapItem *button_widget;

    CardItem *selected;
    QList<CardItem *> m_handCards;

    QGraphicsPathItem *trusting_item;
    QGraphicsSimpleTextItem *trusting_text;

    QSanInvokeSkillDock* _m_skillDock;
    const QSanRoomSkin::DashboardLayout *_dlayout;

    //for animated effects
    EffectAnimation *animations;

    // for parts creation
    void _createLeft();
    void _createRight();
    void _createMiddle();
    void _updateFrames();

    void _paintLeftFrame();
    void _paintMiddleFrame(const QRect &rect);
    void _paintRightFrame();
    // for pendings
    QList<CardItem *> pendings;
    const Card *pending_card;
    const ViewAsSkill *view_as_skill;
    const FilterSkill *filter;
    QMap<QString, QList<int> > _m_pile_expanded;

    // for equip skill/selections
    PixmapAnimation *_m_equipBorders[S_EQUIP_AREA_LENGTH];
    QSanSkillButton *_m_equipSkillBtns[S_EQUIP_AREA_LENGTH];
    bool _m_isEquipsAnimOn[S_EQUIP_AREA_LENGTH];

    void _createEquipBorderAnimations();
    void _setEquipBorderAnimation(int index, bool turnOn);

    void drawEquip(QPainter *painter, const CardItem *equip, int order);
    void setSelectedItem(CardItem *card_item);

    QMenu *_m_sort_menu;
    QMenu *_m_shefu_menu;

    int m_middleFrameAndRightFrameHeightDiff;
protected slots:
    virtual void _onEquipSelectChanged();

private slots:
    void onCardItemClicked();
    void onCardItemDoubleClicked();
    void onCardItemThrown();
    void onCardItemHover();
    void onCardItemLeaveHover();
    void onMarkChanged();

signals:
    void card_selected(const Card *card);
    void card_to_use();
    void progressBarTimedOut();
};

#endif

