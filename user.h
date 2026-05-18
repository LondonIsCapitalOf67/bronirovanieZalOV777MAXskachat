#ifndef USER_H
#define USER_H

#include <QString>

struct User
{
    int id = -1;
    QString fullName;
    QString phone;
    QString sport;
    QString login;
    QString passwordHash;
    bool approved = false;
    bool personalDataConsent = false;
};

#endif // USER_H