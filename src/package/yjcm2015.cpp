#include "yjcm2015.h"
#include "general.h"
#include "player.h"
#include "structs.h"
#include "room.h"
#include "skill.h"
#include "standard.h"
#include "engine.h"
#include "clientplayer.h"
#include "settings.h"


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

    const Card *viewAs() const
    {
        return new FurongCard;
    }

    bool isEnabledAtPlay(const Player *player) const
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

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
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

    bool viewFilter(const Card *to_select) const
    {
        Room *room = Sanguosha->currentRoom();
        ServerPlayer *player = room->getCardOwner(to_select->getId());
        return player != NULL && player->getHp() == 1 && to_select->isKindOf("Jink");
    }

    const Card *viewAs(const Card *originalCard) const
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

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getPhase() == Player::Finish;
    }

    bool onPhaseChange(ServerPlayer *player) const
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

    const Card *viewAs(const Card *originalCard) const
    {
        if (Self->isCardLimited(originalCard, Card::MethodRecast))
            return NULL;

        YjYanyuCard *recast = new YjYanyuCard;
        recast->addSubcard(originalCard);
        return recast;
    }

    bool isEnabledAtPlay(const Player *player) const
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

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getPhase() == Player::Play;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
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

JigongCard::JigongCard()
{
    target_fixed = true;
}

void JigongCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    if (source->isAlive()) {
        source->drawCards(2, "jigong");
        source->setFlags("jigong");
    }
}

class Jigong : public ZeroCardViewAsSkill
{
public:
    Jigong() : ZeroCardViewAsSkill("jigong")
    {

    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("JigongCard");
    }

    const Card *viewAs() const
    {
        return new JigongCard();
    }
};

class JigongMax : public MaxCardsSkill
{
public:
    JigongMax() : MaxCardsSkill("#jigong")
    {

    }

    int getFixed(const Player *target) const
    {
        if (target->hasFlag("jigong"))
            return target->getMark("damage_point_play_phase");
        
        return -1;
    }
};

HuomoDialog::HuomoDialog() : GuhuoDialog("huomo", true, false)
{

}

HuomoDialog *HuomoDialog::getInstance()
{
    static HuomoDialog *instance;
    if (instance == NULL || instance->objectName() != "huomo")
        instance = new HuomoDialog;

    return instance;
}

bool HuomoDialog::isButtonEnabled(const QString &button_name) const
{
    const Card *c = map[button_name];
    QString classname = c->getClassName();

    bool r = Self->getMark("Huomo_" + classname) == 0;
    if (!r)
        return false;

    return GuhuoDialog::isButtonEnabled(button_name);
}

HuomoCard::HuomoCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool HuomoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
    }

    const Card *_card = Self->tag.value("huomo").value<const Card *>();
    if (_card == NULL)
        return false;

    Card *card = Sanguosha->cloneCard(_card->objectName(), Card::NoSuit, 0);
    card->setCanRecast(false);
    card->deleteLater();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool HuomoCard::targetFixed() const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFixed();
    }

    const Card *_card = Self->tag.value("huomo").value<const Card *>();
    if (_card == NULL)
        return false;

    Card *card = Sanguosha->cloneCard(_card->objectName(), Card::NoSuit, 0);
    card->setCanRecast(false);
    card->deleteLater();
    return card && card->targetFixed();
}

bool HuomoCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetsFeasible(targets, Self);
    }

    const Card *_card = Self->tag.value("huomo").value<const Card *>();
    if (_card == NULL)
        return false;

    Card *card = Sanguosha->cloneCard(_card->objectName(), Card::NoSuit, 0);
    card->setCanRecast(false);
    card->deleteLater();
    return card && card->targetsFeasible(targets, Self);
}

const Card *HuomoCard::validate(CardUseStruct &card_use) const
{
    ServerPlayer *zhongyao = card_use.from;
    Room *room = zhongyao->getRoom();

    QString to_guhuo = user_string;
    if (user_string == "slash" && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list = QStringList() << "normal_slash" << "thunder_slash" << "fire_slash";
        to_guhuo = room->askForChoice(zhongyao, "huomo_slash", guhuo_list.join("+"));
        zhongyao->tag["HuomoSlash"] = QVariant(to_guhuo);
    }

    room->moveCardTo(this, NULL, Player::DrawPile, true);

    QString user_str;
    if (to_guhuo == "normal_slash")
        user_str = "slash";
    else
        user_str = to_guhuo;

    Card *c = Sanguosha->cloneCard(user_str, Card::NoSuit, 0);

    QString classname;
    if (c->isKindOf("Slash"))
        classname = "Slash";
    else
        classname = c->getClassName();

    room->setPlayerMark(zhongyao, "Huomo_" + classname, 1);

    QStringList huomoList = zhongyao->tag.value("huomoClassName").toStringList();
    huomoList << classname;
    zhongyao->tag["huomoClassName"] = huomoList;

    c->setSkillName("huomo");
    c->deleteLater();
    return c;
}

const Card *HuomoCard::validateInResponse(ServerPlayer *zhongyao) const
{
    Room *room = zhongyao->getRoom();

    QString to_guhuo = user_string;
    if (user_string == "peach+analeptic") {
        bool can_use_peach = zhongyao->getMark("Huomo_Peach") == 0;
        bool can_use_analeptic = zhongyao->getMark("Huomo_Analeptic") == 0;
        QStringList guhuo_list;
        if (can_use_peach)
            guhuo_list << "peach";
        if (can_use_analeptic && !Config.BanPackages.contains("maneuvering"))
            guhuo_list << "analeptic";
        to_guhuo = room->askForChoice(zhongyao, "huomo_saveself", guhuo_list.join("+"));
        zhongyao->tag["HuomoSaveSelf"] = QVariant(to_guhuo);
    } else if (user_string == "slash") {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list = QStringList() << "normal_slash" << "thunder_slash" << "fire_slash";
        to_guhuo = room->askForChoice(zhongyao, "huomo_slash", guhuo_list.join("+"));
        zhongyao->tag["HuomoSlash"] = QVariant(to_guhuo);
    } else
        to_guhuo = user_string;

    room->moveCardTo(this, NULL, Player::DrawPile, true);

    QString user_str;
    if (to_guhuo == "normal_slash")
        user_str = "slash";
    else
        user_str = to_guhuo;

    Card *c = Sanguosha->cloneCard(user_str, Card::NoSuit, 0);

    QString classname;
    if (c->isKindOf("Slash"))
        classname = "Slash";
    else
        classname = c->getClassName();

    room->setPlayerMark(zhongyao, "Huomo_" + classname, 1);

    QStringList huomoList = zhongyao->tag.value("huomoClassName").toStringList();
    huomoList << classname;
    zhongyao->tag["huomoClassName"] = huomoList;

    c->setSkillName("huomo");
    c->deleteLater();
    return c;

}

class HuomoVS : public OneCardViewAsSkill
{
public:
    HuomoVS() : OneCardViewAsSkill("huomo")
    {
        filter_pattern = "^BasicCard|black";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        QString pattern;
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            const Card *c = Self->tag["huomo"].value<const Card *>();
            if (c == NULL || Self->getMark("Huomo_" + (c->isKindOf("Slash") ? "Slash" : c->getClassName())) > 0)
                return NULL;

            pattern = c->objectName();
        } else {
            pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
            if (pattern == "peach+analeptic" && Self->getMark("Global_PreventPeach") > 0)
                pattern = "analeptic";

            // check if it can use
            bool can_use = false;
            QStringList p = pattern.split("+");
            foreach (const QString &x, p) {
                const Card *c = Sanguosha->cloneCard(x);
                QString us = c->getClassName();
                if (c->isKindOf("Slash"))
                    us = "Slash";

                if (Self->getMark("Huomo_" + us) == 0)
                    can_use = true;

                delete c;
                if (can_use)
                    break;
            }

            if (!can_use)
                return NULL;
        }

        HuomoCard *hm = new HuomoCard;
        hm->setUserString(pattern);
        hm->addSubcard(originalCard);

        return hm;
        
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        QList<const Player *> sib = player->getAliveSiblings();
        if (player->isAlive())
            sib << player;

        bool noround = true;

        foreach (const Player *p, sib) {
            if (p->getPhase() != Player::NotActive) {
                noround = false;
                break;
            }
        }

        return true; // for DIY!!!!!!!
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        QList<const Player *> sib = player->getAliveSiblings();
        if (player->isAlive())
            sib << player;

        bool noround = true;

        foreach (const Player *p, sib) {
            if (p->getPhase() != Player::NotActive) {
                noround = false;
                break;
            }
        }

        if (noround)
            return false;

        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_RESPONSE_USE)
            return false;

#define HUOMO_CAN_USE(x) (player->getMark("Huomo_" #x) == 0)

        if (pattern == "slash")
            return HUOMO_CAN_USE(Slash);
        else if (pattern == "peach")
            return HUOMO_CAN_USE(Peach) && player->getMark("Global_PreventPeach") == 0;
        else if (pattern.contains("analeptic"))
            return HUOMO_CAN_USE(Peach) || HUOMO_CAN_USE(Analeptic);
        else if (pattern == "jink")
            return HUOMO_CAN_USE(Jink);

#undef HUOMO_CAN_USE

        return false;
    }
};

class Huomo : public TriggerSkill
{
public:
    Huomo() : TriggerSkill("huomo")
    {
        view_as_skill = new HuomoVS;
        events << EventPhaseChanging;
    }

    QDialog *getDialog() const
    {
        return HuomoDialog::getInstance();
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;

        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            QStringList sl = p->tag.value("huomoClassName").toStringList();
            foreach (const QString &t, sl)
                room->setPlayerMark(p, "Huomo_" + t, 0);
            
            p->tag["huomoClassName"] = QStringList();
        }

        return false;
    }
};

class Zuoding : public TriggerSkill
{
public:
    Zuoding() : TriggerSkill("zuoding")
    {
        events << TargetSpecified;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getPhase() == Player::Play;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (room->getTag("zuoding").toBool())
            return false;

        CardUseStruct use = data.value<CardUseStruct>();
        if (!(use.card != NULL && use.card->getSuit() == Card::Spade && !use.to.isEmpty()))
            return false;

        foreach (ServerPlayer *zhongyao, room->getAllPlayers()) {
            if (TriggerSkill::triggerable(zhongyao) && player != zhongyao) {
                ServerPlayer *p = room->askForPlayerChosen(zhongyao, use.to, "zuoding", "@zuoding", true, true);
                if (p != NULL)
                    p->drawCards(1, "zuoding");
            }
        }
        
        return false;
    }
};

class ZuodingRecord : public TriggerSkill
{
public:
    ZuodingRecord() : TriggerSkill("#zuoding")
    {
        events << DamageDone << EventPhaseChanging;
        global = true;
    }

    int getPriority(TriggerEvent) const
    {
        return 0;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.from == Player::Play)
                room->setTag("zuoding", false);
        } else {
            bool playphase = false;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->getPhase() == Player::Play) {
                    playphase = true;
                    break;
                }
            }
            if (!playphase)
                return false;

            room->setTag("zuoding", true);
        }

        return false;
    }
};


YJCM2015Package::YJCM2015Package()
    : Package("YJCM2015")
{

    General *zhangyi = new General(this, "zhangyi", "shu", 5, true, true, true);
    zhangyi->addSkill(new Furong);
    zhangyi->addSkill(new Shizhi);
    zhangyi->addSkill(new ShizhiFilter);
    related_skills.insertMulti("shizhi", "#shizhi");

    General *liuchen = new General(this, "liuchen", "shu", true, true, true);
    Q_UNUSED(liuchen);
    General *xiahou = new General(this, "yj_xiahoushi", "shu", 3, false);
    xiahou->addSkill(new Qiaoshi);
    xiahou->addSkill(new YjYanyu);

    General *caoxiu = new General(this, "caoxiu", "wei", true, true, true);
    Q_UNUSED(caoxiu);
    General *guofeng = new General(this, "guotufengji", "qun", 3, true, true, true);
    Q_UNUSED(guofeng);
    General *caorui = new General(this, "caorui$", "wei", 3, true, true, true);
    Q_UNUSED(caorui);

    General *zhongyao = new General(this, "zhongyao", "wei", 3);
    zhongyao->addSkill(new Huomo);
    zhongyao->addSkill(new Zuoding);
    zhongyao->addSkill(new ZuodingRecord);
    related_skills.insertMulti("zuoding", "#zuoding");

    General *quanzong = new General(this, "quanzong", "wu", true, true, true);
    Q_UNUSED(quanzong);
    General *zhuzhi = new General(this, "zhuzhi", "wu");
    Q_UNUSED(zhuzhi);
    General *sunxiu = new General(this, "sunxiu", "wu", 3, true, true, true);
    Q_UNUSED(sunxiu);
    General *gongsun = new General(this, "gongsunyuan", "qun");
    Q_UNUSED(gongsun);

    addMetaObject<FurongCard>();
    addMetaObject<YjYanyuCard>();
    addMetaObject<HuomoCard>();
}
ADD_PACKAGE(YJCM2015)