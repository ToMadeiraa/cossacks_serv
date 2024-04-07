#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_server = new QTcpServer();
    qDebug() << "SERVER MADE";
    if(m_server->listen(QHostAddress::Any, 10001))
    {
        connect(m_server, &QTcpServer::newConnection, this, &MainWindow::newConnection);
    }
    else
    {
        qDebug() << "Unable to start the server";
        exit(EXIT_FAILURE);
    }

}

MainWindow::~MainWindow()
{
    foreach (QTcpSocket* socket, connection_set)
    {
        socket->close();
        socket->deleteLater();
    }
    m_server->close();
    m_server->deleteLater();

}

void MainWindow::newConnection()
{
    while (m_server->hasPendingConnections())
        appendToSocketList(m_server->nextPendingConnection());
    qDebug() << "NEW CONNECTION";
}

void MainWindow::appendToSocketList(QTcpSocket* socket)
{
    connection_set.insert(socket);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::discardSocket);
    qDebug() << "INFO :: Client with sockd:%1 has just entered the room";
}

void MainWindow::readSocket()
{
    qDebug() << "READING SOCKET";
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());

    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_5_15);

    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        qDebug() << "Waiting for more data to come from descriptor  " << socket->socketDescriptor();
        return;
    }

    QString header = buffer.mid(0,128);
    QString fileType = header.split(",")[0].split(":")[1];

    buffer = buffer.mid(128);

    if(fileType=="attachment"){
        QString fileName = header.split(",")[1].split(":")[1];
        QString ext = fileName.split(".")[1];
        QString size = header.split(",")[2].split(":")[1].split(";")[0];

        if (QMessageBox::Yes == QMessageBox::question(this, "QTCPServer", QString("You are receiving an attachment from sd:%1 of size: %2 bytes, called %3. Do you want to accept it?").arg(socket->socketDescriptor()).arg(size).arg(fileName)))
        {
            QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/"+fileName, QString("File (*.%1)").arg(ext));

            QFile file(filePath);
            if(file.open(QIODevice::WriteOnly)){
                file.write(buffer);
                QString message = QString("INFO :: Attachment from sd:%1 successfully stored on disk under the path %2").arg(socket->socketDescriptor()).arg(QString(filePath));
                //emit newMessage(message);
            }else
                QMessageBox::critical(this,"QTCPServer", "An error occurred while trying to write the attachment.");
        }else{
            QString message = QString("INFO :: Attachment from sd:%1 discarded").arg(socket->socketDescriptor());
            //emit newMessage(message);

        }
    }else if(fileType=="message"){
        QString message = QString::fromStdString(buffer.toStdString());
        qDebug() << "\nNEW MESSAGE:";

        QString pth_serv = QDir::currentPath() + "/ver.ini";
        QFile file_vers_serv(pth_serv);
        file_vers_serv.open(QIODevice::ReadOnly);
        QString version_serv = file_vers_serv.readAll();

        //если не равны версии, то
        if (message != version_serv) {
            qDebug() << "CLIENT VERSION:" << message;
            qDebug() << "SERVER VERSION" << version_serv;
            for (int i = message.toInt() +1; i <= version_serv.toInt(); i++) {
                QString update_str = QDir::currentPath() + "/" + QString::number(i) + "/update.txt";
                qDebug() << "update file: " << update_str;
                QFile f(update_str);
                f.open(QIODevice::ReadOnly);
                if (!f.isOpen())
                    return;

                QTextStream stream(&f);
                for (QString line = stream.readLine(); !line.isNull(); line = stream.readLine()) {
                    QString to_send =  QDir::currentPath() + line;
                    qDebug() << "File " << to_send;

                    foreach (QTcpSocket* socket,connection_set)
                    {
                        sendAttachment(socket, to_send);
                        QThread::msleep(1);
                    }
                };
                f.close();
            }


            QString readyToUpdateFile = QDir::currentPath() + "/new_version.txt";
            foreach (QTcpSocket* socket,connection_set)
            {
                sendAttachment(socket, readyToUpdateFile);
                socket->close();
                socket->deleteLater();
            }
        }
    }
}

void MainWindow::discardSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QSet<QTcpSocket*>::iterator it = connection_set.find(socket);
    if (it != connection_set.end()){
        qDebug() << "INFO : A client has just left the room";
        connection_set.remove(*it);
    }
    socket->deleteLater();
}

void MainWindow::sendAttachment(QTcpSocket* socket, QString filePath)
{
    socket->flush();
    if(socket)
    {
        if(socket->isOpen())
        {
            QFile m_file(filePath);
            if(m_file.open(QIODevice::ReadOnly)){

                QFileInfo fileInfo(m_file.fileName());
                QString fileName(fileInfo.fileName());

                QDataStream socketStream(socket);
                socketStream.setVersion(QDataStream::Qt_5_15);

                QByteArray header;
                header.prepend(QString("fileType:attachment,fileName:%1,fileSize:%2;").arg(fileName).arg(m_file.size()).toUtf8());
                header.resize(128);

                QByteArray byteArray = m_file.readAll();
                byteArray.prepend(header);

                socketStream << byteArray;
                qDebug() << "SENT";
                qDebug() << "-----------------------";
            } else
                qDebug() << "Couldn't open the attachmenttttt!";
        }
        else
            qDebug() <<  "Socket doesn't seem to be opened";
    }
    else
        qDebug() << "Not connected";
}
