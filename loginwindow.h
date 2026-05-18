#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QStackedWidget>
#include <QComboBox>

struct User; // forward declaration

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginSuccess(const User &user);  // успешный вход
    void registrationRequested();         // запрос на регистрацию (для оповещения админа)

private slots:
    void switchToLogin();
    void switchToRegister();
    void attemptLogin();
    void attemptRegister();

private:
    void setupUi();

    QStackedWidget *m_stack;

    // Страница входа
    QWidget *m_loginPage;
    QLineEdit *m_loginEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_toRegisterBtn;

    // Страница регистрации
    QWidget *m_registerPage;
    QLineEdit *m_regFullName;
    QLineEdit *m_regPhone;
    QLineEdit *m_regSport;
    QLineEdit *m_regLogin;
    QLineEdit *m_regPassword;
    QCheckBox *m_consentCheck;
    QPushButton *m_registerBtn;
    QPushButton *m_toLoginBtn;

    class Database *m_db;
};

#endif // LOGINWINDOW_H