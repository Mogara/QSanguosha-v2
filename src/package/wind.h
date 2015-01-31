#ifndef _WIND_H
#define _WIND_H

#include "package.h"
#include "card.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>

class HuangtianCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HuangtianCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class ShensuCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShensuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class TianxiangCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE TianxiangCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GuhuoCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE GuhuoCard();
    bool guhuo(ServerPlayer *yuji) const;

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class GuhuoDialog: public QDialog {
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

private:
    QGroupBox *createLeft();
    QGroupBox *createRight();
    QAbstractButton *createButton(const Card *card);
    QButtonGroup *group;
    QHash<QString, const Card *> map;

    QString object_name;
    bool play_only; // whether the dialog will pop only during the Play phase
    bool slash_combined; // create one 'Slash' button instead of 'Slash', 'Fire Slash', 'Thunder Slash'
    bool delayed_tricks; // whether buttons of Delayed Tricks will be created

signals:
    void onButtonClick();
};

#include "skill.h"
class Jushou: public PhaseChangeSkill {
    Q_OBJECT

public:
    Jushou();
    virtual bool onPhaseChange(ServerPlayer *target) const;

protected:
    virtual int getJushouDrawNum(ServerPlayer *caoren) const;
};

class WindPackage: public Package {
    Q_OBJECT

public:
    WindPackage();
};

#endif

