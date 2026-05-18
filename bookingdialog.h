#ifndef BOOKINGDIALOG_H
#define BOOKINGDIALOG_H

#include <QDialog>
#include <QDateEdit>
#include <QTimeEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

struct Booking;

class BookingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookingDialog(QWidget *parent = nullptr);
    // Конструктор для бронирования от имени конкретного пользователя
    BookingDialog(int userId, const QString &userFullName, QWidget *parent = nullptr);

    Booking bookingData() const;

private slots:
    void calculateCost();
    void onAccept();

private:
    void setupUi();
    void setupConnections();

    QComboBox   *m_hallCombo;
    QDateEdit   *m_dateEdit;
    QTimeEdit   *m_timeFromEdit;
    QTimeEdit   *m_timeToEdit;
    QComboBox   *m_sportCombo;
    QCheckBox   *m_recurringCheck;
    QLabel      *m_costLabel;
    QLabel      *m_userInfoLabel;   // показывает, для кого бронь
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;

    class Database *m_db;
    int m_userId = -1;
    QString m_userFullName;
};

#endif // BOOKINGDIALOG_H