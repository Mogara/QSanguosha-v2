#ifndef _H_FORMATION_H
#define _H_FORMATION_H

#include "package.h"
#include "card.h"

class ZiliangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZiliangCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HuyuanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HuyuanCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class HeyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HeyiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShangyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShangyiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class HFormationPackage : public Package
{
    Q_OBJECT

public:
    HFormationPackage();
};

#endif
