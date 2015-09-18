#ifndef _ASSASSINS_H
#define _ASSASSINS_H

#include "package.h"
#include "card.h"
//#include "skill.h"
//#include "standard.h"


class AssassinsPackage : public Package
{
    Q_OBJECT

public:
    AssassinsPackage();
};

class MizhaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MizhaoCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class MixinCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MixinCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class DuyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DuyiCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

#endif

