#ifndef TARIFF_H
#define TARIFF_H

#include <QString>

struct Tariff
{
    int id = -1;
    QString dayOfWeek;   // "Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"
    int hourFrom = 0;
    int hourTo = 0;
    double pricePerHour = 0.0;
};

#endif // TARIFF_H