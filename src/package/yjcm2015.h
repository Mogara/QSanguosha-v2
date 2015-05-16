#ifndef _YJCM2015_H
#define _YJCM2015_H

#include "package.h"
#include "card.h"

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

class YJCM2015Package : public Package
{
    Q_OBJECT

public:
    YJCM2015Package();
};

#endif
