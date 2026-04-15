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
#include <cstdint>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    enum Column {
        ColumnId = 0,
        ColumnWr,
        ColumnAddr,
        ColumnData,
        ColumnStatus,
        ColumnCount
    };

    struct CommandData {
        uint8_t command = 0;
        uint8_t messageId = 0;
        uint8_t flags = 0;
        uint8_t slaveId = 0;
        uint8_t icAddr = 0;
        uint32_t dataRequest = 0;
    };

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
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
    QByteArray createCommand(const CommandData &cmd);

    QTabWidget *tabWidget = nullptr;
    QWidget *connectTab = nullptr;
    QLineEdit *ipLineEdit = nullptr;
    QLineEdit *portLineEdit = nullptr;
    QPushButton *connectButton = nullptr;
    QPushButton *disconnectButton = nullptr;
    QLabel *statusLabel = nullptr;
    QTextEdit *logTextEdit = nullptr;
    QTcpSocket *socket = nullptr;
    bool isConnected = false;
    QByteArray buffer;

    QWidget *dapTab = nullptr;
    QTableWidget *icTable = nullptr;
    QPushButton *addRowBtn = nullptr;
    QPushButton *delRowBtn = nullptr;
    QPushButton *sendBtn = nullptr;
    QPushButton *saveBtn = nullptr;
    QPushButton *loadBtn = nullptr;
};

#endif // MAINWINDOW_H
