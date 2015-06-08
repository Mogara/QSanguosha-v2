#ifndef WISDOM_H
#define WISDOM_H

#include "package.h"
#include "card.h"

class JuaoCard :public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JuaoCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class BawangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BawangCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class FuzuoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FuzuoCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class WeidaiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE WeidaiCard();

    const Card *validate(CardUseStruct &card_use) const;
    const Card *validateInResponse(ServerPlayer *user) const;
};

class HouyuanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HouyuanCard();

    void onEffect(const CardEffectStruct &effect) const;
};

class ShouyeCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShouyeCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class WisdomPackage : public Package
{
    Q_OBJECT

public:
    WisdomPackage();
};

#endif // WISDOM_H
