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
};

class StandardCardPackage : public Package {
    Q_OBJECT

public:
    StandardCardPackage();

    void addWeiSkills();
    void addShuSkills();
    void addWuSkills();
    void addQunSkills();

    QList<Card *> basicCards();
    QList<Card *> equipCards();
    void addEquipSkills();
    QList<Card *> trickCards();
};

#endif
