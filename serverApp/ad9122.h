#ifndef AD9122_H
#define AD9122_H

namespace AD9122_IC {
    /*АДРЕСА РЕГИСТРОВ*/
    static constexpr uint8_t REG_COMM = 0x00;
    static constexpr uint8_t REG_POWER = 0x01;
    static constexpr uint8_t REG_FIFO_CONTROL = 0x17;
    static constexpr uint8_t REG_FIFO_STATUS = 0x18;
    static constexpr uint8_t REG_CHIP_ID = 0x1F;
    static constexpr uint8_t REG_REVISION = 0x7F;

    /*ОБЪЕДИНЕНИЕ РЕГИСТРОВ*/
    #pragma pack(push, 1)
    //0x18
    union FifoStat_u
    {
        uint8_t all_reg;
        struct {
            uint8_t reserved : 1;
            uint8_t fifo_soft_align_req : 1;
            uint8_t fifo_soft_align_acknowlege : 1;
            uint8_t reserved2 : 3;
            uint8_t fifo_warn2 : 1;
            uint8_t fifo_warn1 : 1;
        } bits;
    };

    //0x17
    union FifoControl_u
    {
        uint8_t all_reg;
        struct {
            uint8_t fifo_phase_offset : 3;
            uint8_t reserved : 5;
        } bits;
    };

    //0x00
    union Comm_u
    {
        uint8_t all_reg;
        struct {
            uint8_t reserved : 5;
            uint8_t reset : 1;
            uint8_t lsb_first : 1;
            uint8_t sdio : 1;
        } bits;
    };

    //0x01
    union PowerControl_u
    {
        uint8_t all_reg;
        struct {
            uint8_t reserved : 4;
            uint8_t power_down_auxiliary_adc : 1;
            uint8_t power_down_data_receiver : 1;
            uint8_t power_down_qdac : 1;
            uint8_t power_down_idac : 1;
        };
    };

    //0x1F
    union ChipID_u
    {
        uint8_t chip : 8;
    };

    //0x7F
    union Revision_u
    {
        uint8_t all_reg;
        struct {
            uint8_t reserved : 2;
            uint8_t revision : 4;
            uint8_t reserved2 : 2;
        } bits;
    };
    #pragma pack (pop)

    void processRead (uint8_t ic_addr, uint32_t result);
}

#endif // AD9122_H
