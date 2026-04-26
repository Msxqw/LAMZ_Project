#include "ad9122.h"
#include  <QDebug>

void AD9122_IC::processReadValue(uint8_t ic_addr, uint32_t result) {
    uint8_t value = static_cast<uint8_t>(result);

    switch (ic_addr) {
    case AD9122_IC::REG_COMM: {
        break;
    }

    case AD9122_IC::REG_POWER: {
        break;
    }

    case AD9122_IC::REG_FIFO_CONTROL: {
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
