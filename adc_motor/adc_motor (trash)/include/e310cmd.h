// ­®¬Ґа  ¤®бвгЇ­ле Ї®«м§®ў вҐ«мбЄЁе § Їа®б®ў ¤«п USB (vendor request)

#define V_PUT_ARRAY_E310       0x1
#define V_GET_ARRAY_E310       0x2
#define V_START_ADC_E310       0x4
#define V_STOP_ADC_E310        0x5
#define V_CALL_APP_E310        0xF

#define             CONTROL_BITS_ADDR_REG               (0x0)
#define             NUMBER_OF_INCREMENTS_ADDR_REG       (0x1<<12)
#define             LO_DELTA_FREQUENCY_ADDR_REG      (0x2<<12)
#define             HI_DELTA_FREQUENCY_ADDR_REG     (0x3<<12)
#define             CYCLE_INCREMENT_INTERVAL_ADDR_REG   ((0x4 >> 1)<<13)
#define             CLOCK_INCREMENT_INTERVAL_ADDR_REG   ((0x6 >> 1)<<13)
#define             LO_START_FREQUENCY_ADDR_REG      ((0xC)<<12)
#define             HI_START_FREQUENCY_ADDR_REG     ((0xD)<<12)

#define INTERNAL_DDS_MASTER_CLOCK  50000.0
#define ADC_INPUT_RANGE            3.3
#define MAX_FREQ_DDS_CODE          (0x1 << 24)


//0000 1000 1101 0011 default 0x08D3
#define DDS_CTRL_PRESET               0x08D3
/*
//#define DDS_CR_SYNCSEL              0x0008
#define DDS_CR_SYNCSEL_EACH_INC        0x0000
#define DDS_CR_SYNCSEL_END_SCAN        0x0008

//#define DDS_CR_SYNCOUTEN            0x0004
#define DDS_CR_SYNCOUT_EN            0x0004
#define DDS_CR_SYNCOUT_DIS           0x0000

//#define DDS_CR_INT_OR_EXT_INCR      0x0020
#define DDS_CR_INT_INCR              0x0000
#define DDS_CR_EXT_INCR              0x0020

#define DDS_CR_MSBOUT_EN             0x0100
#define DDS_CR_MSBOUT_DIS            0x0000

#define DDS_CR_SIG_SIN               0x0000
#define DDS_CR_SIG_TRI               0x0200
*/

// тип приращений и скорость
//IncType
#define MCLOCK_INCREMENT            CLOCK_INCREMENT_INTERVAL_ADDR_REG
#define OUTWAV_INCREMENT            CYCLE_INCREMENT_INTERVAL_ADDR_REG

//умножитель
//IncMult
#define  INC_INTERVAL_MULT_1   (0x0<<11)
#define  INC_INTERVAL_MULT_5   (0x1<<11)
#define  INC_INTERVAL_MULT_100 (0x2<<11)
#define  INC_INTERVAL_MULT_500 (0x3<<11)

#define NO_CYCLIC_SCAN 0x00
#define SAWTOOTH_SCAN  0x01
#define TRIANGLE_SCAN  0x02

// CtrlReg DDS Gain defs
#define GG_P10dB                0x30
#define GG_P06dB                0x20
#define GG_P04dB                0x70
#define GG_P03dB                0x10
#define GG_P00dB                0x00
#define GG_M02dB                0xB0
#define GG_M03dB                0x50
#define GG_M06dB                0xA0
#define GG_M09dB                0x90
#define GG_M12dB                0x80
#define GG_M14dB                0xF0
#define GG_M18dB                0xE0
#define GG_M21dB                0xD0
#define GG_M24dB                0xC0

// CtrlReg DDS 10Ohm out offset source
#define BIT_10OHM_OFFSET 3
#define INT_10OHM_OFFSET 0x00
#define EXT_10OHM_OFFSET 0x08

// CtrlReg DDS MasterClock source
#define BIT_MCLK_LINE    2
#define INT_MCLK_LINE    0x00
#define EXT_MCLK_LINE    0x04

// CtrlReg DDS Interrupt source
#define BIT_INTR_LINE    1
#define INT_IRQ_LINE    0x00
#define EXT_IRQ_LINE    0x02

// CtrlReg DDS Ctrl source
#define BIT_CTRL_LINE       0
#define INT_CTRL_LINE       0
#define EXT_CTRL_LINE       1

// DDS_CtrlReg bits
#define BIT_MSBOUTEN         8 //0 1

#define BIT_SIN_OR_TRI       9 //0 1
#define TRI_WAVE             0 //0 1
#define SIN_WAVE             1 //0 1

#define BIT_INT_OR_EXT_INCR  5
#define AUTO_INCREMENT       0
#define CTRL_INCREMENT       1

#define BIT_SYNCOUTEN        2

#define BIT_SYNCSEL          3
#define SYNC_EACH_INC        0   
#define SYNC_EOS             1   //endofscan

            //
#define            BOOT_LOADER_START_ADDRESS_E310         (0x00000000UL)    // условный адрес режима 'Загрузчика' (BootLoader)
#define            BOOT_LOADER_DESCRIPTOR_ADDRESS_E310    (0x00102E00UL)
#define            MODULE_DESCRIPTOR_ADDRESS_E310         (0x00103000UL)
#define            USER_FLASH_ADDRESS_E310                (0x00103100UL)
#define            FIRMWARE_DESCRIPTOR_ADDRESS_E310       (0x00103300UL)
#define            FIRMWARE_START_ADDRESS_E310            (0x00103500UL)
            //
#define            MODULE_PARS_ADDRESS_E310               (0x00201400UL)
#define            FM_SAMPLE_ADDRESS_E310                 (0x00201480UL)
#define            TTL_PARS_ADDRESS_E310                  (0x002014A0UL)
#define            ADC_PARS_ADDRESS_E310                  (0x002014B0UL)
#define            ADC_DATA_ADDRESS_E310                  (0x002014B8UL)

