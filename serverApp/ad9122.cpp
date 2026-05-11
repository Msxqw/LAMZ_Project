#include "ad9122.h"
#include  <QDebug>

bool AD9122_IC::isValidRegister(uint8_t ic_addr) {
    switch (ic_addr) {
    case AD9122_IC::REG_COMM:
    case AD9122_IC::REG_POWER:
    case AD9122_IC::REG_FIFO_CONTROL:
    case AD9122_IC::REG_FIFO_STATUS:
    case AD9122_IC::REG_CHIP_ID:
    case AD9122_IC::REG_REVISION:
        return true;
    default:
        return false;
    }
}

void AD9122_IC::processReadValue(uint8_t ic_addr, uint32_t result) {
    uint8_t value = static_cast<uint8_t>(result);

    switch (ic_addr) {
    case AD9122_IC::REG_COMM: {
        Comm_u reg;
        reg.all_reg = value;
        qDebug() << "[AD9122] COMM: SDIO bidir =" << reg.bits.sdio
                 << "LSB first =" << reg.bits.lsb_first
                 << "Reset =" << reg.bits.reset;
        break;
    }

    case AD9122_IC::REG_POWER: {
        PowerControl_u reg;
        reg.all_reg = value;
        qDebug() << "[AD9122] POWER: I DAC =" << (reg.bits.power_down_idac ? "OFF" : "ON")
                 << "Q DAC =" << (reg.bits.power_down_qdac ? "OFF" : "ON")
                 << "Data Rx =" << (reg.bits.power_down_data_receiver ? "OFF" : "ON")
                 << "Aux ADC =" << (reg.bits.power_down_auxiliary_adc ? "OFF" : "ON");
        break;
    }

    case AD9122_IC::REG_FIFO_CONTROL: {
        FifoControl_u reg;
        reg.all_reg = value;
        qDebug() << "[AD9122] FIFO Control: Phase Offset =" << reg.bits.fifo_phase_offset;
        break;
    }

    case AD9122_IC::REG_FIFO_STATUS: {
        FifoStat_u reg;
        reg.all_reg = value;

        if (reg.bits.fifo_warn1) {
            qDebug() << "[AD9122] WARNING: Flag FIFO Warning 1";
        }
        if (reg.bits.fifo_warn2) {
            qDebug() << "[AD9122] WARNING: FFlag FIFO Warning 2";
        }
        break;
    }

    case AD9122_IC::REG_CHIP_ID: {
        ChipID_u reg;
        reg.chip = value;
        qDebug() << "[AD9122] ChipId: " << value;
        break;
    }

    case AD9122_IC::REG_REVISION: {
        Revision_u reg;
        reg.all_reg = value;
        qDebug() << "[AD9122] Revision: " << reg.bits.revision;
        break;
    }

    default:
        break;
    }
}
