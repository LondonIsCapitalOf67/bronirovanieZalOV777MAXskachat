#include "loginwindow.h"
#include "database.h"
#include "user.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QCryptographicHash>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
    , m_db(Database::instance())
{
    setupUi();
}

void LoginWindow::setupUi()
{
    setWindowTitle("Вход / Регистрация");
    setMinimumSize(400, 300);

    auto *mainLayout = new QVBoxLayout(this);
    m_stack = new QStackedWidget;
    mainLayout->addWidget(m_stack);

    // ----------------- Страница входа -----------------
    m_loginPage = new QWidget;
    auto *loginLayout = new QVBoxLayout(m_loginPage);
    auto *loginForm = new QFormLayout;
    m_loginEdit = new QLineEdit;
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    loginForm->addRow("Логин:", m_loginEdit);
    loginForm->addRow("Пароль:", m_passwordEdit);
    loginLayout->addLayout(loginForm);

    m_loginBtn = new QPushButton("Войти");
    m_toRegisterBtn = new QPushButton("Нет аккаунта? Зарегистрироваться");
    loginLayout->addWidget(m_loginBtn);
    loginLayout->addWidget(m_toRegisterBtn);
    loginLayout->addStretch();

    // ----------------- Страница регистрации -----------------
    m_registerPage = new QWidget;
    auto *regLayout = new QVBoxLayout(m_registerPage);
    auto *regForm = new QFormLayout;
    m_regFullName = new QLineEdit;
    m_regPhone = new QLineEdit;
    m_regSport = new QLineEdit;
    m_regSport->setPlaceholderText("например, Фитнес");
    m_regLogin = new QLineEdit;
    m_regPassword = new QLineEdit;
    m_regPassword->setEchoMode(QLineEdit::Password);
    m_consentCheck = new QCheckBox("Я согласен на обработку персональных данных");

    regForm->addRow("ФИО:", m_regFullName);
    regForm->addRow("Телефон:", m_regPhone);
    regForm->addRow("Вид спорта:", m_regSport);
    regForm->addRow("Логин:", m_regLogin);
    regForm->addRow("Пароль:", m_regPassword);
    regLayout->addLayout(regForm);
    regLayout->addWidget(m_consentCheck);

    m_registerBtn = new QPushButton("Зарегистрироваться");
    m_toLoginBtn = new QPushButton("Уже есть аккаунт? Войти");
    regLayout->addWidget(m_registerBtn);
    regLayout->addWidget(m_toLoginBtn);
    regLayout->addStretch();

    m_stack->addWidget(m_loginPage);
    m_stack->addWidget(m_registerPage);
    m_stack->setCurrentIndex(0); // начать с входа

    // Подключаем сигналы
    connect(m_toRegisterBtn, &QPushButton::clicked, this, &LoginWindow::switchToRegister);
    connect(m_toLoginBtn, &QPushButton::clicked, this, &LoginWindow::switchToLogin);
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWindow::attemptLogin);
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginWindow::attemptRegister);
}

void LoginWindow::switchToLogin()
{
    m_stack->setCurrentIndex(0);
}

void LoginWindow::switchToRegister()
{
    m_stack->setCurrentIndex(1);
}

void LoginWindow::attemptLogin()
{
    QString login = m_loginEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите логин и пароль.");
        return;
    }

    User user = m_db->getUserByLogin(login);
    if (user.id == -1) {
        QMessageBox::warning(this, "Ошибка", "Пользователь с таким логином не найден.");
        return;
    }

    // Проверка пароля (сравниваем хеши)
    QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
    if (user.passwordHash != passwordHash) {
        QMessageBox::warning(this, "Ошибка", "Неверный пароль.");
        return;
    }

    // Проверка подтверждения администратором
    if (!user.approved) {
        QMessageBox::information(this, "Ожидание подтверждения",
                                 "Ваш аккаунт ещё не подтверждён администратором. Пожалуйста, подождите.");
        return;
    }

    // Проверка согласия на обработку ПД (если не было подписано при регистрации админом)
    if (!user.personalDataConsent) {
        QMessageBox::StandardButton btn = QMessageBox::question(this, "Согласие",
                                                                "Вы не дали согласие на обработку персональных данных. Дать согласие сейчас?",
                                                                QMessageBox::Yes | QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            m_db->setPersonalDataConsent(user.id, true);
            user.personalDataConsent = true;
        } else {
            return; // без согласия не пускаем
        }
    }

    emit loginSuccess(user);
    // Окно входа можно закрыть или спрятать, зависит от логики приложения
}

void LoginWindow::attemptRegister()
{
    QString fullName = m_regFullName->text().trimmed();
    QString phone = m_regPhone->text().trimmed();
    QString sport = m_regSport->text().trimmed();
    QString login = m_regLogin->text().trimmed();
    QString password = m_regPassword->text();

    if (fullName.isEmpty() || login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните обязательные поля: ФИО, логин, пароль.");
        return;
    }

    if (!m_consentCheck->isChecked()) {
        QMessageBox::warning(this, "Ошибка", "Необходимо дать согласие на обработку персональных данных.");
        return;
    }

    // Проверка уникальности логина
    if (m_db->getUserByLogin(login).id != -1) {
        QMessageBox::warning(this, "Ошибка", "Пользователь с таким логином уже существует.");
        return;
    }

    bool ok = m_db->addUser(fullName, phone, sport, login, password);
    if (!ok) {
        QMessageBox::critical(this, "Ошибка", "Не удалось зарегистрировать пользователя.");
        return;
    }

    // Автоматически даём согласие (чекбокс был нажат)
    User newUser = m_db->getUserByLogin(login);
    if (newUser.id != -1) {
        m_db->setPersonalDataConsent(newUser.id, true);
    }

    QMessageBox::information(this, "Регистрация",
                             "Регистрация прошла успешно. Ожидайте подтверждения администратора. "
                             "После подтверждения вы сможете войти.");
    switchToLogin();

    // Оповещаем администратора (можно через сигнал в главное окно)
    emit registrationRequested();
}