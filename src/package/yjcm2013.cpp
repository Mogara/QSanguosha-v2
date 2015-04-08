#include "yjcm2013.h"
#include "settings.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "maneuvering.h"

Chengxiang::Chengxiang() : MasochismSkill("chengxiang")
{
    frequency = Frequent;
    total_point = 13;
}

void Chengxiang::onDamaged(ServerPlayer *target, const DamageStruct &damage) const
{
    Room *room = target->getRoom();
    if (!target->askForSkillInvoke(this, QVariant::fromValue(damage))) return;
    room->broadcastSkillInvoke("chengxiang");

    QList<int> card_ids = room->getNCards(4);
    room->fillAG(card_ids);

    QList<int> to_get, to_throw;
    while (true) {
        int sum = 0;
        foreach(int id, to_get)
            sum += Sanguosha->getCard(id)->getNumber();
        foreach (int id, card_ids) {
            if (sum + Sanguosha->getCard(id)->getNumber() > total_point) {
                room->takeAG(NULL, id, false);
                card_ids.removeOne(id);
                to_throw << id;
            }
        }
        if (card_ids.isEmpty()) break;

        int card_id = room->askForAG(target, card_ids, card_ids.length() < 4, objectName());
        if (card_id == -1) break;
        card_ids.removeOne(card_id);
        to_get << card_id;
        room->takeAG(target, card_id, false);
        if (card_ids.isEmpty()) break;
    }
    DummyCard *dummy = new DummyCard;
    if (!to_get.isEmpty()) {
        dummy->addSubcards(to_get);
        target->obtainCard(dummy);
    }
    dummy->clearSubcards();
    if (!to_throw.isEmpty() || !card_ids.isEmpty()) {
        dummy->addSubcards(to_throw + card_ids);
        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, target->objectName(), objectName(), QString());
        room->throwCard(dummy, reason, NULL);
    }
    delete dummy;

    room->clearAG();
}

class Renxin : public TriggerSkill
{
public:
    Renxin() : TriggerSkill("renxin")
    {
        events << DamageInflicted << ChoiceMade;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == DamageInflicted) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.to->getHp() == 1) {
                foreach (ServerPlayer *p, room->getOtherPlayers(damage.to)) {
                    if (TriggerSkill::triggerable(p) && p->canDiscard(p, "he")) {
                        if (room->askForCard(p, ".Equip", "@renxin-card:" + damage.to->objectName(), data, objectName())) {
                            room->broadcastSkillInvoke(objectName());
                            LogMessage log;
                            log.type = "#Renxin";
                            log.from = damage.to;
                            log.arg = objectName();
                            room->sendLog(log);
                            return true;
                        }
                    }
                }
            }
        } else if (triggerEvent == ChoiceMade) {
            QStringList data_list = data.toString().split(":");
            if (data_list.length() > 3 && data_list.at(2) == "@renxin-card" && data_list.last() != "_nil_")
                player->turnOver();
        }
        return false;
    }
};

class Jingce : public TriggerSkill
{
public:
    Jingce() : TriggerSkill("jingce")
    {
        events << EventPhaseEnd;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (player->getPhase() == Player::Play && player->getMark(objectName()) >= player->getHp()) {
            if (room->askForSkillInvoke(player, objectName())) {
                room->broadcastSkillInvoke(objectName());
                player->drawCards(2, objectName());
            }
        }
        return false;
    }
};

class JingceRecord : public TriggerSkill
{
public:
    JingceRecord() : TriggerSkill("#jingce-record")
    {
        events << PreCardUsed << CardResponded << EventPhaseStart;
        global = true;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data) const
    {
        if ((triggerEvent == PreCardUsed || triggerEvent == CardResponded) && player->getPhase() <= Player::Play) {
            const Card *card = NULL;
            if (triggerEvent == PreCardUsed)
                card = data.value<CardUseStruct>().card;
            else {
                CardResponseStruct response = data.value<CardResponseStruct>();
                if (response.m_isUse)
                    card = response.m_card;
            }
            if (card && card->getTypeId() != Card::TypeSkill)
                player->addMark("jingce");
        } else if (triggerEvent == EventPhaseStart && player->getPhase() == Player::RoundStart) {
            player->setMark("jingce", 0);
        }
        return false;
    }
};

JunxingCard::JunxingCard()
{
}

void JunxingCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.first();
    if (!target->isAlive()) return;

    QString type_name[4] = { QString(), "BasicCard", "TrickCard", "EquipCard" };
    QStringList types;
    types << "BasicCard" << "TrickCard" << "EquipCard";
    foreach (int id, subcards) {
        const Card *c = Sanguosha->getCard(id);
        types.removeOne(type_name[c->getTypeId()]);
        if (types.isEmpty()) break;
    }
    if (!target->canDiscard(target, "h") || types.isEmpty()
        || !room->askForCard(target, types.join(",") + "|.|.|hand", "@junxing-discard")) {
        target->turnOver();
        target->drawCards(subcards.length(), "junxing");
    }
}

class Junxing : public ViewAsSkill
{
public:
    Junxing() : ViewAsSkill("junxing")
    {
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        return !to_select->isEquipped() && !Self->isJilei(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.isEmpty())
            return NULL;

        JunxingCard *card = new JunxingCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->canDiscard(player, "h") && !player->hasUsed("JunxingCard");
    }
};

class Yuce : public MasochismSkill
{
public:
    Yuce() : MasochismSkill("yuce")
    {
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const
    {
        if (target->isKongcheng()) return;

        Room *room = target->getRoom();
        QVariant data = QVariant::fromValue(damage);
        const Card *card = room->askForCard(target, ".", "@yuce-show", data, Card::MethodNone);
        if (card) {
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(target, objectName());
            LogMessage log;
            log.from = target;
            log.type = "#InvokeSkill";
            log.arg = objectName();
            room->sendLog(log);

            room->showCard(target, card->getEffectiveId());
            if (!damage.from || damage.from->isDead()) return;

            QString type_name[4] = { QString(), "BasicCard", "TrickCard", "EquipCard" };
            QStringList types;
            types << "BasicCard" << "TrickCard" << "EquipCard";
            types.removeOne(type_name[card->getTypeId()]);
            if (!damage.from->canDiscard(damage.from, "h")
                || !room->askForCard(damage.from, types.join(",") + "|.|.|hand",
                QString("@yuce-discard:%1::%2:%3")
                .arg(target->objectName())
                .arg(types.first()).arg(types.last()),
                data)) {
                room->recover(target, RecoverStruct(target));
            }
        }
    }
};

class Longyin : public TriggerSkill
{
public:
    Longyin() : TriggerSkill("longyin")
    {
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target->getPhase() == Player::Play;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash")) {
            ServerPlayer *guanping = room->findPlayerBySkillName(objectName());
            if (guanping && guanping->canDiscard(guanping, "he")
                && room->askForCard(guanping, "..", "@longyin", data, objectName())) {
                room->broadcastSkillInvoke(objectName(), use.card->isRed() ? 2 : 1);
                if (use.m_addHistory) {
                    room->addPlayerHistory(player, use.card->getClassName(), -1);
                    use.m_addHistory = false;
                    data = QVariant::fromValue(use);
                }
                if (use.card->isRed())
                    guanping->drawCards(1, objectName());
            }
        }
        return false;
    }
};

ExtraCollateralCard::ExtraCollateralCard()
{
}

bool ExtraCollateralCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    const Card *coll = Card::Parse(Self->property("extra_collateral").toString());
    if (!coll) return false;
    QStringList tos = Self->property("extra_collateral_current_targets").toString().split("+");

    if (targets.isEmpty())
        return !tos.contains(to_select->objectName())
        && !Self->isProhibited(to_select, coll) && coll->targetFilter(targets, to_select, Self);
    else
        return coll->targetFilter(targets, to_select, Self);
}

void ExtraCollateralCard::onUse(Room *, const CardUseStruct &card_use) const
{
    Q_ASSERT(card_use.to.length() == 2);
    ServerPlayer *killer = card_use.to.first();
    ServerPlayer *victim = card_use.to.last();

    killer->setFlags("ExtraCollateralTarget");
    killer->tag["collateralVictim"] = QVariant::fromValue(victim);
}

QiaoshuiCard::QiaoshuiCard()
{
}

bool QiaoshuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void QiaoshuiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    bool success = source->pindian(targets.first(), "qiaoshui", NULL);
    if (success)
        source->setFlags("QiaoshuiSuccess");
    else
        room->setPlayerCardLimitation(source, "use", "TrickCard", true);
}

class QiaoshuiViewAsSkill : public ZeroCardViewAsSkill
{
public:
    QiaoshuiViewAsSkill() : ZeroCardViewAsSkill("qiaoshui")
    {
    }

    virtual bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern.startsWith("@@qiaoshui");
    }

    virtual const Card *viewAs() const
    {
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern.endsWith("!"))
            return new ExtraCollateralCard;
        else
            return new QiaoshuiCard;
    }
};

class Qiaoshui : public PhaseChangeSkill
{
public:
    Qiaoshui() : PhaseChangeSkill("qiaoshui")
    {
        view_as_skill = new QiaoshuiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *jianyong) const
    {
        if (jianyong->getPhase() == Player::Play && !jianyong->isKongcheng()) {
            Room *room = jianyong->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(jianyong);
            foreach (ServerPlayer *player, other_players) {
                if (!player->isKongcheng()) {
                    can_invoke = true;
                    break;
                }
            }

            if (can_invoke)
                room->askForUseCard(jianyong, "@@qiaoshui", "@qiaoshui-card", 1);
        }

        return false;
    }
};

class QiaoshuiUse : public TriggerSkill
{
public:
    QiaoshuiUse() : TriggerSkill("#qiaoshui-use")
    {
        events << PreCardUsed;
        view_as_skill = new QiaoshuiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *jianyong, QVariant &data) const
    {
        if (!jianyong->hasFlag("QiaoshuiSuccess")) return false;

        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isNDTrick() || use.card->isKindOf("BasicCard")) {
            jianyong->setFlags("-QiaoshuiSuccess");
            if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY)
                return false;

            QList<ServerPlayer *> available_targets;
            if (!use.card->isKindOf("AOE") && !use.card->isKindOf("GlobalEffect")) {
                room->setPlayerFlag(jianyong, "QiaoshuiExtraTarget");
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (use.to.contains(p) || room->isProhibited(jianyong, p, use.card)) continue;
                    if (use.card->targetFixed()) {
                        if (!use.card->isKindOf("Peach") || p->isWounded())
                            available_targets << p;
                    } else {
                        if (use.card->targetFilter(QList<const Player *>(), p, jianyong))
                            available_targets << p;
                    }
                }
                room->setPlayerFlag(jianyong, "-QiaoshuiExtraTarget");
            }
            QStringList choices;
            choices << "cancel";
            if (use.to.length() > 1) choices.prepend("remove");
            if (!available_targets.isEmpty()) choices.prepend("add");
            if (choices.length() == 1) return false;

            QString choice = room->askForChoice(jianyong, "qiaoshui", choices.join("+"), data);
            if (choice == "cancel")
                return false;
            else if (choice == "add") {
                ServerPlayer *extra = NULL;
                if (!use.card->isKindOf("Collateral"))
                    extra = room->askForPlayerChosen(jianyong, available_targets, "qiaoshui", "@qiaoshui-add:::" + use.card->objectName());
                else {
                    QStringList tos;
                    foreach(ServerPlayer *t, use.to)
                        tos.append(t->objectName());
                    room->setPlayerProperty(jianyong, "extra_collateral", use.card->toString());
                    room->setPlayerProperty(jianyong, "extra_collateral_current_targets", tos.join("+"));
                    room->askForUseCard(jianyong, "@@qiaoshui!", "@qiaoshui-add:::collateral");
                    room->setPlayerProperty(jianyong, "extra_collateral", QString());
                    room->setPlayerProperty(jianyong, "extra_collateral_current_targets", QString("+"));
                    foreach (ServerPlayer *p, room->getOtherPlayers(jianyong)) {
                        if (p->hasFlag("ExtraCollateralTarget")) {
                            p->setFlags("-ExtraCollateralTarget");
                            extra = p;
                            break;
                        }
                    }
                    if (extra == NULL) {
                        extra = available_targets.at(qrand() % available_targets.length() - 1);
                        QList<ServerPlayer *> victims;
                        foreach (ServerPlayer *p, room->getOtherPlayers(extra)) {
                            if (extra->canSlash(p)
                                && (!(p == jianyong && p->hasSkill("kongcheng") && p->isLastHandCard(use.card, true)))) {
                                victims << p;
                            }
                        }
                        Q_ASSERT(!victims.isEmpty());
                        extra->tag["collateralVictim"] = QVariant::fromValue((victims.at(qrand() % victims.length() - 1)));
                    }
                }
                use.to.append(extra);
                room->sortByActionOrder(use.to);

                LogMessage log;
                log.type = "#QiaoshuiAdd";
                log.from = jianyong;
                log.to << extra;
                log.card_str = use.card->toString();
                log.arg = "qiaoshui";
                room->sendLog(log);
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, jianyong->objectName(), extra->objectName());

                if (use.card->isKindOf("Collateral")) {
                    ServerPlayer *victim = extra->tag["collateralVictim"].value<ServerPlayer *>();
                    if (victim) {
                        LogMessage log;
                        log.type = "#CollateralSlash";
                        log.from = jianyong;
                        log.to << victim;
                        room->sendLog(log);
                        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, extra->objectName(), victim->objectName());
                    }
                }
            } else {
                ServerPlayer *removed = room->askForPlayerChosen(jianyong, use.to, "qiaoshui", "@qiaoshui-remove:::" + use.card->objectName());
                use.to.removeOne(removed);

                LogMessage log;
                log.type = "#QiaoshuiRemove";
                log.from = jianyong;
                log.to << removed;
                log.card_str = use.card->toString();
                log.arg = "qiaoshui";
                room->sendLog(log);
            }
        }
        data = QVariant::fromValue(use);

        return false;
    }
};

class QiaoshuiTargetMod : public TargetModSkill
{
public:
    QiaoshuiTargetMod() : TargetModSkill("#qiaoshui-target")
    {
        frequency = NotFrequent;
        pattern = "Slash,TrickCard+^DelayedTrick";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const
    {
        if (from->hasFlag("QiaoshuiExtraTarget"))
            return 1000;
        else
            return 0;
    }
};

class Zongshih : public TriggerSkill
{
public:
    Zongshih() : TriggerSkill("zongshih")
    {
        events << Pindian;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        PindianStruct *pindian = data.value<PindianStruct *>();
        const Card *to_obtain = NULL;
        ServerPlayer *jianyong = NULL;
        if (TriggerSkill::triggerable(pindian->from)) {
            jianyong = pindian->from;
            if (pindian->from_number > pindian->to_number)
                to_obtain = pindian->to_card;
            else
                to_obtain = pindian->from_card;
        } else if (TriggerSkill::triggerable(pindian->to)) {
            jianyong = pindian->to;
            if (pindian->from_number < pindian->to_number)
                to_obtain = pindian->from_card;
            else
                to_obtain = pindian->to_card;
        }
        if (jianyong && to_obtain && room->getCardPlace(to_obtain->getEffectiveId()) == Player::PlaceTable
            && room->askForSkillInvoke(jianyong, objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            jianyong->obtainCard(to_obtain);
        }

        return false;
    }
};

XiansiCard::XiansiCard()
{
}

bool XiansiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    return targets.length() < 2 && !to_select->isNude();
}

void XiansiCard::onEffect(const CardEffectStruct &effect) const
{
    if (effect.to->isNude()) return;
    int id = effect.from->getRoom()->askForCardChosen(effect.from, effect.to, "he", "xiansi");
    effect.from->addToPile("counter", id);
}

class XiansiViewAsSkill : public ZeroCardViewAsSkill
{
public:
    XiansiViewAsSkill() : ZeroCardViewAsSkill("xiansi")
    {
        response_pattern = "@@xiansi";
    }

    virtual const Card *viewAs() const
    {
        return new XiansiCard;
    }
};

class Xiansi : public TriggerSkill
{
public:
    Xiansi() : TriggerSkill("xiansi")
    {
        events << EventPhaseStart;
        view_as_skill = new XiansiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (player->getPhase() == Player::Start)
            room->askForUseCard(player, "@@xiansi", "@xiansi-card");
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const
    {
        int index = qrand() % 2 + 1;
        if (card->isKindOf("Slash"))
            index += 2;
        return index;
    }
};

class XiansiAttach : public TriggerSkill
{
public:
    XiansiAttach() : TriggerSkill("#xiansi-attach")
    {
        events << GameStart << EventAcquireSkill << EventLoseSkill << Debut;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if ((triggerEvent == GameStart && TriggerSkill::triggerable(player))
            || (triggerEvent == EventAcquireSkill && data.toString() == "xiansi")) {
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (!p->hasSkill("xiansi_slash"))
                    room->attachSkillToPlayer(p, "xiansi_slash");
            }
        } else if (triggerEvent == EventLoseSkill && data.toString() == "xiansi") {
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->hasSkill("xiansi_slash"))
                    room->detachSkillFromPlayer(p, "xiansi_slash", true);
            }
        } else if (triggerEvent == Debut) {
            QList<ServerPlayer *> liufengs = room->findPlayersBySkillName("xiansi");
            foreach (ServerPlayer *liufeng, liufengs) {
                if (player != liufeng && !player->hasSkill("xiansi_attach")) {
                    room->attachSkillToPlayer(player, "xiansi_attach");
                    break;
                }
            }
        }
        return false;
    }
};

XiansiSlashCard::XiansiSlashCard()
{
    m_skillName = "xiansi_slash";
}

bool XiansiSlashCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    const Player *liufeng = NULL;
    foreach (const Player *p, targets) {
        if (p->hasSkill("xiansi")) {
            liufeng = p;
            break;
        }
    }

    if (liufeng == NULL)
        return false;

    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->addSpecificAssignee(liufeng);
    bool feasible = slash->targetsFeasible(targets, Self);
    delete slash;
    return feasible;
}

bool XiansiSlashCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    Slash *slash = new Slash(Card::NoSuit, 0);
    if (targets.isEmpty()) {
        bool filter = to_select->hasSkill("xiansi") && to_select->getPile("counter").length() >= 2
            && slash->targetFilter(QList<const Player *>(), to_select, Self);
        delete slash;
        return filter;
    } else {
        slash->addSpecificAssignee(targets.first());
        bool filter = slash->targetFilter(targets, to_select, Self);
        delete slash;
        return filter;
    }
    return false;
}

const Card *XiansiSlashCard::validate(CardUseStruct &cardUse) const
{
    Room *room = cardUse.from->getRoom();
    /*
        if (cardUse.to.isEmpty()) {
        QList<ServerPlayer *> liufengs = room->findPlayersBySkillName("xiansi");
        foreach (ServerPlayer *liufeng, liufengs) {
        if (liufeng->getPile("counter").length() < 2) continue;
        if (cardUse.from->canSlash(liufeng)) {
        cardUse.to << liufeng;
        break;
        }
        }
        if (cardUse.to.isEmpty())
        return NULL;
        }
        */
    /*
        ServerPlayer *liufeng = cardUse.to.first();
        if (liufeng->getPile("counter").length() < 2) return NULL;
        */

    ServerPlayer *source = cardUse.from;

    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "xiansi", QString());
    room->throwCard(this, reason, NULL);

    Slash *slash = new Slash(Card::SuitToBeDecided, -1);
    slash->setSkillName("_xiansi");

    QList<ServerPlayer *> targets = cardUse.to;
    foreach (ServerPlayer *target, targets) {
        if (!source->canSlash(target, slash))
            cardUse.to.removeOne(target);
    }
    if (cardUse.to.length() > 0)
        return slash;
    else {
        delete slash;
        return NULL;
    }
}

class XiansiSlashViewAsSkill : public ViewAsSkill
{
public:
    XiansiSlashViewAsSkill() : ViewAsSkill("xiansi_slash")
    {
        attached_lord_skill = true;
        expand_pile = "%counter";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return Slash::IsAvailable(player) && canSlashLiufeng(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        return pattern == "slash"
            && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE
            && canSlashLiufeng(player);
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (selected.length() >= 2)
            return false;

        foreach (const Player *p, Self->getAliveSiblings()) {
            if (p->hasSkill("xiansi") && p->getPile("counter").length() > 1) {
                return p->getPile("counter").contains(to_select->getId());
            }
        }

        return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() == 2) {
            XiansiSlashCard *xs = new XiansiSlashCard;
            xs->addSubcards(cards);
            return xs;
        }

        return NULL;
    }

private:
    static bool canSlashLiufeng(const Player *player)
    {
        Slash *slash = new Slash(Card::SuitToBeDecided, -1);
        foreach (const Player *p, player->getAliveSiblings()) {
            if (p->hasSkill("xiansi") && p->getPile("counter").length() > 1) {
                if (slash->targetFilter(QList<const Player *>(), p, player)) {
                    delete slash;
                    return true;
                }
            }
        }
        delete slash;
        return false;
    }
};

class Duodao : public MasochismSkill
{
public:
    Duodao() : MasochismSkill("duodao")
    {
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const
    {
        if (!damage.card || !damage.card->isKindOf("Slash") || !target->canDiscard(target, "he"))
            return;
        QVariant data = QVariant::fromValue(damage);
        Room *room = target->getRoom();
        if (room->askForCard(target, "..", "@duodao-get", data, objectName())) {
            if (damage.from && damage.from->getWeapon()) {
                room->broadcastSkillInvoke(objectName());
                target->obtainCard(damage.from->getWeapon());
            }
        }
    }
};

class Anjian : public TriggerSkill
{
public:
    Anjian() : TriggerSkill("anjian")
    {
        events << DamageCaused;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.chain || damage.transfer || !damage.by_user) return false;
        if (damage.from && !damage.to->inMyAttackRange(damage.from)
            && damage.card && damage.card->isKindOf("Slash")) {
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(damage.from, objectName());

            LogMessage log;
            log.type = "#AnjianBuff";
            log.from = damage.from;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

ZongxuanCard::ZongxuanCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
    target_fixed = true;
}

void ZongxuanCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    QVariantList subcardsList;
    foreach(int id, subcards)
        subcardsList << id;
    source->tag["zongxuan"] = QVariant::fromValue(subcardsList);
}

class ZongxuanViewAsSkill : public ViewAsSkill
{
public:
    ZongxuanViewAsSkill() : ViewAsSkill("zongxuan")
    {
        response_pattern = "@@zongxuan";
    }

    bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        QStringList zongxuan = Self->property("zongxuan").toString().split("+");
        foreach (QString id, zongxuan) {
            bool ok;
            if (id.toInt(&ok) == to_select->getEffectiveId() && ok)
                return true;
        }
        return false;
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.isEmpty()) return NULL;

        ZongxuanCard *card = new ZongxuanCard;
        card->addSubcards(cards);
        return card;
    }
};

class Zongxuan : public TriggerSkill
{
public:
    Zongxuan() : TriggerSkill("zongxuan")
    {
        events << BeforeCardsMove;
        view_as_skill = new ZongxuanViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from != player)
            return false;
        if (move.to_place == Player::DiscardPile
            && ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)) {

            int i = 0;
            QList<int> zongxuan_card;
            foreach (int card_id, move.card_ids) {
                if (room->getCardOwner(card_id) == move.from
                    && (move.from_places[i] == Player::PlaceHand || move.from_places[i] == Player::PlaceEquip)) {
                    zongxuan_card << card_id;
                }
                i++;
            }
            if (zongxuan_card.isEmpty())
                return false;

            room->setPlayerProperty(player, "zongxuan", IntList2StringList(zongxuan_card).join("+"));
            do {
                if (!room->askForUseCard(player, "@@zongxuan", "@zongxuan-put")) break;

                QList<int> subcards;
                QVariantList subcards_variant = player->tag["zongxuan"].toList();
                if (!subcards_variant.isEmpty()) {
                    subcards = VariantList2IntList(subcards_variant);
                    QStringList zongxuan = player->property("zongxuan").toString().split("+");
                    foreach (int id, subcards) {
                        zongxuan_card.removeOne(id);
                        zongxuan.removeOne(QString::number(id));
                        room->setPlayerProperty(player, "zongxuan", zongxuan.join("+"));
                        QList<int> _id;
                        _id << id;
                        move.removeCardIds(_id);
                        data = QVariant::fromValue(move);
                        room->setPlayerProperty(player, "zongxuan_move", QString::number(id)); // For UI to translate the move reason
                        room->moveCardTo(Sanguosha->getCard(id), player, NULL, Player::DrawPile, move.reason, true);
                        if (!player->isAlive())
                            break;
                    }
                }
                player->tag.remove("zongxuan");
            } while (!zongxuan_card.isEmpty());
        }
        return false;
    }
};

class Zhiyan : public PhaseChangeSkill
{
public:
    Zhiyan() : PhaseChangeSkill("zhiyan")
    {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const
    {
        if (target->getPhase() != Player::Finish)
            return false;

        Room *room = target->getRoom();
        ServerPlayer *to = room->askForPlayerChosen(target, room->getAlivePlayers(), objectName(), "zhiyan-invoke", true, true);
        if (to) {
            room->broadcastSkillInvoke(objectName());
            QList<int> ids = room->getNCards(1, false);
            const Card *card = Sanguosha->getCard(ids.first());
            room->obtainCard(to, card, false);
            if (!to->isAlive())
                return false;
            room->showCard(to, ids.first());

            if (card->isKindOf("EquipCard")) {
                room->recover(to, RecoverStruct(target));
                if (to->isAlive() && !to->isCardLimited(card, Card::MethodUse))
                    room->useCard(CardUseStruct(card, to, to));
            }
        }
        return false;
    }
};

DanshouCard::DanshouCard()
{
}

bool DanshouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty())
        return false;

    if (Self->getWeapon() && subcards.contains(Self->getWeapon()->getId())) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        int distance_fix = weapon->getRange() - Self->getAttackRange(false);
        if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
            distance_fix += 1;
        return Self->inMyAttackRange(to_select, distance_fix);
    } else if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId())) {
        return Self->inMyAttackRange(to_select, 1);
    } else
        return Self->inMyAttackRange(to_select);
}

void DanshouCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    int len = subcardsLength();
    switch (len) {
    case 0:
        Q_ASSERT(false);
        break;
    case 1:
        if (effect.from->canDiscard(effect.to, "he")) {
            int id = room->askForCardChosen(effect.from, effect.to, "he", "danshou", false, Card::MethodDiscard);
            room->throwCard(id, effect.to, effect.from);
        }
        break;
    case 2:
        if (!effect.to->isNude()) {
            const Card *card = room->askForExchange(effect.to, "danshou", 1, 1, true, "@danshou-give::" + effect.from->objectName());
            if (card) {
                CardMoveReason reason(CardMoveReason::S_REASON_GIVE, effect.to->objectName(), effect.from->objectName(), "danshou", QString());
                room->obtainCard(effect.from, card, reason, false);
                delete card;
            }
        }
        break;
    case 3:
        room->damage(DamageStruct("danshou", effect.from, effect.to));
        break;
    default:
        room->drawCards(effect.from, 2, "danshou");
        room->drawCards(effect.to, 2, "danshou");
        break;
    }
}

class DanshouViewAsSkill : public ViewAsSkill
{
public:
    DanshouViewAsSkill() : ViewAsSkill("danshou")
    {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        return !Self->isJilei(to_select) && selected.length() <= Self->getMark("danshou");
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() != Self->getMark("danshou") + 1) return NULL;
        DanshouCard *danshou = new DanshouCard;
        danshou->addSubcards(cards);
        return danshou;
    }
};

class Danshou : public TriggerSkill
{
public:
    Danshou() : TriggerSkill("danshou")
    {
        events << EventPhaseStart << PreCardUsed;
        view_as_skill = new DanshouViewAsSkill;
    }

    virtual int getPriority(TriggerEvent) const
    {
        return 6;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventPhaseStart && player->getPhase() == Player::Play) {
            room->setPlayerMark(player, "danshou", 0);
        } else if (triggerEvent == PreCardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("DanshouCard"))
                room->addPlayerMark(use.from, "danshou");
        }
        return false;
    }
};

class Juece : public PhaseChangeSkill
{
public:
    Juece() : PhaseChangeSkill("juece")
    {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const
    {
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();
        QList<ServerPlayer *> kongcheng_players;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->isKongcheng())
                kongcheng_players << p;
        }
        if (kongcheng_players.isEmpty()) return false;

        ServerPlayer *to_damage = room->askForPlayerChosen(target, kongcheng_players, objectName(),
            "@juece", true, true);
        if (to_damage) {
            room->broadcastSkillInvoke(objectName());
            room->damage(DamageStruct(objectName(), target, to_damage));
        }
        return false;
    }
};

MiejiCard::MiejiCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool MiejiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void MiejiCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    CardMoveReason reason(CardMoveReason::S_REASON_PUT, effect.from->objectName(), QString(), "mieji", QString());
    room->moveCardTo(this, effect.from, NULL, Player::DrawPile, reason, true);
    
    /*
    int trick_num = 0, nontrick_num = 0;
    foreach (const Card *c, effect.to->getCards("he")) {
        if (effect.to->canDiscard(effect.to, c->getId())) {
            if (c->isKindOf("TrickCard"))
                trick_num++;
            else
                nontrick_num++;
        }
    }
    bool discarded = room->askForDiscard(effect.to, "mieji", 1, qMin(1, trick_num), nontrick_num > 1, true, "@mieji-trick", "TrickCard");
    if (trick_num == 0 || !discarded)
        room->askForDiscard(effect.to, "mieji", 2, 2, false, true, "@mieji-nontrick", "^TrickCard");
        */

    QList<const Card *> cards = effect.to->getCards("he");

    bool instanceDiscard = false;
    int instanceDiscardId = -1;

    if (cards.length() == 1)
        instanceDiscard = true;
    else if (cards.length() == 2) {
        bool bothTrick = true;
        int trickId = -1;
        
        foreach (const Card *c, cards) {
            if (c->getTypeId() != Card::TypeTrick)
                bothTrick = false;
            else
                trickId = c->getId();
        }
        
        instanceDiscard = !bothTrick;
        instanceDiscardId = trickId;
    }

    if (instanceDiscard) {
        DummyCard d;
        if (instanceDiscardId == -1)
            d.addSubcards(cards);
        else
            d.addSubcard(instanceDiscardId);
        room->throwCard(&d, effect.to);
    } else if (!room->askForCard(effect.to, "@@miejidiscard!", "@mieji-discard")) {
        DummyCard d;
        qShuffle(cards);
        int trickId = -1;
        foreach (const Card *c, cards) {
            if (c->getTypeId() == Card::TypeTrick) {
                trickId = c->getId();
                break;
            }
        }
        if (trickId != -1)
            d.addSubcard(trickId);
        else {
            d.addSubcard(cards.first());
            d.addSubcard(cards.last());
        }

        room->throwCard(&d, effect.to);
    }
}

class Mieji : public OneCardViewAsSkill
{
public:
    Mieji() : OneCardViewAsSkill("mieji")
    {
        filter_pattern = "TrickCard|black";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("MiejiCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        MiejiCard *card = new MiejiCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class MiejiDiscard : public ViewAsSkill
{
public:
    MiejiDiscard() : ViewAsSkill("miejidiscard")
    {

    }

    virtual bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "@@miejidiscard!";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (Self->isJilei(to_select))
            return false;

        if (selected.length() == 0)
            return true;
        else if (selected.length() == 1) {
            if (selected.first()->getTypeId() == Card::TypeTrick)
                return false;
            else
                return to_select->getTypeId() != Card::TypeTrick;
        } else
            return false;

        return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        bool ok = false;
        if (cards.length() == 1)
            ok = cards.first()->getTypeId() == Card::TypeTrick;
        else if (cards.length() == 2) {
            ok = true;
            foreach (const Card *c, cards) {
                if (c->getTypeId() == Card::TypeTrick) {
                    ok = false;
                    break;
                }
            }
        }

        if (!ok)
            return NULL;

        DummyCard *dummy = new DummyCard;
        dummy->addSubcards(cards);
        return dummy;
    }
};

class Fencheng : public ZeroCardViewAsSkill
{
public:
    Fencheng() : ZeroCardViewAsSkill("fencheng")
    {
        frequency = Limited;
        limit_mark = "@burn";
    }

    virtual const Card *viewAs() const
    {
        return new FenchengCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("@burn") >= 1;
    }
};

class FenchengMark : public TriggerSkill
{
public:
    FenchengMark() : TriggerSkill("#fencheng")
    {
        events << ChoiceMade;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        QStringList data_str = data.toString().split(":");
        if (data_str.length() != 3 || data_str.first() != "cardDiscard" || data_str.at(1) != "fencheng")
            return false;
        room->setTag("FenchengDiscard", data_str.last().split("+").length());
        return false;
    }
};

FenchengCard::FenchengCard()
{
    mute = true;
    target_fixed = true;
}

void FenchengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    room->removePlayerMark(source, "@burn");
    room->broadcastSkillInvoke("fencheng");
    //room->doLightbox("$FenchengAnimate", 3000);
    room->doSuperLightbox("liru", "fencheng");
    room->setTag("FenchengDiscard", 0);

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    source->setFlags("FenchengUsing");
    try {
        foreach (ServerPlayer *player, players) {
            if (player->isAlive()) {
                room->cardEffect(this, source, player);
                room->getThread()->delay();
            }
        }
        source->setFlags("-FenchengUsing");
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken || triggerEvent == StageChange)
            source->setFlags("-FenchengUsing");
        throw triggerEvent;
    }
}

void FenchengCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();

    int length = room->getTag("FenchengDiscard").toInt() + 1;
    if (!effect.to->canDiscard(effect.to, "he") || effect.to->getCardCount(true) < length
        || !room->askForDiscard(effect.to, "fencheng", 1000, length, true, true, "@fencheng:::" + QString::number(length))) {
        room->setTag("FenchengDiscard", 0);
        room->damage(DamageStruct("fencheng", effect.from, effect.to, 2, DamageStruct::Fire));
    }
}

class Zhuikong : public TriggerSkill
{
public:
    Zhuikong() : TriggerSkill("zhuikong")
    {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (player->getPhase() != Player::RoundStart || player->isKongcheng())
            return false;

        foreach (ServerPlayer *fuhuanghou, room->getAllPlayers()) {
            if (TriggerSkill::triggerable(fuhuanghou)
                && player != fuhuanghou && fuhuanghou->isWounded() && !fuhuanghou->isKongcheng()
                && room->askForSkillInvoke(fuhuanghou, objectName())) {
                room->broadcastSkillInvoke("zhuikong");
                if (fuhuanghou->pindian(player, objectName(), NULL)) {
                    room->setPlayerFlag(player, "zhuikong");
                } else {
                    room->setFixedDistance(player, fuhuanghou, 1);
                    QVariantList zhuikonglist = player->tag[objectName()].toList();
                    zhuikonglist.append(QVariant::fromValue(fuhuanghou));
                    player->tag[objectName()] = QVariant::fromValue(zhuikonglist);
                }
            }
        }
        return false;
    }
};

class ZhuikongClear : public TriggerSkill
{
public:
    ZhuikongClear() : TriggerSkill("#zhuikong-clear")
    {
        events << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;

        QVariantList zhuikonglist = player->tag["zhuikong"].toList();
        if (zhuikonglist.isEmpty()) return false;
        foreach (QVariant p, zhuikonglist) {
            ServerPlayer *fuhuanghou = p.value<ServerPlayer *>();
            room->removeFixedDistance(player, fuhuanghou, 1);
        }
        player->tag.remove("zhuikong");
        return false;
    }
};

class ZhuikongProhibit : public ProhibitSkill
{
public:
    ZhuikongProhibit() : ProhibitSkill("#zhuikong")
    {
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &) const
    {
        if (card->getTypeId() != Card::TypeSkill && from->hasFlag("zhuikong"))
            return to != from;
        return false;
    }
};

class Qiuyuan : public TriggerSkill
{
public:
    Qiuyuan() : TriggerSkill("qiuyuan")
    {
        events << TargetConfirming;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash")) {
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p != use.from)
                    targets << p;
            }
            if (targets.isEmpty()) return false;
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "qiuyuan-invoke", true, true);
            if (target) {
                if (target->getGeneralName().contains("fuwan") || target->getGeneral2Name().contains("fuwan"))
                    room->broadcastSkillInvoke("qiuyuan", 2);
                else
                    room->broadcastSkillInvoke("qiuyuan", 1);
                const Card *card = NULL;
                if (!target->isKongcheng())
                    card = room->askForCard(target, "Jink", "@qiuyuan-give:" + player->objectName(), data, Card::MethodNone);
                CardMoveReason reason(CardMoveReason::S_REASON_GIVE, target->objectName(), player->objectName(), "nosqiuyuan", QString());
                if (!card) {
                    if (use.from->canSlash(target, use.card, false)) {
                        LogMessage log;
                        log.type = "#BecomeTarget";
                        log.from = target;
                        log.card_str = use.card->toString();
                        room->sendLog(log);

                        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), target->objectName());

                        use.to.append(target);
                        room->sortByActionOrder(use.to);
                        data = QVariant::fromValue(use);
                        //room->getThread()->trigger(TargetConfirming, room, target, data);
                    }
                } else {
                    room->obtainCard(player, card, reason);
                }
            }
        }
        return false;
    }
};

YJCM2013Package::YJCM2013Package()
    : Package("YJCM2013")
{
    General *caochong = new General(this, "caochong", "wei", 3); // YJ 201
    caochong->addSkill(new Chengxiang);
    caochong->addSkill(new Renxin);

    General *fuhuanghou = new General(this, "fuhuanghou", "qun", 3, false); // YJ 202
    fuhuanghou->addSkill(new Zhuikong);
    fuhuanghou->addSkill(new ZhuikongClear);
    fuhuanghou->addSkill(new ZhuikongProhibit);
    fuhuanghou->addSkill(new Qiuyuan);
    related_skills.insertMulti("zhuikong", "#zhuikong");
    related_skills.insertMulti("zhuikong", "#zhuikong-clear");

    General *guohuai = new General(this, "guohuai", "wei"); // YJ 203
    guohuai->addSkill(new Jingce);
    guohuai->addSkill(new JingceRecord);
    related_skills.insertMulti("jingce", "#jingce-record");

    General *guanping = new General(this, "guanping", "shu", 4); // YJ 204
    guanping->addSkill(new Longyin);

    General *jianyong = new General(this, "jianyong", "shu", 3); // YJ 205
    jianyong->addSkill(new Qiaoshui);
    jianyong->addSkill(new QiaoshuiUse);
    jianyong->addSkill(new QiaoshuiTargetMod);
    jianyong->addSkill(new Zongshih);
    related_skills.insertMulti("qiaoshui", "#qiaoshui-use");
    related_skills.insertMulti("qiaoshui", "#qiaoshui-target");

    General *liru = new General(this, "liru", "qun", 3); // YJ 206
    liru->addSkill(new Juece);
    liru->addSkill(new Mieji);
    liru->addSkill(new Fencheng);
    liru->addSkill(new FenchengMark);
    related_skills.insertMulti("fencheng", "#fencheng");

    General *liufeng = new General(this, "liufeng", "shu"); // YJ 207
    liufeng->addSkill(new Xiansi);
    liufeng->addSkill(new XiansiAttach);
    related_skills.insertMulti("xiansi", "#xiansi-attach");

    General *manchong = new General(this, "manchong", "wei", 3); // YJ 208
    manchong->addSkill(new Junxing);
    manchong->addSkill(new Yuce);

    General *panzhangmazhong = new General(this, "panzhangmazhong", "wu"); // YJ 209
    panzhangmazhong->addSkill(new Duodao);
    panzhangmazhong->addSkill(new Anjian);

    General *yufan = new General(this, "yufan", "wu", 3); // YJ 210
    yufan->addSkill(new Zongxuan);
    yufan->addSkill(new Zhiyan);

    General *zhuran = new General(this, "zhuran", "wu"); // YJ 211
    zhuran->addSkill(new Danshou);

    addMetaObject<JunxingCard>();
    addMetaObject<QiaoshuiCard>();
    addMetaObject<XiansiCard>();
    addMetaObject<XiansiSlashCard>();
    addMetaObject<ZongxuanCard>();
    addMetaObject<MiejiCard>();
    addMetaObject<FenchengCard>();
    addMetaObject<ExtraCollateralCard>();
    addMetaObject<DanshouCard>();

    skills << new XiansiSlashViewAsSkill << new MiejiDiscard;
}

ADD_PACKAGE(YJCM2013)
