#include "adminwindow.h"
#include "database.h"
#include "booking.h"
#include "user.h"
#include "tariff.h"
#include "bookingdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QToolBar>

AdminWindow::AdminWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_db(Database::instance())
{
    setupUi();
    refreshBookingsTable();
    loadTariffs();
    loadUsers();
}

AdminWindow::~AdminWindow()
{
}

void AdminWindow::setupUi()
{
    setWindowTitle("Панель администратора");
    resize(900, 600);

    // Тулбар с кнопкой "Назад"
    QToolBar *toolbar = addToolBar("Навигация");
    m_backBtn = new QPushButton("← Назад");
    toolbar->addWidget(m_backBtn);
    connect(m_backBtn, &QPushButton::clicked, this, &AdminWindow::onLogout);

    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    m_tabWidget->addTab(createBookingsTab(), "Бронирования");
    m_tabWidget->addTab(createUsersTab(), "Пользователи");
    m_tabWidget->addTab(createTariffsTab(), "Тарифы");

    // Календарь
    m_calendarWidget = new CalendarWidget(true);   // true – показываем имена
    m_tabWidget->addTab(m_calendarWidget, "Календарь");
}

// ---------- Вкладка «Бронирования» ----------
QWidget* AdminWindow::createBookingsTab()
{
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout(w);

    m_bookingsTable = new QTableWidget;
    m_bookingsTable->setColumnCount(6);
    m_bookingsTable->setHorizontalHeaderLabels(
        {"Дата", "Время", "Зал", "Пользователь", "Вид спорта", "Стоимость (₽)"});
    m_bookingsTable->horizontalHeader()->setStretchLastSection(true);
    m_bookingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bookingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHBoxLayout *btnLay = new QHBoxLayout;
    m_refreshBtn = new QPushButton("Обновить");
    m_deleteBookingBtn = new QPushButton("Удалить выбранное");
    btnLay->addWidget(m_refreshBtn);
    btnLay->addWidget(m_deleteBookingBtn);
    btnLay->addStretch();

    lay->addWidget(m_bookingsTable);
    lay->addLayout(btnLay);

    connect(m_refreshBtn, &QPushButton::clicked, this, &AdminWindow::refreshBookingsTable);
    connect(m_deleteBookingBtn, &QPushButton::clicked, this, &AdminWindow::deleteSelectedBooking);

    return w;
}

// ---------- Вкладка «Пользователи» ----------
QWidget* AdminWindow::createUsersTab()
{
    QWidget *w = new QWidget;
    QVBoxLayout *mainLay = new QVBoxLayout(w);

    m_usersTable = new QTableWidget;
    m_usersTable->setColumnCount(7);
    m_usersTable->setHorizontalHeaderLabels(
        {"ID", "ФИО", "Телефон", "Спорт", "Логин", "Подтверждён", "Согласие ПД"});
    m_usersTable->horizontalHeader()->setStretchLastSection(true);
    m_usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_usersTable->setColumnHidden(0, true);
    mainLay->addWidget(m_usersTable);

    QHBoxLayout *btnLay = new QHBoxLayout;
    m_loadUsersBtn = new QPushButton("Обновить список");
    m_approveUserBtn = new QPushButton("Подтвердить выбранного");
    m_deleteUserBtn = new QPushButton("Удалить выбранного");
    m_bookForUserBtn = new QPushButton("Забронировать для выбранного");

    btnLay->addWidget(m_loadUsersBtn);
    btnLay->addWidget(m_approveUserBtn);
    btnLay->addWidget(m_deleteUserBtn);
    btnLay->addWidget(m_bookForUserBtn);
    btnLay->addStretch();
    mainLay->addLayout(btnLay);

    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLay->addWidget(line);

    QLabel *regLabel = new QLabel("Регистрация нового пользователя:");
    mainLay->addWidget(regLabel);

    QFormLayout *form = new QFormLayout;
    m_newUserName = new QLineEdit;
    m_newUserPhone = new QLineEdit;
    m_newUserSport = new QLineEdit;
    m_newUserLogin = new QLineEdit;
    m_newUserPassword = new QLineEdit;
    m_newUserPassword->setEchoMode(QLineEdit::Password);

    form->addRow("ФИО:", m_newUserName);
    form->addRow("Телефон:", m_newUserPhone);
    form->addRow("Вид спорта:", m_newUserSport);
    form->addRow("Логин:", m_newUserLogin);
    form->addRow("Пароль:", m_newUserPassword);

    m_registerUserBtn = new QPushButton("Зарегистрировать пользователя");

    mainLay->addLayout(form);
    mainLay->addWidget(m_registerUserBtn);
    mainLay->addStretch();

    connect(m_loadUsersBtn, &QPushButton::clicked, this, &AdminWindow::loadUsers);
    connect(m_approveUserBtn, &QPushButton::clicked, this, &AdminWindow::approveSelectedUser);
    connect(m_deleteUserBtn, &QPushButton::clicked, this, &AdminWindow::deleteSelectedUser);
    connect(m_bookForUserBtn, &QPushButton::clicked, this, &AdminWindow::openBookingForUser);
    connect(m_registerUserBtn, &QPushButton::clicked, this, &AdminWindow::registerUserByAdmin);

    return w;
}

// ---------- Вкладка «Тарифы» ----------
QWidget* AdminWindow::createTariffsTab()
{
    QWidget *w = new QWidget;
    QVBoxLayout *mainLay = new QVBoxLayout(w);

    m_tariffsTable = new QTableWidget;
    m_tariffsTable->setColumnCount(4);
    m_tariffsTable->setHorizontalHeaderLabels(
        {"День недели", "С...", "По...", "Цена за час (₽)"});
    m_tariffsTable->horizontalHeader()->setStretchLastSection(true);
    m_tariffsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QHBoxLayout *addLay = new QHBoxLayout;
    m_dayCombo = new QComboBox;
    m_dayCombo->addItems({"Пн","Вт","Ср","Чт","Пт","Сб","Вс"});
    m_hourFromSpin = new QSpinBox;   m_hourFromSpin->setRange(0, 23);
    m_hourToSpin = new QSpinBox;     m_hourToSpin->setRange(1, 24);
    m_priceSpin = new QDoubleSpinBox; m_priceSpin->setRange(0, 99999);
    m_addTariffBtn = new QPushButton("Добавить тариф");
    m_deleteTariffBtn = new QPushButton("Удалить выбранный тариф");
    addLay->addWidget(new QLabel("День:"));
    addLay->addWidget(m_dayCombo);
    addLay->addWidget(new QLabel("с:"));
    addLay->addWidget(m_hourFromSpin);
    addLay->addWidget(new QLabel("по:"));
    addLay->addWidget(m_hourToSpin);
    addLay->addWidget(new QLabel("Цена:"));
    addLay->addWidget(m_priceSpin);
    addLay->addWidget(m_addTariffBtn);
    addLay->addWidget(m_deleteTariffBtn);

    QHBoxLayout *monthLay = new QHBoxLayout;
    monthLay->addWidget(new QLabel("Абонемент на месяц (фикс. цена со скидкой):"));
    m_monthlyCostSpin = new QDoubleSpinBox; m_monthlyCostSpin->setRange(0, 999999);
    m_updateMonthlyCostBtn = new QPushButton("Установить");
    monthLay->addWidget(m_monthlyCostSpin);
    monthLay->addWidget(m_updateMonthlyCostBtn);
    monthLay->addStretch();

    mainLay->addWidget(m_tariffsTable);
    mainLay->addLayout(addLay);
    mainLay->addLayout(monthLay);

    connect(m_addTariffBtn, &QPushButton::clicked, this, &AdminWindow::addTariff);
    connect(m_deleteTariffBtn, &QPushButton::clicked, this, &AdminWindow::deleteSelectedTariff);
    connect(m_updateMonthlyCostBtn, &QPushButton::clicked, this, &AdminWindow::updateMonthlyCost);

    return w;
}

// ===================== СЛОТЫ =====================

// --- Бронирования ---
void AdminWindow::refreshBookingsTable()
{
    QList<Booking> bookings = m_db->getAllBookings();
    m_bookingsTable->setRowCount(bookings.size());

    for (int i = 0; i < bookings.size(); ++i) {
        const Booking &b = bookings[i];
        m_bookingsTable->setItem(i, 0, new QTableWidgetItem(b.date.toString("dd.MM.yyyy")));
        m_bookingsTable->setItem(i, 1, new QTableWidgetItem(
                                           b.startTime.toString("HH:mm") + " - " + b.endTime.toString("HH:mm")));
        m_bookingsTable->setItem(i, 2, new QTableWidgetItem(b.hallName));
        m_bookingsTable->setItem(i, 3, new QTableWidgetItem(b.userFullName));
        m_bookingsTable->setItem(i, 4, new QTableWidgetItem(b.sportType));
        m_bookingsTable->setItem(i, 5, new QTableWidgetItem(QString::number(b.cost, 'f', 2)));
    }
    m_bookingsTable->resizeColumnsToContents();
}

void AdminWindow::deleteSelectedBooking()
{
    int row = m_bookingsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Удаление", "Выберите бронирование для удаления.");
        return;
    }
    QMessageBox::warning(this, "Не реализовано", "Удаление по ID ещё не реализовано (нужно хранить id в ячейке).");
}

// --- Пользователи ---
void AdminWindow::loadUsers()
{
    QList<User> users = m_db->getAllUsers();
    m_usersTable->setRowCount(users.size());

    for (int i = 0; i < users.size(); ++i) {
        const User &u = users[i];
        m_usersTable->setItem(i, 0, new QTableWidgetItem(QString::number(u.id)));
        m_usersTable->setItem(i, 1, new QTableWidgetItem(u.fullName));
        m_usersTable->setItem(i, 2, new QTableWidgetItem(u.phone));
        m_usersTable->setItem(i, 3, new QTableWidgetItem(u.sport));
        m_usersTable->setItem(i, 4, new QTableWidgetItem(u.login));
        m_usersTable->setItem(i, 5, new QTableWidgetItem(u.approved ? "Да" : "Нет"));
        m_usersTable->setItem(i, 6, new QTableWidgetItem(u.personalDataConsent ? "Да" : "Нет"));
    }
    m_usersTable->resizeColumnsToContents();
}

void AdminWindow::approveSelectedUser()
{
    int row = m_usersTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Подтверждение", "Выберите пользователя для подтверждения.");
        return;
    }
    int userId = m_usersTable->item(row, 0)->text().toInt();
    if (m_db->approveUser(userId)) {
        QMessageBox::information(this, "Успех", "Пользователь подтверждён.");
        loadUsers();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось подтвердить пользователя.");
    }
}

void AdminWindow::deleteSelectedUser()
{
    int row = m_usersTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Удаление", "Выберите пользователя для удаления.");
        return;
    }
    int userId = m_usersTable->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Подтверждение",
                              "Удалить выбранного пользователя навсегда?") == QMessageBox::Yes) {
        if (m_db->deleteUser(userId)) {
            QMessageBox::information(this, "Удаление", "Пользователь удалён.");
            loadUsers();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить пользователя. Возможно, у него есть бронирования.");
        }
    }
}

void AdminWindow::registerUserByAdmin()
{
    QString name = m_newUserName->text().trimmed();
    QString phone = m_newUserPhone->text().trimmed();
    QString sport = m_newUserSport->text().trimmed();
    QString login = m_newUserLogin->text().trimmed();
    QString password = m_newUserPassword->text();

    if (name.isEmpty() || phone.isEmpty() || login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все обязательные поля.");
        return;
    }

    if (m_db->addUser(name, phone, sport, login, password)) {
        QMessageBox::information(this, "Успех", "Пользователь зарегистрирован.");
        m_newUserName->clear();
        m_newUserPhone->clear();
        m_newUserSport->clear();
        m_newUserLogin->clear();
        m_newUserPassword->clear();
        loadUsers();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось зарегистрировать пользователя.");
    }
}

void AdminWindow::openBookingForUser()
{
    int row = m_usersTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Бронирование", "Выберите пользователя из списка.");
        return;
    }

    int userId = m_usersTable->item(row, 0)->text().toInt();
    QString fullName = m_usersTable->item(row, 1)->text();

    BookingDialog dlg(userId, fullName, this);
    if (dlg.exec() == QDialog::Accepted) {
        Booking booking = dlg.bookingData();
        booking.userId = userId;
        booking.userFullName = fullName;

        if (!m_db->isTimeSlotAvailable(booking.hallName, booking.date,
                                       booking.startTime, booking.endTime)) {
            QMessageBox::warning(this, "Ошибка", "Это время уже занято.");
            return;
        }

        if (m_db->addBooking(booking)) {
            QMessageBox::information(this, "Успех", "Бронирование создано.");
            refreshBookingsTable();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать бронирование.");
        }
    }
}

// --- Тарифы ---
void AdminWindow::loadTariffs()
{
    QList<Tariff> tariffs = m_db->getTariffs();
    m_tariffsTable->setRowCount(tariffs.size());
    for (int i = 0; i < tariffs.size(); ++i) {
        const Tariff &t = tariffs[i];
        m_tariffsTable->setItem(i, 0, new QTableWidgetItem(t.dayOfWeek));
        m_tariffsTable->setItem(i, 1, new QTableWidgetItem(QString::number(t.hourFrom)));
        m_tariffsTable->setItem(i, 2, new QTableWidgetItem(QString::number(t.hourTo)));
        m_tariffsTable->setItem(i, 3, new QTableWidgetItem(QString::number(t.pricePerHour, 'f', 2)));
    }
    m_tariffsTable->resizeColumnsToContents();

    double monthly = m_db->getMonthlySubscriptionCost();
    m_monthlyCostSpin->setValue(monthly);
}

void AdminWindow::addTariff()
{
    QString day = m_dayCombo->currentText();
    int from = m_hourFromSpin->value();
    int to = m_hourToSpin->value();
    double price = m_priceSpin->value();

    if (from >= to) {
        QMessageBox::warning(this, "Ошибка", "Время начала должно быть меньше времени окончания.");
        return;
    }

    if (m_db->addTariff(day, from, to, price)) {
        loadTariffs();
        QMessageBox::information(this, "Тариф", "Тариф добавлен.");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить тариф.");
    }
}

void AdminWindow::deleteSelectedTariff()
{
    int row = m_tariffsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Удаление", "Выберите тариф для удаления.");
        return;
    }
    if (QMessageBox::question(this, "Подтверждение", "Удалить выбранный тариф?") == QMessageBox::Yes) {
        QMessageBox::warning(this, "Не реализовано", "Удаление тарифа по ID ещё не реализовано.");
    }
}

void AdminWindow::updateMonthlyCost()
{
    double cost = m_monthlyCostSpin->value();
    if (m_db->setMonthlySubscriptionCost(cost)) {
        QMessageBox::information(this, "Абонемент", "Стоимость абонемента обновлена.");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить стоимость.");
    }
}

void AdminWindow::onNewBookingNotification(const QString &message)
{
    QMessageBox::information(this, "Новое бронирование", message);
    refreshBookingsTable();
}

void AdminWindow::onLogout()
{
    close();
    emit logoutRequested();
}