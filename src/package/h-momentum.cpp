#include "h-momentum.h"
#include "general.h"
#include "standard.h"
#include "standard-equips.h"
#include "maneuvering.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"
#include "settings.h"
#include "json.h"

class Hengjiang : public MasochismSkill
{
public:
    Hengjiang() : MasochismSkill("hengjiang")
    {
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const
    {
        Room *room = target->getRoom();
        for (int i = 1; i <= damage.damage; i++) {
            ServerPlayer *current = room->getCurrent();
            if (!current || current->isDead() || current->getPhase() == Player::NotActive)
                break;
            if (room->askForSkillInvoke(target, objectName(), QVariant::fromValue(current))) {
                room->broadcastSkillInvoke(objectName());
                room->addPlayerMark(current, "@hengjiang");
            }
        }
    }
};

class HengjiangDraw : public TriggerSkill
{
public:
    HengjiangDraw() : TriggerSkill("#hengjiang-draw")
    {
        events << TurnStart << CardsMoveOneTime << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == TurnStart) {
            room->setPlayerMark(player, "@hengjiang", 0);
        } else if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from && player == move.from && player->getPhase() == Player::Discard
                && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)
                player->setFlags("HengjiangDiscarded");
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive) return false;
            ServerPlayer *zangba = room->findPlayerBySkillName("hengjiang");
            if (!zangba) return false;
            if (player->getMark("@hengjiang") > 0) {
                bool invoke = false;
                if (!player->hasFlag("HengjiangDiscarded")) {
                    LogMessage log;
                    log.type = "#HengjiangDraw";
                    log.from = player;
                    log.to << zangba;
                    log.arg = "hengjiang";
                    room->sendLog(log);

                    invoke = true;
                }
                player->setFlags("-HengjiangDiscarded");
                room->setPlayerMark(player, "@hengjiang", 0);
                if (invoke) zangba->drawCards(1, objectName());
            }
        }
        return false;
    }
};

class HengjiangMaxCards : public MaxCardsSkill
{
public:
    HengjiangMaxCards() : MaxCardsSkill("#hengjiang-maxcard")
    {
    }

    virtual int getExtra(const Player *target) const
    {
        return -target->getMark("@hengjiang");
    }
};

GuixiuCard::GuixiuCard()
{
    target_fixed = true;
}

void GuixiuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    room->removePlayerMark(source, "guixiu");
    source->drawCards(2, "guixiu");
}

class GuixiuViewAsSkill : public ZeroCardViewAsSkill
{
public:
    GuixiuViewAsSkill() : ZeroCardViewAsSkill("guixiu")
    {
    }

    virtual const Card *viewAs() const
    {
        return new GuixiuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("guixiu") >= 1;
    }
};

class Guixiu : public TriggerSkill
{
public:
    Guixiu() : TriggerSkill("guixiu")
    {
        frequency = Limited;
        limit_mark = "guixiu";
        events << EventPhaseStart;
        view_as_skill = new GuixiuViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (player->getMark("guixiu") >= 1 && player->getPhase() == Player::Start
            && room->askForSkillInvoke(player, objectName())) {
            room->broadcastSkillInvoke(objectName(), 1);
            room->removePlayerMark(player, "guixiu");
            player->drawCards(2, objectName());
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const
    {
        return 1;
    }
};

class GuixiuDetach : public DetachEffectSkill
{
public:
    GuixiuDetach() : DetachEffectSkill("guixiu")
    {
    }

    virtual void onSkillDetached(Room *room, ServerPlayer *player) const
    {
        if (player->isWounded() && room->askForSkillInvoke(player, "guixiu_rec", "recover")) {
            room->broadcastSkillInvoke("guixiu", 2);
            room->notifySkillInvoked(player, "guixiu");
            room->recover(player, RecoverStruct(player));
        }
    }
};

CunsiCard::CunsiCard()
{
}

bool CunsiCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const
{
    return targets.isEmpty();
}

void CunsiCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    //room->doLightbox("$CunsiAnimate", 3000);

    room->doSuperLightbox("mifuren", "cunsi");

    room->handleAcquireDetachSkills(effect.from, "-guixiu|-cunsi");

    room->acquireSkill(effect.to, "yongjue");
    if (effect.to != effect.from)
        effect.to->drawCards(2, "cunsi");
}

class Cunsi : public ZeroCardViewAsSkill
{
public:
    Cunsi() : ZeroCardViewAsSkill("cunsi")
    {
        frequency = Limited;
    }

    virtual const Card *viewAs() const
    {
        return new CunsiCard;
    }
};

class Yongjue : public TriggerSkill
{
public:
    Yongjue() : TriggerSkill("yongjue")
    {
        events << BeforeCardsMove;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile
            && move.reason.m_reason == CardMoveReason::S_REASON_USE) {
            const Card *yongjue_card = move.reason.m_extraData.value<const Card *>();
            if (!yongjue_card || !yongjue_card->isKindOf("Slash") || !yongjue_card->hasFlag("yongjue"))
                return false;
            ServerPlayer *yongjue_user = room->getTag("yongjue_user").value<ServerPlayer *>();
            room->removeTag("yongjue_user");
            if (yongjue_user) {
                if (room->askForSkillInvoke(player, objectName(), QVariant::fromValue(yongjue_user))) {
                    yongjue_user->obtainCard(yongjue_card);
                    move.removeCardIds(move.card_ids);
                    data = QVariant::fromValue(move);
                }
            }
        }
        return false;
    }
};

class YongjueRecord : public TriggerSkill
{
public:
    YongjueRecord() : TriggerSkill("#yongjue-record")
    {
        events << PreCardUsed << CardResponded << EventPhaseStart;
        global = true;
    }

    virtual int getPriority(TriggerEvent triggerEvent) const
    {
        if (triggerEvent == EventPhaseStart)
            return 10;
        else
            return TriggerSkill::getPriority(triggerEvent);
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if ((triggerEvent == PreCardUsed || triggerEvent == CardResponded) && player->getPhase() == Player::Play) {
            const Card *card = NULL;
            if (triggerEvent == PreCardUsed)
                card = data.value<CardUseStruct>().card;
            else {
                CardResponseStruct response = data.value<CardResponseStruct>();
                if (response.m_isUse)
                    card = response.m_card;
            }
            if (card && card->getHandlingMethod() == Card::MethodUse
                && player->getPhase() == Player::Play && player->getMark("yongjue") == 0) {
                player->addMark("yongjue");
                if (card->isKindOf("Slash")) {
                    QList<int> ids;
                    if (!card->isVirtualCard())
                        ids << card->getEffectiveId();
                    else if (card->subcardsLength() > 0)
                        ids = card->getSubcards();
                    if (!ids.isEmpty()) {
                        room->setCardFlag(card, "yongjue");
                        room->setTag("yongjue_user", QVariant::fromValue(player));
                    }
                }
            }
        } else if (triggerEvent == EventPhaseStart && player->getPhase() == Player::Play) {
            player->setMark("yongjue", 0);
        }

        return false;
    }
};

DuanxieCard::DuanxieCard()
{
}

bool DuanxieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && !to_select->isChained() && to_select != Self;
}

void DuanxieCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    if (!effect.to->isChained()) {
        effect.to->setChained(true);
        room->broadcastProperty(effect.to, "chained");
        room->setEmotion(effect.to, "chain");
        room->getThread()->trigger(ChainStateChanged, room, effect.to);
    }
    if (!effect.from->isChained()) {
        effect.from->setChained(true);
        room->broadcastProperty(effect.from, "chained");
        room->setEmotion(effect.from, "chain");
        room->getThread()->trigger(ChainStateChanged, room, effect.from);
    }
}

class Duanxie : public ZeroCardViewAsSkill
{
public:
    Duanxie() : ZeroCardViewAsSkill("duanxie")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        if (player->hasUsed("DuanxieCard")) return false;
        foreach (const Player *p, player->getAliveSiblings()) {
            if (!p->isChained()) return true;
        }
        return false;
    }

    virtual const Card *viewAs() const
    {
        return new DuanxieCard;
    }
};

class Fenming : public PhaseChangeSkill
{
public:
    Fenming() : PhaseChangeSkill("fenming")
    {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const
    {
        Room *room = target->getRoom();
        if (target->getPhase() == Player::Finish && target->isChained()
            && room->askForSkillInvoke(target, objectName())) {
            foreach (ServerPlayer *p, room->getAllPlayers()) {
                if (!target->isAlive())
                    break;
                if (!p->isAlive() || !target->canDiscard(p, "he") || !p->isChained())
                    continue;
                if (p == target) {
                    if (!target->isNude())
                        room->askForDiscard(target, objectName(), 1, 1, false, true);
                } else {
                    int id = room->askForCardChosen(target, p, "he", objectName(), false, Card::MethodDiscard);
                    room->throwCard(id, p, target);
                }
            }
        }
        return false;
    }
};

class Yingyang : public TriggerSkill
{
public:
    Yingyang() : TriggerSkill("yingyang")
    {
        events << PindianVerifying;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        PindianStruct *pindian = data.value<PindianStruct *>();
        if (TriggerSkill::triggerable(pindian->from)) {
            QString choice = room->askForChoice(pindian->from, objectName(), "up+down+cancel", data);
            if (choice == "up") {
                pindian->from_number = qMin(pindian->from_number + 3, 13);
                doYingyangLog(room, pindian->from, choice, pindian->from_number);
                data = QVariant::fromValue(pindian);
            } else if (choice == "down") {
                pindian->from_number = qMax(pindian->from_number - 3, 1);
                doYingyangLog(room, pindian->from, choice, pindian->from_number);
                data = QVariant::fromValue(pindian);
            }
        }
        if (TriggerSkill::triggerable(pindian->to)) {
            QString choice = room->askForChoice(pindian->to, objectName(), "up+down+cancel", data);
            if (choice == "up") {
                pindian->to_number = qMin(pindian->to_number + 3, 13);
                doYingyangLog(room, pindian->to, choice, pindian->to_number);
                data = QVariant::fromValue(pindian);
            } else if (choice == "down") {
                pindian->to_number = qMax(pindian->to_number - 3, 1);
                doYingyangLog(room, pindian->to, choice, pindian->to_number);
                data = QVariant::fromValue(pindian);
            }
        }
        return false;
    }

private:
    QString getNumberString(int number) const
    {
        if (number == 10)
            return "10";
        else {
            static const char *number_string = "-A23456789-JQK";
            return QString(number_string[number]);
        }
    }

    void doYingyangLog(Room *room, ServerPlayer *player, const QString &choice, int number) const
    {
        room->notifySkillInvoked(player, objectName());
        if (choice == "up") {
            room->broadcastSkillInvoke(objectName(), 1);

            LogMessage log;
            log.type = "#YingyangUp";
            log.from = player;
            log.arg = getNumberString(number);
            room->sendLog(log);
        } else if (choice == "down") {
            room->broadcastSkillInvoke(objectName(), 2);

            LogMessage log;
            log.type = "#YingyangDown";
            log.from = player;
            log.arg = getNumberString(number);
            room->sendLog(log);
        }
    }
};

class Hengzheng : public PhaseChangeSkill
{
public:
    Hengzheng() : PhaseChangeSkill("hengzheng")
    {
    }

    virtual bool onPhaseChange(ServerPlayer *dongzhuo) const
    {
        if (dongzhuo->getPhase() == Player::Draw && (dongzhuo->isKongcheng() || dongzhuo->getHp() <= 1)) {
            Room *room = dongzhuo->getRoom();
            if (room->askForSkillInvoke(dongzhuo, objectName())) {
                room->broadcastSkillInvoke(objectName());

                dongzhuo->setFlags("HengzhengUsing");
                if (dongzhuo->getMark("HengzhengUsed") == 0)
                    dongzhuo->setMark("HengzhengUsed", 1);
                QList<ServerPlayer *> players = room->getOtherPlayers(dongzhuo);
                /*if (players.length() >= 4)
                    room->doLightbox("$HengzhengAnimate");*/

                room->doSuperLightbox("heg_dongzhuo", "hengzheng");

                foreach (ServerPlayer *player, players) {
                    if (player->isAlive() && !player->isAllNude()) {
                        CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, dongzhuo->objectName());
                        int card_id = room->askForCardChosen(dongzhuo, player, "hej", objectName());
                        room->obtainCard(dongzhuo, Sanguosha->getCard(card_id),
                            reason, room->getCardPlace(card_id) != Player::PlaceHand);
                    }
                }

                dongzhuo->setFlags("-HengzhengUsing");
                return true;
            }
        }

        return false;
    }
};

class Baoling : public TriggerSkill
{
public:
    Baoling() : TriggerSkill("baoling")
    {
        events << EventPhaseEnd;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const
    {
        return TriggerSkill::triggerable(target)
            && target->getPhase() == Player::Play
            && target->getMark("baoling") == 0
            && target->getMark("HengzhengUsed") >= 1;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        room->notifySkillInvoked(player, objectName());

        LogMessage log;
        log.type = "#BaolingWake";
        log.from = player;
        log.arg = objectName();
        log.arg2 = "hengzheng";
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        //room->doLightbox("$BaolingAnimate");

        room->doSuperLightbox("heg_dongzhuo", "baoling");

        room->setPlayerMark(player, "baoling", 1);
        if (room->changeMaxHpForAwakenSkill(player, 3)) {
            room->recover(player, RecoverStruct(player, NULL, 3));
            if (player->getMark("baoling") == 1)
                room->acquireSkill(player, "benghuai");
        }

        return false;
    }
};

class Chuanxin : public TriggerSkill
{
public:
    Chuanxin() : TriggerSkill("chuanxin")
    {
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && (damage.card->isKindOf("Slash") || damage.card->isKindOf("Duel"))
            && !damage.chain && !damage.transfer && damage.by_user
            && room->askForSkillInvoke(player, objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            QStringList choices;
            if (!damage.to->getEquips().isEmpty()) choices << "throw";
            QStringList skills_list;
            if (damage.to->getMark("chuanxin_" + player->objectName()) == 0) {
                foreach (const Skill *skill, damage.to->getVisibleSkillList()) {
                    if (!skill->isAttachedLordSkill())
                        skills_list << skill->objectName();
                }
                if (skills_list.length() > 1) choices << "detach";
            }
            if (choices.isEmpty()) return true;
            QString choice = room->askForChoice(damage.to, objectName(), choices.join("+"), data);
            if (choice == "throw") {
                damage.to->throwAllEquips();
                if (damage.to->isAlive())
                    room->loseHp(damage.to);
            } else {
                room->addPlayerMark(damage.to, "chuanxin_" + player->objectName());
                room->addPlayerMark(damage.to, "@chuanxin");
                QString lost_skill = room->askForChoice(damage.to, "chuanxin_lose", skills_list.join("+"), data);
                room->detachSkillFromPlayer(damage.to, lost_skill);
            }
            return true;
        }
        return false;
    }
};

class Fengshi : public TriggerSkill
{
public:
    Fengshi() : TriggerSkill("fengshi")
    {
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash") && use.from->isAlive()) {
            for (int i = 0; i < use.to.length(); i++) {
                ServerPlayer *to = use.to.at(i);
                if (to->isAlive() && to->isAdjacentTo(player) && to->isAdjacentTo(use.from)
                    && !to->getEquips().isEmpty()) {
                    to->setFlags("FengshiTarget"); // For AI
                    bool invoke = room->askForSkillInvoke(player, objectName(), QVariant::fromValue(to));
                    to->setFlags("-FengshiTarget");
                    if (!invoke) continue;
                    room->broadcastSkillInvoke(objectName());
                    int id = -1;
                    if (to->getEquips().length() == 1)
                        id = to->getEquips().first()->getEffectiveId();
                    else
                        id = room->askForCardChosen(to, to, "e", objectName(), false, Card::MethodDiscard);
                    room->throwCard(id, to);
                }
            }
        }

        return false;
    }
};

HMomentumPackage::HMomentumPackage()
    : Package("h_momentum")
{
    General *zangba = new General(this, "zangba", "wei", 4); // WEI 023
    zangba->addSkill(new Hengjiang);
    zangba->addSkill(new HengjiangDraw);
    zangba->addSkill(new HengjiangMaxCards);
    related_skills.insertMulti("hengjiang", "#hengjiang-draw");
    related_skills.insertMulti("hengjiang", "#hengjiang-maxcard");

    General *heg_madai = new General(this, "heg_madai", "shu", 4, true, true); // SHU 019
    heg_madai->addSkill("mashu");
    heg_madai->addSkill("qianxi");

    General *mifuren = new General(this, "mifuren", "shu", 3, false); // SHU 021
    mifuren->addSkill(new Guixiu);
    mifuren->addSkill(new GuixiuDetach);
    mifuren->addSkill(new Cunsi);
    related_skills.insertMulti("guixiu", "#guixiu-clear");
    mifuren->addRelateSkill("yongjue");

    General *heg_sunce = new General(this, "heg_sunce$", "wu", 4); // WU 010 G
    heg_sunce->addSkill("jiang");
    heg_sunce->addSkill(new Yingyang);
    heg_sunce->addSkill("zhiba");

    General *chenwudongxi = new General(this, "chenwudongxi", "wu", 4); // WU 023
    chenwudongxi->addSkill(new Duanxie);
    chenwudongxi->addSkill(new Fenming);

    General *heg_dongzhuo = new General(this, "heg_dongzhuo$", "qun", 4); // QUN 006 G
    heg_dongzhuo->addSkill(new Hengzheng);
    heg_dongzhuo->addSkill(new Baoling);
    heg_dongzhuo->addSkill("baonue");

    General *zhangren = new General(this, "zhangren", "qun", 4); // QUN 024
    zhangren->addSkill(new Chuanxin);
    zhangren->addSkill(new Fengshi);

    skills << new Yongjue << new YongjueRecord;
    related_skills.insertMulti("yongjue", "#yongjue-record");

    addMetaObject<GuixiuCard>();
    addMetaObject<CunsiCard>();
    addMetaObject<DuanxieCard>();
}

ADD_PACKAGE(HMomentum)
