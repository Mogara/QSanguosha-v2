#ifndef _YJCM2012_H
#define _YJCM2012_H

#include "package.h"
#include "card.h"
#include "wind.h"

class YJCM2012Package : public Package
{
    Q_OBJECT

public:
    YJCM2012Package();
};

class QiceCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QiceCard();

    bool targetFixed() const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    const Card *validate(CardUseStruct &card_use) const;
};

class GongqiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GongqiCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JiefanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JiefanCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class AnxuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE AnxuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ChunlaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ChunlaoCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ChunlaoWineCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ChunlaoWineCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif

