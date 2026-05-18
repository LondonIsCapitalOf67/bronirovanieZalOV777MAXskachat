#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QList>
#include <QString>
#include <QDate>
#include <QTime>
#include "user.h"
#include "booking.h"
#include "tariff.h"

class Database
{
public:
    static Database* instance();

    // Инициализация БД (создание таблиц, предзаполнение, администратор)
    bool initialize();

    // Пользователи
    bool addUser(const QString &fullName, const QString &phone,
                 const QString &sport, const QString &login,
                 const QString &password);
    User getUserByLogin(const QString &login) const;
    bool approveUser(int userId);
    bool setPersonalDataConsent(int userId, bool consent);
    bool deleteUser(int userId);
    QList<User> getAllUsers() const;

    // Бронирования
    bool addBooking(const Booking &booking);
    bool removeBooking(int bookingId);
    QList<Booking> getBookingsForUser(int userId, const QDate &from, const QDate &to) const;
    QList<Booking> getAllBookings(const QDate &from, const QDate &to) const;
    QList<Booking> getAllBookings() const; // без фильтра дат
    bool isTimeSlotAvailable(const QString &hall, const QDate &date,
                             const QTime &start, const QTime &end, int excludeBookingId = -1) const;
    // Автоматическое продление постоянных бронирований на следующий месяц
    void generateRecurringBookingsForMonth(const QDate &monthStart);

    // Тарифы
    bool addTariff(const QString &dayOfWeek, int hourFrom, int hourTo, double price);
    bool removeTariff(int tariffId);
    QList<Tariff> getTariffs() const;
    double getHourlyRate(const QString &dayOfWeek, int hour) const;

    // Настройки
    double getMonthlySubscriptionCost() const;
    bool setMonthlySubscriptionCost(double cost);

    // Залы
    QStringList getHalls() const;

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    QSqlDatabase m_db;
    static Database* s_instance;
};

#endif // DATABASE_H