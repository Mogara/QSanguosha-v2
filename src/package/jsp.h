#ifndef JSP_PACKAGE_H
#define JSP_PACKAGE_H

#include "package.h"
#include "card.h"

class JSPPackage : public Package
{
    Q_OBJECT

public:
    JSPPackage();
};

class JiqiaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JiqiaoCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif
