#include "include/mainwindow.h"
#include "include/loginwindow.h"
#include <QString>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <QLoggingCategory>

#include <iostream>

void applyStyleSheet(QApplication &app, const QString &filePath) {
    QFile file(filePath);

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        app.setStyleSheet(styleSheet);
        file.close();
        qDebug() << "Stylesheet applied successfully!";
    } else {
        qWarning() << "Failed to load stylesheet file:" << filePath;
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QLoggingCategory::setFilterRules("*.debug=false\n");
    uhal::setLogLevelTo(uhal::Error());
    applyStyleSheet(app, ":/style.qss");

    // Create and show the login window
    // LoginWindow loginWin;

    // if (loginWin.exec() == QDialog::Accepted) {
    //     // If login successful, show the main window
    //     MainWindow mainWin(0, loginWin.getUsername()); // Get the username from the login window
    //     mainWin.show();
    //     return app.exec();
    // }
    MainWindow mainWin(0, "DAQ"); // Get the username from the login window
    mainWin.show();
    return app.exec();

    return 0;
}
