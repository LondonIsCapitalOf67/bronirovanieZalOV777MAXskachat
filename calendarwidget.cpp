#include "calendarwidget.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>

CalendarWidget::CalendarWidget(bool showUserNames, QWidget *parent)
    : QWidget(parent)
    , m_showUserNames(showUserNames)
    , m_db(Database::instance())
{
    m_currentMonthStart = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
    setupUi();
    populateTable();
}

void CalendarWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // Панель навигации
    auto *navLayout = new QHBoxLayout;
    m_prevBtn = new QPushButton("<< Предыдущий месяц");
    m_nextBtn = new QPushButton("Следующий месяц >>");
    m_monthCombo = new QComboBox;

    QDate today = QDate::currentDate();
    QDate currentStart(today.year(), today.month(), 1);
    QDate nextStart = currentStart.addMonths(1);
    m_monthCombo->addItem(currentStart.toString("MMMM yyyy"), currentStart);
    m_monthCombo->addItem(nextStart.toString("MMMM yyyy"), nextStart);
    m_monthCombo->setCurrentIndex(0);

    navLayout->addWidget(m_prevBtn);
    navLayout->addWidget(m_monthCombo);
    navLayout->addWidget(m_nextBtn);
    mainLayout->addLayout(navLayout);

    m_table = new QTableWidget;
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(true);   // показываем номера строк
    m_table->setAlternatingRowColors(false);
    mainLayout->addWidget(m_table);

    connect(m_prevBtn, &QPushButton::clicked, this, &CalendarWidget::goToPreviousMonth);
    connect(m_nextBtn, &QPushButton::clicked, this, &CalendarWidget::goToNextMonth);
    connect(m_monthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CalendarWidget::onMonthSelected);
}

QList<QDate> CalendarWidget::getDaysOfMonth(const QDate &firstDay) const
{
    QList<QDate> days;
    int daysInMonth = firstDay.daysInMonth();
    for (int d = 1; d <= daysInMonth; ++d) {
        days.append(QDate(firstDay.year(), firstDay.month(), d));
    }
    return days;
}

void CalendarWidget::populateTable()
{
    m_halls = m_db->getHalls();
    int numHalls = m_halls.size();

    QDate monthEnd = m_currentMonthStart.addMonths(1).addDays(-1);
    QList<Booking> bookings = m_db->getAllBookings(m_currentMonthStart, monthEnd);

    QList<QDate> days = getDaysOfMonth(m_currentMonthStart);
    int numDays = days.size();

    m_table->setColumnCount(numHalls);
    m_table->setHorizontalHeaderLabels(m_halls);

    // Формируем вертикальные заголовки: день недели + число
    QStringList dayLabels;
    QStringList dayOfWeekNames = {"Пн","Вт","Ср","Чт","Пт","Сб","Вс"};
    for (const QDate &d : days) {
        int dow = d.dayOfWeek(); // 1=Пн
        dayLabels << QString("%1 %2").arg(dayOfWeekNames.at(dow-1)).arg(d.day());
    }
    m_table->setRowCount(numDays);
    m_table->setVerticalHeaderLabels(dayLabels);

    // Заполнение ячеек
    for (int col = 0; col < numHalls; ++col) {
        const QString &hall = m_halls.at(col);
        for (int row = 0; row < numDays; ++row) {
            QDate date = days.at(row);
            QStringList occupiers;
            bool occupied = false;
            for (const Booking &b : bookings) {
                if (b.date == date && b.hallName == hall) {
                    occupied = true;
                    if (m_showUserNames)
                        occupiers << b.userFullName;
                }
            }

            QTableWidgetItem *item = new QTableWidgetItem;
            item->setForeground(Qt::black);   // чёрный цвет текста
            if (occupied) {
                item->setText(m_showUserNames ? occupiers.join(", ") : "Занято");
                item->setBackground(QColor(255, 200, 200));  // светло-красный
            } else {
                item->setText("Свободно");
                item->setBackground(QColor(200, 255, 200));  // светло-зелёный
            }
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_table->setItem(row, col, item);
        }
    }

    m_table->resizeColumnsToContents();
    m_table->resizeRowsToContents();
}

void CalendarWidget::goToPreviousMonth()
{
    m_currentMonthStart = m_currentMonthStart.addMonths(-1);
    populateTable();
    emit monthChanged(m_currentMonthStart);
}

void CalendarWidget::goToNextMonth()
{
    m_currentMonthStart = m_currentMonthStart.addMonths(1);
    populateTable();
    emit monthChanged(m_currentMonthStart);
}

void CalendarWidget::onMonthSelected(int index)
{
    if (index < 0) return;
    m_currentMonthStart = m_monthCombo->itemData(index).toDate();
    populateTable();
    emit monthChanged(m_currentMonthStart);
}

void CalendarWidget::refresh()
{
    populateTable();
}