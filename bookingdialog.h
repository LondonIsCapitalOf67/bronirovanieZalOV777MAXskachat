#ifndef BOOKINGDIALOG_H
#define BOOKINGDIALOG_H

#include <QDialog>
#include <QDateEdit>
#include <QTimeEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>

struct Booking;   // ваш класс/структура бронирования

class BookingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookingDialog(QWidget *parent = nullptr);
    BookingDialog(const Booking &booking, QWidget *parent = nullptr); // для редактирования

    Booking bookingData() const;  // возвращает заполненные данные

private slots:
    void calculateCost();         // пересчёт стоимости при изменении параметров
    void onAccept();

private:
    void setupUi();
    void setupConnections();
    void fillFromBooking(const Booking &b); // заполнение полей при редактировании

    // Элементы интерфейса
    QComboBox   *m_hallCombo;
    QDateEdit   *m_dateEdit;
    QTimeEdit   *m_timeFromEdit;
    QTimeEdit   *m_timeToEdit;
    QComboBox   *m_sportCombo;
    QCheckBox   *m_recurringCheck;
    QLabel      *m_costLabel;
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;

    class Database *m_db;   // ваш синглтон БД
    bool m_editing = false; // флаг редактирования существующего бронирования
};

#endif // BOOKINGDIALOG_H