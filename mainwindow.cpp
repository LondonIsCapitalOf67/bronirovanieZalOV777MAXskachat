#include "mainwindow.h"
#include "database.h"
#include "bookingdialog.h"
#include "loginwindow.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QTime>

MainWindow::MainWindow(const User &user, QWidget *parent)
    : QMainWindow(parent)
    , m_user(user)
    , m_db(Database::instance())
    , m_loginWindow(nullptr)
{
    setupUi();
    refreshBookings();
    loadProfile();
}

void MainWindow::setLoginWindow(LoginWindow *loginWindow)
{
    m_loginWindow = loginWindow;
}

void MainWindow::setupUi()
{
    setWindowTitle("Бронирование залов — " + m_user.fullName);
    resize(900, 600);

    // Тулбар
    QToolBar *toolbar = addToolBar("Действия");
    m_backBtn = new QPushButton("← Назад");
    m_bookBtn = new QPushButton("Забронировать");
    toolbar->addWidget(m_backBtn);
    toolbar->addWidget(m_bookBtn);
    toolbar->addSeparator();
    m_userInfoLabel = new QLabel("Пользователь: " + m_user.fullName + "   |   " + m_user.sport);
    toolbar->addWidget(m_userInfoLabel);

    // Центральные вкладки
    m_tabWidget = new QTabWidget;
    setCentralWidget(m_tabWidget);

    m_tabWidget->addTab(createBookingTab(), "Бронирования");
    m_tabWidget->addTab(createProfileTab(), "Мои данные");
    m_tabWidget->addTab(createTariffsTabForUser(), "Тарифы");

    // Календарь
    m_calendarWidget = new CalendarWidget(false);   // false – не показываем имена бронирующих
    m_tabWidget->addTab(m_calendarWidget, "Календарь");

    connect(m_backBtn, &QPushButton::clicked, this, &MainWindow::onLogout);
    connect(m_bookBtn, &QPushButton::clicked, this, &MainWindow::openBookingDialog);
}

// ------------------ Вкладка "Бронирования" ------------------
QWidget* MainWindow::createBookingTab()
{
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout(w);

    QTabWidget *monthTabs = new QTabWidget;
    m_currentMonthTable = new QTableWidget;
    m_nextMonthTable = new QTableWidget;

    QStringList headers = {"Дата", "Время", "Зал", "Вид спорта", "Стоимость (₽)", "Действие"};
    for (auto *table : {m_currentMonthTable, m_nextMonthTable}) {
        table->setColumnCount(6);
        table->setHorizontalHeaderLabels(headers);
        table->horizontalHeader()->setStretchLastSection(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->verticalHeader()->setVisible(false);
    }

    QDate today = QDate::currentDate();
    QDate currentMonthStart(today.year(), today.month(), 1);
    QDate nextMonthStart = currentMonthStart.addMonths(1);

    monthTabs->addTab(m_currentMonthTable,
                      QString("Текущий месяц (%1)").arg(currentMonthStart.toString("MMMM yyyy")));
    monthTabs->addTab(m_nextMonthTable,
                      QString("Следующий месяц (%1)").arg(nextMonthStart.toString("MMMM yyyy")));

    lay->addWidget(monthTabs);
    return w;
}

// ------------------ Вкладка "Мои данные" ------------------
QWidget* MainWindow::createProfileTab()
{
    QWidget *w = new QWidget;
    QVBoxLayout *mainLay = new QVBoxLayout(w);

    QFormLayout *form = new QFormLayout;
    m_profileName = new QLineEdit;
    m_profilePhone = new QLineEdit;
    m_profileSport = new QLineEdit;
    m_profileLogin = new QLabel(m_user.login);
    form->addRow("ФИО:", m_profileName);
    form->addRow("Телефон:", m_profilePhone);
    form->addRow("Вид спорта:", m_profileSport);
    form->addRow("Логин:", m_profileLogin);
    mainLay->addLayout(form);

    m_saveProfileBtn = new QPushButton("Сохранить изменения");
    mainLay->addWidget(m_saveProfileBtn);

    // Смена пароля
    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    mainLay->addWidget(line);

    QFormLayout *passForm = new QFormLayout;
    m_oldPassword = new QLineEdit;
    m_oldPassword->setEchoMode(QLineEdit::Password);
    m_oldPassword->setPlaceholderText("Старый пароль");
    m_newPassword = new QLineEdit;
    m_newPassword->setEchoMode(QLineEdit::Password);
    m_newPassword->setPlaceholderText("Новый пароль");
    passForm->addRow("Старый пароль:", m_oldPassword);
    passForm->addRow("Новый пароль:", m_newPassword);
    mainLay->addLayout(passForm);

    m_changePasswordBtn = new QPushButton("Сменить пароль");
    mainLay->addWidget(m_changePasswordBtn);
    mainLay->addStretch();

    connect(m_saveProfileBtn, &QPushButton::clicked, this, &MainWindow::saveProfile);
    connect(m_changePasswordBtn, &QPushButton::clicked, this, &MainWindow::changePassword);

    return w;
}

// ------------------ Вкладка "Тарифы" (только просмотр) ------------------
QWidget* MainWindow::createTariffsTabForUser()
{
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout(w);

    m_tariffsTable = new QTableWidget;
    m_tariffsTable->setColumnCount(4);
    m_tariffsTable->setHorizontalHeaderLabels(
        {"День недели", "С...", "По...", "Цена за час (₽)"});
    m_tariffsTable->horizontalHeader()->setStretchLastSection(true);
    m_tariffsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tariffsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    lay->addWidget(m_tariffsTable);

    m_monthlyCostLabel = new QLabel;
    lay->addWidget(m_monthlyCostLabel);

    // Загружаем тарифы
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
    m_monthlyCostLabel->setText(QString("Стоимость абонемента на месяц: %1 ₽").arg(monthly, 0, 'f', 2));

    return w;
}

// ------------------ Слоты "Мои данные" ------------------
void MainWindow::loadProfile()
{
    User u = m_db->getUserById(m_user.id);
    if (u.id != -1) {
        m_profileName->setText(u.fullName);
        m_profilePhone->setText(u.phone);
        m_profileSport->setText(u.sport);
    }
}

void MainWindow::saveProfile()
{
    QString name = m_profileName->text().trimmed();
    QString phone = m_profilePhone->text().trimmed();
    QString sport = m_profileSport->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "ФИО не может быть пустым.");
        return;
    }

    if (m_db->updateUserProfile(m_user.id, name, phone, sport)) {
        m_user.fullName = name;
        m_user.phone = phone;
        m_user.sport = sport;
        m_userInfoLabel->setText("Пользователь: " + m_user.fullName + "   |   " + m_user.sport);
        QMessageBox::information(this, "Успех", "Профиль обновлён.");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить профиль.");
    }
}

void MainWindow::changePassword()
{
    QString oldPass = m_oldPassword->text();
    QString newPass = m_newPassword->text();

    if (oldPass.isEmpty() || newPass.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните оба поля.");
        return;
    }

    if (m_db->changePassword(m_user.id, oldPass, newPass)) {
        QMessageBox::information(this, "Успех", "Пароль изменён.");
        m_oldPassword->clear();
        m_newPassword->clear();
    } else {
        QMessageBox::critical(this, "Ошибка", "Неверный старый пароль или ошибка базы данных.");
    }
}

// ------------------ Старые слоты (без изменений) ------------------
void MainWindow::openBookingDialog()
{
    BookingDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Booking booking = dlg.bookingData();
        booking.userId = m_user.id;
        booking.userFullName = m_user.fullName;

        if (!m_db->isTimeSlotAvailable(booking.hallName, booking.date,
                                       booking.startTime, booking.endTime)) {
            QMessageBox::warning(this, "Ошибка", "Выбранное время недоступно.");
            return;
        }

        if (m_db->addBooking(booking)) {
            QMessageBox::information(this, "Успех", "Бронирование добавлено.");
            refreshBookings();
            m_calendarWidget->refresh();

            QString msg = QString("Новое бронирование:\n%1\nЗал %2\n%3, %4 – %5\nСтоимость: %6 ₽")
                              .arg(m_user.fullName,
                                   booking.hallName,
                                   booking.date.toString("dd.MM.yyyy"),
                                   booking.startTime.toString("HH:mm"),
                                   booking.endTime.toString("HH:mm"),
                                   QString::number(booking.cost, 'f', 2));
            emit bookingAddedForAdmin(msg);
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить бронирование.");
        }
    }
}

void MainWindow::refreshBookings()
{
    QDate today = QDate::currentDate();
    QDate currentMonthStart(today.year(), today.month(), 1);
    QDate currentMonthEnd = currentMonthStart.addMonths(1).addDays(-1);
    QDate nextMonthStart = currentMonthStart.addMonths(1);
    QDate nextMonthEnd = nextMonthStart.addMonths(1).addDays(-1);

    populateMonthTable(m_currentMonthTable, currentMonthStart, currentMonthEnd);
    populateMonthTable(m_nextMonthTable, nextMonthStart, nextMonthEnd);
}

void MainWindow::populateMonthTable(QTableWidget *table, const QDate &from, const QDate &to)
{
    QList<Booking> bookings = m_db->getAllBookings(from, to);
    table->setRowCount(bookings.size());

    for (int i = 0; i < bookings.size(); ++i) {
        const Booking &b = bookings[i];

        table->setItem(i, 0, new QTableWidgetItem(b.date.toString("dd.MM.yyyy")));
        table->setItem(i, 1, new QTableWidgetItem(
                                 b.startTime.toString("HH:mm") + " – " + b.endTime.toString("HH:mm")));
        table->setItem(i, 2, new QTableWidgetItem(b.hallName));
        table->setItem(i, 3, new QTableWidgetItem(b.sportType));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(b.cost, 'f', 2)));

        if (b.userId == m_user.id) {
            auto *cancelBtn = new QPushButton("Отменить");
            cancelBtn->setProperty("bookingId", b.id);
            connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::cancelSelectedBooking);
            table->setCellWidget(i, 5, cancelBtn);
        } else {
            table->setItem(i, 5, new QTableWidgetItem("Занято"));
        }
    }

    table->resizeColumnsToContents();
}

void MainWindow::cancelSelectedBooking()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn) return;

    int bookingId = btn->property("bookingId").toInt();
    if (QMessageBox::question(this, "Отмена бронирования",
                              "Вы уверены, что хотите отменить это бронирование?")
        == QMessageBox::Yes) {
        if (m_db->removeBooking(bookingId)) {
            QMessageBox::information(this, "Успех", "Бронирование отменено.");
            refreshBookings();
            m_calendarWidget->refresh();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось отменить бронирование.");
        }
    }
}

void MainWindow::onLogout()
{
    if (m_loginWindow) {
        m_loginWindow->show();
    }
    close();
}