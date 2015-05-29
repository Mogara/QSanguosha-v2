#ifndef _YJCM2015_H
#define _YJCM2015_H

#include "package.h"
#include "card.h"
#include "wind.h"

class FurongCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FurongCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class YjYanyuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YjYanyuCard();
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class JigongCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JigongCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class HuomoDialog : public GuhuoDialog
{
    Q_OBJECT

public:
    static HuomoDialog *getInstance();

protected:
    explicit HuomoDialog();
    bool isButtonEnabled(const QString &button_name) const;
};

class HuomoCard : public SkillCard
{
    Q_OBJECT 

public:
    Q_INVOKABLE HuomoCard();
    bool targetFixed() const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    const Card *validate(CardUseStruct &cardUse) const;
    const Card *validateInResponse(ServerPlayer *user) const;
};

class YJCM2015Package : public Package
{
    Q_OBJECT

public:
    YJCM2015Package();
};

#endif
