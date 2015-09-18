#ifndef _THICKET_H
#define _THICKET_H

#include "package.h"
#include "card.h"

class ThicketPackage : public Package
{
    Q_OBJECT

public:
    ThicketPackage();
};

class HaoshiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HaoshiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DimengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DimengCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LuanwuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LuanwuCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

#endif

