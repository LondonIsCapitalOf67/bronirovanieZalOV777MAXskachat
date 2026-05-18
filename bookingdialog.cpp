#include "bookingdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTime>

BookingDialog::BookingDialog(QWidget *parent)
    : QDialog(parent)
    , m_db(Database::instance())
{
    setupUi();
    setupConnections();
    m_dateEdit->setDate(QDate::currentDate());
    m_timeFromEdit->setTime(QTime(9, 0));
    m_timeToEdit->setTime(QTime(10, 0));
    calculateCost();
}

BookingDialog::BookingDialog(int userId, const QString &userFullName, QWidget *parent)
    : QDialog(parent)
    , m_db(Database::instance())
    , m_userId(userId)
    , m_userFullName(userFullName)
{
    setupUi();
    setupConnections();
    m_userInfoLabel->setText("Бронирование для: " + m_userFullName);
    m_userInfoLabel->setVisible(true);
    m_dateEdit->setDate(QDate::currentDate());
    m_timeFromEdit->setTime(QTime(9, 0));
    m_timeToEdit->setTime(QTime(10, 0));
    calculateCost();
}

void BookingDialog::setupUi()
{
    setWindowTitle("Бронирование зала");
    setMinimumWidth(400);

    auto *mainLayout = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    m_userInfoLabel = new QLabel;
    m_userInfoLabel->setVisible(false);
    mainLayout->addWidget(m_userInfoLabel);

    m_hallCombo = new QComboBox;
    m_hallCombo->addItems({"Зал 1", "Зал 2", "Зал 3"});
    form->addRow("Зал:", m_hallCombo);

    m_dateEdit = new QDateEdit;
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setMinimumDate(QDate::currentDate());
    form->addRow("Дата:", m_dateEdit);

    m_timeFromEdit = new QTimeEdit;
    m_timeFromEdit->setDisplayFormat("HH:mm");
    form->addRow("Начало:", m_timeFromEdit);

    m_timeToEdit = new QTimeEdit;
    m_timeToEdit->setDisplayFormat("HH:mm");
    form->addRow("Окончание:", m_timeToEdit);

    m_sportCombo = new QComboBox;
    m_sportCombo->addItems({"Фитнес", "Йога", "Бокс", "Танцы", "Другое"});
    m_sportCombo->setEditable(true);
    form->addRow("Вид спорта:", m_sportCombo);

    m_recurringCheck = new QCheckBox("Постоянное бронирование (каждую неделю)");
    form->addRow(m_recurringCheck);

    m_costLabel = new QLabel("0.00 ₽");
    m_costLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    form->addRow("Стоимость:", m_costLabel);

    mainLayout->addLayout(form);

    auto *btnLayout = new QHBoxLayout;
    m_okBtn = new QPushButton("Забронировать");
    m_cancelBtn = new QPushButton("Отмена");
    btnLayout->addStretch();
    btnLayout->addWidget(m_okBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);
}

void BookingDialog::setupConnections()
{
    connect(m_hallCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BookingDialog::calculateCost);
    connect(m_dateEdit, &QDateEdit::dateChanged, this, &BookingDialog::calculateCost);
    connect(m_timeFromEdit, &QTimeEdit::timeChanged, this, &BookingDialog::calculateCost);
    connect(m_timeToEdit, &QTimeEdit::timeChanged, this, &BookingDialog::calculateCost);
    connect(m_recurringCheck, &QCheckBox::toggled, this, &BookingDialog::calculateCost);
    connect(m_okBtn, &QPushButton::clicked, this, &BookingDialog::onAccept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void BookingDialog::calculateCost()
{
    QString hall = m_hallCombo->currentText();
    QDate date = m_dateEdit->date();
    QTime from = m_timeFromEdit->time();
    QTime to = m_timeToEdit->time();

    double hours = from.secsTo(to) / 3600.0;
    if (hours <= 0) {
        m_costLabel->setText("Некорректное время");
        return;
    }

    QStringList days = {"Пн","Вт","Ср","Чт","Пт","Сб","Вс"};
    QString dayOfWeek = days.at(date.dayOfWeek() - 1);

    double total = 0.0;
    if (m_recurringCheck->isChecked()) {
        total = m_db->getMonthlySubscriptionCost();
    } else {
        QTime time = from;
        while (time < to) {
            int hour = time.hour();
            double rate = m_db->getHourlyRate(dayOfWeek, hour);
            QTime nextHour(time.hour() + 1, 0, 0);
            if (nextHour > to)
                nextHour = to;
            if (nextHour <= time)
                nextHour = to;
            double fraction = time.secsTo(nextHour) / 3600.0;
            total += rate * fraction;
            time = nextHour;
        }
    }
    m_costLabel->setText(QString::number(total, 'f', 2) + " ₽");
}

void BookingDialog::onAccept()
{
    QTime from = m_timeFromEdit->time();
    QTime to = m_timeToEdit->time();
    int minutes = from.secsTo(to) / 60;

    if (from.minute() % 30 != 0 || to.minute() % 30 != 0) {
        QMessageBox::warning(this, "Ошибка", "Время должно быть кратно 30 минутам.");
        return;
    }
    if (minutes < 60) {
        QMessageBox::warning(this, "Ошибка", "Минимальное время бронирования — 1 час.");
        return;
    }
    accept();
}

Booking BookingDialog::bookingData() const
{
    Booking b;
    b.date = m_dateEdit->date();
    b.startTime = m_timeFromEdit->time();
    b.endTime = m_timeToEdit->time();
    b.hallName = m_hallCombo->currentText();
    b.sportType = m_sportCombo->currentText();
    b.isRecurring = m_recurringCheck->isChecked();
    b.cost = m_costLabel->text().remove(" ₽").toDouble();
    if (m_userId != -1) {
        b.userId = m_userId;
        b.userFullName = m_userFullName;
    }
    return b;
}