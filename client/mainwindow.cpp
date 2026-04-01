#include "mainwindow.h"
#include "protocol.h"
#include "themes.h"
#include <QApplication>
#include <QDateTime>
#include <QNetworkProxy>
#include <QTableWidget>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QtEndian>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    socket = new QTcpSocket(this);
    isConnected = false;
    buffer.clear();

    setupUi();
    setupConnections();
    socket->setProxy(QNetworkProxy::NoProxy);
}

MainWindow::~MainWindow() {}

//НАСТРОЙКА ИНТЕРФЕЙСА
void MainWindow::setupUi()
{
    setWindowTitle("Клиент для ЦАП");
    resize(1100, 750);

    qApp->setStyleSheet(QString::fromUtf8(DARK_THEME));

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    // ВКЛАДКА: ПОДКЛЮЧЕНИЕ
    connectTab = new QWidget();
    tabWidget->addTab(connectTab, "Подключение");

    QVBoxLayout *connectLayout = new QVBoxLayout(connectTab);

    ipLineEdit = new QLineEdit("192.168.100.9");
    portLineEdit = new QLineEdit("12345");

    connectButton   = new QPushButton("Подключиться");
    disconnectButton = new QPushButton("Отключиться");
    disconnectButton->setEnabled(false);
    pingButton = new QPushButton("Ping");
    pingButton->setEnabled(false);

    statusLabel = new QLabel("Не подключён");
    logTextEdit = new QTextEdit();
    logTextEdit->setReadOnly(true);

    connectLayout->addWidget(new QLabel("IP:"));
    connectLayout->addWidget(ipLineEdit);
    connectLayout->addWidget(new QLabel("Порт:"));
    connectLayout->addWidget(portLineEdit);

    QHBoxLayout *btnsLayout = new QHBoxLayout();
    btnsLayout->addWidget(connectButton);
    btnsLayout->addWidget(disconnectButton);
    btnsLayout->addWidget(pingButton);
    connectLayout->addLayout(btnsLayout);

    connectLayout->addWidget(statusLabel);
    connectLayout->addWidget(new QLabel("Лог:"));
    connectLayout->addWidget(logTextEdit);

    // ВКЛАДКА: ЦАП ТЕСТЫ
    QWidget *dapTab = new QWidget();
    tabWidget->addTab(dapTab, "ЦАП Тесты");

    QVBoxLayout *dapLayout = new QVBoxLayout(dapTab);

    icTable = new QTableWidget(0, 5);
    QStringList headers;
    headers << "IC ID" << "W/R" << "IC_ADDR" << "DATA" << "STATUS";
    icTable->setHorizontalHeaderLabels(headers);
    icTable->viewport()->setAutoFillBackground(false);
    icTable->setStyleSheet(
        "QTableWidget { background-color: #1e1e2e; } "
        "QTableWidget::item { background-color: #1e1e2e; color: #e0e6ed; } "
        "QTableCornerButton::section { background-color: #2a2a3a; }"
        );

    dapLayout->addWidget(icTable);

    QHBoxLayout *tableBtns = new QHBoxLayout();
    addRowBtn = new QPushButton("+ строка");
    delRowBtn = new QPushButton("- строка");
    tableBtns->addWidget(addRowBtn);
    tableBtns->addWidget(delRowBtn);
    tableBtns->addStretch();

    QHBoxLayout *cmdBtns = new QHBoxLayout();
    sendBtn = new QPushButton("SEND");
    saveBtn = new QPushButton("SAVE CFG");
    loadBtn = new QPushButton("LOAD CFG");
    cmdBtns->addWidget(sendBtn);
    cmdBtns->addWidget(saveBtn);
    cmdBtns->addWidget(loadBtn);

    dapLayout->addLayout(tableBtns);
    dapLayout->addLayout(cmdBtns);

    tabWidget->addTab(new QWidget(), "Результаты");
}

//ПОДКЛЮЧЕНИЕ КНОПОК
void MainWindow::setupConnections()
{
    connect(connectButton,   SIGNAL(clicked()), this, SLOT(onConnectClicked()));
    connect(disconnectButton,SIGNAL(clicked()), this, SLOT(onDisconnectClicked()));
    connect(pingButton,      SIGNAL(clicked()), this, SLOT(onPingClicked()));

    connect(socket, SIGNAL(connected()),          this, SLOT(onConnected()));
    connect(socket, SIGNAL(disconnected()),       this, SLOT(onDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()),          this, SLOT(onReadyRead()));

    connect(addRowBtn, SIGNAL(clicked()),         this, SLOT(onAddRow()));
    connect(delRowBtn, SIGNAL(clicked()),         this, SLOT(onDelRow()));
    connect(sendBtn,   SIGNAL(clicked()),         this, SLOT(onSend()));
    connect(saveBtn,   SIGNAL(clicked()),         this, SLOT(onSave()));
    connect(loadBtn,   SIGNAL(clicked()),         this, SLOT(onLoad()));
}

//ОБНОВЛЕНИЕ ПОЛЯ DATA ДЛЯ Read/Write
void MainWindow::updateRowDataEditable(int row)
{
    if (row < 0 || row >= icTable->rowCount()) return;

    QWidget *wrWidget = icTable->cellWidget(row, 1);
    if (!wrWidget) return;
    QComboBox *wrCombo = qobject_cast<QComboBox*>(wrWidget);
    if (!wrCombo) return;

    QString wrStr = wrCombo->currentText();
    if (wrStr != "W" && wrStr != "R") return;

    QTableWidgetItem *dataItem = icTable->item(row, 3);
    if (!dataItem) return;

    if (wrStr == "R") {
        //DATA не редачится при R
        dataItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    } else {
        //DATA редачится при W
        dataItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    }
}

//ЛОГ СООБЩЕНИЙ
void MainWindow::logMessage(QString msg)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    logTextEdit->append("[" + time + "] " + msg);
}

//ПОДКЛЮЧЕНИЕ К СЕРВАКУ
void MainWindow::onConnectClicked()
{
    QString ip = ipLineEdit->text();
    int port = portLineEdit->text().toInt();

    socket->connectToHost(ip, port);
    logMessage("Подключаюсь к " + ip + ":" + QString::number(port));
}

void MainWindow::onDisconnectClicked()
{
    socket->disconnectFromHost();
    logMessage("Отключаюсь...");
}

void MainWindow::onPingClicked()
{
    if (isConnected) {
        socket->write("PING\n");
        logMessage("PING отправлен");
    }
}

void MainWindow::onConnected()
{
    isConnected = true;
    connectButton->setEnabled(false);
    disconnectButton->setEnabled(true);
    pingButton->setEnabled(true);
    statusLabel->setText("✅ Подключён");
    logMessage("Готово!");
}

void MainWindow::onDisconnected()
{
    isConnected = false;
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
    pingButton->setEnabled(false);
    statusLabel->setText("❌ Отключён");
    logMessage("Соединение закрыто");
}

void MainWindow::onSocketError(QAbstractSocket::SocketError err)
{
    logMessage("Ошибка сокета: " + socket->errorString());
}

//ПРИЁМ ОТВЕТА
void MainWindow::onReadyRead()
{
    buffer.append(socket->readAll());

    while (true) {
        if (buffer.size() < static_cast<int>(Protocol::HEADER_SIZE)) {
            break;
        }

        Protocol::Header hdr;
        memcpy(&hdr, buffer.constData(), sizeof(hdr));

        if (hdr.magic != Protocol::MAGIC) {
            logMessage("MAGIC неверный, сбрасываю буфер");
            buffer.clear();
            break;
        }

        int fullSize = sizeof(Protocol::Header) + hdr.payloadSize;
        if (buffer.size() < fullSize) {
            break;
        }

        if (hdr.command == Protocol::CMD_SPI_WRITE ||
            hdr.command == Protocol::CMD_SPI_READ) {

            if (hdr.flags == Protocol::FLAG_RESPONSE) {
                Protocol::SpiResponse resp;
                memcpy(&resp, buffer.constData(), sizeof(resp));

                logMessage(QString("Ответ: msgId=%1 status=%2 data=0x%3")
                               .arg(resp.messageId)
                               .arg(resp.status)
                               .arg(resp.data_responce, 8, 16, QChar('0')));

                int row = resp.messageId;
                if (row >= 0 && row < icTable->rowCount()) {
                    if (resp.status == 0) {
                        icTable->setItem(row, 4, new QTableWidgetItem("OK"));
                        if (hdr.command == Protocol::CMD_SPI_READ) {
                            QString dataHex = QString("0x%1").arg(resp.data_responce, 8, 16, QChar('0'));
                            icTable->setItem(row, 3, new QTableWidgetItem(dataHex));
                        }
                    } else {
                        icTable->setItem(row, 4, new QTableWidgetItem(QString("ERR %1").arg(resp.status)));
                    }
                }
            } else if (hdr.flags == Protocol::FLAG_ERROR) {
                logMessage("Ошибка сервера (флаг ERROR)");
            }
        }

        buffer.remove(0, fullSize);
    }
}

//ДОБАВЛЕНИЕ СТРОКИ В ТАБЛИЦУ
void MainWindow::onAddRow()
{
    int row = icTable->rowCount();
    icTable->insertRow(row);

    QComboBox *idCombo = new QComboBox();
    idCombo->addItems({"1", "2", "3"});
    idCombo->setCurrentIndex(0);
    icTable->setCellWidget(row, 0, idCombo);

    QComboBox *wrCombo = new QComboBox();
    wrCombo->addItems({"W", "R"});
    wrCombo->setCurrentIndex(0);

    connect(wrCombo, &QComboBox::currentIndexChanged, [this, row](int) {
        updateRowDataEditable(row);
    });

    icTable->setCellWidget(row, 1, wrCombo);

    QTableWidgetItem *addrItem = new QTableWidgetItem();
    icTable->setItem(row, 2, addrItem);

    QTableWidgetItem *dataItem = new QTableWidgetItem();
    icTable->setItem(row, 3, dataItem);

    QTableWidgetItem *statusItem = new QTableWidgetItem("");
    statusItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    icTable->setItem(row, 4, statusItem);

    updateRowDataEditable(row);

    logMessage("Добавлена пустая строка #" + QString::number(row+1));
}

//УДАЛЕНИЕ СТРОКИ ИЗ ТАБЛИЦЫ
void MainWindow::onDelRow()
{
    int row = icTable->currentRow();
    if (row >= 0) {
        icTable->removeRow(row);
        logMessage("Удалена строка #" + QString::number(row+1));
    }
}

//СБОРКА ПАКЕТА SPI-КОМАНДЫ
QByteArray MainWindow::createCommand(uint8_t command,
                                     uint8_t messageId,
                                     uint8_t flags,
                                     uint8_t slave_id,
                                     uint8_t ic_addr,
                                     uint32_t data_request)
{
    Protocol::SpiRequest req;
    req.magic       = Protocol::MAGIC;
    req.command     = command;          // 0x00 = WRITE, 0x01 = READ
    req.messageId   = messageId;        // номер строки
    req.flags       = flags;
    req.payloadSize = sizeof(Protocol::SpiRequest) - sizeof(Protocol::Header);

    req.slave_id     = slave_id;
    req.ic_addr      = ic_addr;
    req.data_request = data_request;

    QByteArray packet(reinterpret_cast<const char*>(&req),
                      sizeof(Protocol::SpiRequest));

    return packet;
}

//ОТПРАВКА КОМАНД ИЗ ТАБЛИЦЫ
void MainWindow::onSend()
{
    if (!isConnected) {
        QMessageBox::warning(this, "Ошибка", "Сначала подключись!");
        return;
    }

    for (int row = 0; row < icTable->rowCount(); row++) {

        QWidget *idWidget = icTable->cellWidget(row, 0);
        if (!idWidget) continue;
        QComboBox *idCombo = qobject_cast<QComboBox*>(idWidget);
        if (!idCombo) continue;
        bool ok = false;
        uint8_t slave_id = idCombo->currentText().toUInt(&ok);
        if (!ok) {
            icTable->setItem(row, 4, new QTableWidgetItem("BAD IC ID"));
            continue;
        }

        QWidget *wrWidget = icTable->cellWidget(row, 1);
        if (!wrWidget) continue;
        QComboBox *wrCombo = qobject_cast<QComboBox*>(wrWidget);
        if (!wrCombo) continue;
        QString wrStr = wrCombo->currentText();

        uint8_t wr = 0;
        if (wrStr == "W") {
            wr = 0;
        } else if (wrStr == "R") {
            wr = 1;
        } else {
            icTable->setItem(row, 4, new QTableWidgetItem("BAD W/R"));
            continue;
        }

        QString addrStr = icTable->item(row, 2)->text().trimmed();
        uint32_t addrVal = addrStr.toUInt(&ok, 16);
        if (!ok) {
            icTable->setItem(row, 4, new QTableWidgetItem("BAD IC ADDR"));
            continue;
        }
        uint8_t ic_addr = static_cast<uint8_t>(addrVal & 0xFF);

        uint32_t dataVal = 0;
        if (wr == 0) {
            QString dataStr = icTable->item(row, 3)->text().trimmed();
            dataVal = dataStr.toUInt(&ok, 16);
            if (!ok) {
                icTable->setItem(row, 4, new QTableWidgetItem("BAD DATA WR"));
                continue;
            }
        }

        uint8_t command = (wr == 0) ? Protocol::CMD_SPI_WRITE : Protocol::CMD_SPI_READ;
        uint8_t flags   = Protocol::FLAG_REQUEST;
        uint8_t msgId   = static_cast<uint8_t>(row);

        QByteArray packet = createCommand(command, msgId, flags,
                                          slave_id, ic_addr, dataVal);

        socket->write(packet);
        icTable->setItem(row, 4, new QTableWidgetItem("SENT"));
    }
}

//СОХРАНЕНИЕ КОНФИГУРАЦИИ В JSON
void MainWindow::onSave()
{
    QString file = QFileDialog::getSaveFileName(this, "Сохранить", "", "JSON (*.json)");
    if (file.isEmpty()) return;

    QFile f(file);
    if (f.open(QFile::WriteOnly)) {
        QJsonArray array;
        for (int i = 0; i < icTable->rowCount(); i++) {
            QJsonObject obj;

            QComboBox *idCombo = qobject_cast<QComboBox*>(icTable->cellWidget(i, 0));
            obj["id"] = idCombo ? idCombo->currentText() : "";

            QComboBox *wrCombo = qobject_cast<QComboBox*>(icTable->cellWidget(i, 1));
            obj["wr"] = wrCombo ? wrCombo->currentText() : "";

            obj["addr"] = icTable->item(i, 2) ? icTable->item(i, 2)->text() : "";
            obj["data"] = icTable->item(i, 3) ? icTable->item(i, 3)->text() : "";
            array.append(obj);
        }
        QJsonDocument doc(array);
        f.write(doc.toJson());
        f.close();
        logMessage("Сохранено в " + file);
    }
}

//ЗАГРУЗКА КОНФИГУРАЦИИ ИЗ JSON
void MainWindow::onLoad()
{
    QString file = QFileDialog::getOpenFileName(this, "Загрузить", "", "JSON (*.json)");
    if (file.isEmpty()) return;

    QFile f(file);
    if (f.open(QFile::ReadOnly)) {
        icTable->setRowCount(0);
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        QJsonArray array = doc.array();

        for (int i = 0; i < array.size(); i++) {
            QJsonObject obj = array[i].toObject();
            int row = icTable->rowCount();
            icTable->insertRow(row);

            QComboBox *idCombo = new QComboBox();
            idCombo->addItems({"1", "2", "3"});
            idCombo->setCurrentText(obj["id"].toString());
            icTable->setCellWidget(row, 0, idCombo);

            QComboBox *wrCombo = new QComboBox();
            wrCombo->addItems({"W", "R"});
            wrCombo->setCurrentText(obj["wr"].toString());
            icTable->setCellWidget(row, 1, wrCombo);

            QTableWidgetItem *addrItem = new QTableWidgetItem(obj["addr"].toString());
            icTable->setItem(row, 2, addrItem);

            QTableWidgetItem *dataItem = new QTableWidgetItem(obj["data"].toString());
            icTable->setItem(row, 3, dataItem);

            QTableWidgetItem *statusItem = new QTableWidgetItem("");
            icTable->setItem(row, 4, statusItem);

            updateRowDataEditable(row);
        }
        logMessage("Загружено " + QString::number(array.size()) + " строк");
        f.close();
    }
}
