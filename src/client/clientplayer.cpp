#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "clientstruct.h"

ClientPlayer *Self = NULL;

ClientPlayer::ClientPlayer(Client *client)
    : Player(client), handcard_num(0)
{
    mark_doc = new QTextDocument(this);
}

int ClientPlayer::aliveCount() const
{
    return ClientInstance->alivePlayerCount();
}

int ClientPlayer::getHandcardNum() const
{
    return handcard_num;
}

void ClientPlayer::addCard(const Card *card, Place place)
{
    switch (place) {
    case PlaceHand: {
        if (card) known_cards << card;
        handcard_num++;
        break;
    }
    case PlaceEquip: {
        WrappedCard *equip = Sanguosha->getWrappedCard(card->getEffectiveId());
        setEquip(equip);
        break;
    }
    case PlaceDelayedTrick: {
        addDelayedTrick(card);
        break;
    }
    default:
        break;
    }
}

void ClientPlayer::addKnownHandCard(const Card *card)
{
    if (!known_cards.contains(card))
        known_cards << card;
}

bool ClientPlayer::isLastHandCard(const Card *card, bool contain) const
{
    if (!card->isVirtualCard()) {
        if (known_cards.length() != 1)
            return false;
        return known_cards.first()->getId() == card->getEffectiveId();
    } else if (card->getSubcards().length() > 0) {
        if (!contain) {
            foreach (int card_id, card->getSubcards()) {
                if (!known_cards.contains(Sanguosha->getCard(card_id)))
                    return false;
            }
            return known_cards.length() == card->getSubcards().length();
        } else {
            foreach (const Card *ncard, known_cards) {
                if (!card->getSubcards().contains(ncard->getEffectiveId()))
                    return false;
            }
            return true;
        }
    }
    return false;
}

void ClientPlayer::removeCard(const Card *card, Place place)
{
    switch (place) {
    case PlaceHand: {
        handcard_num--;
        if (card)
            known_cards.removeOne(card);
        break;
    }
    case PlaceEquip:{
        WrappedCard *equip = Sanguosha->getWrappedCard(card->getEffectiveId());
        removeEquip(equip);
        break;
    }
    case PlaceDelayedTrick:{
        removeDelayedTrick(card);
        break;
    }
    default:
        break;
    }
}

QList<const Card *> ClientPlayer::getHandcards() const
{
    return known_cards;
}

void ClientPlayer::setCards(const QList<int> &card_ids)
{
    known_cards.clear();
    foreach(int cardId, card_ids)
        known_cards.append(Sanguosha->getCard(cardId));
}

QTextDocument *ClientPlayer::getMarkDoc() const
{
    return mark_doc;
}

void ClientPlayer::changePile(const QString &name, bool add, QList<int> card_ids)
{
    if (add)
        piles[name].append(card_ids);
    else {
        foreach (int card_id, card_ids) {
            if (piles[name].isEmpty()) break;
            if (piles[name].contains(Card::S_UNKNOWN_CARD_ID) && !piles[name].contains(card_id))
                piles[name].removeOne(Card::S_UNKNOWN_CARD_ID);
            else if (piles[name].contains(card_id))
                piles[name].removeOne(card_id);
            else
                piles[name].takeLast();
        }
    }
    if (!name.startsWith("#"))
        emit pile_changed(name);
}

QString ClientPlayer::getDeathPixmapPath() const
{
    QString basename;
    if (ServerInfo.GameMode == "06_3v3" || ServerInfo.GameMode == "06_XMode") {
        if (getRole() == "lord" || getRole() == "renegade")
            basename = "marshal";
        else
            basename = "guard";
    } else if (ServerInfo.EnableHegemony) {
        basename.clear();
    } else
        basename = getRole();

    if (basename.isEmpty())
        basename = "unknown";

    return QString("image/system/death/%1.png").arg(basename);
}

void ClientPlayer::setHandcardNum(int n)
{
    handcard_num = n;
}

QString ClientPlayer::getGameMode() const
{
    return ServerInfo.GameMode;
}

void ClientPlayer::setFlags(const QString &flag)
{
    Player::setFlags(flag);

    if (flag.endsWith("actioned"))
        emit action_taken();
}

void ClientPlayer::setMark(const QString &mark, int value)
{
    if (marks[mark] == value && mark != "@substitute")
        return;
    marks[mark] = value;

    if (mark == "drank")
        emit drank_changed();

    if (!mark.startsWith("@"))
        return;

    // @todo: consider move all the codes below to PlayerCardContainerUI.cpp
    // set mark doc
    QString text = "";

    static QStringList marklist;
    if (marklist.isEmpty())
        marklist << "@huashen" << "@yongsi_test" << "@jushou_test"
        << "@max_cards_test" << "@defensive_distance_test" << "@offensive_distance_test"
        << "@bossExp";
    QStringList keys = marks.keys();
    foreach (QString key, marklist) {
        if (keys.contains(key)) {
            keys.removeAll(key);
            keys.prepend(key);
        }
    }
    foreach (QString key, keys) {
        if (key.startsWith("@") && marks[key] > 0) {
            int val = marks[key];
            QString mark_text = QString("<img src='image/mark/%1.png' />").arg(key);
            if (val != 1)
                mark_text.append(QString("%1").arg(val));
            if (this != Self)
                mark_text.append("<br>");
            text.append(mark_text);
            if (key == "@substitute") {
                QString hp_str = this->property("tishen_hp").toString();
                if (hp_str.isEmpty()) continue;
                int tishen_hp = hp_str.toInt();
                QString mark_text = QString("<img src='image/mark/%1.png' />%2").arg("@substitute_hp").arg(tishen_hp);
                if (this != Self)
                    mark_text.append("<br>");
                text.append(mark_text);
            }
        }
    }

    mark_doc->setHtml(text);

    if (mark == "@duanchang")
        emit duanchang_invoked();
}

