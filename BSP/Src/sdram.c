#include "sdram.h"

/*
 * SDRAM Initialization
 */
uint8_t BSP_SDRAM_Init(void) {

    /* Set SDRAM status to an error */
    sdramState = HAL_SDRAM_STATE_ERROR;

    /*Set Instance of SDRAM_HandleTypeDef to point to the SDRAM banks 5 and 6 (FMC_SDRAM_DEVICE) */
    sdramHandle.Instance = FMC_SDRAM_DEVICE;

    /* Set timing parameters for SDRAM in FMC_SDRAM_TimingTypeDef */
    //TODO: Need to determine values, probably from datasheet for SDRAM, need to decide clock frequency first?
    Timing.LoadToActiveDelay    = 0;
    Timing.ExitSelfRefreshDelay = 0;
    Timing.SelfRefreshTime      = 0;
    Timing.RowCycleDelay        = 0;
    Timing.WriteRecoveryTime    = 2;
    Timing.RPDelay              = 0;
    Timing.RCDDelay             = 0;

    //TODO: Need to determine values that are uncertain
    sdramHandle.Init.SDBank             = FMC_SDRAM_BANK1;
    sdramHandle.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
    sdramHandle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;
    sdramHandle.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32; //not sure, could be 8,16,32
    sdramHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    sdramHandle.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_2; //this and below is unsure, might be based on timing
    sdramHandle.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    sdramHandle.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
    sdramHandle.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
    sdramHandle.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

    /* Takes given parameters, initializes */
    BSP_SDRAM_MspInit();
    if(HAL_SDRAM_Init(&sdramHandle, &Timing) != HAL_OK) {
        sdramState = HAL_SDRAM_STATE_ERROR;
    } else {
        sdramState = HAL_SDRAM_STATE_READY;
    }

     /* Calls SDRAM initialization sequence */
    BSP_SDRAM_Init_Sequence();
    return sdramState;
}


/*
 * Initializes SDRAM MSP
 */
void BSP_SDRAM_MspInit(void) {
    GPIO_InitTypeDef gpio_init_structure;

    /* Enable FMC clock */
    __HAL_RCC_FMC_CLK_ENABLE();

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();


    /* Common configurations */
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    //TODO: FIGURE OUT ALTERNATE VARIABLE
   // gpio_init_structure.Alternate =

   /* GPIOG configuration */
   gpio_init_structure.Pin = SDRAM_SDCLK_PIN | SDRAM_SDNCAS_PIN | SDRAM_BA0_PIN | SDRAM_BA1_PIN | SDRAM_A10_PIN |
           SDRAM_A11_PIN | SDRAM_A12_PIN;
   HAL_GPIO_Init(GPIOG, &gpio_init_structure);

   /* GPIOH configuration */
   gpio_init_structure.Pin = SDRAM_SDCKE_PIN | SDRAM_SDNE_PIN | SDRAM_SDNWE_PIN;
   HAL_GPIO_Init(GPIOH, &gpio_init_structure);

    /* GPIOF configuration */
    gpio_init_structure.Pin = SDRAM_SDNRAS_PIN | SDRAM_A0_PIN | SDRAM_A1_PIN | SDRAM_A2_PIN | SDRAM_A3_PIN |
            SDRAM_A4_PIN | SDRAM_A5_PIN | SDRAM_A6_PIN | SDRAM_A7_PIN | SDRAM_A8_PIN | SDRAM_A9_PIN;
    HAL_GPIO_Init(GPIOF, &gpio_init_structure);

    /* GPIOE configuration */
    gpio_init_structure.Pin = SDRAM_LDQM_PIN | SDRAM_UDQM_PIN | SDRAM_D4_PIN | SDRAM_D5_PIN | SDRAM_D6_PIN |
            SDRAM_D7_PIN | SDRAM_D8_PIN | SDRAM_D9_PIN | SDRAM_D10_PIN | SDRAM_D11_PIN | SDRAM_D12_PIN;
    HAL_GPIO_Init(GPIOE, &gpio_init_structure);

    /* GPIOD configuration */
    gpio_init_structure.Pin = SDRAM_D0_PIN | SDRAM_D1_PIN | SDRAM_D2_PIN | SDRAM_D3_PIN | SDRAM_D13_PIN |
            SDRAM_D14_PIN | SDRAM_D15_PIN;
    HAL_GPIO_Init(GPIOD, &gpio_init_structure);
}


/*
 * SDRAM Initialization Sequence (See page 42 of datasheet)
 */
void BSP_SDRAM_Init_Sequence(void) {

    /* Apply power simultaneously to Vdd and Vddq */

    /* Assert and hold CKE at LVTTL logic LOW. Provide stable CLOCK signal */
    Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1; //TODO: check these values for commands
    Command.ModeRegisterDefinition = 0;
    //TODO: Find appropriate timeout for sending commands
    HAL_SDRAM_SendCommand(&sdramHandle, &Command, (uint32_t)0xFFFF);

    /* Wait at least 100μs prior to issuing any command */
    HAL_Delay(1); //takes ms

    /* Perform a PRECHARGE ALL command */
    Command.CommandMode            = FMC_SDRAM_CMD_PALL;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(&sdramHandle, &Command, (uint32_t)0xFFFF);

    /* Issue an AUTO REFRESH command */
    Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 8;
    Command.ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(&sdramHandle, &Command, (uint32_t)0xFFFF);

    /* Issue an AUTO REFRESH command */
    //TODO: See if necessary

    /* SDRAM is now ready for mode register programming */
    Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0; //TODO: Figure out how to program the register
    HAL_SDRAM_SendCommand(&sdramHandle, &Command, (uint32_t)0xFFFF);

    //TODO: See if setting the refresh counter is appropriate here

}


