#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QDate>
#include <QList>
#include <QStringList>

class CalendarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarWidget(bool showUserNames = false, QWidget *parent = nullptr);
    void refresh();                     // перезагрузить данные для текущего месяца

signals:
    void monthChanged(const QDate &firstDay);

private slots:
    void goToPreviousMonth();
    void goToNextMonth();
    void onMonthSelected(int index);
    void onCellClicked(int row, int col);   // новый слот

private:
    void setupUi();
    void populateTable();
    QList<QDate> getDaysOfMonth(const QDate &firstDay) const;

    bool m_showUserNames;
    QTableWidget *m_table;
    QComboBox *m_monthCombo;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;

    QDate m_currentMonthStart;
    QStringList m_halls;
    QList<QDate> m_days;                    // храним дни текущего месяца для быстрого доступа

    class Database *m_db;
};

#endif // CALENDARWIDGET_H