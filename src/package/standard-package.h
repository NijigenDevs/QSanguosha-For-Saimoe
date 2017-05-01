#ifndef _STANDARD_PACKAGE_H
#define _STANDARD_PACKAGE_H

#include "package.h"

class StandardPackage : public Package {
    Q_OBJECT

public:
    StandardPackage();
};

class TestPackage : public Package {
    Q_OBJECT

public:
    TestPackage();
    void addWeiSkills();
    void addShuSkills();
    void addWuSkills();
    void addQunSkills();
};

class StandardCardPackage : public Package {
    Q_OBJECT

public:
    StandardCardPackage();

    QList<Card *> basicCards();
    QList<Card *> equipCards();
    void addEquipSkills();
    QList<Card *> trickCards();
};

#endif
