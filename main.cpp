#include <QApplication>
#include "loginwindow.h"
#include "mainwindow.h"
#include "adminwindow.h"
#include "database.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Database *db = Database::instance();
    if (!db->initialize()) {
        qCritical() << "Не удалось инициализировать базу данных";
        return -1;
    }

    LoginWindow loginWindow;
    loginWindow.show();

    QObject::connect(&loginWindow, &LoginWindow::loginSuccess, [&](const User &user) {
        loginWindow.hide();

        if (user.login == "admin") {
            AdminWindow *adminWin = new AdminWindow;
            adminWin->show();
        } else {
            MainWindow *userWin = new MainWindow(user);
            userWin->setLoginWindow(&loginWindow);   // передаём указатель
            userWin->show();
        }
    });

    return a.exec();
}