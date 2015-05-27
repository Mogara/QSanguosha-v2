#include "ol.h"
#include "sp.h"
#include "engine.h"
#include "clientplayer.h"
#include "json.h"
#include "settings.h"


class AocaiViewAsSkill : public ZeroCardViewAsSkill
{
public:
    AocaiViewAsSkill() : ZeroCardViewAsSkill("aocai")
    {
    }

    bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        if (player->getPhase() != Player::NotActive || player->hasFlag("Global_AocaiFailed")) return false;
        if (pattern == "slash")
            return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
        else if (pattern == "peach")
            return player->getMark("Global_PreventPeach") == 0;
        else if (pattern.contains("analeptic"))
            return true;
        return false;
    }

    const Card *viewAs() const
    {
        AocaiCard *aocai_card = new AocaiCard;
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "peach+analeptic" && Self->getMark("Global_PreventPeach") > 0)
            pattern = "analeptic";
        aocai_card->setUserString(pattern);
        return aocai_card;
    }
};

class Aocai : public TriggerSkill
{
public:
    Aocai() : TriggerSkill("aocai")
    {
        events << CardAsked;
        view_as_skill = new AocaiViewAsSkill;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        QString pattern = data.toStringList().first();
        if (player->getPhase() == Player::NotActive
            && (pattern == "slash" || pattern == "jink")
            && room->askForSkillInvoke(player, objectName(), data)) {
            QList<int> ids = room->getNCards(2, false);
            QList<int> enabled, disabled;
            foreach (int id, ids) {
                if (Sanguosha->getCard(id)->objectName().contains(pattern))
                    enabled << id;
                else
                    disabled << id;
            }
            int id = Aocai::view(room, player, ids, enabled, disabled);
            if (id != -1) {
                const Card *card = Sanguosha->getCard(id);
                room->provide(card);
                return true;
            }
        }
        return false;
    }

    static int view(Room *room, ServerPlayer *player, QList<int> &ids, QList<int> &enabled, QList<int> &disabled)
    {
        int result = -1, index = -1;
        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = player;
        log.card_str = IntList2StringList(ids).join("+");
        room->sendLog(log, player);

        room->broadcastSkillInvoke("aocai");
        room->notifySkillInvoked(player, "aocai");
        if (enabled.isEmpty()) {
            JsonArray arg;
            arg << "." << false << JsonUtils::toJsonArray(ids);
            room->doNotify(player, QSanProtocol::S_COMMAND_SHOW_ALL_CARDS, arg);
        } else {
            room->fillAG(ids, player, disabled);
            int id = room->askForAG(player, enabled, true, "aocai");
            if (id != -1) {
                index = ids.indexOf(id);
                ids.removeOne(id);
                result = id;
            }
            room->clearAG(player);
        }

        QList<int> &drawPile = room->getDrawPile();
        for (int i = ids.length() - 1; i >= 0; i--)
            drawPile.prepend(ids.at(i));
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_UPDATE_PILE, QVariant(drawPile.length()));
        if (result == -1)
            room->setPlayerFlag(player, "Global_AocaiFailed");
        else {
            LogMessage log;
            log.type = "#AocaiUse";
            log.from = player;
            log.arg = "aocai";
            log.arg2 = QString("CAPITAL(%1)").arg(index + 1);
            room->sendLog(log);
        }
        return result;
    }
};


AocaiCard::AocaiCard()
{
}

bool AocaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool AocaiCard::targetFixed() const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
        return true;

    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetFixed();
}

bool AocaiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetsFeasible(targets, Self);
}

const Card *AocaiCard::validateInResponse(ServerPlayer *user) const
{
    Room *room = user->getRoom();
    QList<int> ids = room->getNCards(2, false);
    QStringList names = user_string.split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";

    QList<int> enabled, disabled;
    foreach (int id, ids) {
        if (names.contains(Sanguosha->getCard(id)->objectName()))
            enabled << id;
        else
            disabled << id;
    }

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "aocai";
    room->sendLog(log);

    int id = Aocai::view(room, user, ids, enabled, disabled);
    return Sanguosha->getCard(id);
}

const Card *AocaiCard::validate(CardUseStruct &cardUse) const
{
    cardUse.m_isOwnerUse = false;
    ServerPlayer *user = cardUse.from;
    Room *room = user->getRoom();
    QList<int> ids = room->getNCards(2, false);
    QStringList names = user_string.split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";

    QList<int> enabled, disabled;
    foreach (int id, ids) {
        if (names.contains(Sanguosha->getCard(id)->objectName()))
            enabled << id;
        else
            disabled << id;
    }

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "aocai";
    room->sendLog(log);

    int id = Aocai::view(room, user, ids, enabled, disabled);
    return Sanguosha->getCard(id);
}

DuwuCard::DuwuCard()
{
    mute = true;
}

bool DuwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty() || qMax(0, to_select->getHp()) != subcardsLength())
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

void DuwuCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();

    if (subcards.length() <= 1)
        room->broadcastSkillInvoke("duwu", 2);
    else
        room->broadcastSkillInvoke("duwu", 1);

    room->damage(DamageStruct("duwu", effect.from, effect.to));
}

class DuwuViewAsSkill : public ViewAsSkill
{
public:
    DuwuViewAsSkill() : ViewAsSkill("duwu")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->canDiscard(player, "he") && !player->hasFlag("DuwuEnterDying");
    }

    bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        return !Self->isJilei(to_select);
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        DuwuCard *duwu = new DuwuCard;
        if (!cards.isEmpty())
            duwu->addSubcards(cards);
        return duwu;
    }
};

class Duwu : public TriggerSkill
{
public:
    Duwu() : TriggerSkill("duwu")
    {
        events << QuitDying;
        view_as_skill = new DuwuViewAsSkill;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.damage && dying.damage->getReason() == "duwu" && !dying.damage->chain && !dying.damage->transfer) {
            ServerPlayer *from = dying.damage->from;
            if (from && from->isAlive()) {
                room->setPlayerFlag(from, "DuwuEnterDying");
                room->loseHp(from, 1);
            }
        }
        return false;
    }
};

class Dujin : public DrawCardsSkill
{
public:
    Dujin() : DrawCardsSkill("dujin")
    {
        frequency = Frequent;
    }

    int getDrawNum(ServerPlayer *player, int n) const
    {
        if (player->askForSkillInvoke(this)) {
            player->getRoom()->broadcastSkillInvoke(objectName());
            return n + player->getEquips().length() / 2 + 1;
        } else
            return n;
    }
};

QingyiCard::QingyiCard()
{
    mute = true;
}

bool QingyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    Slash *slash = new Slash(NoSuit, 0);
    slash->setSkillName("qingyi");
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

void QingyiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    foreach (ServerPlayer *target, targets) {
        if (!source->canSlash(target, NULL, false))
            targets.removeOne(target);
    }

    if (targets.length() > 0) {
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_qingyi");
        room->useCard(CardUseStruct(slash, source, targets));
    }
}

class QingyiViewAsSkill : public ZeroCardViewAsSkill
{
public:
    QingyiViewAsSkill() : ZeroCardViewAsSkill("qingyi")
    {
        response_pattern = "@@qingyi";
    }

    bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "@@qingyi";
    }

    const Card *viewAs() const
    {
        return new QingyiCard;
    }
};

class Qingyi : public TriggerSkill
{
public:
    Qingyi() : TriggerSkill("qingyi")
    {
        events << EventPhaseChanging;
        view_as_skill = new QingyiViewAsSkill;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::Judge && !player->isSkipped(Player::Judge)
            && !player->isSkipped(Player::Draw)) {
            if (Slash::IsAvailable(player) && room->askForUseCard(player, "@@qingyi", "@qingyi-slash")) {
                player->skip(Player::Judge, true);
                player->skip(Player::Draw, true);
            }
        }
        return false;
    }
};

class Shixin : public TriggerSkill
{
public:
    Shixin() : TriggerSkill("shixin")
    {
        events << DamageInflicted;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.nature == DamageStruct::Fire) {
            room->notifySkillInvoked(player, objectName());
            room->broadcastSkillInvoke(objectName());

            LogMessage log;
            log.type = "#ShixinProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = "fire_nature";
            room->sendLog(log);
            return true;
        }
        return false;
    }
};

SanyaoCard::SanyaoCard()
{
}

bool SanyaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty()) return false;
    QList<const Player *> players = Self->getAliveSiblings();
    players << Self;
    int max = -1000;
    foreach (const Player *p, players) {
        if (max < p->getHp()) max = p->getHp();
    }
    return to_select->getHp() == max;
}

void SanyaoCard::onEffect(const CardEffectStruct &effect) const
{
    effect.from->getRoom()->damage(DamageStruct("sanyao", effect.from, effect.to));
}

class Sanyao : public OneCardViewAsSkill
{
public:
    Sanyao() : OneCardViewAsSkill("sanyao")
    {
        filter_pattern = ".!";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->canDiscard(player, "he") && !player->hasUsed("SanyaoCard");
    }

    const Card *viewAs(const Card *originalcard) const
    {
        SanyaoCard *first = new SanyaoCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Zhiman : public TriggerSkill
{
public:
    Zhiman() : TriggerSkill("zhiman")
    {
        events << DamageCaused;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();

        if (player->askForSkillInvoke(this, data)) {
            room->broadcastSkillInvoke(objectName());
            LogMessage log;
            log.type = "#Yishi";
            log.from = player;
            log.arg = objectName();
            log.to << damage.to;
            room->sendLog(log);

            if (damage.to->getEquips().isEmpty() && damage.to->getJudgingArea().isEmpty())
                return true;
            int card_id = room->askForCardChosen(player, damage.to, "ej", objectName());
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
            room->obtainCard(player, Sanguosha->getCard(card_id), reason);
            return true;
        }
        return false;
    }
};



JieyueCard::JieyueCard()
{

}

void JieyueCard::onEffect(const CardEffectStruct &effect) const
{
    if (!effect.to->isNude()) {
        Room *room = effect.to->getRoom();
        const Card *card = room->askForExchange(effect.to, "jieyue", 1, 1, true, QString("@jieyue_put:%1").arg(effect.from->objectName()), true);

        if (card != NULL)
            effect.from->addToPile("jieyue_pile", card);
        else if (effect.from->canDiscard(effect.to, "he")) {
            int id = room->askForCardChosen(effect.from, effect.to, "he", objectName(), false, Card::MethodDiscard);
            room->throwCard(id, effect.to, effect.from);
        }
    }
}

class JieyueVS : public OneCardViewAsSkill
{
public:
    JieyueVS() : OneCardViewAsSkill("jieyue")
    {
    }

    bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        if (to_select->isEquipped())
            return false;
        QString pattern = Sanguosha->getCurrentCardUsePattern();
        if (pattern == "@@jieyue") {
            return !Self->isJilei(to_select);
        }

        if (pattern == "jink")
            return to_select->isRed();
        else if (pattern == "nullification")
            return to_select->isBlack();
        return false;
    }

    bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        return (!player->getPile("jieyue_pile").isEmpty() && (pattern == "jink" || pattern == "nullification")) || (pattern == "@@jieyue" && !player->isKongcheng());
    }

    bool isEnabledAtNullification(const ServerPlayer *player) const
    {
        return !player->getPile("jieyue_pile").isEmpty();
    }

    const Card *viewAs(const Card *card) const
    {
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "@@jieyue") {
            JieyueCard *jy = new JieyueCard;
            jy->addSubcard(card);
            return jy;
        }

        if (card->isRed()) {
            Jink *jink = new Jink(Card::SuitToBeDecided, 0);
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        } else if (card->isBlack()) {
            Nullification *nulli = new Nullification(Card::SuitToBeDecided, 0);
            nulli->addSubcard(card);
            nulli->setSkillName(objectName());
            return nulli;
        }
        return NULL;
    }

    int getEffectIndex(const ServerPlayer *, const Card *card) const
    {
        if (card->isKindOf("Nullification"))
            return 3;
        else if (card->isKindOf("Jink"))
            return 2;

        return 1;
    }
};

class Jieyue : public TriggerSkill
{
public:
    Jieyue() : TriggerSkill("jieyue")
    {
        events << EventPhaseStart;
        view_as_skill = new JieyueVS;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (player->getPhase() == Player::Start && !player->getPile("jieyue_pile").isEmpty()) {
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            DummyCard *dummy = new DummyCard(player->getPile("jieyue_pile"));
            player->obtainCard(dummy);
            delete dummy;
        } else if (player->getPhase() == Player::Finish) {
            room->askForUseCard(player, "@@jieyue", "@jieyue", -1, Card::MethodDiscard, false);
        }
        return false;
    }
};


class OlZishou : public DrawCardsSkill
{
public:
    OlZishou() : DrawCardsSkill("olzishou")
    {

    }

    int getDrawNum(ServerPlayer *player, int n) const
    {
        if (player->askForSkillInvoke(this)) {
            Room *room = player->getRoom();
            room->broadcastSkillInvoke(objectName());

            room->setPlayerFlag(player, "olzishou");

            QSet<QString> kingdomSet;
            foreach(ServerPlayer *p, room->getAlivePlayers())
                kingdomSet.insert(p->getKingdom());

            return n + kingdomSet.count();
        }

        return n;
    }
};

class OlZishouProhibit : public ProhibitSkill
{
public:
    OlZishouProhibit() : ProhibitSkill("#olzishou")
    {

    }

    bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> & /* = QList<const Player *>() */) const
    {
        if (card->isKindOf("SkillCard"))
            return false;

        if (from->hasFlag("olzishou"))
            return from != to;

        return false;
    }
};

class Fenyin : public TriggerSkill
{
public:
    Fenyin() : TriggerSkill("fenyin")
    {
        events << EventPhaseStart << CardUsed << CardResponded;
        global = true;
        frequency = Frequent;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    int getPriority(TriggerEvent triggerEvent) const
    {
        if (triggerEvent == EventPhaseStart)
            return 6;

        return TriggerSkill::getPriority(triggerEvent);
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventPhaseStart) {
            if (player->getPhase() == Player::RoundStart)
                player->setMark(objectName(), 0);

            return false;
        }

        const Card *c = NULL;
        if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (player == use.from)
                c = use.card;
        } else {
            CardResponseStruct resp = data.value<CardResponseStruct>();
            if (resp.m_isUse)
                c = resp.m_card;
        }

        if (c == NULL || c->isKindOf("SkillCard") || player->getPhase() == Player::NotActive)
            return false;

        if (player->getMark(objectName()) != 0) {
            Card::Color old_color = static_cast<Card::Color>(player->getMark(objectName()) - 1);
            if (old_color != c->getColor() && player->hasSkill(this) && player->askForSkillInvoke(this, QVariant::fromValue(c))) {
                room->broadcastSkillInvoke(objectName());
                player->drawCards(1);
            }
        }

        player->setMark(objectName(), static_cast<int>(c->getColor()) + 1);

        return false;
    }
};

class TunchuDraw : public DrawCardsSkill
{
public:
    TunchuDraw() : DrawCardsSkill("tunchu")
    {

    }

    int getDrawNum(ServerPlayer *player, int n) const
    {
        if (player->askForSkillInvoke("tunchu")) {
            player->setFlags("tunchu");
            player->getRoom()->broadcastSkillInvoke("tunchu");
            return n + 2;
        }

        return n;
    }
};

class TunchuEffect : public TriggerSkill
{
public:
    TunchuEffect() : TriggerSkill("#tunchu-effect")
    {
        events << AfterDrawNCards;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (player->hasFlag("tunchu") && !player->isKongcheng()) {
            const Card *c = room->askForExchange(player, "tunchu", 1, 1, false, "@tunchu-put");
            if (c != NULL)
                player->addToPile("food", c);
        }

        return false;
    }
};

class Tunchu : public TriggerSkill
{
public:
    Tunchu() : TriggerSkill("#tunchu-disable")
    {
        events << EventLoseSkill << EventAcquireSkill << CardsMoveOneTime;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventLoseSkill && data.toString() == "tunchu") {
            room->removePlayerCardLimitation(player, "use", "Slash,Duel$0");
        } else if (triggerEvent == EventAcquireSkill && data.toString() == "tunchu") {
            if (!player->getPile("food").isEmpty())
                room->setPlayerCardLimitation(player, "use", "Slash,Duel", false);
        } else if (triggerEvent == CardsMoveOneTime && player->isAlive() && player->hasSkill("tunchu", true)) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.to == player && move.to_place == Player::PlaceSpecial && move.to_pile_name == "food") {
                if (player->getPile("food").length() == 1)
                    room->setPlayerCardLimitation(player, "use", "Slash,Duel", false);
            } else if (move.from == player && move.from_places.contains(Player::PlaceSpecial)
                && move.from_pile_names.contains("food")) {
                if (player->getPile("food").isEmpty())
                    room->removePlayerCardLimitation(player, "use", "Slash,Duel$0");
            }
        }
        return false;
    }
};

ShuliangCard::ShuliangCard()
{
    target_fixed = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void ShuliangCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    CardMoveReason r(CardMoveReason::S_REASON_REMOVE_FROM_PILE, source->objectName(), objectName(), QString());
    room->moveCardTo(this, NULL, Player::DiscardPile, r, true);
}

class ShuliangVS : public OneCardViewAsSkill
{
public:
    ShuliangVS() : OneCardViewAsSkill("shuliang")
    {
        response_pattern = "@@shuliang";
        filter_pattern = ".|.|.|food";
        expand_pile = "food";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        ShuliangCard *c = new ShuliangCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class Shuliang : public TriggerSkill
{
public:
    Shuliang() : TriggerSkill("shuliang")
    {
        events << EventPhaseStart;
        view_as_skill = new ShuliangVS;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getPhase() == Player::Finish && target->isKongcheng();
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        foreach (ServerPlayer *const &p, room->getAllPlayers()) {
            if (!TriggerSkill::triggerable(p))
                continue;

            if (!p->getPile("food").isEmpty()) {
                if (room->askForUseCard(p, "@@shuliang", "@shuliang", -1, Card::MethodNone))
                    player->drawCards(2, objectName());
            }
        }

        return false;
    }
};


ZhanyiViewAsBasicCard::ZhanyiViewAsBasicCard()
{
    m_skillName = "_zhanyi";
    will_throw = false;
}

bool ZhanyiViewAsBasicCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return false;
    }

    const Card *card = Self->tag.value("zhanyi").value<const Card *>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool ZhanyiViewAsBasicCard::targetFixed() const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFixed();
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    const Card *card = Self->tag.value("zhanyi").value<const Card *>();
    return card && card->targetFixed();
}

bool ZhanyiViewAsBasicCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetsFeasible(targets, Self);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    const Card *card = Self->tag.value("zhanyi").value<const Card *>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *ZhanyiViewAsBasicCard::validate(CardUseStruct &card_use) const
{
    ServerPlayer *zhuling = card_use.from;
    Room *room = zhuling->getRoom();

    QString to_zhanyi = user_string;
    if (user_string == "slash" && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "normal_slash" << "thunder_slash" << "fire_slash";
        to_zhanyi = room->askForChoice(zhuling, "zhanyi_slash", guhuo_list.join("+"));
    }

    const Card *card = Sanguosha->getCard(subcards.first());
    QString user_str;
    if (to_zhanyi == "slash") {
        if (card->isKindOf("Slash"))
            user_str = card->objectName();
        else
            user_str = "slash";
    } else if (to_zhanyi == "normal_slash")
        user_str = "slash";
    else
        user_str = to_zhanyi;
    Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
    use_card->setSkillName("_zhanyi");
    use_card->addSubcard(subcards.first());
    use_card->deleteLater();
    return use_card;
}

const Card *ZhanyiViewAsBasicCard::validateInResponse(ServerPlayer *zhuling) const
{
    Room *room = zhuling->getRoom();

    QString to_zhanyi;
    if (user_string == "peach+analeptic") {
        QStringList guhuo_list;
        guhuo_list << "peach";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "analeptic";
        to_zhanyi = room->askForChoice(zhuling, "zhanyi_saveself", guhuo_list.join("+"));
    } else if (user_string == "slash") {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "normal_slash" << "thunder_slash" << "fire_slash";
        to_zhanyi = room->askForChoice(zhuling, "zhanyi_slash", guhuo_list.join("+"));
    } else
        to_zhanyi = user_string;

    const Card *card = Sanguosha->getCard(subcards.first());
    QString user_str;
    if (to_zhanyi == "slash") {
        if (card->isKindOf("Slash"))
            user_str = card->objectName();
        else
            user_str = "slash";
    } else if (to_zhanyi == "normal_slash")
        user_str = "slash";
    else
        user_str = to_zhanyi;
    Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
    use_card->setSkillName("_zhanyi");
    use_card->addSubcard(subcards.first());
    use_card->deleteLater();
    return use_card;
}

ZhanyiCard::ZhanyiCard()
{
    target_fixed = true;
}

void ZhanyiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    room->loseHp(source);
    if (source->isAlive()) {
        const Card *c = Sanguosha->getCard(subcards.first());
        if (c->getTypeId() == Card::TypeBasic) {
            room->setPlayerMark(source, "ViewAsSkill_zhanyiEffect", 1);
        } else if (c->getTypeId() == Card::TypeEquip)
            source->setFlags("zhanyiEquip");
        else if (c->getTypeId() == Card::TypeTrick) {
            source->drawCards(2, "zhanyi");
            room->setPlayerFlag(source, "zhanyiTrick");
        }
    }
}

class ZhanyiNoDistanceLimit : public TargetModSkill
{
public:
    ZhanyiNoDistanceLimit() : TargetModSkill("#zhanyi-trick")
    {
        pattern = "^SkillCard";
    }

    int getDistanceLimit(const Player *from, const Card *) const
    {
        return from->hasFlag("zhanyiTrick") ? 1000 : 0;
    }
};

class ZhanyiDiscard2 : public TriggerSkill
{
public:
    ZhanyiDiscard2() : TriggerSkill("#zhanyi-equip")
    {
        events << TargetSpecified;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->hasFlag("zhanyiEquip");
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card == NULL || !use.card->isKindOf("Slash"))
            return false;


        foreach (ServerPlayer *p, use.to) {
            if (p->isNude())
                continue;

            if (p->getCardCount() <= 2) {
                DummyCard dummy;
                dummy.addSubcards(p->getCards("he"));
                room->throwCard(&dummy, p);
            } else
                room->askForDiscard(p, "zhanyi_equip", 2, 2, false, true, "@zhanyiequip_discard");
        }
        return false;
    }
};

class Zhanyi : public OneCardViewAsSkill
{
public:
    Zhanyi() : OneCardViewAsSkill("zhanyi")
    {

    }

    bool isResponseOrUse() const
    {
        return Self->getMark("ViewAsSkill_zhanyiEffect") > 0;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        if (!player->hasUsed("ZhanyiCard"))
            return true;

        if (player->getMark("ViewAsSkill_zhanyiEffect") > 0)
            return true;

        return false;
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        if (player->getMark("ViewAsSkill_zhanyiEffect") == 0) return false;
        if (pattern.startsWith(".") || pattern.startsWith("@")) return false;
        if (pattern == "peach" && player->getMark("Global_PreventPeach") > 0) return false;
        for (int i = 0; i < pattern.length(); i++) {
            QChar ch = pattern[i];
            if (ch.isUpper() || ch.isDigit()) return false; // This is an extremely dirty hack!! For we need to prevent patterns like 'BasicCard'
        }
        return !(pattern == "nullification");
    }

    QDialog *getDialog() const
    {
        return GuhuoDialog::getInstance("zhanyi", true, false);
    }

    bool viewFilter(const Card *to_select) const
    {
        if (Self->getMark("ViewAsSkill_zhanyiEffect") > 0)
            return to_select->isKindOf("BasicCard");
        else
            return true;
    }

    const Card *viewAs(const Card *originalCard) const
    {
        if (Self->getMark("ViewAsSkill_zhanyiEffect") == 0) {
            ZhanyiCard *zy = new ZhanyiCard;
            zy->addSubcard(originalCard);
            return zy;
        }

        if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE
            || Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
            ZhanyiViewAsBasicCard *card = new ZhanyiViewAsBasicCard;
            card->setUserString(Sanguosha->getCurrentCardUsePattern());
            card->addSubcard(originalCard);
            return card;
        }

        const Card *c = Self->tag.value("zhanyi").value<const Card *>();
        if (c) {
            ZhanyiViewAsBasicCard *card = new ZhanyiViewAsBasicCard;
            card->setUserString(c->objectName());
            card->addSubcard(originalCard);
            return card;
        } else
            return NULL;
    }
};

class ZhanyiRemove : public TriggerSkill
{
public:
    ZhanyiRemove() : TriggerSkill("#zhanyi-basic")
    {
        events << EventPhaseChanging;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getMark("ViewAsSkill_zhanyiEffect") > 0;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::NotActive)
            room->setPlayerMark(player, "ViewAsSkill_zhanyiEffect", 0);

        return false;
    }
};

class OlShenxian : public TriggerSkill
{
public:
    OlShenxian() : TriggerSkill("olshenxian")
    {
        events << CardsMoveOneTime << EventPhaseStart;
        frequency = Frequent;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventPhaseStart) {
            if (player->getPhase() == Player::NotActive) {
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (p->getMark(objectName()) > 0)
                        p->setMark(objectName(), 0);
                }
            }
        } else {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (TriggerSkill::triggerable(player) && player->getPhase() == Player::NotActive && player->getMark(objectName()) == 0 && move.from && move.from->isAlive()
                && move.from->objectName() != player->objectName() && (move.from_places.contains(Player::PlaceHand) || move.from_places.contains(Player::PlaceEquip))
                && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
                foreach (int id, move.card_ids) {
                    if (Sanguosha->getCard(id)->getTypeId() == Card::TypeBasic) {
                        if (room->askForSkillInvoke(player, objectName(), data)) {
                            room->broadcastSkillInvoke(objectName());
                            player->drawCards(1, "olshenxian");
                            player->setMark(objectName(), 1);
                        }
                        break;
                    }
                }
            }
        }
        return false;
    }
};


class OlMeibu : public TriggerSkill
{
public:
    OlMeibu() : TriggerSkill("olmeibu")
    {
        events << EventPhaseStart << EventPhaseChanging << CardUsed;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    int getPriority(TriggerEvent triggerEvent) const
    {
        if (triggerEvent == CardUsed)
            return 6;

        return TriggerSkill::getPriority(triggerEvent);
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == EventPhaseStart && player->getPhase() == Player::Play) {
            foreach (ServerPlayer *sunluyu, room->getOtherPlayers(player)) {
                if (!player->inMyAttackRange(sunluyu) && TriggerSkill::triggerable(sunluyu) && room->askForSkillInvoke(sunluyu, objectName())) {
                    room->broadcastSkillInvoke(objectName());
                    if (!player->hasSkill("#olmeibu-filter", true)) {
                        room->acquireSkill(player, "#olmeibu-filter", false);
                        room->filterCards(player, player->getCards("he"), false);
                    }
                    QVariantList sunluyus = player->tag[objectName()].toList();
                    sunluyus << QVariant::fromValue(sunluyu);
                    player->tag[objectName()] = QVariant::fromValue(sunluyus);
                    room->insertAttackRangePair(player, sunluyu);
                }
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive) return false;

            QVariantList sunluyus = player->tag[objectName()].toList();
            foreach (QVariant sunluyu, sunluyus) {
                ServerPlayer *s = sunluyu.value<ServerPlayer *>();
                room->removeAttackRangePair(player, s);
            }
            if (player->hasSkill("#olmeibu-filter", true)) {
                room->detachSkillFromPlayer(player, "#olmeibu-filter");
                room->filterCards(player, player->getCards("he"), true);
            }
        } else if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (player->hasSkill("#olmeibu-filter", true) && use.card != NULL && use.card->getSkillName() == "olmeibu") {
                room->detachSkillFromPlayer(player, "#olmeibu-filter");
                room->filterCards(player, player->getCards("he"), true);
            }
        }
        return false;
    }

    int getEffectIndex(const ServerPlayer *, const Card *card)
    {
        if (card->isKindOf("Slash"))
            return -2;

        return -1;
    }
};

OlMumuCard::OlMumuCard()
{

}

bool OlMumuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (targets.isEmpty()) {
        if (to_select->getWeapon() && Self->canDiscard(to_select, to_select->getWeapon()->getEffectiveId()))
            return true;
        if (to_select != Self && to_select->getArmor())
            return true;
    }

    return false;
}

void OlMumuCard::onEffect(const CardEffectStruct &effect) const
{
    ServerPlayer *target = effect.to;
    ServerPlayer *player = effect.from;

    QStringList choices;
    if (target->getWeapon() && player->canDiscard(target, target->getWeapon()->getEffectiveId()))
        choices << "weapon";
    if (target != player && target->getArmor())
        choices << "armor";

    if (choices.length() == 0)
        return;

    Room *r = target->getRoom();
    QString choice = choices.length() == 1 ? choices.first() : r->askForChoice(player, "olmumu", choices.join("+"), QVariant::fromValue(target));

    if (choice == "weapon") {
        r->throwCard(target->getWeapon(), target, player == target ? NULL : player);
        player->drawCards(1, "olmumu");
    } else {
        int equip = target->getArmor()->getEffectiveId();
        QList<CardsMoveStruct> exchangeMove;
        CardsMoveStruct move1(equip, player, Player::PlaceEquip, CardMoveReason(CardMoveReason::S_REASON_ROB, player->objectName()));
        exchangeMove.push_back(move1);
        if (player->getArmor()) {
            CardsMoveStruct move2(player->getArmor()->getEffectiveId(), NULL, Player::DiscardPile, CardMoveReason(CardMoveReason::S_REASON_CHANGE_EQUIP, player->objectName()));
            exchangeMove.push_back(move2);
        }
        r->moveCardsAtomic(exchangeMove, true);
    }
}

class OlMumu : public OneCardViewAsSkill
{
public:
    OlMumu() : OneCardViewAsSkill("olmumu")
    {
        filter_pattern = "Slash#TrickCard|black!";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        OlMumuCard *mm = new OlMumuCard;
        mm->addSubcard(originalCard);
        return mm;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("OlMumuCard");
    }
};



OLPackage::OLPackage()
    : Package("OL")
{
    General *zhugeke = new General(this, "zhugeke", "wu", 3); // OL 002
    zhugeke->addSkill(new Aocai);
    zhugeke->addSkill(new Duwu);

    General *lingcao = new General(this, "lingcao", "wu", 4);
    lingcao->addSkill(new Dujin);

    General *sunru = new General(this, "sunru", "wu", 3, false);
    sunru->addSkill(new Qingyi);
    sunru->addSkill(new SlashNoDistanceLimitSkill("qingyi"));
    sunru->addSkill(new Shixin);
    related_skills.insertMulti("qingyi", "#qingyi-slash-ndl");

    General *liuzan = new General(this, "liuzan", "wu");
    liuzan->addSkill(new Fenyin);

    General *lifeng = new General(this, "lifeng", "shu", 3);
    lifeng->addSkill(new TunchuDraw);
    lifeng->addSkill(new TunchuEffect);
    lifeng->addSkill(new Tunchu);
    lifeng->addSkill(new Shuliang);
    related_skills.insertMulti("tunchu", "#tunchu-effect");
    related_skills.insertMulti("tunchu", "#tunchu-disable");

    General *zhuling = new General(this, "zhuling", "wei");
    zhuling->addSkill(new Zhanyi);
    zhuling->addSkill(new ZhanyiDiscard2);
    zhuling->addSkill(new ZhanyiNoDistanceLimit);
    zhuling->addSkill(new ZhanyiRemove);
    related_skills.insertMulti("zhanyi", "#zhanyi-basic");
    related_skills.insertMulti("zhanyi", "#zhanyi-equip");
    related_skills.insertMulti("zhanyi", "#zhanyi-trick");

    General *ol_fazheng = new General(this, "ol_fazheng", "shu", 3, true, true);
    ol_fazheng->addSkill("enyuan");
    ol_fazheng->addSkill("xuanhuo");

    General *ol_masu = new General(this, "ol_masu", "shu", 3);
    ol_masu->addSkill(new Sanyao);
    ol_masu->addSkill(new Zhiman);

    General *ol_xushu = new General(this, "ol_xushu", "shu", 3, true, true);
    ol_xushu->addSkill("wuyan");
    ol_xushu->addSkill("jujian");

    General *ol_guanxingzhangbao = new General(this, "ol_guanxingzhangbao", "shu", 4, true, true);
    ol_guanxingzhangbao->addSkill("fuhun");

    General *ol_madai = new General(this, "ol_madai", "shu", 4, true, true);
    ol_madai->addSkill("mashu");
    ol_madai->addSkill("qianxi");

    General *ol_wangyi = new General(this, "ol_wangyi", "wei", 3, false, true);
    ol_wangyi->addSkill("zhenlie");
    ol_wangyi->addSkill("miji");

    General *ol_yujin = new General(this, "ol_yujin", "wei");
    ol_yujin->addSkill(new Jieyue);

    General *ol_liubiao = new General(this, "ol_liubiao", "qun", 3);
    ol_liubiao->addSkill(new OlZishou);
    ol_liubiao->addSkill(new OlZishouProhibit);
    ol_liubiao->addSkill("zongshi");

    General *ol_xingcai = new General(this, "ol_xingcai", "shu", 3, false);
    ol_xingcai->addSkill(new OlShenxian);
    ol_xingcai->addSkill("qiangwu");

    General *ol_xiaohu = new General(this, "ol_sunluyu", "wu", 3, false);
    ol_xiaohu->addSkill(new OlMeibu);
    ol_xiaohu->addSkill(new OlMumu);

    addMetaObject<AocaiCard>();
    addMetaObject<DuwuCard>();
    addMetaObject<QingyiCard>();
    addMetaObject<SanyaoCard>();
    addMetaObject<JieyueCard>();
    addMetaObject<ShuliangCard>();
    addMetaObject<ZhanyiCard>();
    addMetaObject<OlMumuCard>();
    addMetaObject<ZhanyiViewAsBasicCard>();

    skills << new MeibuFilter("olmeibu");
}

ADD_PACKAGE(OL)