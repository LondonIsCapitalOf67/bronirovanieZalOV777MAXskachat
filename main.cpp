#include <QApplication>
#include "loginwindow.h"
#include "mainwindow.h"
#include "adminwindow.h"
#include "database.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Инициализация базы данных
    Database *db = Database::instance();
    if (!db->initialize()) {
        qCritical() << "Не удалось инициализировать базу данных";
        return -1;
    }

    LoginWindow loginWindow;
    loginWindow.show();

    // При успешном входе открываем соответствующее окно
    QObject::connect(&loginWindow, &LoginWindow::loginSuccess,
                     [&](const User &user) {
                         loginWindow.close();
                         // Проверка на администратора – например, по специальному логину "admin"
                         if (user.login == "admin") {
                             AdminWindow *adminWin = new AdminWindow;
                             adminWin->show();
                         } else {
                             MainWindow *userWin = new MainWindow(user);
                             userWin->show();
                         }
                     });

    return a.exec();
}