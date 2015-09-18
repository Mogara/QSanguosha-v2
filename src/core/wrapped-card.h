#ifndef _WRAPPED_CARD_H
#define _WRAPPED_CARD_H

#include "card.h"

// This is a wrapper class around a card. Each card id should have one and only one WrappedCard
// copy in each room after game initialization is done. Each room's WrappedCards are isolated,
// but inside the room they are shared and synced between server/client.
//
// WrappedCard's internal card is only intended to provide CardEffect (the card face). The suit,
// number should not be modified to refelct the updated suit/number of WrappedCard. The modified
// suit/number/flags/... are maintained in WrappedCard's own member variables.
//
// All WrappedCard's member function that takes a Card as parameter will take over the Card passed
// in, meaning that the caller is resposible for allocating the memory, but WrappedCard is responsible
// for destroying it. No caller should ever delete any card that has been passed in to any member function
// of WrappedCard that takes Card * as parameter (unless the parameter is (const Card *)).
//
// WrappedCard should never have any subcard!!! It's a concrete, single piece card in the room no matter when.

class WrappedCard : public Card
{
    Q_OBJECT

public:
    Q_INVOKABLE WrappedCard(Card *card);
    ~WrappedCard();

    inline void setId(int id)
    {
        m_id = id;
        Q_ASSERT(m_card != NULL);
        m_card->setId(id);
    }

    inline void setNumber(int number)
    {
        m_number = number;
        Q_ASSERT(m_card != NULL);
        m_card->setNumber(number);
    }

    inline void setSuit(Suit suit)
    {
        m_suit = suit;
        Q_ASSERT(m_card != NULL);
        m_card->setSuit(suit);
    }

    inline void setSkillName(const QString &skillName)
    {
        m_skillName = skillName;
        Q_ASSERT(m_card != NULL);
        m_card->setSkillName(skillName);
    }

    // Set the internal card to be the new card, update everything related
    // to CardEffect including objectName.
    void takeOver(Card *card);
    void copyEverythingFrom(Card *card);
    void setModified(bool modified)
    {
        m_isModified = modified;
    }

    // Inherited member functions
    inline void onNullified(ServerPlayer *target) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->onNullified(target);

    }
    inline bool isModified() const
    {
        return m_isModified;
    }
    inline QString getClassName() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->getClassName();
    }

    inline const Card *getRealCard() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card;
    }

    inline bool isMute() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->isMute();
    }

    inline bool willThrow() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->willThrow();
    }

    inline bool canRecast() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->canRecast();
    }

    inline Card::HandlingMethod getHandlingMethod() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->getHandlingMethod();
    }

    inline bool hasPreAction() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->hasPreAction();
    }

    inline QString getPackage() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->getPackage();
    }

    inline bool isVirtualCard() const
    {
        return false;
    }
    //inline bool isEquipped() const{ return m_card->isEquipped(); }
    inline QString getCommonEffectName() const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->getCommonEffectName();
    }

    inline bool match(const QString &pattern) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->match(pattern);
    }

    void setFlags(const QString &flag) const;
    inline void addSubcard(int)
    {
        Q_ASSERT(false);
    }
    inline void addSubcard(const Card *)
    {
        Q_ASSERT(false);
    }
    inline void addSubcards(const QList<const Card *> &)
    {
        Q_ASSERT(false);
    }
    inline void addSubcards(const QList<int> &)
    {
        Q_ASSERT(false);
    }
    // inline QList<int> getSubcards() const;
    // inline void clearSubcards();
    // inline QString subcardString() const;
    // inline int subcardsLength() const;

    void setTag(const QString &key, const QVariant &data) const;
    void removeTag(const QString &key) const;

    inline QString getType() const
    {
        return m_card->getType();
    }
    inline QString getSubtype() const
    {
        return m_card->getSubtype();
    }
    inline CardType getTypeId() const
    {
        return m_card->getTypeId();
    }
    inline QString toString(bool = false) const
    {
        return QString::number(m_id);
    }
    inline bool isNDTrick() const
    {
        return m_card->isNDTrick();
    }

    // card target selection
    inline bool targetFixed() const
    {
        return m_card->targetFixed();
    }
    inline bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
    {
        return m_card->targetsFeasible(targets, Self);
    }

    // @todo: the following two functions should be merged into one.
    inline bool targetFilter(const QList<const Player *> &targets,
        const Player *to_select, const Player *Self) const
    {
        return m_card->targetFilter(targets, to_select, Self);
    }

    inline bool targetFilter(const QList<const Player *> &targets, const Player *to_select,
        const Player *Self, int &maxVotes) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->targetFilter(targets, to_select, Self, maxVotes);
    }

    inline bool isAvailable(const Player *player) const
    {
        return m_card->isAvailable(player);
    }

    inline const Card *validate(CardUseStruct &cardUse) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->validate(cardUse);
    }

    inline const Card *validateInResponse(ServerPlayer *user) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->validateInResponse(user);
    }

    inline void doPreAction(Room *room, const CardUseStruct &cardUse) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->doPreAction(room, cardUse);
    }

    inline void onUse(Room *room, const CardUseStruct &cardUse) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->onUse(room, cardUse);
    }

    inline void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->use(room, source, targets);
    }

    inline void onEffect(const CardEffectStruct &effect) const
    {
        Q_ASSERT(m_card != NULL);
        m_card->onEffect(effect);
    }

    inline bool isCancelable(const CardEffectStruct &effect) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->isCancelable(effect);
    }

    inline bool isKindOf(const char *cardType) const
    {
        Q_ASSERT(m_card != NULL);
        return m_card->isKindOf(cardType);
    }

protected:
    Card *m_card;
    mutable bool m_isModified;
};

#endif

