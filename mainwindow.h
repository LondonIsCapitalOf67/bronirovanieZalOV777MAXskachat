#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "user.h"            // <-- полное определение User

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const User &user, QWidget *parent = nullptr);

signals:
    void bookingAddedForAdmin(const QString &message);

private slots:
    void openBookingDialog();
    void refreshBookings();
    void cancelSelectedBooking();

private:
    void setupUi();
    void populateMonthTable(QTableWidget *table, const QDate &from, const QDate &to);

    User m_user;             // теперь тип полностью определён
    QTabWidget *m_tabWidget;
    QTableWidget *m_currentMonthTable;
    QTableWidget *m_nextMonthTable;
    QPushButton *m_bookBtn;
    QLabel *m_userInfoLabel;

    class Database *m_db;
};

#endif // MAINWINDOW_H