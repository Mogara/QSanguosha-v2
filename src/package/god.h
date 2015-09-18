#ifndef _GOD_H
#define _GOD_H

#include "package.h"
#include "card.h"
#include "skill.h"

class GodPackage : public Package
{
    Q_OBJECT

public:
    GodPackage();
};

class GongxinCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GongxinCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class YeyanCard : public SkillCard
{
    Q_OBJECT

public:
    void damage(ServerPlayer *shenzhouyu, ServerPlayer *target, int point) const;
};

class GreatYeyanCard : public YeyanCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GreatYeyanCard();

    bool targetFilter(const QList<const Player *> &targets,const Player *to_select, const Player *Self) const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select,const Player *Self, int &maxVotes) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SmallYeyanCard : public YeyanCard
{
    Q_OBJECT

public:
    Q_INVOKABLE SmallYeyanCard();
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class ShenfenCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShenfenCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class WuqianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE WuqianCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class QixingCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QixingCard();
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class KuangfengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE KuangfengCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class DawuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DawuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JilveCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JilveCard();

    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class Longhun : public ViewAsSkill
{
public:
    Longhun();
    bool isEnabledAtResponse(const Player *player, const QString &pattern) const;
    bool isEnabledAtPlay(const Player *player) const;
    bool viewFilter(const QList<const Card *> &selected, const Card *card) const;
    const Card *viewAs(const QList<const Card *> &cards) const;
    int getEffectIndex(const ServerPlayer *player, const Card *card) const;
    bool isEnabledAtNullification(const ServerPlayer *player) const;

protected:
    virtual int getEffHp(const Player *zhaoyun) const;
};

#endif

