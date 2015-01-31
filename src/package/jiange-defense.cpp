#include "jiange-defense.h"
#include "settings.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "maneuvering.h"

bool isJianGeFriend(const Player *a, const Player *b) {
    return a->getRole() == b->getRole();
}

// WEI Souls

class JGChiying: public TriggerSkill {
public:
    JGChiying(): TriggerSkill("jgchiying") {
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *zidan = room->findPlayerBySkillName(objectName());
        if (zidan && isJianGeFriend(zidan, damage.to) && damage.damage > 1) {
            LogMessage log;
            log.type = "#JGChiying";
            log.from = zidan;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = objectName();
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(zidan, objectName());

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class JGJingfan: public DistanceSkill {
public:
    JGJingfan(): DistanceSkill("jgjingfan") {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int dist = 0;
        if (!isJianGeFriend(from, to)) {
            foreach (const Player *p, from->getAliveSiblings()) {
                if (p->hasSkill(objectName()) && isJianGeFriend(p, from))
                    dist--;
            }
            return dist;
        }
        return 0;
    }
};

class JGKonghun: public PhaseChangeSkill {
public:
    JGKonghun(): PhaseChangeSkill("jgkonghun") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Play || !target->isWounded()) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> enemies;
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (!isJianGeFriend(p, target))
                enemies << p;
        }

        int enemy_num = enemies.length();
        if (target->getLostHp() >= enemy_num && room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());
            foreach (ServerPlayer *p, enemies)
                room->damage(DamageStruct(objectName(), target, p, 1, DamageStruct::Thunder));
            if (target->isWounded())
                room->recover(target, RecoverStruct(target));
        }
        return false;
    }
};

class JGFanshi: public PhaseChangeSkill {
public:
    JGFanshi(): PhaseChangeSkill("jgfanshi") {
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(target, objectName());

        room->loseHp(target, 1);
        return false;
    }
};

class JGXuanlei: public PhaseChangeSkill {
public:
    JGXuanlei(): PhaseChangeSkill("jgxuanlei") {
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Start) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> enemies;
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (!p->getJudgingArea().isEmpty() && !isJianGeFriend(p, target))
                enemies << p;
        }

        if (!enemies.isEmpty()) {
            room->broadcastSkillInvoke(objectName());
            room->sendCompulsoryTriggerLog(target, objectName());

            foreach (ServerPlayer *p, enemies)
                room->damage(DamageStruct(objectName(), target, p, 1, DamageStruct::Thunder));
        }
        return false;
    }
};

class JGChuanyun: public PhaseChangeSkill {
public:
    JGChuanyun(): PhaseChangeSkill("jgchuanyun") {
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
        ServerPlayer *player = room->askForPlayerChosen(target, players, objectName(), "jgchuanyun-invoke", true, true);
        if (player) {
            room->broadcastSkillInvoke(objectName());
            room->damage(DamageStruct(objectName(), target, player, 1));
        }
        return false;
    }
};

class JGLeili: public TriggerSkill {
public:
    JGLeili(): TriggerSkill("jgleili") {
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isKindOf("Slash")) {
            QList<ServerPlayer *> enemies;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (!isJianGeFriend(p, player) && p != damage.to)
                    enemies << p;
            }
            ServerPlayer *target = room->askForPlayerChosen(player, enemies, objectName(), "jgleili-invoke", true, true);
            if (target) {
                room->broadcastSkillInvoke(objectName());
                room->damage(DamageStruct(objectName(), player, target, 1, DamageStruct::Thunder));
            }
        }
        return false;
    }
};

class JGFengxing: public PhaseChangeSkill {
public:
    JGFengxing(): PhaseChangeSkill("jgfengxing") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Start) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> enemies;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (!isJianGeFriend(p, target) && target->canSlash(p, false))
                enemies << p;
        }
        if (enemies.isEmpty()) return false;

        ServerPlayer *player = room->askForPlayerChosen(target, enemies, objectName(), "jgfengxing-invoke", true);
        if (player) {
            room->broadcastSkillInvoke(objectName());

            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            room->useCard(CardUseStruct(slash, target, player));
        }
        return false;
    }
};

class JGHuodi: public PhaseChangeSkill {
public:
    JGHuodi(): PhaseChangeSkill("jghuodi") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> enemies;
        bool turnedFriend = false;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (isJianGeFriend(p, target)) {
                if (!p->faceUp() && !turnedFriend) turnedFriend = true;
            } else {
                enemies << p;
            }
        }
        if (turnedFriend) {
            ServerPlayer *player = room->askForPlayerChosen(target, enemies, objectName(), "jghuodi-invoke", true);
            if (player) {
                room->broadcastSkillInvoke(objectName());
                player->turnOver();
            }
        }
        return false;
    }
};

class JGJueji: public DrawCardsSkill {
public:
    JGJueji(): DrawCardsSkill("jgjueji") {
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();

        if (!player->isWounded()) return n;
        int reduce = 0;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!isJianGeFriend(p, player) && TriggerSkill::triggerable(p)
                && room->askForSkillInvoke(p, objectName()))
                reduce++;
        }
        return n - reduce;
    }
};

// Offensive Machines

class JGJiguan: public ProhibitSkill {
public:
    JGJiguan(): ProhibitSkill("jgjiguan") {
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const{
        return to->hasSkill(objectName()) && card->isKindOf("Indulgence");
    }
};

class JGTanshi: public DrawCardsSkill {
public:
    JGTanshi(): DrawCardsSkill("jgtanshi") {
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();

        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(player, objectName());

        return n - 1;
    }
};

class JGTunshi: public PhaseChangeSkill {
public:
    JGTunshi(): PhaseChangeSkill("jgtunshi") {
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Start) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> to_damage;
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (p->getHandcardNum() > target->getHandcardNum() && !isJianGeFriend(p, target))
                to_damage << p;
        }

        if (!to_damage.isEmpty()) {
            room->broadcastSkillInvoke(objectName());
            room->sendCompulsoryTriggerLog(target, objectName());

            foreach (ServerPlayer *p, to_damage)
                room->damage(DamageStruct(objectName(), target, p));
        }
        return false;
    }
};

class JGLianyu: public PhaseChangeSkill {
public:
    JGLianyu(): PhaseChangeSkill("jglianyu") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish)
            return false;

        Room *room = target->getRoom();
        if (room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());

            QList<ServerPlayer *> enemies;
            foreach (ServerPlayer *p, room->getAllPlayers()) {
                if (!isJianGeFriend(p, target))
                    enemies << p;
            }
            foreach (ServerPlayer *p, enemies)
                room->damage(DamageStruct(objectName(), target, p, 1, DamageStruct::Fire));
        }
        return false;
    }
};

class JGDidong: public PhaseChangeSkill {
public:
    JGDidong(): PhaseChangeSkill("jgdidong") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> enemies;
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (!isJianGeFriend(p, target))
                enemies << p;
        }
        ServerPlayer *player = room->askForPlayerChosen(target, enemies, objectName(), "jgdidong-invoke", true, true);
        if (player) {
            room->broadcastSkillInvoke(objectName());
            player->turnOver();
        }
        return false;
    }
};

class JGDixian: public PhaseChangeSkill {
public:
    JGDixian(): PhaseChangeSkill("jgdixian") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> enemies;
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (!isJianGeFriend(p, target))
                enemies << p;
        }
        if (room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());
            target->turnOver();
            foreach (ServerPlayer *p, enemies) {
                if (p->isAlive() && !p->getEquips().isEmpty())
                    p->throwAllEquips();
            }
        }
        return false;
    }
};

// SHU Souls

class JGJizhen: public PhaseChangeSkill {
public:
    JGJizhen(): PhaseChangeSkill("jgjizhen") {
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> to_draw;
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (p->isWounded() && isJianGeFriend(p, target))
                to_draw << p;
        }

        if (!to_draw.isEmpty()) {
            room->broadcastSkillInvoke(objectName());
            room->sendCompulsoryTriggerLog(target, objectName());
            room->drawCards(to_draw, 1, objectName());
        }
        return false;
    }
};

class JGLingfeng: public PhaseChangeSkill {
public:
    JGLingfeng(): PhaseChangeSkill("jglingfeng") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Draw) return false;
        Room *room = target->getRoom();
        if (target->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());

            int card1 = room->drawCard();
            int card2 = room->drawCard();
            QList<int> ids;
            ids << card1 << card2;
            bool diff = (Sanguosha->getCard(card1)->getColor() != Sanguosha->getCard(card2)->getColor());

            CardsMoveStruct move;
            move.card_ids = ids;
            move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, target->objectName(), objectName(), QString());
            move.to_place = Player::PlaceTable;
            room->moveCardsAtomic(move, true);
            room->getThread()->delay();

            DummyCard *dummy = new DummyCard(move.card_ids);
            room->obtainCard(target, dummy);
            delete dummy;

            if (diff) {
                QList<ServerPlayer *> enemies;
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (!isJianGeFriend(p, target))
                        enemies << p;
                }
                Q_ASSERT(!enemies.isEmpty());
                ServerPlayer *enemy = room->askForPlayerChosen(target, enemies, objectName(), "@jglingfeng");
                if (enemy)
                    room->loseHp(enemy);
            }
        }
        return true;
    }
};

class JGBiantian: public TriggerSkill {
public:
    JGBiantian(): TriggerSkill("jgbiantian") {
        events << EventPhaseStart << FinishJudge;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart && TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Start) {
            room->broadcastSkillInvoke(objectName());
            room->sendCompulsoryTriggerLog(player, objectName());

            JudgeStruct judge;
            judge.good = true;
            judge.play_animation = false;
            judge.who = player;
            judge.reason = objectName();

            room->judge(judge);

            if (!player->isAlive()) return false;
            player->tag["Qixing_user"] = true;
            Card::Color color = (Card::Color)(judge.pattern.toInt());
            if (color == Card::Red) {
                const TriggerSkill *kuangfeng = Sanguosha->getTriggerSkill("kuangfeng");
                room->getThread()->addTriggerSkill(kuangfeng);
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (!isJianGeFriend(p, player)) {
                        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
                        p->gainMark("@gale");
                    }
                }
            } else if (color == Card::Black) {
                const TriggerSkill *dawu = Sanguosha->getTriggerSkill("dawu");
                room->getThread()->addTriggerSkill(dawu);
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (isJianGeFriend(p, player)) {
                        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());
                        p->gainMark("@fog");
                    }
                }
            }
        } else if (triggerEvent == FinishJudge) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason != objectName()) return false;
            judge->pattern = QString::number(int(judge->card->getColor()));
        }
        return false;
    }
};

class JGGongshen: public PhaseChangeSkill {
public:
    JGGongshen(): PhaseChangeSkill("jggongshen") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        ServerPlayer *offensive_machine = NULL, *defensive_machine = NULL;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->property("jiange_defense_type").toString() == "machine") {
                if (isJianGeFriend(p, target))
                    defensive_machine = p;
                else
                    offensive_machine = p;
            }
        }
        QStringList choicelist;
        if (defensive_machine && defensive_machine->isWounded())
            choicelist << "recover";
        if (offensive_machine)
            choicelist << "damage";
        if (choicelist.isEmpty())
            return false;

        choicelist << "cancel";
        QString choice = room->askForChoice(target, objectName(), choicelist.join("+"));
        if (choice != "cancel") {
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = target;
            log.arg = objectName();
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(target, objectName());

            if (choice == "recover")
                room->recover(defensive_machine, RecoverStruct(target));
            else
                room->damage(DamageStruct(objectName(), target, offensive_machine, 1, DamageStruct::Fire));
        }
        return false;
    }
};

class JGZhinang: public PhaseChangeSkill {
public:
    JGZhinang(): PhaseChangeSkill("jgzhinang") {
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Start) return false;
        Room *room = target->getRoom();

        if (room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());

            QList<int> ids = room->getNCards(3, false);
            CardsMoveStruct move(ids, target, Player::PlaceTable,
                                 CardMoveReason(CardMoveReason::S_REASON_TURNOVER, target->objectName(), objectName(), QString()));
            room->moveCardsAtomic(move, true);

            room->getThread()->delay();
            room->getThread()->delay();

            QList<int> card_to_throw;
            QList<int> card_to_give;
            for (int i = 0; i < 3; i++) {
                if (Sanguosha->getCard(ids[i])->getTypeId() == Card::TypeBasic)
                    card_to_throw << ids[i];
                else
                    card_to_give << ids[i];
            }
            ServerPlayer *togive = NULL;
            if (!card_to_give.isEmpty()) {
                QList<ServerPlayer *> friends;
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (isJianGeFriend(p, target))
                        friends << p;
                }
                togive = room->askForPlayerChosen(target, friends, objectName(), "@jgzhinang");
            }
            if (!card_to_throw.isEmpty()) {
                DummyCard *dummy = new DummyCard(card_to_throw);
                CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, target->objectName(), objectName(), QString());
                room->throwCard(dummy, reason, NULL);
                delete dummy;
            }
            if (togive) {
                DummyCard *dummy2 = new DummyCard(card_to_give);
                room->obtainCard(togive, dummy2);
                delete dummy2;
            }
        }
        return false;
    }
};

class JGJingmiao: public TriggerSkill {
public:
    JGJingmiao(): TriggerSkill("jgjingmiao") {
        frequency = Compulsory;
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Nullification")) {
            foreach (ServerPlayer *p, room->getAllPlayers()) {
                if (!player->isAlive())
                    return false;
                if (TriggerSkill::triggerable(p) && !isJianGeFriend(p, player)) {
                    room->broadcastSkillInvoke(objectName());
                    room->sendCompulsoryTriggerLog(p, objectName());
                    room->loseHp(player);
                }
            }
        }
        return false;
    }
};

class JGYuhuo: public TriggerSkill {
public:
    JGYuhuo(): TriggerSkill("jgyuhuo") {
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.nature == DamageStruct::Fire) {
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(player, objectName());

            LogMessage log;
            log.type = "#JGYuhuoProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = "fire_nature";
            room->sendLog(log);
            return true;
        }
        return false;
    }
};

class JGQiwu: public TriggerSkill {
public:
    JGQiwu(): TriggerSkill("jgqiwu") {
        events << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (player == move.from
            && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
            int count = 0;
            foreach (int id, move.card_ids) {
                if (Sanguosha->getCard(id)->getSuit() == Card::Club)
                    count++;
            }
            if (count > 0) {
                for (int i = 0; i < count; i++) {
                    QList<ServerPlayer *> friends;
                    foreach (ServerPlayer *p, room->getAlivePlayers()) {
                        if (isJianGeFriend(p, player) && p->isWounded())
                            friends << p;
                    }
                    if (friends.isEmpty()) return false;
                    ServerPlayer *rec_friend = room->askForPlayerChosen(player, friends, objectName(), "jgqiwu-invoke", true, true);
                    if (!rec_friend) return false;
                    room->recover(rec_friend, RecoverStruct(player));
                }
            }
        }
        return false;
    }
};

class JGTianyu: public PhaseChangeSkill {
public:
    JGTianyu(): PhaseChangeSkill("jgtianyu") {
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        bool sendLog = false;
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (!p->isChained() && !isJianGeFriend(p, target)) {
                if (!sendLog) {
                    room->broadcastSkillInvoke(objectName());
                    room->sendCompulsoryTriggerLog(target, objectName());
                    sendLog = true;
                }
                p->setChained(true);
                room->broadcastProperty(p, "chained");
                room->setEmotion(p, "chain");
                room->getThread()->trigger(ChainStateChanged, room, p);
            }
        }
        return false;
    }
};

// Defensive Machines

class JGMojian: public PhaseChangeSkill {
public:
    JGMojian(): PhaseChangeSkill("jgmojian") {
        frequency = Compulsory;
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

        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(target, objectName());
        room->useCard(CardUseStruct(aa, target, QList<ServerPlayer *>()));
        return false;
    }
};

class JGMojianProhibit: public ProhibitSkill {
public:
    JGMojianProhibit(): ProhibitSkill("#jgmojian-prohibit") {
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &) const{
        return isJianGeFriend(from, to) && card->isKindOf("ArcheryAttack") && card->getSkillName() == "jgmojian";
    }
};

class JGBenlei: public PhaseChangeSkill {
public:
    JGBenlei(): PhaseChangeSkill("jgbenlei") {
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Start) return false;
        Room *room = target->getRoom();

        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(target, objectName());

        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (!isJianGeFriend(p, target) && p->property("jiange_defense_type").toString() == "machine") {
                room->damage(DamageStruct(objectName(), target, p, 1, DamageStruct::Thunder));
                break;
            }
        }
        return false;
    }
};

class JGLingyu: public PhaseChangeSkill {
public:
    JGLingyu(): PhaseChangeSkill("jglingyu") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        if (room->askForSkillInvoke(target, objectName())) {
            room->broadcastSkillInvoke(objectName());

            target->turnOver();
            foreach (ServerPlayer *p, room->getAllPlayers()) {
                if (p->isWounded() && isJianGeFriend(p, target) && p != target)
                    room->recover(p, RecoverStruct(target));
            }
        }
        return false;
    }
};

class JGTianyun: public PhaseChangeSkill {
public:
    JGTianyun(): PhaseChangeSkill("jgtianyun") {
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();

        QList<ServerPlayer *> enemies;
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (!isJianGeFriend(p, target))
                enemies << p;
        }
        ServerPlayer *player = room->askForPlayerChosen(target, enemies, objectName(), "jgtianyun-invoke", true, true);
        if (player) {
            room->broadcastSkillInvoke(objectName());
            room->loseHp(target);

            room->damage(DamageStruct(objectName(), target, player, 2, DamageStruct::Fire));
            player->throwAllEquips();
        }
        return false;
    }
};

JianGeDefensePackage::JianGeDefensePackage()
    : Package("JianGeDefense")
{
    typedef General Soul;
    typedef General Machine;

    Soul *jg_soul_caozhen = new Soul(this, "jg_soul_caozhen", "wei", 5, true, true);
    jg_soul_caozhen->addSkill(new JGChiying);
    jg_soul_caozhen->addSkill(new JGJingfan);

    Soul *jg_soul_simayi = new Soul(this, "jg_soul_simayi", "wei", 5, true, true);
    jg_soul_simayi->addSkill(new JGKonghun);
    jg_soul_simayi->addSkill(new JGFanshi);
    jg_soul_simayi->addSkill(new JGXuanlei);

    Soul *jg_soul_xiahouyuan = new Soul(this, "jg_soul_xiahouyuan", "wei", 4, true, true);
    jg_soul_xiahouyuan->addSkill(new JGChuanyun);
    jg_soul_xiahouyuan->addSkill(new JGLeili);
    jg_soul_xiahouyuan->addSkill(new JGFengxing);

    Soul *jg_soul_zhanghe = new Soul(this, "jg_soul_zhanghe", "wei", 4, true, true);
    jg_soul_zhanghe->addSkill(new JGHuodi);
    jg_soul_zhanghe->addSkill(new JGJueji);

    Machine *jg_machine_tuntianchiwen = new Machine(this, "jg_machine_tuntianchiwen", "wei", 5, true, true);
    jg_machine_tuntianchiwen->addSkill(new JGJiguan);
    jg_machine_tuntianchiwen->addSkill(new JGTanshi);
    jg_machine_tuntianchiwen->addSkill(new JGTunshi);

    Machine *jg_machine_shihuosuanni = new Machine(this, "jg_machine_shihuosuanni", "wei", 3, true, true);
    jg_machine_shihuosuanni->addSkill("jgjiguan");
    jg_machine_shihuosuanni->addSkill(new JGLianyu);

    Machine *jg_machine_fudibian = new Machine(this, "jg_machine_fudibian", "wei", 4, true, true);
    jg_machine_fudibian->addSkill("jgjiguan");
    jg_machine_fudibian->addSkill(new JGDidong);

    Machine *jg_machine_lieshiyazi = new Machine(this, "jg_machine_lieshiyazi", "wei", 4, true, true);
    jg_machine_lieshiyazi->addSkill("jgjiguan");
    jg_machine_lieshiyazi->addSkill(new JGDixian);

    Soul *jg_soul_liubei = new Soul(this, "jg_soul_liubei", "shu", 5, true, true);
    jg_soul_liubei->addSkill(new JGJizhen);
    jg_soul_liubei->addSkill(new JGLingfeng);

    Soul *jg_soul_zhugeliang = new Soul(this, "jg_soul_zhugeliang", "shu", 4, true, true);
    jg_soul_zhugeliang->addSkill(new JGBiantian);
    jg_soul_zhugeliang->addSkill("bazhen");
    related_skills.insertMulti("jgbiantian", "#qixing-clear");

    Soul *jg_soul_huangyueying = new Soul(this, "jg_soul_huangyueying", "shu", 4, false, true);
    jg_soul_huangyueying->addSkill(new JGGongshen);
    jg_soul_huangyueying->addSkill(new JGZhinang);
    jg_soul_huangyueying->addSkill(new JGJingmiao);

    Soul *jg_soul_pangtong = new Soul(this, "jg_soul_pangtong", "shu", 4, true, true);
    jg_soul_pangtong->addSkill(new JGYuhuo);
    jg_soul_pangtong->addSkill(new JGQiwu);
    jg_soul_pangtong->addSkill(new JGTianyu);

    Machine *jg_machine_yunpingqinglong = new Machine(this, "jg_machine_yunpingqinglong", "shu", 4, true, true);
    jg_machine_yunpingqinglong->addSkill("jgjiguan");
    jg_machine_yunpingqinglong->addSkill(new JGMojian);
    jg_machine_yunpingqinglong->addSkill(new JGMojianProhibit);
    related_skills.insertMulti("jgmojian", "#jgmojian-prohibit");

    Machine *jg_machine_jileibaihu = new Machine(this, "jg_machine_jileibaihu", "shu", 4, true, true);
    jg_machine_jileibaihu->addSkill("jgjiguan");
    jg_machine_jileibaihu->addSkill("zhenwei");
    jg_machine_jileibaihu->addSkill(new JGBenlei);

    Machine *jg_machine_lingjiaxuanwu = new Machine(this, "jg_machine_lingjiaxuanwu", "shu", 5, true, true);
    jg_machine_lingjiaxuanwu->addSkill("jgjiguan");
    jg_machine_lingjiaxuanwu->addSkill("yizhong");
    jg_machine_lingjiaxuanwu->addSkill(new JGLingyu);

    Machine *jg_machine_chiyuzhuque = new Machine(this, "jg_machine_chiyuzhuque", "shu", 5, true, true);
    jg_machine_chiyuzhuque->addSkill("jgjiguan");
    jg_machine_chiyuzhuque->addSkill("jgyuhuo");
    jg_machine_chiyuzhuque->addSkill(new JGTianyun);
}

ADD_PACKAGE(JianGeDefense)
