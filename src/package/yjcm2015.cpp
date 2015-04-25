#include "yjcm2015.h"
#include "general.h"
#include "player.h"
#include "structs.h"
#include "room.h"
#include "skill.h"
#include "standard.h"
#include "engine.h"
#include "clientplayer.h"
#include "wrapped-card.h"
#include "roomthread.h"

FurongCard::FurongCard()
{

}

bool FurongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (targets.length() > 0 || to_select == Self)
        return false;
    return Self->inMyAttackRange(to_select);
}

void FurongCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();

    // copy Room::askForCard twice!!
    // anti-programming skill!!
    // wait for the final version.

    const Card *card1 = NULL;
    const Card *card2 = NULL;

    if (card1 == NULL || card2 == NULL)
        return;

    //for future use

    if (card1->isKindOf("Slash") && !card2->isKindOf("Jink"))
        room->damage(DamageStruct(objectName(), effect.from, effect.to));
    else if (!card1->isKindOf("Slash") && card2->isKindOf("Jink")) {
        if (!effect.to->isNude()) {
            int id = room->askForCardChosen(effect.from, effect.to, "he", objectName());
            room->obtainCard(effect.from, id, false);
        }
    }
}

class Furong : public ZeroCardViewAsSkill
{
public:
    Furong() : ZeroCardViewAsSkill("furong")
    {
        
    }

    virtual const Card *viewAs() const
    {
        return new FurongCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("FurongCard");
    }
};

class Shizhi : public TriggerSkill
{
public:
    Shizhi() : TriggerSkill("#shizhi")
    {
        events << HpChanged << MaxHpChanged << EventAcquireSkill << EventLoseSkill << GameStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventAcquireSkill || triggerEvent == EventLoseSkill) {
            if (data.toString() != "shizhi")
                return false;
        }

        if (triggerEvent == HpChanged || triggerEvent == MaxHpChanged || triggerEvent == GameStart) {
            if (!player->hasSkill(this))
                return false;
        }

        bool skillStateBefore = false;
        if (triggerEvent != EventAcquireSkill && triggerEvent != GameStart)
            skillStateBefore = player->getMark("shizhi") > 0;

        bool skillStateAfter = false;
        if (triggerEvent == EventLoseSkill)
            skillStateAfter = true;
        else
            skillStateAfter = player->getHp() == 1;

        if (skillStateAfter != skillStateBefore) 
            room->filterCards(player, player->getCards("he"), true);

        player->setMark("shizhi", skillStateAfter ? 1 : 0);

        return false;
    }
};

class ShizhiFilter : public FilterSkill
{
public:
    ShizhiFilter() : FilterSkill("shizhi")
    {

    }

    virtual bool viewFilter(const Card *to_select) const
    {
        Room *room = Sanguosha->currentRoom();
        ServerPlayer *player = room->getCardOwner(to_select->getId());
        return player != NULL && player->getHp() == 1 && to_select->isKindOf("Jink");
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->setSkillName(objectName());
        WrappedCard *card = Sanguosha->getWrappedCard(originalCard->getId());
        card->takeOver(slash);
        return card;
    }
};

class Qiaoshi : public PhaseChangeSkill
{
public:
    Qiaoshi() : PhaseChangeSkill("qiaoshi")
    {

    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getPhase() == Player::Finish;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const
    {
        Room *room = player->getRoom();
        foreach (ServerPlayer *const &p, room->getOtherPlayers(player)) {
            if (!TriggerSkill::triggerable(p) || p->getHandcardNum() != player->getHandcardNum())
                continue;

            if (p->askForSkillInvoke(objectName(), QVariant::fromValue(player))) {
                QList<ServerPlayer *> l;
                l << p << player;
                room->sortByActionOrder(l);
                room->drawCards(l, 1, objectName());
            }
        }

        return false;
    }
};

YjYanyuCard::YjYanyuCard()
{
    will_throw = false;
    can_recast = true;
    handling_method = Card::MethodRecast;
    target_fixed = true;
}

void YjYanyuCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    ServerPlayer *xiahou = card_use.from;

    CardMoveReason reason(CardMoveReason::S_REASON_RECAST, xiahou->objectName());
    reason.m_skillName = this->getSkillName();
    room->moveCardTo(this, xiahou, NULL, Player::DiscardPile, reason);
    xiahou->broadcastSkillInvoke("@recast");

    int id = card_use.card->getSubcards().first();

    LogMessage log;
    log.type = "#UseCard_Recast";
    log.from = xiahou;
    log.card_str = QString::number(id);
    room->sendLog(log);

    xiahou->drawCards(1, "recast");

    QVariantList yanyuList = xiahou->tag.value("yjyanyu", QVariantList()).toList();
    yanyuList << id;
    xiahou->tag["yjyanyu"] = yanyuList;
}

class YjYanyuVS : public OneCardViewAsSkill
{
public:
    YjYanyuVS() : OneCardViewAsSkill("yjyanyu")
    {
        filter_pattern = "Slash";
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        if (Self->isCardLimited(originalCard, Card::MethodRecast))
            return NULL;

        YjYanyuCard *recast = new YjYanyuCard;
        recast->addSubcard(originalCard);
        return recast;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        Slash *s = new Slash(Card::NoSuit, 0);
        s->deleteLater();
        return !player->isCardLimited(s, Card::MethodRecast);
    }
};

class YjYanyu : public TriggerSkill
{
public:
    YjYanyu() : TriggerSkill("yjyanyu")
    {
        view_as_skill = new YjYanyuVS;
        events << EventPhaseEnd;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getPhase() == Player::Play;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        QVariantList yjyanyuList = player->tag.value("yjyanyu", QVariantList()).toList();
        player->tag.remove("yjyanyu");

        if (yjyanyuList.length() < 3)
            return false;

        QList<int> giveList;
        foreach (const QVariant &v, yjyanyuList) {
            int id = v.toInt();
            if (room->getCardPlace(id) == Player::DiscardPile)
                giveList << id;
        }

        if (giveList.isEmpty())
            return false;

        ServerPlayer *p = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@yjyanyu-give", true);

        if (p != NULL) {
            DummyCard yanyuDummy;
            yanyuDummy.addSubcards(giveList);
            CardMoveReason r(CardMoveReason::S_REASON_GIVE, player->objectName(), p->objectName(), objectName(), QString());
            room->obtainCard(p, &yanyuDummy, r);
        }

        return false;
    }
};



YJCM2015Package::YJCM2015Package()
    : Package("YJCM2015")
{

    General *zhangyi = new General(this, "zhangyi", "shu", 5);
    zhangyi->addSkill(new Furong);
    zhangyi->addSkill(new Shizhi);
    zhangyi->addSkill(new ShizhiFilter);
    related_skills.insertMulti("shizhi", "#shizhi");

    General *liuchen = new General(this, "liuchen", "shu");
    Q_UNUSED(liuchen);
    General *xiahou = new General(this, "yj_xiahoushi", "shu", 3, false);
    xiahou->addSkill(new Qiaoshi);
    xiahou->addSkill(new YjYanyu);

    General *caoxiu = new General(this, "caoxiu", "wei");
    Q_UNUSED(caoxiu);
    General *guofeng = new General(this, "guotufengji", "qun", 3);
    Q_UNUSED(guofeng);
    General *caorui = new General(this, "caorui$", "wei", 3);
    Q_UNUSED(caorui);
    General *zhongyao = new General(this, "zhongyao", "wei", 3);
    Q_UNUSED(zhongyao);
    General *quanzong = new General(this, "quanzong", "wu");
    Q_UNUSED(quanzong);
    General *zhuzhi = new General(this, "zhuzhi", "wu");
    Q_UNUSED(zhuzhi);
    General *sunxiu = new General(this, "sunxiu", "wu", 3);
    Q_UNUSED(sunxiu);
    General *gongsun = new General(this, "gongsunyuan", "qun");
    Q_UNUSED(gongsun);

    addMetaObject<FurongCard>();
    addMetaObject<YjYanyuCard>();
}
ADD_PACKAGE(YJCM2015)
