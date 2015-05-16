#ifndef _FANCHENG_SCENARIO_H
#define _FANCHENG_SCENARIO_H

#include "scenario.h"
#include "card.h"

class FanchengScenario : public Scenario
{
    Q_OBJECT

public:
    FanchengScenario();

    void onTagSet(Room *room, const QString &key) const;
};

class DujiangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DujiangCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FloodCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FloodCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class TaichenFightCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TaichenFightCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZhiyuanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhiyuanCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif

