#ifndef YITIANPACKAGE_H
#define YITIANPACKAGE_H

#include "package.h"
#include "standard.h"

class YitianPackage : public Package
{
    Q_OBJECT

public:
    YitianPackage();
};

class YTChengxiangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YTChengxiangCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class JuejiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JuejiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class YitianSword :public Weapon
{
    Q_OBJECT

public:
    Q_INVOKABLE YitianSword(Card::Suit suit = Spade, int number = 6);

    void onUninstall(ServerPlayer *player) const;
};

/*
class LianliCard: public SkillCard{
Q_OBJECT

public:
Q_INVOKABLE LianliCard();

void onEffect(const CardEffectStruct &effect) const;
bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};*/

class LianliSlashCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LianliSlashCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    const Card *validate(CardUseStruct &cardUse) const;
};

class GuihanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GuihanCard();

    void onEffect(const CardEffectStruct &effect) const;
};

class LexueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LexueCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class XunzhiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XunzhiCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YtYisheCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YtYisheCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YtYisheAskCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YtYisheAskCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class TaichenCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TaichenCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class TouduCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TouduCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class YitianCardPackage : public Package
{
    Q_OBJECT

public:
    YitianCardPackage();
};

#endif // YITIANPACKAGE_H
