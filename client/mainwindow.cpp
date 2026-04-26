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

    ipLineEdit = new QLineEdit("127.0.0.1");
    qDebug() << "IP line edit text:" << ipLineEdit->text();
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

    icTable = new QTableWidget(0, COLUMN_COUNT);
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

    // ВКЛАДКА: ПРОВЕРКА УЗЛА СТРОБИРОВАНИЯ
    QWidget *strobeTab = new QWidget();
    tabWidget->addTab(strobeTab, "Проверка узла стробирования");

    QVBoxLayout *strobeLayout = new QVBoxLayout(strobeTab);

    // Частота дискретизации (фиксированная, без редактирования)
    QLabel *freqLabel = new QLabel("Частота дискретизации, МГц: 500");
    freqLabel->setStyleSheet("font-weight: bold;");
    strobeLayout->addWidget(freqLabel);


    // Период следования импульсов, мкс
    QLabel *periodLabel = new QLabel("Период следования импульсов, мкс:");
    strobeLayout->addWidget(periodLabel);

    periodSpinBox = new QSpinBox();
    periodSpinBox->setRange(1, 1000000);   // от 1 мкс до 1000 мс
    periodSpinBox->setSingleStep(1);       // шаг +1/-1
    periodSpinBox->setValue(100);          // по умолчанию 100 мкс
    periodSpinBox->setSuffix(" мкс");
    strobeLayout->addWidget(periodSpinBox);

    // Длительность импульса, мс
    QLabel *pulseLabel = new QLabel("Длительность импульса стробирования, мс:");
    strobeLayout->addWidget(pulseLabel);

    pulseSpinBox = new QSpinBox();
    pulseSpinBox->setRange(1, 10000);      // от 1 мс до 10 секунд
    pulseSpinBox->setSingleStep(1);        // шаг +1/-1
    pulseSpinBox->setValue(10);            // по умолчанию 10 мс
    pulseSpinBox->setSuffix(" мс");
    strobeLayout->addWidget(pulseSpinBox);

    // Кнопка для применения параметров
    applyStrobeButton = new QPushButton("Применить параметры стробирования");
    strobeLayout->addWidget(applyStrobeButton);

    // Пространство внизу
    strobeLayout->addStretch();

    tabWidget->addTab(new QWidget(), "Результаты");


}

void MainWindow::setupConnections()
{
    QObject::connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    QObject::connect(disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);

    QObject::connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    QObject::connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    QObject::connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    QObject::connect(socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        logMessage("Ошибка сокета: " + socket->errorString());
    });

    QObject::connect(addRowBtn, &QPushButton::clicked, this, &MainWindow::onAddRow);
    QObject::connect(delRowBtn, &QPushButton::clicked, this, &MainWindow::onDelRow);
    QObject::connect(sendBtn, &QPushButton::clicked, this, &MainWindow::onSend);
    QObject::connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSave);
    QObject::connect(loadBtn, &QPushButton::clicked, this, &MainWindow::onLoad);
    QObject::connect(applyStrobeButton, &QPushButton::clicked, this, &MainWindow::onApplyStrobe);
}

void MainWindow::updateRowDataEditable(int row)
{
    if (row < 0 || row >= icTable->rowCount()) {
        return;
    }

    QWidget *wrWidget = icTable->cellWidget(row, COLUMN_WR);
    if (wrWidget == nullptr) {
        return;
    }

    QComboBox *wrCombo = qobject_cast<QComboBox*>(wrWidget);
    if (wrCombo == nullptr) {
        return;
    }

    QTableWidgetItem *dataItem = icTable->item(row, COLUMN_DATA);
    if (!dataItem) return;


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


void MainWindow::onReadyRead()
{
    buffer.append(socket->readAll());

    while (true) {
        if (buffer.size() < static_cast<int>(Protocol::HEADER_SIZE)) {
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

        // 1. ЦАП‑тесты (CMD_SPI_WRITE / CMD_SPI_READ)
        if (hdr.command == Protocol::CMD_SPI_WRITE ||
            hdr.command == Protocol::CMD_SPI_READ)
        {
            if (hdr.flags == Protocol::FLAG_RESPONSE) {
                Protocol::SpiResponse resp;
                std::memcpy(&resp, buffer.constData(), sizeof(resp));

                const char* statusStr = Protocol::statusToString(resp.status);
                logMessage(QString("Ответ (SPI): msgId=%1 status=%2 data=0x%3")
                               .arg(resp.messageId)
                               .arg(statusStr)
                               .arg(resp.data_responce, 8, 16, QChar('0')));

                int row = resp.messageId;
                if (row >= 0 && row < icTable->rowCount()) {
                    icTable->setItem(row, COLUMN_STATUS, new QTableWidgetItem(statusStr));

                    if (hdr.command == Protocol::CMD_SPI_READ) {
                        QString dataHex = QString("0x%1").arg(resp.data_responce, 8, 16, QChar('0'));
                        icTable->setItem(row, COLUMN_DATA, new QTableWidgetItem(dataHex));
                    }
                }
            } else if (hdr.flags == Protocol::FLAG_ERROR) {
                logMessage("Ошибка сервера (флаг ERROR) [SPI]");
            }
        }

        // 2. СТРОБ‑команды (проверка узла стробирования)
        else if (hdr.command == Protocol::CMD_STROBE_PERIOD ||
                 hdr.command == Protocol::CMD_STROBE_PULSE)
        {
            if (hdr.flags == Protocol::FLAG_RESPONSE) {
                Protocol::StrobeResponse resp;
                std::memcpy(&resp, buffer.constData(), sizeof(resp));

                const char* statusStr = Protocol::statusToString(resp.status);
                logMessage(QString("Ответ (строб): cmd=%1, msgId=%2, status=%3, value=%4")
                               .arg(hdr.command, 2, 16, QChar('0'))
                               .arg(resp.messageId)
                               .arg(statusStr)
                               .arg(resp.value));

                // ------ ПРОСТО ВОТ ЭТА ЧАСТЬ ----- //
                if (resp.status == 0) {
                    QMessageBox::information(
                        this,
                        "Проверка узла стробирования",
                        "Параметры стробирования успешно применились.");
                } else {
                    QString msg = QString("Проверка не прошла:\n%1").arg(statusStr);
                    QMessageBox::warning(
                        this,
                        "Проверка узла стробирования",
                        msg);
                }
                // ------ ПРОСТО ВОТ ЭТА ЧАСТЬ ----- //
            } else if (hdr.flags == Protocol::FLAG_ERROR) {
                logMessage("Ошибка сервера (флаг ERROR) [STROBE]");
                QMessageBox::warning(
                    this,
                    "Проверка узла стробирования",
                    "Сервер вернул флаг ERROR при обработке строб‑команды.");
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
    icTable->setCellWidget(row, COLUMN_ID, idCombo);

    QComboBox *wrCombo = new QComboBox();
    wrCombo->addItem("W");
    wrCombo->addItem("R");
    wrCombo->setCurrentIndex(0);
    QObject::connect(wrCombo, &QComboBox::currentIndexChanged, this, [this, row](int) {
        updateRowDataEditable(row);
    });
    icTable->setCellWidget(row, COLUMN_WR, wrCombo);

    QTableWidgetItem *addrItem = new QTableWidgetItem();
    icTable->setItem(row, COLUMN_ADDR, addrItem);

    QTableWidgetItem *dataItem = new QTableWidgetItem();
    icTable->setItem(row, COLUMN_DATA, dataItem);

    QTableWidgetItem *statusItem = new QTableWidgetItem();
    statusItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    icTable->setItem(row, COLUMN_STATUS, statusItem);

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

    QByteArray packet(reinterpret_cast<const char*>(&req), sizeof(Protocol::SpiRequest));

    logMessage(QString("Отправляю пакет %1 байт: magic=0x%2, cmd=0x%3, msgId=%4, flags=%5, "
                       "payloadSize=%6, slaveId=%7, icAddr=%8, data=0x%9")
                   .arg(packet.size())
                   .arg(req.magic, 8, 16, QChar('0'))
                   .arg(req.command, 2, 16, QChar('0'))
                   .arg(req.messageId)
                   .arg(req.flags, 2, 16, QChar('0'))
                   .arg(req.payloadSize)
                   .arg(req.slave_id)
                   .arg(req.ic_addr, 2, 16, QChar('0'))
                   .arg(req.data_request, 8, 16, QChar('0')));

    return packet;
}

void MainWindow::onSend()
{
    if (!isConnected) {
        QMessageBox::warning(this, "Ошибка", "Сначала подключись!");
        return;
    }

    for (int row = 0; row < icTable->rowCount(); row++) {
        QWidget *idWidget = icTable->cellWidget(row, COLUMN_ID);
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
            icTable->setItem(row, COLUMN_STATUS, new QTableWidgetItem("BAD IC ID"));
            continue;
        }

        QWidget *wrWidget = icTable->cellWidget(row, COLUMN_WR);
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

        QTableWidgetItem *addrItem = icTable->item(row, COLUMN_ADDR);
        if (addrItem == nullptr) {
            icTable->setItem(row, COLUMN_STATUS, new QTableWidgetItem("BAD IC ADDR"));
            continue;
        }

        uint32_t addrVal = addrItem->text().trimmed().toUInt(&ok, 16);
        if (!ok) {
            icTable->setItem(row, COLUMN_STATUS, new QTableWidgetItem("BAD IC ADDR"));
            continue;
        }
        uint8_t icAddr = static_cast<uint8_t>(addrVal & 0xFF);

        uint32_t dataVal = 0;
        if (wrCombo->currentText() == "W") {
            QTableWidgetItem *dataItem = icTable->item(row, COLUMN_DATA);
            if (dataItem == nullptr) {
                icTable->setItem(row, COLUMN_STATUS, new QTableWidgetItem("BAD DATA WR"));
                continue;
            }

            dataVal = dataItem->text().trimmed().toUInt(&ok, 16);
            if (!ok) {
                icTable->setItem(row, COLUMN_STATUS, new QTableWidgetItem("BAD DATA WR"));
                continue;
            }
        }

        CommandData cmd{
            command,
            static_cast<uint8_t>(row),
            flags,
            slaveId,
            icAddr,
            dataVal
        };

        QByteArray packet = createCommand(cmd);
        socket->write(packet);
        icTable->setItem(row, COLUMN_STATUS, new QTableWidgetItem("SENT"));
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

            QComboBox *idCombo = qobject_cast<QComboBox*>(icTable->cellWidget(i, COLUMN_ID));
            obj["id"] = idCombo ? idCombo->currentText() : "";

            QComboBox *wrCombo = qobject_cast<QComboBox*>(icTable->cellWidget(i, COLUMN_WR));
            obj["wr"] = wrCombo ? wrCombo->currentText() : "";

            QTableWidgetItem *addrItem = icTable->item(i, COLUMN_ADDR);
            if (addrItem != nullptr) {
                obj["addr"] = addrItem->text();
            } else {
                obj["addr"] = "";
            }

            QTableWidgetItem *dataItem = icTable->item(i, COLUMN_DATA);
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
            icTable->setCellWidget(row, COLUMN_ID, idCombo);

            QComboBox *wrCombo = new QComboBox();
            wrCombo->addItems({"W", "R"});
            wrCombo->setCurrentText(obj["wr"].toString());
            QObject::connect(wrCombo, &QComboBox::currentIndexChanged, this, [this, row](int) {
                updateRowDataEditable(row);
            });
            icTable->setCellWidget(row, COLUMN_WR, wrCombo);

            QTableWidgetItem *addrItem = new QTableWidgetItem(obj["addr"].toString());
            icTable->setItem(row, COLUMN_ADDR, addrItem);

            QTableWidgetItem *dataItem = new QTableWidgetItem(obj["data"].toString());
            icTable->setItem(row, COLUMN_DATA, dataItem);

            QTableWidgetItem *statusItem = new QTableWidgetItem();
            statusItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            icTable->setItem(row, COLUMN_STATUS, statusItem);

            updateRowDataEditable(row);
        }

        logMessage("Загружено " + QString::number(array.size()) + " строк");
        f.close();
    }
}

void MainWindow::onApplyStrobe()
{
    int periodUs = periodSpinBox->value();   // период следования импульсов, мкс
    int pulseMs  = pulseSpinBox->value();    // длительность импульса, мс

    if (!isConnected) {
        QMessageBox::warning(this, "Ошибка", "Сначала подключись к серверу!");
        return;
    }

    logMessage(QString("Проверка узла стробирования: период = %1 мкс, длительность = %2 мс")
                   .arg(periodUs).arg(pulseMs));

    // 1) Команда: период строба
    Protocol::StrobeRequest req1;
    req1.magic = Protocol::MAGIC;
    req1.command = Protocol::CMD_STROBE_PERIOD;
    req1.messageId = 0;
    req1.flags = Protocol::FLAG_REQUEST;
    req1.payloadSize = sizeof(Protocol::StrobeRequest) - Protocol::HEADER_SIZE;
    req1.value = static_cast<uint32_t>(periodUs);

    QByteArray packet1(reinterpret_cast<const char*>(&req1), sizeof(Protocol::StrobeRequest));
    socket->write(packet1);

    logMessage(QString("Отправлен CMD_STROBE_PERIOD: %1 мкс").arg(req1.value));

    // 2) Команда: длительность импульса
    Protocol::StrobeRequest req2;
    req2.magic = Protocol::MAGIC;
    req2.command = Protocol::CMD_STROBE_PULSE;
    req2.messageId = 1;
    req2.flags = Protocol::FLAG_REQUEST;
    req2.payloadSize = sizeof(Protocol::StrobeRequest) - Protocol::HEADER_SIZE;
    req2.value = static_cast<uint32_t>(pulseMs);

    QByteArray packet2(reinterpret_cast<const char*>(&req2), sizeof(Protocol::StrobeRequest));
    socket->write(packet2);

    logMessage(QString("Отправлен CMD_STROBE_PULSE: %1 мс").arg(req2.value));
}

