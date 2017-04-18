

#include "package.h"
#include "skill.h"

void Package::insertRelatedSkills(const QString &main_skill, int n, ...)
{
    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; ++i)
    {
        QString c = va_arg(ap, const char *);
        related_skills.insertMulti(main_skill, c);
    }
    va_end(ap);
}

Package::~Package()
{
    foreach(const Skill *skill, skills)
        delete skill;

    foreach(const QString key, patterns.keys())
        delete patterns[key];
}

Q_GLOBAL_STATIC(PackageHash, Packages)
PackageHash &PackageAdder::packages()
{
    return *(::Packages());
}

