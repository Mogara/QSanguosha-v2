#ifndef _YJCM2013_H
#define _YJCM2013_H

#include "package.h"
#include "card.h"
#include "skill.h"

class YJCM2013Package : public Package
{
    Q_OBJECT

public:
    YJCM2013Package();
};

class JunxingCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JunxingCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class QiaoshuiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QiaoshuiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ExtraCollateralCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ExtraCollateralCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class XiansiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XiansiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class XiansiSlashCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XiansiSlashCard();

    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    const Card *validate(CardUseStruct &cardUse) const;
};

class ZongxuanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZongxuanCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class MiejiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MiejiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class FenchengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FenchengCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class DanshouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DanshouCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class Chengxiang : public MasochismSkill
{
public:
    Chengxiang();
    void onDamaged(ServerPlayer *target, const DamageStruct &damage) const;

protected:
    int total_point;
};

#endif
