#ifndef _MOESEN_PACKAGE_H
#define _MOESEN_PACKAGE_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class MoesenPackage : public Package
{
    Q_OBJECT

public:
    MoesenPackage();

    void addGameGenerals();
    void addNovelGenerals();
    void addAnimationGenerals();
    void addComicGenerals();
};
/*

class StandardCardPackage: public Package {
    Q_OBJECT

public:
    StandardCardPackage();

    QList<Card *> basicCards();
    QList<Card *> equipCards();
    void addEquipSkills();
    QList<Card *> trickCards();
};
*/
#endif