#ifndef _STANDARD_SKILLCARDS_H
#define _STANDARD_SKILLCARDS_H

#include "skill.h"
#include "card.h"

class ZhihengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhihengCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class RendeCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE RendeCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YijueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YijueCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JieyinCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JieyinCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class TuxiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TuxiCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class FanjianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FanjianCard();
    void onEffect(const CardEffectStruct &effect) const;
};

class KurouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE KurouCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LianyingCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LianyingCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class LijianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LijianCard(bool cancelable = true);

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;

private:
    bool duel_cancelable;
};

class ChuliCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ChuliCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LiuliCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LiuliCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class FenweiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FenweiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YijiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YijiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JianyanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JianyanCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class GuoseCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GuoseCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    const Card *validate(CardUseStruct &cardUse) const;
    void onUse(Room *room, const CardUseStruct &use) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class JijiangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JijiangCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    const Card *validate(CardUseStruct &cardUse) const;
};

class JijiangViewAsSkill : public ZeroCardViewAsSkill
{
public:
    JijiangViewAsSkill();

    bool isEnabledAtPlay(const Player *player) const;
    bool isEnabledAtResponse(const Player *player, const QString &pattern) const;
    const Card *viewAs() const;

private:
    static bool hasShuGenerals(const Player *player);
};

#endif
