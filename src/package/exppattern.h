#ifndef _EXPPATTERN_H
#define _EXPPATTERN_H

#include "package.h"

class ExpPattern : public CardPattern
{
public:
    ExpPattern(const QString &exp);
    virtual bool match(const Player *player, const Card *card) const;
    virtual QString getPatternString() const
    {
        return exp;
    }
private:
    QString exp;
    bool matchOne(const Player *player, const Card *card, QString exp) const;
};

#endif
