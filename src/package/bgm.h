#ifndef _BGM_H
#define _BGM_H

#include "package.h"
#include "card.h"
#include "standard.h"

class BGMPackage : public Package
{
    Q_OBJECT

public:
    BGMPackage();
};

class LihunCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LihunCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class DaheCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DaheCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class TanhuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TanhuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShichouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShichouCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class YanxiaoCard : public DelayedTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE YanxiaoCard(Card::Suit suit, int number);

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void takeEffect(ServerPlayer *) const;

    QString getType() const
    {
        return "skill_card";
    }
    QString getSubtype() const
    {
        return "skill_card";
    }
    CardType getTypeId() const
    {
        return TypeSkill;
    }
    bool isKindOf(const char *cardType) const
    {
        if (strcmp(cardType, "SkillCard") == 0)
            return true;
        else
            return inherits(cardType);
    }

};

class YinlingCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YinlingCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class JunweiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JunweiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class BGMDIYPackage : public Package
{
    Q_OBJECT

public:
    BGMDIYPackage();
};

class ZhaoxinCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhaoxinCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FuluanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FuluanCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class HuangenCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HuangenCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class HantongCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HantongCard();
    const Card *validate(CardUseStruct &cardUse) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class DIYYicongCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DIYYicongCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif

