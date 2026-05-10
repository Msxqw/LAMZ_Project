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

    // Проверка флеш-памяти
    eepromTab = new QWidget();
    tabWidget->addTab(eepromTab, "Проверка EEPROM");

    QVBoxLayout *eepromMainLayout = new QVBoxLayout(eepromTab);

    QHBoxLayout *eepromTopLayout = new QHBoxLayout();

    QLabel *eepromAddressLabel = new QLabel("Адрес:");
    eepromAddressSpinBox = new QSpinBox();
    eepromAddressSpinBox->setRange(0, 65535);
    eepromAddressSpinBox->setValue(0);

    QLabel *eepromSizeLabel = new QLabel("Размер блока:");
    eepromSizeComboBox = new QComboBox();
    eepromSizeComboBox->addItem("128 байт");
    eepromSizeComboBox->addItem("256 байт");
    eepromSizeComboBox->addItem("512 байт");

    eepromTopLayout->addWidget(eepromAddressLabel);
    eepromTopLayout->addWidget(eepromAddressSpinBox);
    eepromTopLayout->addWidget(eepromSizeLabel);
    eepromTopLayout->addWidget(eepromSizeComboBox);

    QHBoxLayout *eepromButtonsLayout = new QHBoxLayout();

    eepromWriteButton = new QPushButton("Записать");
    eepromReadButton = new QPushButton("Прочитать и сравнить");

    eepromButtonsLayout->addWidget(eepromWriteButton);
    eepromButtonsLayout->addWidget(eepromReadButton);

    eepromStatusLabel = new QLabel("Статус: ожидание");

    eepromMainLayout->addLayout(eepromTopLayout);
    eepromMainLayout->addLayout(eepromButtonsLayout);
    eepromMainLayout->addWidget(eepromStatusLabel);
    eepromMainLayout->addStretch();

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

    syncTab = new QWidget();
    tabWidget->addTab(syncTab, "Проверка узла синхронизации");

    QVBoxLayout *syncLayout = new QVBoxLayout(syncTab);

    QLabel *syncTitleLabel = new QLabel("Источник опорной частоты:");
    syncTitleLabel->setStyleSheet("font-weight: bold;");
    syncLayout->addWidget(syncTitleLabel);

    internalSourceRadio = new QRadioButton("Внутренний генератор");
    externalSourceRadio = new QRadioButton("Внешний источник");

    internalSourceRadio->setChecked(true);

    syncSourceGroup = new QButtonGroup(this);
    syncSourceGroup->addButton(internalSourceRadio);
    syncSourceGroup->addButton(externalSourceRadio);

    syncLayout->addWidget(internalSourceRadio);
    syncLayout->addWidget(externalSourceRadio);

    QLabel *syncFreqLabel = new QLabel("Выходная частота, МГц:");
    syncLayout->addWidget(syncFreqLabel);

    syncFreqSpinBox = new QSpinBox();
    syncFreqSpinBox->setRange(1, 1000);
    syncFreqSpinBox->setValue(100);
    syncFreqSpinBox->setSuffix(" МГц");
    syncLayout->addWidget(syncFreqSpinBox);

    syncHintLabel = new QLabel(
        "При выборе внутреннего генератора задаётся выходная частота.\n"
        "При выборе внешнего источника частота поступает извне."
        );
    syncHintLabel->setStyleSheet("color: #a6adc8;");
    syncHintLabel->setWordWrap(true);
    syncLayout->addWidget(syncHintLabel);

    applySyncButton = new QPushButton("Применить параметры синхронизации");
    syncLayout->addWidget(applySyncButton);

    syncLayout->addStretch();

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

    QObject::connect(applySyncButton, &QPushButton::clicked, this, &MainWindow::onApplySync);
    QObject::connect(internalSourceRadio, &QRadioButton::toggled, this, &MainWindow::onSyncSourceChanged);
    QObject::connect(externalSourceRadio, &QRadioButton::toggled, this, &MainWindow::onSyncSourceChanged);
    QObject::connect(eepromWriteButton, &QPushButton::clicked, this, &MainWindow::onEepromWriteClicked);
    QObject::connect(eepromReadButton, &QPushButton::clicked, this, &MainWindow::onEepromReadClicked);
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
                if (resp.status == Protocol::STATUS_OK) {
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

        // 3. КОМАНДЫ СИНХРОНИЗАЦИИ
        else if (hdr.command == Protocol::CMD_SYNC_SOURCE ||
                 hdr.command == Protocol::CMD_SYNC_FREQUENCY)
        {
            if (hdr.flags == Protocol::FLAG_RESPONSE) {
                Protocol::SyncResponse resp;
                std::memcpy(&resp, buffer.constData(), sizeof(resp));

                const char* statusStr = Protocol::statusToString(resp.status);
                QString sourceStr = (resp.source == Protocol::SYNC_SOURCE_INTERNAL)
                                        ? "внутренний генератор"
                                        : "внешний источник";

                logMessage(QString("Ответ (синхронизация): cmd=%1, msgId=%2, status=%3, source=%4, freq=%5 МГц")
                               .arg(hdr.command, 2, 16, QChar('0'))
                               .arg(resp.messageId)
                               .arg(statusStr)
                               .arg(sourceStr)
                               .arg(resp.frequency));

                if (resp.status == Protocol::STATUS_OK) {
                    if (hdr.command == Protocol::CMD_SYNC_SOURCE) {
                        QMessageBox::information(
                            this,
                            "Проверка узла синхронизации",
                            QString("Источник опорной частоты успешно установлен: %1.")
                                .arg(sourceStr));
                    } else if (hdr.command == Protocol::CMD_SYNC_FREQUENCY) {
                        QMessageBox::information(
                            this,
                            "Проверка узла синхронизации",
                            QString("Выходная частота успешно установлена: %1 МГц.")
                                .arg(resp.frequency));
                    }
                } else {
                    QMessageBox::warning(
                        this,
                        "Проверка узла синхронизации",
                        QString("Сервер вернул ошибку: %1").arg(statusStr));
                }
            } else if (hdr.flags == Protocol::FLAG_ERROR) {
                logMessage("Ошибка сервера (флаг ERROR) [SYNC]");
                QMessageBox::warning(
                    this,
                    "Проверка узла синхронизации",
                    "Сервер вернул флаг ERROR при обработке команды синхронизации.");
            }
        }
        // 4. КОМАНДЫ EEPROM
        else if (hdr.command == Protocol::CMD_EEPROM_READ_128 ||
                 hdr.command == Protocol::CMD_EEPROM_READ_256 ||
                 hdr.command == Protocol::CMD_EEPROM_READ_512)
        {
            if (hdr.flags == Protocol::FLAG_RESPONSE) {
                int dataSize = 0;
                uint8_t responseStatus = Protocol::STATUS_OK;
                quint32 responseAddress = 0;

                eepromReadData.clear();

                if (hdr.command == Protocol::CMD_EEPROM_READ_128) {
                    Protocol::EepromRead128Response resp;
                    std::memcpy(&resp, buffer.constData(), sizeof(resp));

                    responseStatus = resp.status;
                    responseAddress = resp.address;
                    dataSize = 128;

                    logMessage(QString("EEPROM: получен ответ чтения 128 байт, status=%1, address=%2")
                                   .arg(Protocol::statusToString(resp.status))
                                   .arg(resp.address));

                    if (resp.status == Protocol::STATUS_OK) {
                        for (int i = 0; i < 128; i++) {
                            eepromReadData.append(static_cast<char>(resp.data[i]));
                        }
                    }
                }
                else if (hdr.command == Protocol::CMD_EEPROM_READ_256) {
                    Protocol::EepromRead256Response resp;
                    std::memcpy(&resp, buffer.constData(), sizeof(resp));

                    responseStatus = resp.status;
                    responseAddress = resp.address;
                    dataSize = 256;

                    logMessage(QString("EEPROM: получен ответ чтения 256 байт, status=%1, address=%2")
                                   .arg(Protocol::statusToString(resp.status))
                                   .arg(resp.address));

                    if (resp.status == Protocol::STATUS_OK) {
                        for (int i = 0; i < 256; i++) {
                            eepromReadData.append(static_cast<char>(resp.data[i]));
                        }
                    }
                }
                else if (hdr.command == Protocol::CMD_EEPROM_READ_512) {
                    Protocol::EepromRead512Response resp;
                    std::memcpy(&resp, buffer.constData(), sizeof(resp));

                    responseStatus = resp.status;
                    responseAddress = resp.address;
                    dataSize = 512;

                    logMessage(QString("EEPROM: получен ответ чтения 512 байт, status=%1, address=%2")
                                   .arg(Protocol::statusToString(resp.status))
                                   .arg(resp.address));

                    if (resp.status == Protocol::STATUS_OK) {
                        for (int i = 0; i < 512; i++) {
                            eepromReadData.append(static_cast<char>(resp.data[i]));
                        }
                    }
                }

                if (responseStatus != Protocol::STATUS_OK) {
                    QString statusStr = Protocol::statusToString(responseStatus);

                    logMessage(QString("EEPROM: ошибка чтения, status=%1, address=%2")
                                   .arg(statusStr)
                                   .arg(responseAddress));

                    eepromStatusLabel->setText(QString("Статус: ошибка чтения (%1)").arg(statusStr));

                    QMessageBox::warning(
                        this,
                        "Проверка EEPROM",
                        QString("Сервер вернул ошибку чтения: %1").arg(statusStr));
                }
                else if (eepromWriteData.isEmpty()) {
                    logMessage("EEPROM: нет записанных данных для сравнения");
                    eepromStatusLabel->setText("Статус: нет данных для сравнения");

                    QMessageBox::warning(
                        this,
                        "Проверка EEPROM",
                        "Нет записанных данных для сравнения. Сначала выполните запись.");
                }
                else if (eepromReadData.size() != eepromWriteData.size()) {
                    logMessage("EEPROM: размеры данных не совпадают");
                    eepromStatusLabel->setText("Статус: ошибка размера данных");

                    QMessageBox::warning(
                        this,
                        "Проверка EEPROM",
                        "Размер прочитанных данных не совпадает с размером записанных.");
                }
                else {
                    bool match = true;

                    for (int i = 0; i < eepromReadData.size(); i++) {
                        if (eepromReadData[i] != eepromWriteData[i]) {
                            match = false;
                            break;
                        }
                    }

                    if (match) {
                        logMessage("EEPROM: данные совпадают ✅");
                        eepromStatusLabel->setText("Статус: данные совпадают ✅");

                        QMessageBox::information(
                            this,
                            "Проверка EEPROM",
                            QString("Проверка успешна!\nПрочитано %1 байт, все данные совпадают.")
                                .arg(dataSize));
                    } else {
                        logMessage("EEPROM: данные НЕ совпадают ❌");
                        eepromStatusLabel->setText("Статус: данные НЕ совпадают ❌");

                        QMessageBox::warning(
                            this,
                            "Проверка EEPROM",
                            "Ошибка! Прочитанные данные не совпадают с записанными.");
                    }
                }
            } else if (hdr.flags == Protocol::FLAG_ERROR) {
                logMessage("EEPROM: сервер вернул ERROR при чтении");
                eepromStatusLabel->setText("Статус: ошибка сервера");

                QMessageBox::warning(
                    this,
                    "Проверка EEPROM",
                    "Сервер вернул флаг ERROR при обработке команды чтения EEPROM.");
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

void MainWindow::onSyncSourceChanged()
{
    bool internalSelected = internalSourceRadio->isChecked();
    syncFreqSpinBox->setEnabled(internalSelected);

    if (internalSelected) {
        syncHintLabel->setText(
            "Выбран внутренний генератор.\n"
            "Задайте выходную частоту в МГц."
            );
    } else {
        syncHintLabel->setText(
            "Выбран внешний источник.\n"
            "Опорная частота подаётся на устройство извне."
            );
    }
}

void MainWindow::onApplySync()
{
    if (!isConnected) {
        QMessageBox::warning(this, "Ошибка", "Сначала подключись к серверу!");
        return;
    }

    uint8_t source = internalSourceRadio->isChecked()
                         ? Protocol::SYNC_SOURCE_INTERNAL
                         : Protocol::SYNC_SOURCE_EXTERNAL;

    uint32_t freqMHz = static_cast<uint32_t>(syncFreqSpinBox->value());

    logMessage(QString("Проверка узла синхронизации: источник = %1, частота = %2 МГц")
                   .arg(source == Protocol::SYNC_SOURCE_INTERNAL ? "внутренний" : "внешний")
                   .arg(freqMHz));

    // 1) Команда: источник опорной частоты
    Protocol::SyncRequest reqSource;
    reqSource.magic = Protocol::MAGIC;
    reqSource.command = Protocol::CMD_SYNC_SOURCE;
    reqSource.messageId = 0;
    reqSource.flags = Protocol::FLAG_REQUEST;
    reqSource.payloadSize = sizeof(Protocol::SyncRequest) - Protocol::HEADER_SIZE;
    reqSource.source = source;
    reqSource.frequency = 0;

    QByteArray packetSource(reinterpret_cast<const char*>(&reqSource), sizeof(Protocol::SyncRequest));
    socket->write(packetSource);

    logMessage(QString("Отправлен CMD_SYNC_SOURCE: %1")
                   .arg(source == Protocol::SYNC_SOURCE_INTERNAL ? "внутренний генератор"
                                                                 : "внешний источник"));

    // 2) Если внутренний генератор — отдельно отправляем частоту
    if (source == Protocol::SYNC_SOURCE_INTERNAL) {
        Protocol::SyncRequest reqFreq;
        reqFreq.magic = Protocol::MAGIC;
        reqFreq.command = Protocol::CMD_SYNC_FREQUENCY;
        reqFreq.messageId = 1;
        reqFreq.flags = Protocol::FLAG_REQUEST;
        reqFreq.payloadSize = sizeof(Protocol::SyncRequest) - Protocol::HEADER_SIZE;
        reqFreq.source = source;
        reqFreq.frequency = freqMHz;

        QByteArray packetFreq(reinterpret_cast<const char*>(&reqFreq), sizeof(Protocol::SyncRequest));
        socket->write(packetFreq);

        logMessage(QString("Отправлен CMD_SYNC_FREQUENCY: %1 МГц").arg(freqMHz));
    }
}

void MainWindow::onEepromWriteClicked()
{
    if (!isConnected || !socket) {
        logMessage("EEPROM: нет подключения к серверу");
        eepromStatusLabel->setText("Статус: нет подключения");
        return;
    }

    eepromCurrentAddress = static_cast<quint32>(eepromAddressSpinBox->value());

    QString sizeText = eepromSizeComboBox->currentText();
    if (sizeText == "128 байт") {
        eepromCurrentSize = 128;
    } else if (sizeText == "256 байт") {
        eepromCurrentSize = 256;
    } else {
        eepromCurrentSize = 512;
    }

    eepromWriteData.clear();

    for (int i = 0; i < eepromCurrentSize; i++) {
        char value = static_cast<char>(QRandomGenerator::global()->bounded(256));
        eepromWriteData.append(value);
    }

    if (eepromCurrentSize == 128) {
        Protocol::EepromWrite128Request req;
        req.magic = Protocol::MAGIC;
        req.command = Protocol::CMD_EEPROM_WRITE_128;
        req.messageId = 0;
        req.flags = Protocol::FLAG_REQUEST;
        req.payloadSize = sizeof(Protocol::EepromWrite128Request) - Protocol::HEADER_SIZE;
        req.address = eepromCurrentAddress;

        for (int i = 0; i < 128; i++) {
            req.data[i] = static_cast<uint8_t>(eepromWriteData[i]);
        }

        socket->write(QByteArray(reinterpret_cast<const char*>(&req), sizeof(req)));
    }
    else if (eepromCurrentSize == 256) {
        Protocol::EepromWrite256Request req;
        req.magic = Protocol::MAGIC;
        req.command = Protocol::CMD_EEPROM_WRITE_256;
        req.messageId = 0;
        req.flags = Protocol::FLAG_REQUEST;
        req.payloadSize = sizeof(Protocol::EepromWrite256Request) - Protocol::HEADER_SIZE;
        req.address = eepromCurrentAddress;

        for (int i = 0; i < 256; i++) {
            req.data[i] = static_cast<uint8_t>(eepromWriteData[i]);
        }

        socket->write(QByteArray(reinterpret_cast<const char*>(&req), sizeof(req)));
    }
    else if (eepromCurrentSize == 512) {
        Protocol::EepromWrite512Request req;
        req.magic = Protocol::MAGIC;
        req.command = Protocol::CMD_EEPROM_WRITE_512;
        req.messageId = 0;
        req.flags = Protocol::FLAG_REQUEST;
        req.payloadSize = sizeof(Protocol::EepromWrite512Request) - Protocol::HEADER_SIZE;
        req.address = eepromCurrentAddress;

        for (int i = 0; i < 512; i++) {
            req.data[i] = static_cast<uint8_t>(eepromWriteData[i]);
        }

        socket->write(QByteArray(reinterpret_cast<const char*>(&req), sizeof(req)));
    }

    logMessage(QString("EEPROM: отправлена запись %1 байт по адресу %2")
                   .arg(eepromCurrentSize)
                   .arg(eepromCurrentAddress));

    eepromStatusLabel->setText("Статус: запись отправлена");
}

void MainWindow::onEepromReadClicked()
{
    if (!isConnected || !socket) {
        logMessage("EEPROM: нет подключения к серверу");
        eepromStatusLabel->setText("Статус: нет подключения");
        return;
    }

    eepromCurrentAddress = static_cast<quint32>(eepromAddressSpinBox->value());

    QString sizeText = eepromSizeComboBox->currentText();
    if (sizeText == "128 байт") {
        eepromCurrentSize = 128;
    } else if (sizeText == "256 байт") {
        eepromCurrentSize = 256;
    } else {
        eepromCurrentSize = 512;
    }

    if (eepromCurrentSize == 128) {
        Protocol::EepromRead128Request req;
        req.magic = Protocol::MAGIC;
        req.command = Protocol::CMD_EEPROM_READ_128;
        req.messageId = 0;
        req.flags = Protocol::FLAG_REQUEST;
        req.payloadSize = sizeof(Protocol::EepromRead128Request) - Protocol::HEADER_SIZE;
        req.address = eepromCurrentAddress;

        socket->write(QByteArray(reinterpret_cast<const char*>(&req), sizeof(req)));
    }
    else if (eepromCurrentSize == 256) {
        Protocol::EepromRead256Request req;
        req.magic = Protocol::MAGIC;
        req.command = Protocol::CMD_EEPROM_READ_256;
        req.messageId = 0;
        req.flags = Protocol::FLAG_REQUEST;
        req.payloadSize = sizeof(Protocol::EepromRead256Request) - Protocol::HEADER_SIZE;
        req.address = eepromCurrentAddress;

        socket->write(QByteArray(reinterpret_cast<const char*>(&req), sizeof(req)));
    }
    else if (eepromCurrentSize == 512) {
        Protocol::EepromRead512Request req;
        req.magic = Protocol::MAGIC;
        req.command = Protocol::CMD_EEPROM_READ_512;
        req.messageId = 0;
        req.flags = Protocol::FLAG_REQUEST;
        req.payloadSize = sizeof(Protocol::EepromRead512Request) - Protocol::HEADER_SIZE;
        req.address = eepromCurrentAddress;

        socket->write(QByteArray(reinterpret_cast<const char*>(&req), sizeof(req)));
    }

    logMessage(QString("EEPROM: отправлен запрос чтения %1 байт по адресу %2")
                   .arg(eepromCurrentSize)
                   .arg(eepromCurrentAddress));

    eepromStatusLabel->setText("Статус: запрос чтения отправлен");
}
