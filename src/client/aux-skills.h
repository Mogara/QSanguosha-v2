#ifndef _AUX_SKILLS_H
#define _AUX_SKILLS_H

#include "skill.h"

class DiscardSkill : public ViewAsSkill
{
    Q_OBJECT

public:
    explicit DiscardSkill();

    void setNum(int num);
    void setMinNum(int minnum);
    void setIncludeEquip(bool include_equip);
    void setIsDiscard(bool is_discard);
    void setPattern(const QString &pattern);

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    const Card *viewAs(const QList<const Card *> &cards) const;

private:
    DummyCard *card;
    int num;
    int minnum;
    bool include_equip;
    bool is_discard;
    QString pattern;
};

class CardPattern;

class ResponseSkill : public OneCardViewAsSkill
{
    Q_OBJECT

public:
    ResponseSkill();
    bool matchPattern(const Player *player, const Card *card) const;

    void setPattern(const QString &pattern);
    void setRequest(const Card::HandlingMethod request);
    bool viewFilter(const Card *to_select) const;
    const Card *viewAs(const Card *originalCard) const;

    inline Card::HandlingMethod getRequest() const
    {
        return request;
    }

protected:
    const CardPattern *pattern;
    Card::HandlingMethod request;
};

class ShowOrPindianSkill : public ResponseSkill
{
    Q_OBJECT

public:
    ShowOrPindianSkill();
    bool matchPattern(const Player *player, const Card *card) const;
};

class NosYijiCard;

class NosYijiViewAsSkill : public ViewAsSkill
{
    Q_OBJECT

public:
    explicit NosYijiViewAsSkill();
    void setCards(const QString &card_str);
    void setMaxNum(int max_num);
    void setPlayerNames(const QStringList &names);

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const;
    const Card *viewAs(const QList<const Card *> &cards) const;

private:
    NosYijiCard *card;
    QList<int> ids;
    int max_num;
};

class ChoosePlayerCard;

class ChoosePlayerSkill : public ZeroCardViewAsSkill
{
    Q_OBJECT

public:
    explicit ChoosePlayerSkill();
    void setPlayerNames(const QStringList &names);

    const Card *viewAs() const;

private:
    ChoosePlayerCard *card;
};

#endif

