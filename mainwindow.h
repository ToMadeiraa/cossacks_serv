#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qtimer.h"
#include <QMainWindow>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaType>
#include <QSet>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void Zip(QString filename , QString zipfilename);

signals:
private slots:
    void newConnection();
    void appendToSocketList(QTcpSocket* socket);

    void readSocket();
    void discardSocket();

    void sendAttachment(QTcpSocket* socket, QString filePath);

private:
    Ui::MainWindow *ui;

    QTcpServer* m_server;
    QSet<QTcpSocket*> connection_set;
};

#endif // MAINWINDOW_H
