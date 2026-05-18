#include "bookingdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTime>
#include <QTimeZone>

BookingDialog::BookingDialog(QWidget *parent)
    : QDialog(parent)
    , m_db(Database::instance())
{
    setupUi();
    setupConnections();
    // По умолчанию дата сегодня, время с 9:00 до 10:00
    m_dateEdit->setDate(QDate::currentDate());
    m_timeFromEdit->setTime(QTime(9, 0));
    m_timeToEdit->setTime(QTime(10, 0));
    calculateCost(); // первичный расчёт
}

BookingDialog::BookingDialog(const Booking &booking, QWidget *parent)
    : QDialog(parent)
    , m_db(Database::instance())
    , m_editing(true)
{
    setupUi();
    setupConnections();
    fillFromBooking(booking);
    calculateCost();
}

void BookingDialog::setupUi()
{
    setWindowTitle("Бронирование зала");
    setMinimumWidth(400);

    auto *mainLayout = new QVBoxLayout(this);

    // Форма ввода
    auto *form = new QFormLayout;

    m_hallCombo = new QComboBox;
    m_hallCombo->addItems({"Зал 1", "Зал 2", "Зал 3"});  // список залов из БД в реальном проекте
    form->addRow("Зал:", m_hallCombo);

    m_dateEdit = new QDateEdit;
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setMinimumDate(QDate::currentDate());
    form->addRow("Дата:", m_dateEdit);

    // Время начала
    m_timeFromEdit = new QTimeEdit;
    m_timeFromEdit->setDisplayFormat("HH:mm");
    // Разрешаем только целые и получасы: шаг 30 минут, минимальное 00:00, максимальное 23:30
    // setTimeSpec больше не нужен, по умолчанию LocalTime
    form->addRow("Начало:", m_timeFromEdit);

    m_timeToEdit = new QTimeEdit;
    m_timeToEdit->setDisplayFormat("HH:mm");
    form->addRow("Окончание:", m_timeToEdit);

    m_sportCombo = new QComboBox;
    m_sportCombo->addItems({"Фитнес", "Йога", "Бокс", "Танцы", "Другое"});
    m_sportCombo->setEditable(true);
    form->addRow("Вид спорта:", m_sportCombo);

    m_recurringCheck = new QCheckBox("Постоянное бронирование (каждую неделю)");
    m_recurringCheck->setToolTip("Бронирование будет автоматически повторяться в тот же день недели и время на следующий месяц");
    form->addRow(m_recurringCheck);

    // Расчёт стоимости
    m_costLabel = new QLabel("0.00 ₽");
    m_costLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    form->addRow("Стоимость:", m_costLabel);

    mainLayout->addLayout(form);

    // Кнопки
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
    // При изменении любого параметра пересчитываем стоимость
    connect(m_hallCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BookingDialog::calculateCost);
    connect(m_dateEdit, &QDateEdit::dateChanged, this, &BookingDialog::calculateCost);
    connect(m_timeFromEdit, &QTimeEdit::timeChanged, this, &BookingDialog::calculateCost);
    connect(m_timeToEdit, &QTimeEdit::timeChanged, this, &BookingDialog::calculateCost);
    connect(m_recurringCheck, &QCheckBox::toggled, this, &BookingDialog::calculateCost);
    // Кнопки
    connect(m_okBtn, &QPushButton::clicked, this, &BookingDialog::onAccept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void BookingDialog::fillFromBooking(const Booking &b)
{
    m_hallCombo->setCurrentText(b.hallName);
    m_dateEdit->setDate(b.date);
    m_timeFromEdit->setTime(b.startTime);
    m_timeToEdit->setTime(b.endTime);
    m_sportCombo->setCurrentText(b.sportType);
    m_recurringCheck->setChecked(b.isRecurring);
}

void BookingDialog::calculateCost()
{
    QString hall = m_hallCombo->currentText();
    QDate date = m_dateEdit->date();
    QTime from = m_timeFromEdit->time();
    QTime to = m_timeToEdit->time();

    // Длительность в часах (дробное)
    double hours = from.secsTo(to) / 3600.0;

    if (hours <= 0) {
        m_costLabel->setText("Некорректное время");
        return;
    }

    // Определяем день недели: Пн,Вт,Ср,Чт,Пт,Сб,Вс
    QStringList days = {"Пн","Вт","Ср","Чт","Пт","Сб","Вс"};
    QString dayOfWeek = days.at(date.dayOfWeek() - 1);

    double total = 0.0;

    // Если постоянное бронирование – цена абонемента
    if (m_recurringCheck->isChecked()) {
        total = m_db->getMonthlySubscriptionCost();
    } else {
        // Почасовой расчёт с учётом тарифов для каждого часа
        QTime time = from;
        while (time < to) {
            int hour = time.hour();
            double rate = m_db->getHourlyRate(dayOfWeek, hour);

            // Определяем следующий час или конец интервала
            QTime nextHour(time.hour() + 1, 0, 0);
            if (nextHour > to)
                nextHour = to;
            if (nextHour <= time) // на случай перехода через полночь (маловероятно)
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
    // Проверка минимальной длительности 1 час или 1.5 часа (кратность 30 мин, но не менее 1 часа)
    QTime from = m_timeFromEdit->time();
    QTime to = m_timeToEdit->time();
    int minutes = from.secsTo(to) / 60;

    // Проверка, что время начала и конца кратны 30 минутам (0 или 30)
    if (from.minute() % 30 != 0 || to.minute() % 30 != 0) {
        QMessageBox::warning(this, "Ошибка", "Время должно быть кратно 30 минутам (целый час или половина).");
        return;
    }
    if (minutes < 60) {
        QMessageBox::warning(this, "Ошибка", "Минимальное время бронирования — 1 час.");
        return;
    }

    // Если всё хорошо, принимаем диалог
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
    b.cost = m_costLabel->text().remove(" ₽").toDouble(); // или сохранить из calculateCost
    return b;
}