#ifndef _STANDARD_H
#define _STANDARD_H

#include "package.h"
#include "card.h"
#include "skill.h"

class StandardPackage : public Package
{
    Q_OBJECT

public:
    StandardPackage();
    void addGenerals();
};

class TestPackage : public Package
{
    Q_OBJECT

public:
    TestPackage();
};

class BasicCard : public Card
{
    Q_OBJECT

public:
    BasicCard(Suit suit, int number) : Card(suit, number)
    {
        handling_method = Card::MethodUse;
    }
    virtual QString getType() const;
    virtual CardType getTypeId() const;
};

class TrickCard :public Card
{
    Q_OBJECT

public:
    TrickCard(Suit suit, int number);
    void setCancelable(bool cancelable);

    virtual QString getType() const;
    virtual CardType getTypeId() const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

private:
    bool cancelable;
};

class EquipCard : public Card
{
    Q_OBJECT
    Q_ENUMS(Location)

public:
    enum Location
    {
        WeaponLocation,
        ArmorLocation,
        DefensiveHorseLocation,
        OffensiveHorseLocation,
        TreasureLocation
    };

    EquipCard(Suit suit, int number) : Card(suit, number, true)
    {
        handling_method = MethodUse;
    }

    virtual QString getType() const;
    virtual CardType getTypeId() const;

    virtual bool isAvailable(const Player *player) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual Location location() const = 0;
};

class GlobalEffect : public TrickCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GlobalEffect(Card::Suit suit, int number) : TrickCard(suit, number)
    {
        target_fixed = true;
    }
    virtual QString getSubtype() const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual bool isAvailable(const Player *player) const;
};

class GodSalvation : public GlobalEffect
{
    Q_OBJECT

public:
    Q_INVOKABLE GodSalvation(Card::Suit suit = Heart, int number = 1);
    bool isCancelable(const CardEffectStruct &effect) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class AmazingGrace : public GlobalEffect
{
    Q_OBJECT

public:
    Q_INVOKABLE AmazingGrace(Card::Suit suit, int number);
    void doPreAction(Room *room, const CardUseStruct &card_use) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;

private:
    void clearRestCards(Room *room) const;
};

class AOE : public TrickCard
{
    Q_OBJECT

public:
    AOE(Suit suit, int number) : TrickCard(suit, number)
    {
        target_fixed = true;
    }
    virtual QString getSubtype() const;
    virtual bool isAvailable(const Player *player) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class SavageAssault :public AOE
{
    Q_OBJECT

public:
    Q_INVOKABLE SavageAssault(Card::Suit suit, int number);
    void onEffect(const CardEffectStruct &effect) const;
};

class ArcheryAttack : public AOE
{
    Q_OBJECT

public:
    Q_INVOKABLE ArcheryAttack(Card::Suit suit = Heart, int number = 1);
    void onEffect(const CardEffectStruct &effect) const;
};

class SingleTargetTrick : public TrickCard
{
    Q_OBJECT

public:
    SingleTargetTrick(Suit suit, int number) : TrickCard(suit, number)
    {
    }
    virtual QString getSubtype() const;

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class Collateral : public SingleTargetTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE Collateral(Card::Suit suit, int number);
    bool isAvailable(const Player *player) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void onEffect(const CardEffectStruct &effect) const;

private:
    bool doCollateral(Room *room, ServerPlayer *killer, ServerPlayer *victim, const QString &prompt) const;
};

class ExNihilo : public SingleTargetTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE ExNihilo(Card::Suit suit, int number);
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void onEffect(const CardEffectStruct &effect) const;
    bool isAvailable(const Player *player) const;
};

class Duel : public SingleTargetTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE Duel(Card::Suit suit, int number);
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class DelayedTrick : public TrickCard
{
    Q_OBJECT

public:
    DelayedTrick(Suit suit, int number, bool movable = false);
    virtual void onNullified(ServerPlayer *target) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual QString getSubtype() const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void takeEffect(ServerPlayer *target) const = 0;

protected:
    JudgeStruct judge;

private:
    bool movable;
};

class Indulgence : public DelayedTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE Indulgence(Card::Suit suit, int number);

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void takeEffect(ServerPlayer *target) const;
};

class Disaster : public DelayedTrick
{
    Q_OBJECT

public:
    Disaster(Card::Suit suit, int number);
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual bool isAvailable(const Player *player) const;
};

class Lightning : public Disaster
{
    Q_OBJECT

public:
    Q_INVOKABLE Lightning(Card::Suit suit, int number);

    void takeEffect(ServerPlayer *target) const;
};

class Nullification : public SingleTargetTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE Nullification(Card::Suit suit, int number);

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    bool isAvailable(const Player *player) const;
};

class Weapon : public EquipCard
{
    Q_OBJECT

public:
    Weapon(Suit suit, int number, int range);
    int getRange() const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual QString getCommonEffectName() const;
    virtual bool isAvailable(const Player *player) const;

protected:
    int range;
};

class Armor : public EquipCard
{
    Q_OBJECT

public:
    Armor(Suit suit, int number) : EquipCard(suit, number)
    {
    }
    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual QString getCommonEffectName() const;
};

class Horse : public EquipCard
{
    Q_OBJECT

public:
    Horse(Suit suit, int number, int correct);
    int getCorrect() const;

    virtual Location location() const;
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

    virtual QString getCommonEffectName() const;

private:
    int correct;
};

class Treasure : public EquipCard
{
    Q_OBJECT

public:
    Treasure(Suit suit, int number) : EquipCard(suit, number)
    {
    }
    virtual QString getSubtype() const;

    virtual Location location() const;

    virtual QString getCommonEffectName() const;
};

class OffensiveHorse : public Horse
{
    Q_OBJECT

public:
    Q_INVOKABLE OffensiveHorse(Card::Suit suit, int number, int correct = -1);
    QString getSubtype() const;
};

class DefensiveHorse : public Horse
{
    Q_OBJECT

public:
    Q_INVOKABLE DefensiveHorse(Card::Suit suit, int number, int correct = +1);
    QString getSubtype() const;
};

// cards of standard package

class Slash : public BasicCard
{
    Q_OBJECT

public:
    Q_INVOKABLE Slash(Card::Suit suit, int number);

    inline void setNature(DamageStruct::Nature nature)
    {
        this->nature = nature;
    }
    inline DamageStruct::Nature getNature() const
    {
        return nature;
    }
    inline void addSpecificAssignee(const Player *player)
    {
        specific_assignee << player->objectName();
    }
    inline bool hasSpecificAssignee(const Player *player) const
    {
        return specific_assignee.contains(player->objectName());
    }

    virtual QString getSubtype() const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;

    static bool IsAvailable(const Player *player, const Card *slash = NULL, bool considerSpecificAssignee = true);
    static bool IsSpecificAssignee(const Player *player, const Player *from, const Card *slash);

protected:
    DamageStruct::Nature nature;
    QStringList specific_assignee;
};

class Jink : public BasicCard
{
    Q_OBJECT

public:
    Q_INVOKABLE Jink(Card::Suit suit, int number);
    QString getSubtype() const;
    bool isAvailable(const Player *player) const;
};

class Peach : public BasicCard
{
    Q_OBJECT

public:
    Q_INVOKABLE Peach(Card::Suit suit, int number);
    QString getSubtype() const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void onEffect(const CardEffectStruct &effect) const;
    bool isAvailable(const Player *player) const;
};

class Snatch :public SingleTargetTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE Snatch(Card::Suit suit, int number);

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class Dismantlement : public SingleTargetTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE Dismantlement(Card::Suit suit, int number);

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

#endif

