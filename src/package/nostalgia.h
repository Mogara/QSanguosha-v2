#ifndef _NOSTALGIA_H
#define _NOSTALGIA_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "standard-skillcards.h"

class NostalgiaPackage : public Package
{
    Q_OBJECT

public:
    NostalgiaPackage();
};

class MoonSpear : public Weapon
{
    Q_OBJECT

public:
    Q_INVOKABLE MoonSpear(Card::Suit suit = Diamond, int number = 12);
};

class NostalStandardPackage : public Package
{
    Q_OBJECT

public:
    NostalStandardPackage();
};

class NostalWindPackage : public Package
{
    Q_OBJECT

public:
    NostalWindPackage();
};

class NostalYJCMPackage : public Package
{
    Q_OBJECT

public:
    NostalYJCMPackage();
};

class NostalYJCM2012Package : public Package
{
    Q_OBJECT

public:
    NostalYJCM2012Package();
};

class NostalYJCM2013Package : public Package
{
    Q_OBJECT

public:
    NostalYJCM2013Package();
};

class NosJujianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosJujianCard();

    void onEffect(const CardEffectStruct &effect) const;
};

class NosXuanhuoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosXuanhuoCard();

    void onEffect(const CardEffectStruct &effect) const;
};

class NosJiefanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosJiefanCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosRenxinCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosRenxinCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosFenchengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosFenchengCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class NosTuxiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosTuxiCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class NosRendeCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosRendeCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosKurouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosKurouCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosFanjianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosFanjianCard();
    void onEffect(const CardEffectStruct &effect) const;
};

class NosLijianCard : public LijianCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosLijianCard();
};

class QingnangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class NosGuhuoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosGuhuoCard();
    bool nosguhuo(ServerPlayer *yuji) const;

    bool targetFixed() const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    const Card *validate(CardUseStruct &card_use) const;
    const Card *validateInResponse(ServerPlayer *user) const;
};

class NosYiji : public MasochismSkill
{
public:
    NosYiji();
    void onDamaged(ServerPlayer *target, const DamageStruct &damage) const;

protected:
    int n;
};

#endif

