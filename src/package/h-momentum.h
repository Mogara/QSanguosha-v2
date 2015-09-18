#ifndef _H_MOMENTUM_H
#define _H_MOMENTUM_H

#include "package.h"
#include "card.h"

class GuixiuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GuixiuCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class CunsiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE CunsiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class DuanxieCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DuanxieCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class HMomentumPackage : public Package
{
    Q_OBJECT

public:
    HMomentumPackage();
};

#endif
