#ifndef _YJCM_H
#define _YJCM_H

#include "package.h"
#include "card.h"
#include "skill.h"

class YJCMPackage : public Package
{
    Q_OBJECT

public:
    YJCMPackage();
};

class Shangshi : public TriggerSkill
{
public:
    Shangshi();
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *zhangchunhua, QVariant &data) const;

protected:
    virtual int getMaxLostHp(ServerPlayer *zhangchunhua) const;
};

class MingceCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MingceCard();

    void onEffect(const CardEffectStruct &effect) const;
};

class GanluCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GanluCard();
    void swapEquip(ServerPlayer *first, ServerPlayer *second) const;

    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XianzhenCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XianzhenCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class JujianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JujianCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class XinzhanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XinzhanCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class PaiyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE PaiyiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

#endif

