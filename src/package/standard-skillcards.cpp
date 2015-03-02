#include "standard.h"
#include "standard-skillcards.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"

ZhihengCard::ZhihengCard() {
    target_fixed = true;
    mute = true;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    if (source->hasInnateSkill("zhiheng") || !source->hasSkill("jilve"))
        room->broadcastSkillInvoke("zhiheng");
    else
        room->broadcastSkillInvoke("jilve", 4);
    if (source->isAlive())
        room->drawCards(source, subcards.length(), "zhiheng");
}

RendeCard::RendeCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

void RendeCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    int old_value = source->getMark("rende");
    QList<int> rende_list;
    if (old_value > 0)
        rende_list = StringList2IntList(source->property("rende").toString().split("+"));
    else
        rende_list = source->handCards();
    foreach (int id, this->subcards)
        rende_list.removeOne(id);
    room->setPlayerProperty(source, "rende", IntList2StringList(rende_list).join("+"));

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "rende", QString());
    room->obtainCard(target, this, reason, false);

    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "rende", new_value);

    if (old_value < 2 && new_value >= 2)
        room->recover(source, RecoverStruct(source));

    if (room->getMode() == "04_1v3" && source->getMark("rende") >= 2) return;
    if (source->isKongcheng() || source->isDead() || rende_list.isEmpty()) return;
    room->addPlayerHistory(source, "RendeCard", -1);
    if (!room->askForUseCard(source, "@@rende", "@rende-give", -1, Card::MethodNone))
        room->addPlayerHistory(source, "RendeCard");
}

YijueCard::YijueCard() {
}

bool YijueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void YijueCard::use(Room *room, ServerPlayer *guanyu, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    bool success = guanyu->pindian(target, "yijue", NULL);
    if (success) {
        target->addMark("yijue");
        room->setPlayerCardLimitation(target, "use,response", ".|.|.|hand", true);
        room->addPlayerMark(target, "@skill_invalidity");

        foreach (ServerPlayer *p, room->getAllPlayers())
            room->filterCards(p, p->getCards("he"), true);
        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
    } else {
        if (!target->isWounded()) return;
        target->setFlags("YijueTarget");
        QString choice = room->askForChoice(guanyu, "yijue", "recover+cancel");
        target->setFlags("-YijueTarget");
        if (choice == "recover")
            room->recover(target, RecoverStruct(guanyu));
    }
}

JieyinCard::JieyinCard() {
}

bool JieyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    return to_select->isMale() && to_select->isWounded() && to_select != Self;
}

void JieyinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    RecoverStruct recover(effect.from);
    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
}

TuxiCard::TuxiCard() {
}

bool TuxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (targets.length() >= Self->getMark("tuxi") || to_select->getHandcardNum() < Self->getHandcardNum() || to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void TuxiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->setFlags("TuxiTarget");
}

FanjianCard::FanjianCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

void FanjianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();
    Card::Suit suit = getSuit();

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, zhouyu->objectName(), target->objectName(), "fanjian", QString());
    room->obtainCard(target, this, reason);

    if (target->isAlive()) {
        if (target->isNude()) {
            room->loseHp(target);
        } else {
            target->setMark("FanjianSuit", int(suit)); // For AI
            if (room->askForSkillInvoke(target, "fanjian_discard", "prompt:::" + Card::Suit2String(suit))) {
                room->showAllCards(target);
                DummyCard *dummy = new DummyCard;
                foreach (const Card *card, target->getCards("he")) {
                    if (card->getSuit() == suit)
                        dummy->addSubcard(card);
                }
                if (dummy->subcardsLength() > 0)
                    room->throwCard(dummy, target);
                delete dummy;
            } else {
                room->loseHp(target);
            }
        }
    }
}

KurouCard::KurouCard() {
    target_fixed = true;
}

void KurouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->loseHp(source);
}

LianyingCard::LianyingCard() {
}

bool LianyingCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *Self) const{
    return targets.length() < Self->getMark("lianying");
}

void LianyingCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(1, "lianying");
}

LijianCard::LijianCard(bool cancelable): duel_cancelable(cancelable) {
    mute = true;
}

bool LijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!to_select->isMale())
        return false;

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->deleteLater();
    if (targets.isEmpty() && Self->isProhibited(to_select, duel))
        return false;

    if (targets.length() == 1 && to_select->isCardLimited(duel, Card::MethodUse))
        return false;

    return targets.length() < 2 && to_select != Self;
}

bool LijianCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

void LijianCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    QVariant data = QVariant::fromValue(use);
    RoomThread *thread = room->getThread();

    thread->trigger(PreCardUsed, room, card_use.from, data);
    use = data.value<CardUseStruct>();

    room->broadcastSkillInvoke("lijian");

    LogMessage log;
    log.from = card_use.from;
    log.to << card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, card_use.from->objectName(), QString(), "lijian", QString());
    room->moveCardTo(this, card_use.from, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, card_use.from, data);
    use = data.value<CardUseStruct>();
    thread->trigger(CardFinished, room, card_use.from, data);
}

void LijianCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setCancelable(duel_cancelable);
    duel->setSkillName(QString("_%1").arg(getSkillName()));
    if (!from->isCardLimited(duel, Card::MethodUse) && !from->isProhibited(to, duel))
        room->useCard(CardUseStruct(duel, from, to));
    else
        delete duel;
}

ChuliCard::ChuliCard() {
}

bool ChuliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (to_select == Self) return false;
    QSet<QString> kingdoms;
    foreach (const Player *p, targets)
        kingdoms << p->getKingdom();
    return Self->canDiscard(to_select, "he") && !kingdoms.contains(to_select->getKingdom());
}

void ChuliCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> draw_card;
    if (Sanguosha->getCard(getEffectiveId())->getSuit() == Card::Spade)
        draw_card << source;
    foreach (ServerPlayer *target, targets) {
        if (!source->canDiscard(target, "he")) continue;
        int id = room->askForCardChosen(source, target, "he", "chuli", false, Card::MethodDiscard);
        room->throwCard(id, target, source);
        if (Sanguosha->getCard(id)->getSuit() == Card::Spade)
            draw_card << target;
    }

    foreach (ServerPlayer *p, draw_card)
        room->drawCards(p, 1, "chuli");
}

LiuliCard::LiuliCard() {
}

bool LiuliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    if (to_select->hasFlag("LiuliSlashSource") || to_select == Self)
        return false;

    const Player *from = NULL;
    foreach (const Player *p, Self->getAliveSiblings()) {
        if (p->hasFlag("LiuliSlashSource")) {
            from = p;
            break;
        }
    }

    const Card *slash = Card::Parse(Self->property("liuli").toString());
    if (from && !from->canSlash(to_select, slash, false))
        return false;

    int card_id = subcards.first();
    int range_fix = 0;
    if (Self->getWeapon() && Self->getWeapon()->getId() == card_id) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        range_fix += weapon->getRange() - Self->getAttackRange(false);
    } else if (Self->getOffensiveHorse() && Self->getOffensiveHorse()->getId() == card_id) {
        range_fix += 1;
    }

    return Self->inMyAttackRange(to_select, range_fix);
}

void LiuliCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->setFlags("LiuliTarget");
}

FenweiCard::FenweiCard() {
}

bool FenweiCard::targetFilter(const QList<const Player *> &, const Player *to_select, const Player *Self) const{
    QStringList targetslist = Self->property("fenwei_targets").toString().split("+");
    return targetslist.contains(to_select->objectName());
}

void FenweiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->removePlayerMark(source, "@fenwei");
    //room->doLightbox("$FenweiAnimate");
    room->doSuperLightbox("ganning", "fenwei");

    CardUseStruct use = source->tag["fenwei"].value<CardUseStruct>();
    foreach (ServerPlayer *p, targets)
        use.nullified_list << p->objectName();
    source->tag["fenwei"] = QVariant::fromValue(use);
}

GuoseCard::GuoseCard() {
    handling_method = Card::MethodNone;
}

bool GuoseCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty()) return false;
    int id = getEffectiveId();

    Indulgence *indulgence = new Indulgence(getSuit(), getNumber());
    indulgence->addSubcard(id);
    indulgence->setSkillName("guose");
    indulgence->deleteLater();

    bool canUse = !Self->isLocked(indulgence);
    if (canUse && to_select != Self && !to_select->containsTrick("indulgence") && !Self->isProhibited(to_select, indulgence))
        return true;
    bool canDiscard = false;
    foreach (const Card *card, (Self->getHandcards() + Self->getEquips())) {
        if (card->getEffectiveId() == id && !Self->isJilei(Sanguosha->getCard(id))) {
            canDiscard = true;
            break;
        }
    }
    if (!canDiscard || !to_select->containsTrick("indulgence"))
        return false;
    foreach (const Card *card, to_select->getJudgingArea()) {
        if (card->isKindOf("Indulgence") && Self->canDiscard(to_select, card->getEffectiveId()))
            return true;
    }
    return false;
}

const Card *GuoseCard::validate(CardUseStruct &cardUse) const{
    ServerPlayer *to = cardUse.to.first();
    if (!to->containsTrick("indulgence")) {
        Indulgence *indulgence = new Indulgence(getSuit(), getNumber());
        indulgence->addSubcard(getEffectiveId());
        indulgence->setSkillName("guose");
        return indulgence;
    }
    return this;
}

void GuoseCard::onUse(Room *room, const CardUseStruct &use) const{
    CardUseStruct card_use = use;

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();
    thread->trigger(PreCardUsed, room, card_use.from, data);
    card_use = data.value<CardUseStruct>();

    LogMessage log;
    log.from = card_use.from;
    log.to = card_use.to;
    log.type = "#UseCard";
    log.card_str = card_use.card->toString();
    room->sendLog(log);

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, card_use.from->objectName(), QString(), "guose", QString());
    room->moveCardTo(this, card_use.from, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, card_use.from, data);
    card_use = data.value<CardUseStruct>();
    thread->trigger(CardFinished, room, card_use.from, data);
}

void GuoseCard::onEffect(const CardEffectStruct &effect) const{
    foreach (const Card *judge, effect.to->getJudgingArea()) {
        if (judge->isKindOf("Indulgence") && effect.from->canDiscard(effect.to, judge->getEffectiveId())) {
            effect.from->getRoom()->throwCard(judge, NULL, effect.from);
            effect.from->drawCards(1, "guose");
            return;
        }
    }
}

JijiangCard::JijiangCard() {
}

bool JijiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    Slash *slash = new Slash(NoSuit, 0);
    slash->deleteLater();
    return slash->targetFilter(targets, to_select, Self);
}

const Card *JijiangCard::validate(CardUseStruct &cardUse) const{
    cardUse.m_isOwnerUse = false;
    ServerPlayer *liubei = cardUse.from;
    QList<ServerPlayer *> targets = cardUse.to;
    Room *room = liubei->getRoom();
    liubei->broadcastSkillInvoke(this);
    room->notifySkillInvoked(liubei, "jijiang");

    LogMessage log;
    log.from = liubei;
    log.to = targets;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    const Card *slash = NULL;

    QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
    foreach (ServerPlayer *target, targets)
        target->setFlags("JijiangTarget");
    foreach (ServerPlayer *liege, lieges) {
        try {
            slash = room->askForCard(liege, "slash", "@jijiang-slash:" + liubei->objectName(),
                                     QVariant(), Card::MethodResponse, liubei, false, QString(), true);
        }
        catch (TriggerEvent triggerEvent) {
            if (triggerEvent == TurnBroken || triggerEvent == StageChange) {
                foreach (ServerPlayer *target, targets)
                    target->setFlags("-JijiangTarget");
            }
            throw triggerEvent;
        }

        if (slash) {
            foreach (ServerPlayer *target, targets)
                target->setFlags("-JijiangTarget");

            foreach (ServerPlayer *target, targets) {
                if (!liubei->canSlash(target, slash))
                    cardUse.to.removeOne(target);
            }
            if (cardUse.to.length() > 0)
                return slash;
            else {
                delete slash;
                return NULL;
            }
        }
    }
    foreach (ServerPlayer *target, targets)
        target->setFlags("-JijiangTarget");
    room->setPlayerFlag(liubei, "Global_JijiangFailed");
    return NULL;
}

YijiCard::YijiCard() {
    mute = true;
}

bool YijiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (to_select == Self) return false;
    if (Self->getHandcardNum() == 1)
        return targets.isEmpty();
    else
        return targets.length() < 2;
}

void YijiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    foreach (ServerPlayer *target, targets) {
        if (!source->isAlive() || source->isKongcheng()) break;
        if (!target->isAlive()) continue;
        int max = qMin(2, source->getHandcardNum());
        if (source->getHandcardNum() == 2 && targets.length() == 2 && targets.last()->isAlive() && target == targets.first())
            max = 1;
        const Card *dummy = room->askForExchange(source, "yiji", max, 1, false, "YijiGive::" + target->objectName());
        target->addToPile("yiji", dummy, false);
        delete dummy;
    }
}

JianyanCard::JianyanCard() {
    target_fixed = true;
}

void JianyanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    QStringList choice_list, pattern_list;
    choice_list << "basic" << "trick" << "equip" << "red" << "black";
    pattern_list << "BasicCard" << "TrickCard" << "EquipCard" << ".|red" << ".|black";

    QString choice = room->askForChoice(source, "jianyan", choice_list.join("+"));
    QString pattern = pattern_list.at(choice_list.indexOf(choice));

    LogMessage log;
    log.type = "#JianyanChoice";
    log.from = source;
    log.arg = choice;
    room->sendLog(log);

    QList<int> cardIds;
    while (true) {
        int id = room->drawCard();
        cardIds << id;
        CardsMoveStruct move(id, NULL, Player::PlaceTable,
                             CardMoveReason(CardMoveReason::S_REASON_TURNOVER, source->objectName(), "jianyan", QString()));
        room->moveCardsAtomic(move, true);
        room->getThread()->delay();

        const Card *card = Sanguosha->getCard(id);
        if (Sanguosha->matchExpPattern(pattern, NULL, card)) {
            QList<ServerPlayer *> males;
            foreach (ServerPlayer *player, room->getAlivePlayers()) {
                if (player->isMale())
                    males << player;
            }
            if (!males.isEmpty()) {
                QList<int> ids;
                ids << id;
                cardIds.removeOne(id);
                room->fillAG(ids, source);
                source->setMark("jianyan", id); // For AI
                ServerPlayer *target = room->askForPlayerChosen(source, males, "jianyan",
                                                                QString("@jianyan-give:::%1:%2\\%3").arg(card->objectName())
                                                                                                    .arg(card->getSuitString() + "_char")
                                                                                                    .arg(card->getNumberString()));
                room->clearAG(source);
                room->obtainCard(target, card);
            }
            break;
        }
    }
    if (!cardIds.isEmpty()) {
        DummyCard *dummy = new DummyCard(cardIds);
        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, source->objectName(), "jianyan", QString());
        room->throwCard(dummy, reason, NULL);
        delete dummy;
    }
}
