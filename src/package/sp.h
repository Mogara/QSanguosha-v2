#ifndef _SP_H
#define _SP_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "wind.h"

class SPPackage : public Package
{
    Q_OBJECT

public:
    SPPackage();
};

class MiscellaneousPackage : public Package
{
    Q_OBJECT

public:
    MiscellaneousPackage();
};

class SPCardPackage : public Package
{
    Q_OBJECT

public:
    SPCardPackage();
};

class HegemonySPPackage : public Package
{
    Q_OBJECT

public:
    HegemonySPPackage();
};

class SPMoonSpear : public Weapon
{
    Q_OBJECT

public:
    Q_INVOKABLE SPMoonSpear(Card::Suit suit = Diamond, int number = 12);
};

class Yongsi : public TriggerSkill
{
public:
    Yongsi();
    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *yuanshu, QVariant &data) const;

protected:
    virtual int getKingdoms(ServerPlayer *yuanshu) const;
};

class WeidiDialog : public QDialog
{
    Q_OBJECT

public:
    static WeidiDialog *getInstance();

public slots:
    void popup();
    void selectSkill(QAbstractButton *button);

private:
    explicit WeidiDialog();

    QAbstractButton *createSkillButton(const QString &skill_name);
    QButtonGroup *group;
    QVBoxLayout *button_layout;

signals:
    void onButtonClick();
};

class YuanhuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YuanhuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class XuejiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XuejiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class BifaCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BifaCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SongciCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE SongciCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class QiangwuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QiangwuCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YinbingCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YinbingCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XiemuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XiemuCard();

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShefuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShefuCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShefuDialog : public GuhuoDialog
{
    Q_OBJECT

public:
    static ShefuDialog *getInstance(const QString &object);

protected:
    explicit ShefuDialog(const QString &object);
    bool isButtonEnabled(const QString &button_name) const;
};

class ZhoufuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhoufuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class QujiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QujiCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class XintanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XintanCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class MeibuFilter : public FilterSkill
{
public:
    MeibuFilter(const QString &skill_name);

    bool viewFilter(const Card *to_select) const;

    const Card *viewAs(const Card *originalCard) const;

private:
    QString n;
};


#endif

