#include "yjcm2015.h"
#include "general.h"
#include "player.h"
#include "structs.h"
#include "room.h"
#include "skill.h"
#include "standard.h"
#include "engine.h"
#include "clientplayer.h"
#include "clientstruct.h"
#include "settings.h"
#include "wrapped-card.h"
#include "roomthread.h"
#include "standard-equips.h"
#include "standard-skillcards.h"
#include "json.h"

class Huituo : public MasochismSkill
{
public:
    Huituo() : MasochismSkill("huituo")
    {

    }

    void onDamaged(ServerPlayer *target, const DamageStruct &damage) const
    {
        Room *room = target->getRoom();
        JudgeStruct j;

        j.who = room->askForPlayerChosen(target, room->getAlivePlayers(), objectName(), "@huituo-select", true, true);
        if (j.who == NULL)
            return;

        j.pattern = ".";
        j.play_animation = false;
        j.reason = "huituo";
        room->judge(j);

        if (j.pattern == "red")
            room->recover(j.who, RecoverStruct(target));
        else if (j.pattern == "black")
            room->drawCards(j.who, damage.damage, objectName());
    }
};

class HuituoJudge : public TriggerSkill
{
public:
    HuituoJudge() : TriggerSkill("#huituo")
    {
        events << FinishJudge;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent, Room *, ServerPlayer *, QVariant &data) const
    {
        JudgeStruct *j = data.value<JudgeStruct *>();
        if (j->reason == "huituo")
            j->pattern = j->card->isRed() ? "red" : (j->card->isBlack() ? "black" : "no_suit");

        return false;
    }
};

class Mingjian : public TriggerSkill
{
public:
    Mingjian() : TriggerSkill("mingjian")
    {
        events << EventPhaseChanging;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::Play)
            return false;

        if (player->isSkipped(Player::Play))
            return false;

        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@mingjian-give", true, true);
        if (target == NULL)
            return false;

        CardMoveReason r(CardMoveReason::S_REASON_GIVE, player->objectName(), target->objectName(), objectName(), QString());
        DummyCard d(player->handCards());
        room->obtainCard(target, &d, r, false);

        player->tag["mingjian"] = QVariant::fromValue(target);
        throw TurnBroken;

        return false;
    }
};

class MingjianGive : public PhaseChangeSkill
{
public:
    MingjianGive() : PhaseChangeSkill("#mingjian-give")
    {
        
    }

    int getPriority(TriggerEvent) const
    {
        return -1;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->getPhase() == Player::NotActive && target->tag.contains("mingjian");
    }

    bool onPhaseChange(ServerPlayer *target) const
    {
        ServerPlayer *p = target->tag.value("mingjian").value<ServerPlayer *>();
        target->tag.remove("mingjian");

        if (p == NULL)
            return false;

        QList<Player::Phase> phase;
        phase << Player::Play;

        p->play(phase);

        return false;
    }
};

class Xingshuai : public TriggerSkill
{
public:
    Xingshuai() : TriggerSkill("xingshuai$")
    {
        events << Dying;
        limit_mark = "@xingshuai";
        frequency = Limited;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->hasLordSkill(this) && target->getMark("xingshuai_act") == 0 && hasWeiGens(target);
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who != player)
            return false;


        if (player->askForSkillInvoke(this, data)) {
            if (!player->isLord() && player->hasSkill("weidi")) {
                room->broadcastSkillInvoke("weidi");
                QString generalName = "yuanshu";
                if (player->getGeneralName() == "tw_yuanshu" || (player->getGeneral2() != NULL && player->getGeneral2Name() == "tw_yuanshu"))
                    generalName = "tw_yuanshu";

                room->doSuperLightbox(generalName, "xingshuai");
            } else {
                room->broadcastSkillInvoke(objectName());
                room->doSuperLightbox("caorui", "xingshuai");
            }

            room->setPlayerMark(player, limit_mark, 0);
            player->setMark("xingshuai_act", 1);

            QList<ServerPlayer *> weis = room->getLieges("wei", player);
            QList<ServerPlayer *> invokes;

            room->sortByActionOrder(weis);
            foreach (ServerPlayer *wei, weis) {
                if (wei->askForSkillInvoke("_xingshuai", "xing")) {
                    invokes << wei;
                    room->recover(player, RecoverStruct(wei));
                }
            }

            room->sortByActionOrder(invokes);
            foreach (ServerPlayer *wei, invokes)
                room->damage(DamageStruct(objectName(), NULL, wei));
        }

        return false;
    }

private:
    static bool hasWeiGens(const Player *lord)
    {
        QList<const Player *> sib = lord->getAliveSiblings();
        foreach (const Player *p, sib) {
            if (p->getKingdom() == "wei")
                return true;
        }

        return false;
    }
};

class Taoxi : public TriggerSkill
{
public:
    Taoxi() : TriggerSkill("taoxi")
    {
        events << TargetSpecified << CardsMoveOneTime << EventPhaseChanging;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == TargetSpecified && TriggerSkill::triggerable(player)
            && !player->hasFlag("TaoxiUsed") && player->getPhase() == Player::Play) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card && use.card->getTypeId() != Card::TypeSkill && use.to.length() == 1) {
                ServerPlayer *to = use.to.first();
                player->tag["taoxi_carduse"] = data;
                if (to != player && !to->isKongcheng() && player->askForSkillInvoke(objectName(), QVariant::fromValue(to))) {
                    room->setPlayerFlag(player, "TaoxiUsed");
                    room->setPlayerFlag(player, "TaoxiRecord");
                    int id = room->askForCardChosen(player, to, "h", objectName(), false);
                    room->showCard(to, id);
                    TaoxiMove(id, true, player);
                    player->tag["TaoxiId"] = id;
                }
            }
        } else if (triggerEvent == CardsMoveOneTime && player->hasFlag("TaoxiRecord")) {
            bool ok = false;
            int id = player->tag["TaoxiId"].toInt(&ok);
            if (!ok) {
                room->setPlayerFlag(player, "-TaoxiRecord");
                return false;
            }
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != NULL && move.card_ids.contains(id)) {
                if (move.from_places[move.card_ids.indexOf(id)] == Player::PlaceHand) {
                    TaoxiMove(id, false, player);
                    if (room->getCardOwner(id) != NULL)
                        room->showCard(room->getCardOwner(id), id);
                    room->setPlayerFlag(player, "-TaoxiRecord");
                    player->tag.remove("TaoxiId");
                }
            }
        } else if (triggerEvent == EventPhaseChanging && player->hasFlag("TaoxiRecord")) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
            bool ok = false;
            int id = player->tag["TaoxiId"].toInt(&ok);
            if (!ok) {
                room->setPlayerFlag(player, "-TaoxiRecord");
                return false;
            }

            if (TaoxiHere(player))
                TaoxiMove(id, false, player);

            ServerPlayer *owner = room->getCardOwner(id);
            if (owner && room->getCardPlace(id) == Player::PlaceHand) {
                room->sendCompulsoryTriggerLog(player, objectName());
                room->showCard(owner, id);
                room->loseHp(player);
                room->setPlayerFlag(player, "-TaoxiRecord");
                player->tag.remove("TaoxiId");
            }
        }
        return false;
    }

private:
    static void TaoxiMove(int id, bool movein, ServerPlayer *caoxiu)
    {
        Room *room = caoxiu->getRoom();
        if (movein) {
            CardsMoveStruct move(id, NULL, caoxiu, Player::PlaceTable, Player::PlaceSpecial,
                CardMoveReason(CardMoveReason::S_REASON_PUT, caoxiu->objectName(), "taoxi", QString()));
            move.to_pile_name = "&taoxi";
            QList<CardsMoveStruct> moves;
            moves.append(move);
            QList<ServerPlayer *> _caoxiu;
            _caoxiu << caoxiu;
            room->notifyMoveCards(true, moves, false, _caoxiu);
            room->notifyMoveCards(false, moves, false, _caoxiu);
        } else {
            CardsMoveStruct move(id, caoxiu, NULL, Player::PlaceSpecial, Player::PlaceTable,
                CardMoveReason(CardMoveReason::S_REASON_PUT, caoxiu->objectName(), "taoxi", QString()));
            move.from_pile_name = "&taoxi";
            QList<CardsMoveStruct> moves;
            moves.append(move);
            QList<ServerPlayer *> _caoxiu;
            _caoxiu << caoxiu;
            room->notifyMoveCards(true, moves, false, _caoxiu);
            room->notifyMoveCards(false, moves, false, _caoxiu);
        }
        caoxiu->tag["TaoxiHere"] = movein;
    }

    static bool TaoxiHere(ServerPlayer *caoxiu)
    {
        return caoxiu->tag.value("TaoxiHere", false).toBool();
    }
};

HuaiyiCard::HuaiyiCard()
{
    target_fixed = true;
}

void HuaiyiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    room->showAllCards(source);

    QList<int> blacks;
    QList<int> reds;
    foreach (const Card *c, source->getHandcards()) {
        if (c->isRed())
            reds << c->getId();
        else
            blacks << c->getId();
    }

    if (reds.isEmpty() || blacks.isEmpty())
        return;

    QString to_discard = room->askForChoice(source, "huaiyi", "black+red");
    QList<int> *pile = NULL;
    if (to_discard == "black")
        pile = &blacks;
    else
        pile = &reds;

    int n = pile->length();

    room->setPlayerMark(source, "huaiyi_num", n);

    DummyCard dm(*pile);
    room->throwCard(&dm, source);

    room->askForUseCard(source, "@@huaiyi", "@huaiyi:::" + QString::number(n), -1, Card::MethodNone);
}

HuaiyiSnatchCard::HuaiyiSnatchCard()
{
    handling_method = Card::MethodNone;
    m_skillName = "_huaiyi";
}

bool HuaiyiSnatchCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    int n = Self->getMark("huaiyi_num");
    if (targets.length() >= n)
        return false;

    if (to_select == Self)
        return false;

    if (to_select->isNude())
        return false;

    return true;
}

void HuaiyiSnatchCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    ServerPlayer *player = card_use.from;

    QList<ServerPlayer *> to = card_use.to;

    room->sortByActionOrder(to);

    foreach (ServerPlayer *p, to) {
        int id = room->askForCardChosen(player, p, "he", "huaiyi");
        player->obtainCard(Sanguosha->getCard(id), false);
    }

    if (to.length() >= 2)
        room->loseHp(player);
}

class Huaiyi : public ZeroCardViewAsSkill
{
public:
    Huaiyi() : ZeroCardViewAsSkill("huaiyi")
    {

    }

    const Card *viewAs() const
    {
        if (Sanguosha->currentRoomState()->getCurrentCardUsePattern() == "@@huaiyi")
            return new HuaiyiSnatchCard;
        else
            return new HuaiyiCard;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("HuaiyiCard");
    }

    bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "@@huaiyi";
    }
};

class Jigong : public PhaseChangeSkill
{
public:
    Jigong() : PhaseChangeSkill("jigong")
    {

    }

    bool triggerable(const ServerPlayer *target) const
    {
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Play;
    }

    bool onPhaseChange(ServerPlayer *target) const
    {
        if (target->askForSkillInvoke(this)) {
            target->drawCards(2, "jigong");
            target->getRoom()->setPlayerFlag(target, "jigong");
        }

        return false;
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

class Shifei : public TriggerSkill
{
public:
    Shifei() : TriggerSkill("shifei")
    {
        events << CardAsked;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        QStringList ask = data.toStringList();
        if (ask.first() != "jink")
            return false;

        ServerPlayer *current = room->getCurrent();
        if (current == NULL || current->isDead() || current->getPhase() == Player::NotActive)
            return false;

        if (player->askForSkillInvoke(this)) {
            current->drawCards(1, objectName());

            QList<ServerPlayer *> mosts;
            int most = -1;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                int h = p->getHandcardNum();
                if (h > most) {
                    mosts.clear();
                    most = h;
                    mosts << p;
                } else if (most == h)
                    mosts << p;
            }
            if (most < 0 || mosts.contains(current))
                return false;

            QList<ServerPlayer *> mosts_copy = mosts;
            foreach (ServerPlayer *p, mosts_copy) {
                if (!player->canDiscard(p, "he"))
                    mosts.removeOne(p);
            }

            if (mosts.isEmpty())
                return false;

            ServerPlayer *vic = room->askForPlayerChosen(player, mosts, objectName(), "@shifei-dis");
            // it is impossible that vic == NULL
            if (vic == player)
                room->askForDiscard(player, objectName(), 1, 1, false, true);
            else {
                int id = room->askForCardChosen(player, vic, "he", objectName(), false, Card::MethodDiscard);
                room->throwCard(id, vic, player);
            }
                
            Jink *jink = new Jink(Card::NoSuit, 0);
            jink->setSkillName("_shifei");
            room->provide(jink);
            return true;
        }

        return false;
    }
};

class ZhanjueVS : public ZeroCardViewAsSkill
{
public:
    ZhanjueVS() : ZeroCardViewAsSkill("zhanjue")
    {

    }

    const Card *viewAs() const
    {
        Duel *duel = new Duel(Card::SuitToBeDecided, -1);
        duel->addSubcards(Self->getHandcards());
        duel->setSkillName("zhanjue");
        return duel;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("zhanjuedraw") < 2 && !player->isKongcheng();
    }
};

class Zhanjue : public TriggerSkill
{
public:
    Zhanjue() : TriggerSkill("zhanjue")
    {
        view_as_skill = new ZhanjueVS;
        events << CardFinished << PreDamageDone << EventPhaseChanging;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == PreDamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card != NULL && damage.card->isKindOf("Duel") && damage.card->getSkillName() == "zhanjue" && damage.from != NULL) {
                QVariantMap m = room->getTag("zhanjue").toMap();
                QVariantList l = m.value(damage.card->toString(), QVariantList()).toList();
                l << QVariant::fromValue(damage.to);
                m[damage.card->toString()] = l;
                room->setTag("zhanjue", m);
            }
        } else if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->isKindOf("Duel") && use.card->getSkillName() == "zhanjue") {
                QVariantMap m = room->getTag("zhanjue").toMap();
                QVariantList l = m.value(use.card->toString(), QVariantList()).toList();
                if (!l.isEmpty()) {
                    QList<ServerPlayer *> l_copy;
                    foreach (const QVariant &s, l)
                        l_copy << s.value<ServerPlayer *>();
                    l_copy << use.from;
                    int n = l_copy.count(use.from);
                    room->addPlayerMark(use.from, "zhanjuedraw", n);
                    room->sortByActionOrder(l_copy);
                    room->drawCards(l_copy, 1, objectName());
                }
                m.remove(use.card->toString());
                room->setTag("zhanjue", m);
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive)
                room->setPlayerMark(player, "zhanjuedraw", 0);
        }
        return false;
    }
};

QinwangCard::QinwangCard()
{

}

bool QinwangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    Slash *slash = new Slash(NoSuit, 0);
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

const Card *QinwangCard::validate(CardUseStruct &cardUse) const
{
    cardUse.from->getRoom()->throwCard(cardUse.card, cardUse.from);

    JijiangCard jj;
    cardUse.from->setFlags("qinwangjijiang");
    try {
        const Card *vs = jj.validate(cardUse);
        if (cardUse.from->hasFlag("qinwangjijiang"))
            cardUse.from->setFlags("-qinwangjijiang");

        return vs;
    }
    catch (TriggerEvent e) {
        if (e == TurnBroken || e == StageChange)
            cardUse.from->setFlags("-qinwangjijiang");

        throw e;
    }

    return NULL;
}

class QinwangVS : public OneCardViewAsSkill
{
public:
    QinwangVS() : OneCardViewAsSkill("qinwang$")
    {
        filter_pattern = ".!";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        JijiangViewAsSkill jj;
        return jj.isEnabledAtPlay(player);
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        JijiangViewAsSkill jj;
        return jj.isEnabledAtResponse(player, pattern);
    }

    const Card *viewAs(const Card *originalCard) const
    {
        QinwangCard *qw = new QinwangCard;
        qw->addSubcard(originalCard);
        return qw;
    }
};

class Qinwang : public TriggerSkill
{
public:
    Qinwang() : TriggerSkill("qinwang$")
    {
        view_as_skill = new QinwangVS;
        events << CardAsked;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->hasLordSkill("qinwang");
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        const TriggerSkill *jj = Sanguosha->getTriggerSkill("jijiang");
        if (jj == NULL)
            return false;

        QString pattern = data.toStringList().first();
        QString prompt = data.toStringList().at(1);
        if (pattern != "slash" || prompt.startsWith("@jijiang-slash"))
            return false;

        QList<ServerPlayer *> lieges = room->getLieges("shu", player);
        if (lieges.isEmpty())
            return false;

        if (!room->askForCard(player, "..", "@qinwang-discard", data, "qinwang"))
            return false;

        player->setFlags("qinwangjijiang");
        try {
            bool t = jj->trigger(triggerEvent, room, player, data);
            if (player->hasFlag("qinwangjijiang"))
                player->setFlags("-qinwangjijiang");

            return t;
        }
        catch (TriggerEvent e) {
            if (e == TurnBroken || e == StageChange)
                player->setFlags("-qinwangjijiang");

            throw e;
        }

        return false;
    }
};

class QinwangDraw : public TriggerSkill
{
public:
    QinwangDraw() : TriggerSkill("qinwang-draw")
    {
        events << CardResponded;
        global = true;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &data) const
    {
        CardResponseStruct resp = data.value<CardResponseStruct>();
        if (resp.m_card->isKindOf("Slash") && !resp.m_isUse && resp.m_who->hasFlag("qinwangjijiang")) {
            resp.m_who->setFlags("-qinwangjijiang");
            player->drawCards(1, "qinwang");
        }

        return false;
    }
};

ZhenshanCard::ZhenshanCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool ZhenshanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
    }

    const Card *_card = Self->tag.value("zhenshan").value<const Card *>();
    if (_card == NULL)
        return false;

    Card *card = Sanguosha->cloneCard(_card->objectName(), Card::NoSuit, 0);
    card->setCanRecast(false);
    card->deleteLater();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool ZhenshanCard::targetFixed() const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFixed();
    }

    const Card *_card = Self->tag.value("zhenshan").value<const Card *>();
    if (_card == NULL)
        return false;

    Card *card = Sanguosha->cloneCard(_card->objectName(), Card::NoSuit, 0);
    card->setCanRecast(false);
    card->deleteLater();
    return card && card->targetFixed();
}

bool ZhenshanCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetsFeasible(targets, Self);
    }

    const Card *_card = Self->tag.value("zhenshan").value<const Card *>();
    if (_card == NULL)
        return false;

    Card *card = Sanguosha->cloneCard(_card->objectName(), Card::NoSuit, 0);
    card->setCanRecast(false);
    card->deleteLater();
    return card && card->targetsFeasible(targets, Self);
}

const Card *ZhenshanCard::validate(CardUseStruct &card_use) const
{
    ServerPlayer *quancong = card_use.from;
    Room *room = quancong->getRoom();

    QString user_str = user_string;
    if (user_string == "slash" && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        QStringList use_list;
        use_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            use_list << "thunder_slash" << "fire_slash";
        user_str = room->askForChoice(quancong, "zhenshan_slash", use_list.join("+"));
    }

    askForExchangeHand(quancong);

    Card *c = Sanguosha->cloneCard(user_str, Card::NoSuit, 0);
    c->setSkillName("zhenshan");
    c->deleteLater();
    return c;
}

const Card *ZhenshanCard::validateInResponse(ServerPlayer *quancong) const
{
    Room *room = quancong->getRoom();

    QString user_str = user_string;
    if (user_string == "peach+analeptic") {
        QStringList use_list;
        use_list << "peach";
        if (!Config.BanPackages.contains("maneuvering"))
            use_list << "analeptic";
        user_str = room->askForChoice(quancong, "zhenshan_saveself", use_list.join("+"));
    } else if (user_string == "slash") {
        QStringList use_list;
        use_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            use_list << "thunder_slash" << "fire_slash";
        user_str = room->askForChoice(quancong, "zhenshan_slash", use_list.join("+"));
    } else
        user_str = user_string;

    askForExchangeHand(quancong);

    Card *c = Sanguosha->cloneCard(user_str, Card::NoSuit, 0);
    c->setSkillName("zhenshan");
    c->deleteLater();
    return c;
}

void ZhenshanCard::askForExchangeHand(ServerPlayer *quancong)
{
    Room *room = quancong->getRoom();
    QList<ServerPlayer *> targets;
    foreach (ServerPlayer *p, room->getOtherPlayers(quancong)) {
        if (quancong->getHandcardNum() > p->getHandcardNum())
            targets << p;
    }
    ServerPlayer *target = room->askForPlayerChosen(quancong, targets, "zhenshan", "@zhenshan");
    QList<CardsMoveStruct> moves;
    if (!quancong->isKongcheng()) {
        CardMoveReason reason(CardMoveReason::S_REASON_SWAP, quancong->objectName(), target->objectName(), "zhenshan", QString());
        CardsMoveStruct move(quancong->handCards(), target, Player::PlaceHand, reason);
        moves << move;
    }
    if (!target->isKongcheng()) {
        CardMoveReason reason(CardMoveReason::S_REASON_SWAP, target->objectName(), quancong->objectName(), "zhenshan", QString());
        CardsMoveStruct move(target->handCards(), quancong, Player::PlaceHand, reason);
        moves << move;
    }
    if (!moves.isEmpty())
        room->moveCardsAtomic(moves, false);

    room->setPlayerFlag(quancong, "ZhenshanUsed");
}

class ZhenshanVS : public ZeroCardViewAsSkill
{
public:
    ZhenshanVS() : ZeroCardViewAsSkill("zhenshan")
    {
    }

    const Card *viewAs() const
    {
        QString pattern;
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            const Card *c = Self->tag["zhenshan"].value<const Card *>();
            if (c == NULL)
                return NULL;
            pattern = c->objectName();
        } else {
            pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
            if (pattern == "peach+analeptic" && Self->getMark("Global_PreventPeach") > 0)
                pattern = "analeptic";
        }

        ZhenshanCard *zs = new ZhenshanCard;
        zs->setUserString(pattern);
        return zs;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return canExchange(player);
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        if (!canExchange(player))
            return false;
        if (pattern == "peach")
            return player->getMark("Global_PreventPeach") == 0;
        if (pattern == "slash")
            return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
        if (pattern.contains("analeptic"))
            return true;
        return false;
    }

    static bool canExchange(const Player *player)
    {
        if (player->hasFlag("ZhenshanUsed"))
            return false;
        bool current = player->getPhase() != Player::NotActive, less_hand = false;
        foreach(const Player *p, player->getAliveSiblings()) {
            if (p->getPhase() != Player::NotActive)
                current = true;
            if (player->getHandcardNum() > p->getHandcardNum())
                less_hand = true;
            if (current && less_hand)
                return true;
        }
        return false;
    }
};

class Zhenshan : public TriggerSkill
{
public:
    Zhenshan() : TriggerSkill("zhenshan")
    {
        view_as_skill = new ZhenshanVS;
        events << EventPhaseChanging << CardAsked;
    }

    QDialog *getDialog() const
    {
        return GuhuoDialog::getInstance("zhenshan", true, false);
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;

            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasFlag("ZhenshanUsed"))
                    room->setPlayerFlag(p, "-ZhenshanUsed");
            }
        } else if (triggerEvent == CardAsked && TriggerSkill::triggerable(player)) {
            QString pattern = data.toStringList().first();
            if (ZhenshanVS::canExchange(player) && (pattern == "slash" || pattern == "jink")
                && player->askForSkillInvoke(objectName(), data)) {
                ZhenshanCard::askForExchangeHand(player);
                room->setPlayerFlag(player, "ZhenshanUsed");
                Card *card = Sanguosha->cloneCard(pattern);
                card->setSkillName(objectName());
                room->provide(card);
                return true;
            }
        }
        return false;
    }
};

YanzhuCard::YanzhuCard()
{

}

bool YanzhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select != Self && !to_select->isNude();
}

void YanzhuCard::onEffect(const CardEffectStruct &effect) const
{
    ServerPlayer *target = effect.to;
    Room *r = target->getRoom();

    if (!r->askForDiscard(target, "yanzhu", 1, 1, !target->getEquips().isEmpty(), true, "@yanzhu-discard")) {
        if (!target->getEquips().isEmpty()) {
            DummyCard dummy;
            dummy.addSubcards(target->getEquips());
            r->obtainCard(effect.from, &dummy);
        }

        if (effect.from->hasSkill("yanzhu", true)) {
            r->setPlayerMark(effect.from, "yanzhu_lost", 1);
            r->handleAcquireDetachSkills(effect.from, "-yanzhu");
        }
    }
}

class Yanzhu : public ZeroCardViewAsSkill
{
public:
    Yanzhu() : ZeroCardViewAsSkill("yanzhu")
    {

    }

    const Card *viewAs() const
    {
        return new YanzhuCard;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("YanzhuCard");
    }
};

/*
class YanzhuTrig : public TriggerSkill
{
public:
    YanzhuTrig() : TriggerSkill("yanzhu")
    {
        events << EventLoseSkill;
        view_as_skill = new Yanzhu;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (data.toString() == "yanzhu")
            room->setPlayerMark(player, "yanzhu_lost", 1);

        return false;
    }
};
*/

XingxueCard::XingxueCard()
{

}

bool XingxueCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *Self) const
{
    int n = Self->getMark("yanzhu_lost") == 0 ? Self->getHp() : Self->getMaxHp();

    return targets.length() < n;
}

void XingxueCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const
{
    room->drawCards(targets, 1, "xingxue");

    foreach (ServerPlayer *t, targets) {
        if (t->isAlive() && !t->isNude()) {
            const Card *c = room->askForExchange(t, "xingxue", 1, 1, true, "@xingxue-put");
            int id = c->getSubcards().first();
            delete c;

            CardsMoveStruct m(id, NULL, Player::DrawPile, CardMoveReason(CardMoveReason::S_REASON_PUT, t->objectName()));
            room->setPlayerFlag(t, "Global_GongxinOperator");
            room->moveCardsAtomic(m, false);
            room->setPlayerFlag(t, "-Global_GongxinOperator");
        }
    }
}

class XingxueVS : public ZeroCardViewAsSkill
{
public:
    XingxueVS() : ZeroCardViewAsSkill("xingxue")
    {
        response_pattern = "@@xingxue";
    }

    const Card *viewAs() const
    {
        return new XingxueCard;
    }
};

class Xingxue : public PhaseChangeSkill
{
public:
    Xingxue() : PhaseChangeSkill("xingxue")
    {
        view_as_skill = new XingxueVS;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Finish;
    }

    bool onPhaseChange(ServerPlayer *target) const
    {
        target->getRoom()->askForUseCard(target, "@@xingxue", "@xingxue");
        return false;
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

    xiahou->addMark("yjyanyu");
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
        int recastNum = player->getMark("yjyanyu");
        player->setMark("yjyanyu", 0);

        if (recastNum < 2)
            return false;

        QList<ServerPlayer *> malelist;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->isMale())
                malelist << p;
        }

        if (malelist.isEmpty())
            return false;

        ServerPlayer *male = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@yjyanyu-give", true);

        if (male != NULL)
            male->drawCards(2, objectName());

        return false;
    }
};

FurongCard::FurongCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool FurongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (targets.length() > 0 || to_select == Self)
        return false;
    return !to_select->isKongcheng();
}

void FurongCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();

    const Card *c = room->askForExchange(effect.to, "furong", 1, 1, false, "@furong-show");

    room->showCard(effect.from, subcards.first());
    room->showCard(effect.to, c->getSubcards().first());

    const Card *card1 = Sanguosha->getCard(subcards.first());
    const Card *card2 = Sanguosha->getCard(c->getSubcards().first());

    if (card1->isKindOf("Slash") && !card2->isKindOf("Jink")) {
        room->throwCard(this, effect.from);
        room->damage(DamageStruct(objectName(), effect.from, effect.to));
    } else if (!card1->isKindOf("Slash") && card2->isKindOf("Jink")) {
        room->throwCard(this, effect.from);
        if (!effect.to->isNude()) {
            int id = room->askForCardChosen(effect.from, effect.to, "he", objectName());
            room->obtainCard(effect.from, id, false);
        }
    }

    delete c;
}

class Furong : public OneCardViewAsSkill
{
public:
    Furong() : OneCardViewAsSkill("furong")
    {
        filter_pattern = ".|.|.|hand";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        FurongCard *fr = new FurongCard;
        fr->addSubcard(originalCard);
        return fr;
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
    if (c->isKindOf("Slash"))
        classname = "Slash";

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
    } else if (user_string == "slash") {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list = QStringList() << "normal_slash" << "thunder_slash" << "fire_slash";
        to_guhuo = room->askForChoice(zhongyao, "huomo_slash", guhuo_list.join("+"));
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
        if (!(use.card != NULL && !use.card->isKindOf("SkillCard") && use.card->getSuit() == Card::Spade && !use.to.isEmpty()))
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

AnguoCard::AnguoCard()
{

}

bool AnguoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty() || to_select == Self)
        return false;

    return !to_select->getEquips().isEmpty();
}

void AnguoCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    int beforen = 0;
    foreach (ServerPlayer *p, room->getAlivePlayers()) {
        if (effect.to->inMyAttackRange(p))
            beforen++;
    }

    int id = room->askForCardChosen(effect.from, effect.to, "e", "anguo");
    effect.to->obtainCard(Sanguosha->getCard(id));

    int aftern = 0;
    foreach (ServerPlayer *p, room->getAlivePlayers()) {
        if (effect.to->inMyAttackRange(p))
            aftern++;
    }

    if (aftern < beforen)
        effect.from->drawCards(1, "anguo");
}

class Anguo : public ZeroCardViewAsSkill
{
public:
    Anguo() : ZeroCardViewAsSkill("anguo")
    {

    }

    const Card *viewAs() const
    {
        return new AnguoCard;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("AnguoCard");
    }
};

YJCM2015Package::YJCM2015Package()
    : Package("YJCM2015")
{

    General *caorui = new General(this, "caorui$", "wei", 3);
    caorui->addSkill(new Huituo);
    caorui->addSkill(new HuituoJudge);
    related_skills.insertMulti("huituo", "#huituo");
    caorui->addSkill(new Mingjian);
    caorui->addSkill(new MingjianGive);
    related_skills.insertMulti("mingjian", "#mingjian-give");
    caorui->addSkill(new Xingshuai);

    General *caoxiu = new General(this, "caoxiu", "wei");
    caoxiu->addSkill(new Taoxi);

    General *gongsun = new General(this, "gongsunyuan", "qun");
    gongsun->addSkill(new Huaiyi);

    General *guofeng = new General(this, "guotufengji", "qun", 3);
    guofeng->addSkill(new Jigong);
    guofeng->addSkill(new JigongMax);
    related_skills.insertMulti("jigong", "#jigong");
    guofeng->addSkill(new Shifei);

    General *liuchen = new General(this, "liuchen$", "shu");
    liuchen->addSkill(new Zhanjue);
    liuchen->addSkill(new Qinwang);

    General *quancong = new General(this, "quancong", "wu");
    quancong->addSkill(new Zhenshan);

    General *sunxiu = new General(this, "sunxiu$", "wu", 3);
    sunxiu->addSkill(new Yanzhu);
    sunxiu->addSkill(new Xingxue);
    sunxiu->addSkill(new Skill("zhaofu$", Skill::Compulsory));

    General *xiahou = new General(this, "yj_xiahoushi", "shu", 3, false);
    xiahou->addSkill(new Qiaoshi);
    xiahou->addSkill(new YjYanyu);

    General *zhangyi = new General(this, "zhangyi", "shu", 4);
    zhangyi->addSkill(new Furong);
    zhangyi->addSkill(new Shizhi);
    zhangyi->addSkill(new ShizhiFilter);
    related_skills.insertMulti("shizhi", "#shizhi");

    General *zhongyao = new General(this, "zhongyao", "wei", 3);
    zhongyao->addSkill(new Huomo);
    zhongyao->addSkill(new Zuoding);
    zhongyao->addSkill(new ZuodingRecord);
    related_skills.insertMulti("zuoding", "#zuoding");

    General *zhuzhi = new General(this, "zhuzhi", "wu");
    zhuzhi->addSkill(new Anguo);

    addMetaObject<HuaiyiCard>();
    addMetaObject<HuaiyiSnatchCard>();
    addMetaObject<QinwangCard>();
    addMetaObject<ZhenshanCard>();
    addMetaObject<YanzhuCard>();
    addMetaObject<XingxueCard>();
    addMetaObject<YjYanyuCard>();
    addMetaObject<FurongCard>();
    addMetaObject<HuomoCard>();
    addMetaObject<AnguoCard>();

    skills << new QinwangDraw;
}
ADD_PACKAGE(YJCM2015)
