#ifndef TW_PACKAGE_H
#define TW_PACKAGE_H

#include "package.h"

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

#endif