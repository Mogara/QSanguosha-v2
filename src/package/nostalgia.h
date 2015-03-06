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

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NosXuanhuoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosXuanhuoCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NosJiefanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosJiefanCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosRenxinCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosRenxinCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosFenchengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosFenchengCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NosTuxiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosTuxiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NosRendeCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosRendeCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosKurouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosKurouCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NosFanjianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosFanjianCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
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
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class NosGuhuoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NosGuhuoCard();
    bool nosguhuo(ServerPlayer *yuji) const;

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class NosYiji : public MasochismSkill
{
public:
    NosYiji();
    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const;

protected:
    int n;
};

#endif

