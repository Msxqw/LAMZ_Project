#include "mainwindow.h"
#include "protocol.h"
#include "themes.h"

#include <QApplication>
#include <QDateTime>
#include <QNetworkProxy>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QComboBox>
#include <QHeaderView>
#include <QtEndian>
#include <cstring>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    socket = new QTcpSocket(this);
    setupUi();
    setupConnections();
    socket->setProxy(QNetworkProxy::NoProxy);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle("Клиент для ЦАП");
    resize(1100, 750);

    qApp->setStyleSheet(QString::fromUtf8(DARK_THEME));

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    connectTab = new QWidget();
    tabWidget->addTab(connectTab, "Подключение");

    QVBoxLayout *connectLayout = new QVBoxLayout(connectTab);

    ipLineEdit = new QLineEdit("192.168.100.9");
    portLineEdit = new QLineEdit("12345");

    connectButton = new QPushButton("Подключиться");
    disconnectButton = new QPushButton("Отключиться");
    disconnectButton->setEnabled(false);

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
    connectLayout->addLayout(btnsLayout);

    connectLayout->addWidget(statusLabel);
    connectLayout->addWidget(new QLabel("Лог:"));
    connectLayout->addWidget(logTextEdit);

    dapTab = new QWidget();
    tabWidget->addTab(dapTab, "ЦАП Тесты");

    QVBoxLayout *dapLayout = new QVBoxLayout(dapTab);

    icTable = new QTableWidget(0, ColumnCount);
    QStringList headers;
    headers << "IC ID" << "W/R" << "IC_ADDR" << "DATA" << "STATUS";
    icTable->setHorizontalHeaderLabels(headers);
    icTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    icTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    icTable->setSelectionMode(QAbstractItemView::SingleSelection);
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

void MainWindow::setupConnections()
{
    QObject::connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    QObject::connect(disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);

    QObject::connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    QObject::connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    QObject::connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    QObject::connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketError);

    QObject::connect(addRowBtn, &QPushButton::clicked, this, &MainWindow::onAddRow);
    QObject::connect(delRowBtn, &QPushButton::clicked, this, &MainWindow::onDelRow);
    QObject::connect(sendBtn, &QPushButton::clicked, this, &MainWindow::onSend);
    QObject::connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSave);
    QObject::connect(loadBtn, &QPushButton::clicked, this, &MainWindow::onLoad);
}

void MainWindow::updateRowDataEditable(int row)
{
    if (row < 0 || row >= icTable->rowCount()) {
        return;
    }

    QWidget *wrWidget = icTable->cellWidget(row, ColumnWr);
    if (wrWidget == nullptr) {
        return;
    }

    QComboBox *wrCombo = qobject_cast<QComboBox*>(wrWidget);
    if (wrCombo == nullptr) {
        return;
    }

    QTableWidgetItem *dataItem = icTable->item(row, ColumnData);
    if (dataItem == nullptr) {
        return;
    }

    if (wrCombo->currentText() == "R") {
        dataItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    } else {
        dataItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    }
}

void MainWindow::logMessage(QString msg)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    logTextEdit->append("[" + time + "] " + msg);
}

void MainWindow::onConnectClicked()
{
    QString ip = ipLineEdit->text();
    quint16 port = portLineEdit->text().toUShort();
    socket->connectToHost(ip, port);
    logMessage("Подключаюсь к " + ip + ":" + QString::number(port));
}

void MainWindow::onDisconnectClicked()
{
    socket->disconnectFromHost();
    logMessage("Отключаюсь...");
}

void MainWindow::onConnected()
{
    isConnected = true;
    connectButton->setEnabled(false);
    disconnectButton->setEnabled(true);
    statusLabel->setText("✅ Подключён");
    logMessage("Готово!");
}

void MainWindow::onDisconnected()
{
    isConnected = false;
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
    statusLabel->setText("❌ Отключён");
    logMessage("Соединение закрыто");
}

void MainWindow::onSocketError(QAbstractSocket::SocketError)
{
    logMessage("Ошибка сокета: " + socket->errorString());
}

void MainWindow::onReadyRead()
{
    buffer.append(socket->readAll());

    while (true) {
        if (buffer.size() < Protocol::HEADER_SIZE) {
            break;
        }

        Protocol::Header hdr;
        std::memcpy(&hdr, buffer.constData(), sizeof(hdr));

        if (hdr.magic != Protocol::MAGIC) {
            logMessage("MAGIC неверный, сбрасываю буфер");
            buffer.clear();
            break;
        }

        qsizetype fullSize = static_cast<qsizetype>(sizeof(Protocol::Header) + hdr.payloadSize);
        if (buffer.size() < fullSize) {
            break;
        }

        if (hdr.command == Protocol::CMD_SPI_WRITE || hdr.command == Protocol::CMD_SPI_READ) {
            if (hdr.flags == Protocol::FLAG_RESPONSE) {
                Protocol::SpiResponse resp;
                std::memcpy(&resp, buffer.constData(), sizeof(resp));

                logMessage(QString("Ответ: msgId=%1 status=%2 data=0x%3")
                               .arg(resp.messageId)
                               .arg(resp.status)
                               .arg(resp.data_responce, 8, 16, QChar('0')));

                int row = resp.messageId;
                if (row >= 0 && row < icTable->rowCount()) {
                    if (resp.status == 0) {
                        icTable->setItem(row, ColumnStatus, new QTableWidgetItem("OK"));
                        if (hdr.command == Protocol::CMD_SPI_READ) {
                            QString dataHex = QString("0x%1").arg(resp.data_responce, 8, 16, QChar('0'));
                            icTable->setItem(row, ColumnData, new QTableWidgetItem(dataHex));
                        }
                    } else {
                        icTable->setItem(row, ColumnStatus, new QTableWidgetItem(QString("ERR %1").arg(resp.status)));
                    }
                }
            } else if (hdr.flags == Protocol::FLAG_ERROR) {
                logMessage("Ошибка сервера (флаг ERROR)");
            }
        }

        buffer.remove(0, fullSize);
    }
}

void MainWindow::onAddRow()
{
    int row = icTable->rowCount();
    icTable->insertRow(row);

    QComboBox *idCombo = new QComboBox();
    idCombo->addItem("1");
    idCombo->addItem("2");
    idCombo->addItem("3");
    idCombo->setCurrentIndex(0);
    icTable->setCellWidget(row, ColumnId, idCombo);

    QComboBox *wrCombo = new QComboBox();
    wrCombo->addItem("W");
    wrCombo->addItem("R");
    wrCombo->setCurrentIndex(0);
    QObject::connect(wrCombo, &QComboBox::currentIndexChanged, this, [this, row](int) {
        updateRowDataEditable(row);
    });
    icTable->setCellWidget(row, ColumnWr, wrCombo);

    QTableWidgetItem *addrItem = new QTableWidgetItem();
    icTable->setItem(row, ColumnAddr, addrItem);

    QTableWidgetItem *dataItem = new QTableWidgetItem();
    icTable->setItem(row, ColumnData, dataItem);

    QTableWidgetItem *statusItem = new QTableWidgetItem();
    statusItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    icTable->setItem(row, ColumnStatus, statusItem);

    updateRowDataEditable(row);
    logMessage("Добавлена пустая строка #" + QString::number(row + 1));
}

void MainWindow::onDelRow()
{
    int row = icTable->currentRow();
    if (row >= 0) {
        icTable->removeRow(row);
        logMessage("Удалена строка #" + QString::number(row + 1));
    }
}

QByteArray MainWindow::createCommand(const CommandData &cmd)
{
    Protocol::SpiRequest req;
    req.magic = Protocol::MAGIC;
    req.command = cmd.command;
    req.messageId = cmd.messageId;
    req.flags = cmd.flags;
    req.payloadSize = sizeof(Protocol::SpiRequest) - sizeof(Protocol::Header);
    req.slave_id = cmd.slaveId;
    req.ic_addr = cmd.icAddr;
    req.data_request = cmd.dataRequest;

    return QByteArray(reinterpret_cast<const char*>(&req), sizeof(Protocol::SpiRequest));
}

void MainWindow::onSend()
{
    if (!isConnected) {
        QMessageBox::warning(this, "Ошибка", "Сначала подключись!");
        return;
    }

    for (int row = 0; row < icTable->rowCount(); row++) {
        QWidget *idWidget = icTable->cellWidget(row, ColumnId);
        if (idWidget == nullptr) {
            continue;
        }
        QComboBox *idCombo = qobject_cast<QComboBox*>(idWidget);
        if (idCombo == nullptr) {
            continue;
        }

        bool ok = false;
        uint8_t slaveId = static_cast<uint8_t>(idCombo->currentText().toUInt(&ok));
        if (!ok) {
            icTable->setItem(row, ColumnStatus, new QTableWidgetItem("BAD IC ID"));
            continue;
        }

        QWidget *wrWidget = icTable->cellWidget(row, ColumnWr);
        if (wrWidget == nullptr) {
            continue;
        }
        QComboBox *wrCombo = qobject_cast<QComboBox*>(wrWidget);
        if (wrCombo == nullptr) {
            continue;
        }

        uint8_t command = 0;
        if (wrCombo->currentText() == "W") {
            command = Protocol::CMD_SPI_WRITE;
        } else {
            command = Protocol::CMD_SPI_READ;
        }

        uint8_t flags = Protocol::FLAG_REQUEST;

        QTableWidgetItem *addrItem = icTable->item(row, ColumnAddr);
        if (addrItem == nullptr) {
            icTable->setItem(row, ColumnStatus, new QTableWidgetItem("BAD IC ADDR"));
            continue;
        }

        uint32_t addrVal = addrItem->text().trimmed().toUInt(&ok, 16);
        if (!ok) {
            icTable->setItem(row, ColumnStatus, new QTableWidgetItem("BAD IC ADDR"));
            continue;
        }
        uint8_t icAddr = static_cast<uint8_t>(addrVal & 0xFF);

        uint32_t dataVal = 0;
        if (wrCombo->currentText() == "W") {
            QTableWidgetItem *dataItem = icTable->item(row, ColumnData);
            if (dataItem == nullptr) {
                icTable->setItem(row, ColumnStatus, new QTableWidgetItem("BAD DATA WR"));
                continue;
            }

            dataVal = dataItem->text().trimmed().toUInt(&ok, 16);
            if (!ok) {
                icTable->setItem(row, ColumnStatus, new QTableWidgetItem("BAD DATA WR"));
                continue;
            }
        }

        CommandData cmd;
        cmd.command = command;
        cmd.messageId = static_cast<uint8_t>(row);
        cmd.flags = flags;
        cmd.slaveId = slaveId;
        cmd.icAddr = icAddr;
        cmd.dataRequest = dataVal;

        QByteArray packet = createCommand(cmd);
        socket->write(packet);
        icTable->setItem(row, ColumnStatus, new QTableWidgetItem("SENT"));
    }
}

void MainWindow::onSave()
{
    QString file = QFileDialog::getSaveFileName(this, "Сохранить", "", "JSON (*.json)");
    if (file.isEmpty()) {
        return;
    }

    QFile f(file);
    if (f.open(QFile::WriteOnly)) {
        QJsonArray array;
        for (int i = 0; i < icTable->rowCount(); i++) {
            QJsonObject obj;

            QComboBox *idCombo = qobject_cast<QComboBox*>(icTable->cellWidget(i, ColumnId));
            obj["id"] = idCombo ? idCombo->currentText() : "";

            QComboBox *wrCombo = qobject_cast<QComboBox*>(icTable->cellWidget(i, ColumnWr));
            obj["wr"] = wrCombo ? wrCombo->currentText() : "";

            QTableWidgetItem *addrItem = icTable->item(i, ColumnAddr);
            if (addrItem != nullptr) {
                obj["addr"] = addrItem->text();
            } else {
                obj["addr"] = "";
            }

            QTableWidgetItem *dataItem = icTable->item(i, ColumnData);
            if (dataItem != nullptr) {
                obj["data"] = dataItem->text();
            } else {
                obj["data"] = "";
            }

            array.append(obj);
        }

        QJsonDocument doc(array);
        f.write(doc.toJson());
        f.close();
        logMessage("Сохранено в " + file);
    }
}

void MainWindow::onLoad()
{
    QString file = QFileDialog::getOpenFileName(this, "Загрузить", "", "JSON (*.json)");
    if (file.isEmpty()) {
        return;
    }

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
            idCombo->addItem("1");
            idCombo->addItem("2");
            idCombo->addItem("3");
            idCombo->setCurrentText(obj["id"].toString());
            icTable->setCellWidget(row, ColumnId, idCombo);

            QComboBox *wrCombo = new QComboBox();
            wrCombo->addItem("W");
            wrCombo->addItem("R");
            wrCombo->setCurrentText(obj["wr"].toString());
            QObject::connect(wrCombo, &QComboBox::currentIndexChanged, this, [this, row](int) {
                updateRowDataEditable(row);
            });
            icTable->setCellWidget(row, ColumnWr, wrCombo);

            QTableWidgetItem *addrItem = new QTableWidgetItem(obj["addr"].toString());
            icTable->setItem(row, ColumnAddr, addrItem);

            QTableWidgetItem *dataItem = new QTableWidgetItem(obj["data"].toString());
            icTable->setItem(row, ColumnData, dataItem);

            QTableWidgetItem *statusItem = new QTableWidgetItem();
            statusItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            icTable->setItem(row, ColumnStatus, statusItem);

            updateRowDataEditable(row);
        }

        logMessage("Загружено " + QString::number(array.size()) + " строк");
        f.close();
    }
}
