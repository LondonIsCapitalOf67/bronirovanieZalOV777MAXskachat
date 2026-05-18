#include "mainwindow.h"
#include "database.h"
#include "bookingdialog.h"

#include <QToolBar>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QTime>

MainWindow::MainWindow(const User &user, QWidget *parent)
    : QMainWindow(parent)
    , m_user(user)
    , m_db(Database::instance())
{
    setupUi();
    refreshBookings();
}

void MainWindow::setupUi()
{
    setWindowTitle("Бронирование залов — " + m_user.fullName);
    resize(850, 550);

    // Панель инструментов
    QToolBar *toolbar = addToolBar("Действия");
    m_bookBtn = new QPushButton("Забронировать");
    toolbar->addWidget(m_bookBtn);
    toolbar->addSeparator();
    m_userInfoLabel = new QLabel("Пользователь: " + m_user.fullName + "   |   " + m_user.sport);
    toolbar->addWidget(m_userInfoLabel);

    // Центральный виджет с вкладками месяцев
    m_tabWidget = new QTabWidget;
    setCentralWidget(m_tabWidget);

    QDate today = QDate::currentDate();
    QDate currentMonthStart(today.year(), today.month(), 1);
    QDate nextMonthStart = currentMonthStart.addMonths(1);

    // Создаём таблицы
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

    m_tabWidget->addTab(m_currentMonthTable,
                        QString("Текущий месяц (%1)").arg(currentMonthStart.toString("MMMM yyyy")));
    m_tabWidget->addTab(m_nextMonthTable,
                        QString("Следующий месяц (%1)").arg(nextMonthStart.toString("MMMM yyyy")));

    connect(m_bookBtn, &QPushButton::clicked, this, &MainWindow::openBookingDialog);
}

void MainWindow::openBookingDialog()
{
    BookingDialog dlg(this);
    // Можно предзаполнить вид спорта пользователя
    if (dlg.exec() == QDialog::Accepted) {
        Booking booking = dlg.bookingData();
        booking.userId = m_user.id;
        booking.userFullName = m_user.fullName;

        // Проверка доступности слота
        if (!m_db->isTimeSlotAvailable(booking.hallName, booking.date,
                                       booking.startTime, booking.endTime)) {
            QMessageBox::warning(this, "Ошибка", "Выбранное время недоступно.");
            return;
        }

        if (m_db->addBooking(booking)) {
            QMessageBox::information(this, "Успех", "Бронирование добавлено.");
            refreshBookings();

            // Уведомление администратора
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

        // Если бронирование принадлежит текущему пользователю — показываем кнопку "Отменить"
        if (b.userId == m_user.id) {
            auto *cancelBtn = new QPushButton("Отменить");
            cancelBtn->setProperty("bookingId", b.id);
            connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::cancelSelectedBooking);
            table->setCellWidget(i, 5, cancelBtn);
        } else {
            // Для чужих бронирований не показываем имя пользователя, только "Занято"
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
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось отменить бронирование.");
        }
    }
}