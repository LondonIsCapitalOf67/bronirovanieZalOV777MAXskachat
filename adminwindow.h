#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QMessageBox>

class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();

signals:
    void bookingAdded(const QString &user, const QString &date, const QString &time);

private slots:
    // Бронирования
    void refreshBookingsTable();
    void deleteSelectedBooking();

    // Пользователи
    void loadUsers();
    void approveSelectedUser();
    void deleteSelectedUser();
    void registerUserByAdmin();
    void openBookingForUser();         // бронирование для выбранного пользователя

    // Тарифы
    void loadTariffs();
    void addTariff();
    void deleteSelectedTariff();
    void updateMonthlyCost();

    void onNewBookingNotification(const QString &message);

private:
    void setupUi();
    QWidget* createBookingsTab();
    QWidget* createUsersTab();
    QWidget* createTariffsTab();

    QTabWidget *m_tabWidget;

    // Вкладка "Бронирования"
    QTableWidget *m_bookingsTable;
    QPushButton *m_refreshBtn;
    QPushButton *m_deleteBookingBtn;

    // Вкладка "Пользователи"
    QTableWidget *m_usersTable;
    QPushButton *m_loadUsersBtn;
    QPushButton *m_approveUserBtn;
    QPushButton *m_deleteUserBtn;
    QPushButton *m_bookForUserBtn;     // новая кнопка

    QLineEdit *m_newUserName;
    QLineEdit *m_newUserPhone;
    QLineEdit *m_newUserSport;
    QLineEdit *m_newUserLogin;
    QLineEdit *m_newUserPassword;
    QPushButton *m_registerUserBtn;

    // Вкладка "Тарифы"
    QTableWidget *m_tariffsTable;
    QComboBox *m_dayCombo;
    QSpinBox *m_hourFromSpin;
    QSpinBox *m_hourToSpin;
    QDoubleSpinBox *m_priceSpin;
    QPushButton *m_addTariffBtn;
    QPushButton *m_deleteTariffBtn;
    QDoubleSpinBox *m_monthlyCostSpin;
    QPushButton *m_updateMonthlyCostBtn;

    class Database *m_db;
};

#endif // ADMINWINDOW_H