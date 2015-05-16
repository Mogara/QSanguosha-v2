#ifndef _SP_H
#define _SP_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "wind.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>
#include <QVBoxLayout>

class SPPackage : public Package
{
    Q_OBJECT

public:
    SPPackage();
};

class OLPackage : public Package
{
    Q_OBJECT

public:
    OLPackage();
};

class TaiwanSPPackage : public Package
{
    Q_OBJECT

public:
    TaiwanSPPackage();
};

class TaiwanYJCMPackage : public Package
{
    Q_OBJECT

public:
    TaiwanYJCMPackage();
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

class JSPPackage : public Package
{
    Q_OBJECT

public:
    JSPPackage();
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
    int getKingdoms(ServerPlayer *yuanshu) const;
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

class ZhoufuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhoufuCard();

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

class JieyueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JieyueCard();

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

#endif

