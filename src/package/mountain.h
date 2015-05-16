#ifndef _MOUNTAIN_H
#define _MOUNTAIN_H

#include "package.h"
#include "card.h"
#include "generaloverview.h"

class QiaobianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QiaobianCard();

    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class TiaoxinCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TiaoxinCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class ZhijianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhijianCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class GuzhengCard : public SkillCard
{
    Q_OBJECT
        
public:
    Q_INVOKABLE GuzhengCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZhibaCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhibaCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FangquanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FangquanCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class HuashenDialog : public GeneralOverview
{
    Q_OBJECT

public:
    HuashenDialog();

public slots:
    void popup();
};

class MountainPackage : public Package
{
    Q_OBJECT

public:
    MountainPackage();
};

#endif

