#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

Database* Database::s_instance = nullptr;

Database::Database()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString dbPath = dataDir + "/hall_booking.db";
    m_db.setDatabaseName(dbPath);
}

Database::~Database()
{
    if (m_db.isOpen())
        m_db.close();
}

Database* Database::instance()
{
    if (!s_instance) {
        s_instance = new Database;
    }
    return s_instance;
}

bool Database::initialize()
{
    if (!m_db.open()) {
        qCritical() << "Не удалось открыть БД:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.exec("PRAGMA foreign_keys = ON");

    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "full_name TEXT NOT NULL, "
            "phone TEXT, "
            "sport TEXT, "
            "login TEXT UNIQUE NOT NULL, "
            "password_hash TEXT NOT NULL, "
            "approved INTEGER DEFAULT 0, "
            "personal_data_consent INTEGER DEFAULT 0)")) {
        qCritical() << "Ошибка создания таблицы users:" << q.lastError().text();
        return false;
    }

    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS halls ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT UNIQUE NOT NULL)")) {
        qCritical() << "Ошибка создания таблицы halls:" << q.lastError().text();
        return false;
    }

    q.exec("SELECT COUNT(*) FROM halls");
    if (q.next() && q.value(0).toInt() == 0) {
        q.exec("INSERT INTO halls (name) VALUES ('Зал 1')");
        q.exec("INSERT INTO halls (name) VALUES ('Зал 2')");
        q.exec("INSERT INTO halls (name) VALUES ('Зал 3')");
    }

    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS bookings ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "user_id INTEGER NOT NULL, "
            "date TEXT NOT NULL, "
            "start_time TEXT NOT NULL, "
            "end_time TEXT NOT NULL, "
            "hall_name TEXT NOT NULL, "
            "sport_type TEXT, "
            "cost REAL DEFAULT 0.0, "
            "is_recurring INTEGER DEFAULT 0, "
            "FOREIGN KEY (user_id) REFERENCES users(id))")) {
        qCritical() << "Ошибка создания таблицы bookings:" << q.lastError().text();
        return false;
    }

    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS tariffs ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "day_of_week TEXT NOT NULL, "
            "hour_from INTEGER NOT NULL, "
            "hour_to INTEGER NOT NULL, "
            "price_per_hour REAL NOT NULL)")) {
        qCritical() << "Ошибка создания таблицы tariffs:" << q.lastError().text();
        return false;
    }

    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS settings ("
            "key TEXT PRIMARY KEY, "
            "value TEXT)")) {
        qCritical() << "Ошибка создания таблицы settings:" << q.lastError().text();
        return false;
    }

    q.exec("INSERT OR IGNORE INTO settings (key, value) VALUES ('monthly_cost', '5000')");

    q.exec("SELECT COUNT(*) FROM users WHERE login = 'admin'");
    if (q.next() && q.value(0).toInt() == 0) {
        QString adminPasswordHash = QString::fromLatin1(
            QCryptographicHash::hash("admin", QCryptographicHash::Sha256).toHex()
            );
        q.prepare("INSERT INTO users (full_name, phone, sport, login, password_hash, approved, personal_data_consent) "
                  "VALUES (?, ?, ?, ?, ?, 1, 1)");
        q.addBindValue("Администратор");
        q.addBindValue("");
        q.addBindValue("");
        q.addBindValue("admin");
        q.addBindValue(adminPasswordHash);
        if (!q.exec()) {
            qWarning() << "Не удалось создать администратора:" << q.lastError().text();
        }
    }

    return true;
}

// ---------- Пользователи ----------
bool Database::addUser(const QString &fullName, const QString &phone,
                       const QString &sport, const QString &login,
                       const QString &password)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO users (full_name, phone, sport, login, password_hash) "
              "VALUES (?, ?, ?, ?, ?)");
    q.addBindValue(fullName);
    q.addBindValue(phone);
    q.addBindValue(sport);
    q.addBindValue(login);
    q.addBindValue(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    return q.exec();
}

User Database::getUserByLogin(const QString &login) const
{
    User u;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, full_name, phone, sport, login, password_hash, approved, personal_data_consent "
              "FROM users WHERE login = ?");
    q.addBindValue(login);
    if (q.exec() && q.next()) {
        u.id = q.value(0).toInt();
        u.fullName = q.value(1).toString();
        u.phone = q.value(2).toString();
        u.sport = q.value(3).toString();
        u.login = q.value(4).toString();
        u.passwordHash = q.value(5).toString();
        u.approved = q.value(6).toBool();
        u.personalDataConsent = q.value(7).toBool();
    }
    return u;
}

User Database::getUserById(int userId) const
{
    User u;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, full_name, phone, sport, login, password_hash, approved, personal_data_consent "
              "FROM users WHERE id = ?");
    q.addBindValue(userId);
    if (q.exec() && q.next()) {
        u.id = q.value(0).toInt();
        u.fullName = q.value(1).toString();
        u.phone = q.value(2).toString();
        u.sport = q.value(3).toString();
        u.login = q.value(4).toString();
        u.passwordHash = q.value(5).toString();
        u.approved = q.value(6).toBool();
        u.personalDataConsent = q.value(7).toBool();
    }
    return u;
}

bool Database::approveUser(int userId)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE users SET approved = 1 WHERE id = ?");
    q.addBindValue(userId);
    return q.exec();
}

bool Database::setPersonalDataConsent(int userId, bool consent)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE users SET personal_data_consent = ? WHERE id = ?");
    q.addBindValue(consent ? 1 : 0);
    q.addBindValue(userId);
    return q.exec();
}

bool Database::deleteUser(int userId)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM bookings WHERE user_id = ?");
    q.addBindValue(userId);
    if (q.exec() && q.next()) {
        if (q.value(0).toInt() > 0) {
            qWarning() << "Невозможно удалить пользователя: у него есть бронирования.";
            return false;
        }
    }
    q.prepare("DELETE FROM users WHERE id = ?");
    q.addBindValue(userId);
    return q.exec();
}

QList<User> Database::getAllUsers() const
{
    QList<User> users;
    QSqlQuery q("SELECT id, full_name, phone, sport, login, approved, personal_data_consent FROM users", m_db);
    while (q.next()) {
        User u;
        u.id = q.value(0).toInt();
        u.fullName = q.value(1).toString();
        u.phone = q.value(2).toString();
        u.sport = q.value(3).toString();
        u.login = q.value(4).toString();
        u.approved = q.value(5).toBool();
        u.personalDataConsent = q.value(6).toBool();
        users.append(u);
    }
    return users;
}

bool Database::updateUserProfile(int userId, const QString &fullName,
                                 const QString &phone, const QString &sport)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE users SET full_name = ?, phone = ?, sport = ? WHERE id = ?");
    q.addBindValue(fullName);
    q.addBindValue(phone);
    q.addBindValue(sport);
    q.addBindValue(userId);
    return q.exec();
}

bool Database::changePassword(int userId, const QString &oldPassword,
                              const QString &newPassword)
{
    User u = getUserById(userId);
    if (u.id == -1) return false;

    QString oldHash = QCryptographicHash::hash(oldPassword.toUtf8(), QCryptographicHash::Sha256).toHex();
    if (u.passwordHash != oldHash) {
        return false;
    }

    QSqlQuery q(m_db);
    q.prepare("UPDATE users SET password_hash = ? WHERE id = ?");
    q.addBindValue(QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256).toHex());
    q.addBindValue(userId);
    return q.exec();
}

// ---------- Бронирования ----------
bool Database::addBooking(const Booking &booking)
{
    if (!isTimeSlotAvailable(booking.hallName, booking.date,
                             booking.startTime, booking.endTime))
        return false;

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO bookings (user_id, date, start_time, end_time, hall_name, "
              "sport_type, cost, is_recurring) "
              "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    q.addBindValue(booking.userId);
    q.addBindValue(booking.date.toString("yyyy-MM-dd"));
    q.addBindValue(booking.startTime.toString("HH:mm"));
    q.addBindValue(booking.endTime.toString("HH:mm"));
    q.addBindValue(booking.hallName);
    q.addBindValue(booking.sportType);
    q.addBindValue(booking.cost);
    q.addBindValue(booking.isRecurring ? 1 : 0);
    return q.exec();
}

bool Database::removeBooking(int bookingId)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM bookings WHERE id = ?");
    q.addBindValue(bookingId);
    return q.exec();
}

QList<Booking> Database::getBookingsForUser(int userId, const QDate &from, const QDate &to) const
{
    QList<Booking> list;
    QSqlQuery q(m_db);
    q.prepare("SELECT b.id, b.date, b.start_time, b.end_time, b.hall_name, "
              "u.full_name, b.sport_type, b.cost, b.is_recurring "
              "FROM bookings b "
              "JOIN users u ON b.user_id = u.id "
              "WHERE b.user_id = ? AND b.date BETWEEN ? AND ? "
              "ORDER BY b.date, b.start_time");
    q.addBindValue(userId);
    q.addBindValue(from.toString("yyyy-MM-dd"));
    q.addBindValue(to.toString("yyyy-MM-dd"));
    if (q.exec()) {
        while (q.next()) {
            Booking b;
            b.id = q.value(0).toInt();
            b.date = QDate::fromString(q.value(1).toString(), "yyyy-MM-dd");
            b.startTime = QTime::fromString(q.value(2).toString(), "HH:mm");
            b.endTime = QTime::fromString(q.value(3).toString(), "HH:mm");
            b.hallName = q.value(4).toString();
            b.userFullName = q.value(5).toString();
            b.sportType = q.value(6).toString();
            b.cost = q.value(7).toDouble();
            b.isRecurring = q.value(8).toBool();
            b.userId = userId;
            list.append(b);
        }
    }
    return list;
}

QList<Booking> Database::getAllBookings(const QDate &from, const QDate &to) const
{
    QList<Booking> list;
    QSqlQuery q(m_db);
    q.prepare("SELECT b.id, b.date, b.start_time, b.end_time, b.hall_name, "
              "u.full_name, b.sport_type, b.cost, b.is_recurring, b.user_id "
              "FROM bookings b "
              "JOIN users u ON b.user_id = u.id "
              "WHERE b.date BETWEEN ? AND ? "
              "ORDER BY b.date, b.start_time");
    q.addBindValue(from.toString("yyyy-MM-dd"));
    q.addBindValue(to.toString("yyyy-MM-dd"));
    if (q.exec()) {
        while (q.next()) {
            Booking b;
            b.id = q.value(0).toInt();
            b.date = QDate::fromString(q.value(1).toString(), "yyyy-MM-dd");
            b.startTime = QTime::fromString(q.value(2).toString(), "HH:mm");
            b.endTime = QTime::fromString(q.value(3).toString(), "HH:mm");
            b.hallName = q.value(4).toString();
            b.userFullName = q.value(5).toString();
            b.sportType = q.value(6).toString();
            b.cost = q.value(7).toDouble();
            b.isRecurring = q.value(8).toBool();
            b.userId = q.value(9).toInt();
            list.append(b);
        }
    }
    return list;
}

QList<Booking> Database::getAllBookings() const
{
    return getAllBookings(QDate(2000,1,1), QDate(2100,12,31));
}

bool Database::isTimeSlotAvailable(const QString &hall, const QDate &date,
                                   const QTime &start, const QTime &end,
                                   int excludeBookingId) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM bookings "
              "WHERE hall_name = ? AND date = ? "
              "AND start_time < ? AND end_time > ? "
              "AND id != ?");
    q.addBindValue(hall);
    q.addBindValue(date.toString("yyyy-MM-dd"));
    q.addBindValue(end.toString("HH:mm"));
    q.addBindValue(start.toString("HH:mm"));
    q.addBindValue(excludeBookingId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt() == 0;
    }
    return false;
}

void Database::generateRecurringBookingsForMonth(const QDate &monthStart)
{
    // ... реализация остаётся прежней ...
    QSqlQuery origQuery(m_db);
    origQuery.prepare("SELECT id, date FROM bookings WHERE is_recurring = 1 AND date < ?");
    origQuery.addBindValue(monthStart.toString("yyyy-MM-dd"));
    origQuery.exec();

    QMap<int, QDate> origDates;
    while (origQuery.next()) {
        origDates.insert(origQuery.value(0).toInt(),
                         QDate::fromString(origQuery.value(1).toString(), "yyyy-MM-dd"));
    }

    QSqlQuery templQ(m_db);
    templQ.prepare("SELECT id, user_id, start_time, end_time, hall_name, sport_type, cost "
                   "FROM bookings WHERE is_recurring = 1 AND date < ?");
    templQ.addBindValue(monthStart.toString("yyyy-MM-dd"));
    templQ.exec();

    QDate endOfMonth = monthStart.addMonths(1).addDays(-1);
    while (templQ.next()) {
        int bookingId = templQ.value(0).toInt();
        int userId = templQ.value(1).toInt();
        QTime start = QTime::fromString(templQ.value(2).toString(), "HH:mm");
        QTime end = QTime::fromString(templQ.value(3).toString(), "HH:mm");
        QString hall = templQ.value(4).toString();
        QString sport = templQ.value(5).toString();
        double cost = templQ.value(6).toDouble();
        QDate origDate = origDates.value(bookingId);
        if (!origDate.isValid()) continue;

        int dayOfWeek = origDate.dayOfWeek();
        QDate d = monthStart;
        while (d.dayOfWeek() != dayOfWeek) d = d.addDays(1);
        while (d <= endOfMonth) {
            if (isTimeSlotAvailable(hall, d, start, end)) {
                Booking b;
                b.userId = userId;
                b.date = d;
                b.startTime = start;
                b.endTime = end;
                b.hallName = hall;
                b.sportType = sport;
                b.cost = cost;
                b.isRecurring = true;
                addBooking(b);
            }
            d = d.addDays(7);
        }
    }
}

// ---------- Тарифы ----------
bool Database::addTariff(const QString &dayOfWeek, int hourFrom, int hourTo, double price)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO tariffs (day_of_week, hour_from, hour_to, price_per_hour) "
              "VALUES (?, ?, ?, ?)");
    q.addBindValue(dayOfWeek);
    q.addBindValue(hourFrom);
    q.addBindValue(hourTo);
    q.addBindValue(price);
    return q.exec();
}

bool Database::removeTariff(int tariffId)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM tariffs WHERE id = ?");
    q.addBindValue(tariffId);
    return q.exec();
}

QList<Tariff> Database::getTariffs() const
{
    QList<Tariff> list;
    QSqlQuery q("SELECT id, day_of_week, hour_from, hour_to, price_per_hour FROM tariffs", m_db);
    while (q.next()) {
        Tariff t;
        t.id = q.value(0).toInt();
        t.dayOfWeek = q.value(1).toString();
        t.hourFrom = q.value(2).toInt();
        t.hourTo = q.value(3).toInt();
        t.pricePerHour = q.value(4).toDouble();
        list.append(t);
    }
    return list;
}

double Database::getHourlyRate(const QString &dayOfWeek, int hour) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT price_per_hour FROM tariffs "
              "WHERE day_of_week = ? AND hour_from <= ? AND hour_to > ? "
              "ORDER BY price_per_hour DESC LIMIT 1");
    q.addBindValue(dayOfWeek);
    q.addBindValue(hour);
    q.addBindValue(hour);
    if (q.exec() && q.next()) {
        return q.value(0).toDouble();
    }
    return 100.0;
}

// ---------- Настройки ----------
double Database::getMonthlySubscriptionCost() const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT value FROM settings WHERE key = 'monthly_cost'");
    if (q.exec() && q.next())
        return q.value(0).toDouble();
    return 5000.0;
}

bool Database::setMonthlySubscriptionCost(double cost)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES ('monthly_cost', ?)");
    q.addBindValue(QString::number(cost));
    return q.exec();
}

// ---------- Залы ----------
QStringList Database::getHalls() const
{
    QStringList halls;
    QSqlQuery q("SELECT name FROM halls ORDER BY id", m_db);
    while (q.next())
        halls << q.value(0).toString();
    return halls;
}