#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QDate>

class CalendarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarWidget(bool showUserNames = false, QWidget *parent = nullptr);
    void refresh();                     // перезагрузить данные для текущего месяца

signals:
    void monthChanged(const QDate &firstDay);   // опционально

private slots:
    void goToPreviousMonth();
    void goToNextMonth();
    void onMonthSelected(int index);

private:
    void setupUi();
    void populateTable();
    QList<QDate> getDaysOfMonth(const QDate &firstDay) const;

    bool m_showUserNames;               // показывать ли имена бронирующих (для админа)
    QTableWidget *m_table;
    QComboBox *m_monthCombo;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;

    QDate m_currentMonthStart;          // первое число отображаемого месяца
    QStringList m_halls;                // список залов из БД

    class Database *m_db;
};

#endif // CALENDARWIDGET_H