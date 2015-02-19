#ifndef _YJCM2014_H
#define _YJCM2014_H

#include "package.h"
#include "card.h"

class DingpinCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE DingpinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShenxingCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShenxingCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class BingyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE BingyiCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XianzhouCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XianzhouCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XianzhouDamageCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE XianzhouDamageCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SidiCard : public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE SidiCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YJCM2014Package: public Package {
    Q_OBJECT

public:
    YJCM2014Package();
};

#endif
