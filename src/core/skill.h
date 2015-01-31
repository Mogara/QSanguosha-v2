#ifndef _SKILL_H
#define _SKILL_H

class Player;
class Card;
class ServerPlayer;
class QDialog;

#include "room.h"

#include <QObject>

class Skill: public QObject {
    Q_OBJECT
    Q_ENUMS(Frequency)

public:
    enum Frequency {
        Frequent,
        NotFrequent,
        Compulsory,
        Limited,
        Wake
    };

    explicit Skill(const QString &name, Frequency frequent = NotFrequent);
    bool isLordSkill() const;
    bool isAttachedLordSkill() const;
    virtual bool shouldBeVisible(const Player *Self) const; // usually for attached skill
    QString getDescription() const;
    QString getNotice(int index) const;
    bool isVisible() const;

    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const;
    virtual QDialog *getDialog() const;

    void initMediaSource();
    void playAudioEffect(int index = -1, bool superpose = true) const;
    Frequency getFrequency() const;
    QString getLimitMark() const;
    QStringList getSources() const;

protected:
    Frequency frequency;
    QString limit_mark;
    QString default_choice;
    bool attached_lord_skill;

private:
    bool lord_skill;
    QStringList sources;
};

class ViewAsSkill: public Skill {
    Q_OBJECT

public:
    ViewAsSkill(const QString &name);

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const = 0;
    virtual const Card *viewAs(const QList<const Card *> &cards) const = 0;

    bool isAvailable(const Player *invoker, CardUseStruct::CardUseReason reason, const QString &pattern) const;
    virtual bool isEnabledAtPlay(const Player *player) const;
    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const;
    virtual bool isEnabledAtNullification(const ServerPlayer *player) const;
    static const ViewAsSkill *parseViewAsSkill(const Skill *skill);

    inline bool isResponseOrUse() const{ return response_or_use; }
    inline QString getExpandPile() const{ return expand_pile; }

protected:
    QString response_pattern;
    bool response_or_use;
    QString expand_pile;
};

class ZeroCardViewAsSkill: public ViewAsSkill {
    Q_OBJECT

public:
    ZeroCardViewAsSkill(const QString &name);

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    virtual const Card *viewAs(const QList<const Card *> &cards) const;
    virtual const Card *viewAs() const = 0;
};

class OneCardViewAsSkill: public ViewAsSkill {
    Q_OBJECT

public:
    OneCardViewAsSkill(const QString &name);

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    virtual const Card *viewAs(const QList<const Card *> &cards) const;

    virtual bool viewFilter(const Card *to_select) const;
    virtual const Card *viewAs(const Card *originalCard) const = 0;

protected:
    QString filter_pattern;
};

class FilterSkill: public OneCardViewAsSkill {
    Q_OBJECT

public:
    FilterSkill(const QString &name);
};

class TriggerSkill: public Skill {
    Q_OBJECT

public:
    TriggerSkill(const QString &name);
    const ViewAsSkill *getViewAsSkill() const;
    QList<TriggerEvent> getTriggerEvents() const;

    virtual int getPriority(TriggerEvent triggerEvent) const;
    virtual bool triggerable(const ServerPlayer *target, Room *room) const;
    virtual bool triggerable(const ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const = 0;

    inline double getDynamicPriority() const{ return dynamic_priority; }
    inline void setDynamicPriority(double value) { dynamic_priority = value; }

    inline bool isGlobal() const{ return global; }

protected:
    const ViewAsSkill *view_as_skill;
    QList<TriggerEvent> events;
    bool global;

private:
    mutable double dynamic_priority;
};

class Scenario;

class ScenarioRule: public TriggerSkill {
    Q_OBJECT

public:
    ScenarioRule(Scenario *scenario);

    virtual int getPriority(TriggerEvent triggerEvent) const;
    virtual bool triggerable(const ServerPlayer *target) const;
};

class MasochismSkill: public TriggerSkill {
    Q_OBJECT

public:
    MasochismSkill(const QString &name);

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const = 0;
};

class PhaseChangeSkill: public TriggerSkill {
    Q_OBJECT

public:
    PhaseChangeSkill(const QString &name);

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual bool onPhaseChange(ServerPlayer *target) const = 0;
};

class DrawCardsSkill: public TriggerSkill {
    Q_OBJECT

public:
    DrawCardsSkill(const QString &name, bool is_initial = false);

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual int getDrawNum(ServerPlayer *player, int n) const = 0;

protected:
    bool is_initial;
};

class GameStartSkill: public TriggerSkill {
    Q_OBJECT

public:
    GameStartSkill(const QString &name);

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual void onGameStart(ServerPlayer *player) const = 0;
};

class SPConvertSkill: public GameStartSkill {
    Q_OBJECT

public:
    SPConvertSkill(const QString &from, const QString &to);

    virtual bool triggerable(const ServerPlayer *target) const;
    virtual void onGameStart(ServerPlayer *player) const;

private:
    QString from, to;
    QStringList to_list;
};

class ProhibitSkill: public Skill {
    Q_OBJECT

public:
    ProhibitSkill(const QString &name);

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const = 0;
};

class DistanceSkill: public Skill {
    Q_OBJECT

public:
    DistanceSkill(const QString &name);

    virtual int getCorrect(const Player *from, const Player *to) const = 0;
};

class MaxCardsSkill: public Skill {
    Q_OBJECT

public:
    MaxCardsSkill(const QString &name);

    virtual int getExtra(const Player *target) const;
    virtual int getFixed(const Player *target) const;
};

class TargetModSkill: public Skill {
    Q_OBJECT
    Q_ENUMS(ModType)

public:
    enum ModType {
        Residue,
        DistanceLimit,
        ExtraTarget
    };

    TargetModSkill(const QString &name);
    virtual QString getPattern() const;

    virtual int getResidueNum(const Player *from, const Card *card) const;
    virtual int getDistanceLimit(const Player *from, const Card *card) const;
    virtual int getExtraTargetNum(const Player *from, const Card *card) const;

protected:
    QString pattern;
};

class SlashNoDistanceLimitSkill: public TargetModSkill {
    Q_OBJECT

public:
    SlashNoDistanceLimitSkill(const QString &skill_name);

    virtual int getDistanceLimit(const Player *from, const Card *card) const;

protected:
    QString name;
};

class InvaliditySkill: public Skill {
    Q_OBJECT

public:
    InvaliditySkill(const QString &skill_name);

    virtual bool isSkillValid(const Player *player, const Skill *skill) const = 0;
};

// a nasty way for 'fake moves', usually used in the process of multi-card chosen
class FakeMoveSkill: public TriggerSkill {
    Q_OBJECT

public:
    FakeMoveSkill(const QString &skillname);

    virtual int getPriority(TriggerEvent triggerEvent) const;
    virtual bool triggerable(const ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;

private:
    QString name;
};

class DetachEffectSkill: public TriggerSkill {
    Q_OBJECT

public:
    DetachEffectSkill(const QString &skillname, const QString &pilename = QString());

    virtual bool triggerable(const ServerPlayer *target) const;
    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const;
    virtual void onSkillDetached(Room *room, ServerPlayer *player) const;

private:
    QString name, pile_name;
};

class WeaponSkill: public TriggerSkill {
    Q_OBJECT

public:
    WeaponSkill(const QString &name);

    virtual bool triggerable(const ServerPlayer *target) const;
};

class ArmorSkill: public TriggerSkill {
    Q_OBJECT

public:
    ArmorSkill(const QString &name);

    virtual bool triggerable(const ServerPlayer *target) const;
};

class TreasureSkill: public TriggerSkill {
    Q_OBJECT

public:
    TreasureSkill(const QString &name);

    virtual bool triggerable(const ServerPlayer *target) const;
};

class MarkAssignSkill: public GameStartSkill {
    Q_OBJECT

public:
    MarkAssignSkill(const QString &mark, int n);

    virtual void onGameStart(ServerPlayer *player) const;

private:
    QString mark_name;
    int n;
};

#endif

