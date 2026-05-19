#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include "user.h"
#include "calendarwidget.h"

class LoginWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const User &user, QWidget *parent = nullptr);
    void setLoginWindow(LoginWindow *loginWindow);

signals:
    void bookingAddedForAdmin(const QString &message);

private slots:
    void openBookingDialog();
    void refreshBookings();
    void cancelSelectedBooking();
    void onLogout();

    // Слоты для вкладки "Мои данные"
    void loadProfile();
    void saveProfile();
    void changePassword();

private:
    void setupUi();
    QWidget* createBookingTab();
    QWidget* createProfileTab();
    QWidget* createTariffsTabForUser();

    void populateMonthTable(QTableWidget *table, const QDate &from, const QDate &to);

    User m_user;
    QTabWidget *m_tabWidget;

    // Вкладка бронирований
    QTableWidget *m_currentMonthTable;
    QTableWidget *m_nextMonthTable;
    QPushButton *m_bookBtn;
    QPushButton *m_backBtn;
    QLabel *m_userInfoLabel;

    // Вкладка "Мои данные"
    QLineEdit *m_profileName;
    QLineEdit *m_profilePhone;
    QLineEdit *m_profileSport;
    QLabel *m_profileLogin;
    QPushButton *m_saveProfileBtn;
    QLineEdit *m_oldPassword;
    QLineEdit *m_newPassword;
    QPushButton *m_changePasswordBtn;

    // Вкладка "Тарифы" (просмотр)
    QTableWidget *m_tariffsTable;
    QLabel *m_monthlyCostLabel;

    // Вкладка "Календарь"
    CalendarWidget *m_calendarWidget;

    LoginWindow *m_loginWindow = nullptr;
    class Database *m_db;
};

#endif // MAINWINDOW_H