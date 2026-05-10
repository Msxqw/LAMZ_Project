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
#include <QSpinBox>
#include <cstdint>
#include <QRadioButton>
#include <QButtonGroup>
#include <QRandomGenerator>
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    enum Column {
        COLUMN_ID = 0,
        COLUMN_WR,
        COLUMN_ADDR,
        COLUMN_DATA,
        COLUMN_STATUS,
        COLUMN_COUNT
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
    void onReadyRead();
    void onAddRow();
    void onDelRow();
    void onSend();
    void onSave();
    void onLoad();
    void onApplyStrobe();
    void updateRowDataEditable(int row);
    void logMessage(QString msg);
    void onApplySync();
    void onSyncSourceChanged();
    void onEepromWriteClicked();
    void onEepromReadClicked();

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

    QSpinBox *periodSpinBox = nullptr;
    QSpinBox *pulseSpinBox = nullptr;
    QPushButton *applyStrobeButton = nullptr;

    QWidget *syncTab = nullptr;
    QRadioButton *internalSourceRadio = nullptr;
    QRadioButton *externalSourceRadio = nullptr;
    QButtonGroup *syncSourceGroup = nullptr;
    QSpinBox *syncFreqSpinBox = nullptr;
    QPushButton *applySyncButton = nullptr;
    QLabel *syncHintLabel = nullptr;

    QWidget *eepromTab = nullptr;
    QSpinBox *eepromAddressSpinBox = nullptr;
    QComboBox *eepromSizeComboBox = nullptr;
    QPushButton *eepromWriteButton = nullptr;
    QPushButton *eepromReadButton = nullptr;
    QLabel *eepromStatusLabel = nullptr;
    QByteArray eepromWriteData;
    QByteArray eepromReadData;
    int eepromCurrentSize = 0;
    quint32 eepromCurrentAddress = 0;

};

#endif // MAINWINDOW_H
