#include "boss.h"
#include "settings.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "maneuvering.h"

class BossGuimei: public ProhibitSkill {
public:
    BossGuimei(): ProhibitSkill("bossguimei") {
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const{
        return to->hasSkill(objectName()) && card->isKindOf("DelayedTrick");
    }
};

class BossDidong: public PhaseChangeSkill {
public:
    BossDidong(): PhaseChangeSkill("bossdidong") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        ServerPlayer *player = room->askForPlayerChosen(target, room->getOtherPlayers(target), objectName(), "bossdidong-invoke", true, true);
        if (player) {
            room->broadcastSkillInvoke(objectName());
            player->turnOver();
        }
        return false;
    }
};

class BossShanbeng: public TriggerSkill {
public:
    BossShanbeng(): TriggerSkill("bossshanbeng") {
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (player != death.who) return false;

        bool sendLog = false;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->getEquips().isEmpty()) continue;
            if (!sendLog) {
                sendLog = true;
                room->broadcastSkillInvoke(objectName());
                room->sendCompulsoryTriggerLog(player, objectName());
            }
            p->throwAllEquips();
        }
        return false;
    }
};

class BossBeiming: public TriggerSkill {
public:
    BossBeiming():TriggerSkill("bossbeiming") {
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return false;
        ServerPlayer *killer = death.damage ? death.damage->from : NULL;
        if (killer && killer != player) {
            LogMessage log;
            log.type = "#BeimingThrow";
            log.from = player;
            log.to << killer;
            log.arg = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(player, objectName());

            room->broadcastSkillInvoke(objectName());

            killer->throwAllHandCards();
        }

        return false;
    }
};

class BossLuolei: public PhaseChangeSkill {
public:
    BossLuolei(): PhaseChangeSkill("bossluolei") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Start) return false;
        Room *room = target->getRoom();

        ServerPlayer *player = room->askForPlayerChosen(target, room->getOtherPlayers(target), objectName(), "bossluolei-invoke", true, true);
        if (player) {
            room->broadcastSkillInvoke(objectName());
            room->damage(DamageStruct(objectName(), target, player, 1, DamageStruct::Thunder));
        }
        return false;
    }
};

class BossGuihuo: public PhaseChangeSkill {
public:
    BossGuihuo(): PhaseChangeSkill("bossguihuo") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Start) return false;
        Room *room = target->getRoom();

        ServerPlayer *player = room->askForPlayerChosen(target, room->getOtherPlayers(target), objectName(), "bossguihuo-invoke", true, true);
        if (player) {
            room->broadcastSkillInvoke(objectName());
            room->damage(DamageStruct(objectName(), target, player, 1, DamageStruct::Fire));
        }
        return false;
    }
};

class BossMingbao: public TriggerSkill {
public:
    BossMingbao(): TriggerSkill("bossmingbao") {
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (player != death.who) return false;

        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(player, objectName());

        foreach (ServerPlayer *p, room->getOtherPlayers(player))
            room->damage(DamageStruct(objectName(), NULL, p, 1, DamageStruct::Fire));
        return false;
    }
};

class BossBaolian: public PhaseChangeSkill {
public:
    BossBaolian(): PhaseChangeSkill("bossbaolian") {
        frequency = Compulsory;
    }

    virtual int getPriority(TriggerEvent) const{
        return 4;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(target, objectName());

        target->drawCards(2, objectName());
        return false;
    }
};

class BossManjia: public TriggerSkill {
public:
    BossManjia(): TriggerSkill("bossmanjia") {
        events << DamageInflicted << SlashEffected << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->getArmor() && target->hasArmorEffect("vine");
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.nature == DamageStruct::Normal) {
                room->broadcastSkillInvoke(objectName());
                room->sendCompulsoryTriggerLog(player, objectName());

                room->setEmotion(player, "armor/vine");
                LogMessage log;
                log.from = player;
                log.type = "#ArmorNullify";
                log.arg = objectName();
                log.arg2 = effect.slash->objectName();
                room->sendLog(log);

                effect.to->setFlags("Global_NonSkillNullify");
                return true;
            }
        } else if (triggerEvent == CardEffected) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.card->isKindOf("AOE")) {
                room->broadcastSkillInvoke(objectName());
                room->sendCompulsoryTriggerLog(player, objectName());

                room->setEmotion(player, "armor/vine");
                LogMessage log;
                log.from = player;
                log.type = "#ArmorNullify";
                log.arg = objectName();
                log.arg2 = effect.card->objectName();
                room->sendLog(log);

                effect.to->setFlags("Global_NonSkillNullify");
                return true;
            }
        } else if (triggerEvent == DamageInflicted) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.nature == DamageStruct::Fire) {
                room->broadcastSkillInvoke(objectName());
                room->sendCompulsoryTriggerLog(player, objectName());

                room->setEmotion(player, "armor/vineburn");
                LogMessage log;
                log.type = "#VineDamage";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(++damage.damage);
                room->sendLog(log);

                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

class BossXiaoshou: public PhaseChangeSkill {
public:
    BossXiaoshou(): PhaseChangeSkill("bossxiaoshou") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> players;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->getHp() > target->getHp())
                players << p;
        }
        if (players.isEmpty()) return false;
        ServerPlayer *player = room->askForPlayerChosen(target, players, objectName(), "bossxiaoshou-invoke", true, true);
        if (player) {
            room->broadcastSkillInvoke(objectName());
            room->damage(DamageStruct(objectName(), target, player, 2));
        }
        return false;
    }
};

class BossGuiji: public TriggerSkill {
public:
    BossGuiji(): TriggerSkill("bossguiji") {
        events << EventPhaseEnd;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        if (player->getPhase() != Player::Start || player->getJudgingArea().isEmpty())
            return false;

        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(player, objectName());

        QList<const Card *> dtricks = player->getJudgingArea();
        int index = qrand() % dtricks.length();
        room->throwCard(dtricks.at(index), NULL, player);
        return false;
    }
};

class BossLianyu: public PhaseChangeSkill {
public:
    BossLianyu(): PhaseChangeSkill("bosslianyu") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish)
            return false;

        Room *room = target->getRoom();
        if (room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());
            foreach (ServerPlayer *p, room->getOtherPlayers(target))
                room->damage(DamageStruct(objectName(), target, p, 1, DamageStruct::Fire));
        }
        return false;
    }
};

class BossTaiping: public DrawCardsSkill {
public:
    BossTaiping(): DrawCardsSkill("bosstaiping") {
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();

        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(player, objectName());

        return n + 2;
    }
};

class BossSuoming: public PhaseChangeSkill {
public:
    BossSuoming(): PhaseChangeSkill("bosssuoming") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> to_chain;
        foreach (ServerPlayer *p, room->getOtherPlayers(target)) {
            if (!p->isChained())
                to_chain << p;
        }

        if (!to_chain.isEmpty() && room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());

            foreach (ServerPlayer *p, to_chain) {
                if (p->isChained()) continue;
                p->setChained(true);
                room->broadcastProperty(p, "chained");
                room->setEmotion(p, "chain");
                room->getThread()->trigger(ChainStateChanged, room, p);
            }
        }
        return false;
    }
};

class BossXixing: public PhaseChangeSkill {
public:
    BossXixing(): PhaseChangeSkill("bossxixing") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Start) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> chain;
        foreach (ServerPlayer *p, room->getOtherPlayers(target)) {
            if (p->isChained())
                chain << p;
        }
        if (chain.isEmpty()) return false;
        ServerPlayer *player = room->askForPlayerChosen(target, chain, objectName(), "bossxixing-invoke", true, true);
        if (player) {
            room->broadcastSkillInvoke(objectName());
            target->setFlags("bossxixing");
            try {
                room->damage(DamageStruct(objectName(), target, player, 1, DamageStruct::Thunder));
                if (target->isAlive() && target->hasFlag("bossxixing")) {
                    target->setFlags("-bossxixing");
                    if (target->isWounded())
                        room->recover(target, RecoverStruct(target));
                }
            }
            catch (TriggerEvent triggerEvent) {
                if (triggerEvent == TurnBroken || triggerEvent == StageChange)
                    target->setFlags("-bossxixing");
                throw triggerEvent;
            }
        }
        return false;
    }
};

class BossQiangzheng: public PhaseChangeSkill {
public:
    BossQiangzheng(): PhaseChangeSkill("bossqiangzheng") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        bool can_invoke = false;
        foreach (ServerPlayer *p, room->getOtherPlayers(target)) {
            if (!p->isKongcheng()) {
                can_invoke = true;
                break;
            }
        }
        if (!can_invoke) return false;

        if (room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());
            foreach (ServerPlayer *p, room->getOtherPlayers(target)) {
                if (p->isAlive() && !p->isKongcheng()) {
                    int card_id = room->askForCardChosen(target, p, "h", "bossqiangzheng");

                    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, target->objectName());
                    room->obtainCard(target, Sanguosha->getCard(card_id), reason, false);
                }
            }
        }
        return false;
    }
};

class BossZuijiu: public TriggerSkill {
public:
    BossZuijiu(): TriggerSkill("bosszuijiu") {
        events << ConfirmDamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isKindOf("Slash")) {
            LogMessage log;
            log.type = "#ZuijiuBuff";
            log.from = damage.from;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++damage.damage);
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(damage.from, objectName());

            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class BossModao: public PhaseChangeSkill {
public:
    BossModao(): PhaseChangeSkill("bossmodao") {
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Start) return false;
        Room *room = target->getRoom();

        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(target, objectName());

        target->drawCards(2, objectName());
        return false;
    }
};

class BossQushou: public PhaseChangeSkill {
public:
    BossQushou(): PhaseChangeSkill("bossqushou") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Play) return false;
        Room *room = target->getRoom();

        SavageAssault *sa = new SavageAssault(Card::NoSuit, 0);
        sa->setSkillName("_" + objectName());
        bool can_invoke = false;
        if (!target->isCardLimited(sa, Card::MethodUse)) {
            foreach (ServerPlayer *p, room->getOtherPlayers(target)) {
                if (!room->isProhibited(target, p, sa)) {
                    can_invoke = true;
                    break;
                }
            }
        }
        if (!can_invoke) {
            delete sa;
            return false;
        }
        if (room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());
            room->useCard(CardUseStruct(sa, target, QList<ServerPlayer *>()));
        } else {
            delete sa;
        }
        return false;
    }
};

class BossMojian: public PhaseChangeSkill {
public:
    BossMojian(): PhaseChangeSkill("bossmojian") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Play) return false;
        Room *room = target->getRoom();

        ArcheryAttack *aa = new ArcheryAttack(Card::NoSuit, 0);
        aa->setSkillName("_" + objectName());
        bool can_invoke = false;
        if (!target->isCardLimited(aa, Card::MethodUse)) {
            foreach (ServerPlayer *p, room->getOtherPlayers(target)) {
                if (!room->isProhibited(target, p, aa)) {
                    can_invoke = true;
                    break;
                }
            }
        }
        if (!can_invoke) {
            delete aa;
            return false;
        }
        if (room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());
            room->useCard(CardUseStruct(aa, target, QList<ServerPlayer *>()));
        } else {
            delete aa;
        }
        return false;
    }
};

class BossDanshu: public TriggerSkill {
public:
    BossDanshu(): TriggerSkill("bossdanshu") {
        events << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (player != move.from || player->getPhase() != Player::NotActive
            || (!move.from_places.contains(Player::PlaceHand) && !move.from_places.contains(Player::PlaceEquip)))
            return false;
        if (room->askForSkillInvoke(player, objectName(), data)) {
            JudgeStruct judge;
            judge.who = player;
            judge.reason = objectName();
            judge.good = true;
            judge.pattern = ".|red";
            room->judge(judge);

            if (judge.isGood() && player->isAlive() && player->isWounded())
                room->recover(player, RecoverStruct(player));
        }
        return false;
    }
};

BossModePackage::BossModePackage()
    : Package("BossMode")
{
    General *chi = new General(this, "boss_chi", "god", 5, true, true);
    chi->addSkill(new BossGuimei);
    chi->addSkill(new BossDidong);
    chi->addSkill(new BossShanbeng);

    General *mei = new General(this, "boss_mei", "god", 5, true, true);
    mei->addSkill("bossguimei");
    mei->addSkill("nosenyuan");
    mei->addSkill(new BossBeiming);

    General *wang = new General(this, "boss_wang", "god", 5, true, true);
    wang->addSkill("bossguimei");
    wang->addSkill(new BossLuolei);
    wang->addSkill("huilei");

    General *liang = new General(this, "boss_liang", "god", 5, true, true);
    liang->addSkill("bossguimei");
    liang->addSkill(new BossGuihuo);
    liang->addSkill(new BossMingbao);

    General *niutou = new General(this, "boss_niutou", "god", 10, true, true);
    niutou->addSkill(new BossBaolian);
    niutou->addSkill("mengjin");
    niutou->addSkill(new BossManjia);
    niutou->addSkill(new BossXiaoshou);

    General *mamian = new General(this, "boss_mamian", "god", 9, true, true);
    mamian->addSkill(new BossGuiji);
    mamian->addSkill("nosfankui");
    mamian->addSkill(new BossLianyu);
    mamian->addSkill("nosjuece");

    General *heiwuchang = new General(this, "boss_heiwuchang", "god", 15, true, true);
    heiwuchang->addSkill("bossguiji");
    heiwuchang->addSkill(new BossTaiping);
    heiwuchang->addSkill(new BossSuoming);
    heiwuchang->addSkill(new BossXixing);

    General *baiwuchang = new General(this, "boss_baiwuchang", "god", 18, true, true);
    baiwuchang->addSkill("bossbaolian");
    baiwuchang->addSkill(new BossQiangzheng);
    baiwuchang->addSkill(new BossZuijiu);
    baiwuchang->addSkill("nosjuece");

    General *luocha = new General(this, "boss_luocha", "god", 20, true, true);
    luocha->addSkill(new BossModao);
    luocha->addSkill(new BossQushou);
    luocha->addSkill("yizhong");
    luocha->addSkill("kuanggu");

    General *yecha = new General(this, "boss_yecha", "god", 18, false, true);
    yecha->addSkill("bossmodao");
    yecha->addSkill(new BossMojian);
    yecha->addSkill("bazhen");
    yecha->addSkill(new BossDanshu);
}

ADD_PACKAGE(BossMode)
