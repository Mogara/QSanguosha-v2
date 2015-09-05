#ifndef OL_PACKAGE_H
#define OL_PACKAGE_H

#include "package.h"
#include "card.h"

class OLPackage : public Package
{
    Q_OBJECT

public:
    OLPackage();
};


class AocaiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE AocaiCard();

    bool targetFixed() const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    const Card *validateInResponse(ServerPlayer *user) const;
    const Card *validate(CardUseStruct &cardUse) const;
};

class DuwuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DuwuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class QingyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QingyiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SanyaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE SanyaoCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class JieyueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JieyueCard();

    void onEffect(const CardEffectStruct &effect) const;
};

class ShuliangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShuliangCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

};

class ZhanyiViewAsBasicCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhanyiViewAsBasicCard();

    bool targetFixed() const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    const Card *validate(CardUseStruct &cardUse) const;
    const Card *validateInResponse(ServerPlayer *user) const;
};

class ZhanyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhanyiCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class OlMumuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE OlMumuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class OlMumu2Card : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE OlMumu2Card();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class BushiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BushiCard();
    void onUse(Room *, const CardUseStruct &card_use) const;
};

class MidaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MidaoCard();

    void onUse(Room *, const CardUseStruct &card_use) const;
};

class OlRendeCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE OlRendeCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class OlQingjianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE OlQingjianCard();
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class OlAnxuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE OlAnxuCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif
