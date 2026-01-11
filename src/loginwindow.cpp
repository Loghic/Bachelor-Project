#include "include/loginwindow.h"

#include <QMessageBox>

#include <QMap> // for login

LoginWindow::LoginWindow(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Login");
    setFixedSize(350, 200);

    leUsername = new QLineEdit(this);
    lePasswordField = new QLineEdit(this);
    lePasswordField->setEchoMode(QLineEdit::Password);

    leUsername->setPlaceholderText("Username");
    lePasswordField->setPlaceholderText("Password");

    btnLogin = new QPushButton("Login", this);
    btnCancel = new QPushButton("Cancel", this);
    btnCancel->setObjectName("btnCancel");

    lLogin = new QVBoxLayout(this);
    lLogin->setSpacing(10);
    lLogin->addWidget(leUsername);
    lLogin->addWidget(lePasswordField);
    lLogin->addWidget(btnLogin);
    lLogin->addWidget(btnCancel);

    setLayout(lLogin);

    connect(btnLogin, &QPushButton::clicked, this, &LoginWindow::attemptLogin);
    connect(btnCancel, &QPushButton::clicked, this, &LoginWindow::reject); // Close on cancel
}

LoginWindow::~LoginWindow() {}

void LoginWindow::attemptLogin() {
    QString username = leUsername->text();
    QString password = lePasswordField->text();

    QMap<QString, QString> users;
    users["admin"] = "pswd";
    users["test"] = "test";
    users["user2"] = "pswd2";
    users["user3"] = "pswd3";

    if (users.contains(username) && users[username] == password) {
        accept(); // Login successful, close login window
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password!");
    }
}

QString LoginWindow::getUsername() const {
    return leUsername->text();
}
