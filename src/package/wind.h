#ifndef _WIND_H
#define _WIND_H

#include "package.h"
#include "card.h"
#include "skill.h"

class HuangtianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HuangtianCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class ShensuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShensuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class TianxiangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TianxiangCard();

    void onEffect(const CardEffectStruct &effect) const;
};

class GuhuoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GuhuoCard();
    bool guhuo(ServerPlayer *yuji) const;

    bool targetFixed() const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    const Card *validate(CardUseStruct &card_use) const;
    const Card *validateInResponse(ServerPlayer *user) const;
};

class GuhuoDialog : public QDialog
{
    Q_OBJECT

public:
    static GuhuoDialog *getInstance(const QString &object, bool left = true, bool right = true,
        bool play_only = true, bool slash_combined = false, bool delayed_tricks = false);

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

protected:
    explicit GuhuoDialog(const QString &object, bool left = true, bool right = true,
        bool play_only = true, bool slash_combined = false, bool delayed_tricks = false);
    virtual bool isButtonEnabled(const QString &button_name) const;
    QAbstractButton *createButton(const Card *card);

    QHash<QString, const Card *> map;

private:
    QGroupBox *createLeft();
    QGroupBox *createRight();
    QButtonGroup *group;

    QString object_name;
    bool play_only; // whether the dialog will pop only during the Play phase
    bool slash_combined; // create one 'Slash' button instead of 'Slash', 'Fire Slash', 'Thunder Slash'
    bool delayed_tricks; // whether buttons of Delayed Tricks will be created

signals:
    void onButtonClick();
};

class Jushou : public PhaseChangeSkill
{
public:
    Jushou();
    bool onPhaseChange(ServerPlayer *target) const;

protected:
    virtual int getJushouDrawNum(ServerPlayer *caoren) const;
};

class WindPackage : public Package
{
    Q_OBJECT

public:
    WindPackage();
};

#endif

