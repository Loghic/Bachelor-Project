#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class QLineEdit;
class QPushButton;

class LoginWindow : public QDialog {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

    QString getUsername() const;

private slots:
    void attemptLogin();

private:
    QLineEdit *leUsername, *lePasswordField;
    QPushButton *btnLogin, *btnCancel;
    QVBoxLayout *lLogin;
};

#endif // LOGINWINDOW_H
