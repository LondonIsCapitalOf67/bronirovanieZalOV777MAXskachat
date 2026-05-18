#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "user.h"

class LoginWindow;  // forward declaration

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const User &user, QWidget *parent = nullptr);
    void setLoginWindow(LoginWindow *loginWindow);  // установить указатель на окно входа

signals:
    void bookingAddedForAdmin(const QString &message);

private slots:
    void openBookingDialog();
    void refreshBookings();
    void cancelSelectedBooking();
    void onLogout();

private:
    void setupUi();
    void populateMonthTable(QTableWidget *table, const QDate &from, const QDate &to);

    User m_user;
    QTabWidget *m_tabWidget;
    QTableWidget *m_currentMonthTable;
    QTableWidget *m_nextMonthTable;
    QPushButton *m_bookBtn;
    QPushButton *m_backBtn;
    QLabel *m_userInfoLabel;

    LoginWindow *m_loginWindow = nullptr;  // храним указатель
    class Database *m_db;
};

#endif // MAINWINDOW_H