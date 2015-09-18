#ifndef BOSSCHALLENGE_H
#define BOSSCHALLENGE_H

#include "scenario.h"
//#include "maneuvering.h"

class ImpasseScenario : public Scenario
{
    Q_OBJECT

public:
    explicit ImpasseScenario();

    bool exposeRoles() const;
    void assign(QStringList &generals, QStringList &roles) const;
    int getPlayerCount() const;
    QString getRoles() const;
    void onTagSet(Room *room, const QString &key) const;
    bool generalSelection() const;
};

#endif // BOSSCHALLENGE_H
