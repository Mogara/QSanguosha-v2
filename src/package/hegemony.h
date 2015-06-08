#ifndef _HEGEMONY_H
#define _HEGEMONY_H

#include "package.h"
#include "card.h"

class HegemonyPackage : public Package
{
    Q_OBJECT

public:
    HegemonyPackage();
};

class DuoshiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DuoshiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class FenxunCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FenxunCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class ShuangrenCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShuangrenCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class XiongyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XiongyiCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class QingchengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QingchengCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

#endif

