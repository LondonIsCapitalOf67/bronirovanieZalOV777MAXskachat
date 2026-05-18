#ifndef BOOKING_H
#define BOOKING_H

#include <QDate>
#include <QTime>
#include <QString>

struct Booking
{
    int         id = -1;
    QDate       date;
    QTime       startTime;
    QTime       endTime;
    QString     hallName;
    QString     userFullName;
    QString     sportType;
    double      cost = 0.0;
    int         userId = -1;        // при необходимости
    bool        isRecurring = false; // постоянное бронирование
};

#endif // BOOKING_H