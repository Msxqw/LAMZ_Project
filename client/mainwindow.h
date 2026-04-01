#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QByteArray>
#include <QComboBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onPingClicked();
    void onConnected();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError);
    void onReadyRead();
    void onAddRow();
    void onDelRow();
    void onSend();
    void onSave();
    void onLoad();

    void updateRowDataEditable(int row);
    void logMessage(QString msg);

private:
    void setupUi();
    void setupConnections();
    QByteArray createCommand(uint8_t command,
                             uint8_t messageId,
                             uint8_t flags,
                             uint8_t slave_id,
                             uint8_t ic_addr,
                             uint32_t data_request);

    //Поля интерфейса
    QTabWidget *tabWidget;
    QWidget *connectTab;
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *pingButton;
    QLabel *statusLabel;
    QTextEdit *logTextEdit;
    QTcpSocket *socket;
    bool isConnected;
    QByteArray buffer;

    //Поля таблицы ЦАП
    QWidget *dapTab;
    QTableWidget *icTable;
    QPushButton *addRowBtn;
    QPushButton *delRowBtn;
    QPushButton *sendBtn;
    QPushButton *saveBtn;
    QPushButton *loadBtn;
};

#endif
