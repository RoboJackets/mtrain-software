#include "bsp.h"



QSPI_HandleTypeDef QSPIHandle;

static uint8_t QSPI_ResetMemory (QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_EnterFourBytesAddress(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi, uint32_t Timeout);
static uint8_t QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi);

static uint8_t QSPI_Set4KSector();


uint8_t BSP_QSPI_Init(void)
{
    QSPIHandle.Instance = QUADSPI;

    if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK) {
        return QSPI_ERROR;
    }

    // TODO
    /* QSPI freq = SYSCLK /(1 + ClockPrescaler) */
    QSPIHandle.Init.ClockPrescaler     = 1;                                 /* 216 MHz/(1+1) = 108 Mhz */
    QSPIHandle.Init.FifoThreshold      = 4;                                 // TODO: correct?
    QSPIHandle.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;    // TODO: correct?
    QSPIHandle.Init.FlashSize          = POSITION_VAL(FLASH_SIZE) - 1;
    QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_6_CYCLE;         /* Min 50ns */
    QSPIHandle.Init.ClockMode          = QSPI_CLOCK_MODE_0;                 // TODO: probably correct
    QSPIHandle.Init.FlashID            = QSPI_FLASH_ID_1;                   // TODO: correct?
    QSPIHandle.Init.DualFlash          = QSPI_DUALFLASH_ENABLE;

    if (HAL_QSPI_Init(&QSPIHandle) != HAL_OK) {
        return QSPI_ERROR;
    }

    if (QSPI_ResetMemory(&QSPIHandle) != QSPI_OK) {
        return QSPI_NOT_SUPPORTED;
    }

    if (QSPI_EnterFourBytesAddress(&QSPIHandle) != QSPI_OK) {
        return QSPI_NOT_SUPPORTED;
    }

    // if (QSPI_Set4KSector() != QSPI_OK) {
    //     return QSPI_NOT_SUPPORTED;
    // }

    // TODO
    return QSPI_OK;
}

uint8_t BSP_QSPI_DeInit(void)
{
    QSPIHandle.Instance = QUADSPI;

    if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK) {
        return QSPI_ERROR;
    }

    return QSPI_OK;
}

uint8_t BSP_QSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = FLASH_CMD_4FAST_READ;
  s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.Address           = ReadAddr;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = DUMMY_CYCLES_READ_QUAD;
  s_command.NbData            = Size*2; // TODO: remove hack
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Set S# timing for Read command */
  MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_3_CYCLE);

  /* GPIO_InitTypeDef  GPIO_InitStruct; */
  /* GPIO_InitStruct.Pin = GPIO_PIN_11; */
  /* GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; */
  /* GPIO_InitStruct.Pull = GPIO_NOPULL; */
  /* GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; */

  /* HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); */
  /* HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, (GPIO_PinState)0); */

  /* Reception of the data */
  if (HAL_QSPI_Receive(&QSPIHandle, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, (GPIO_PinState)1); */

  /* Restore S# timing for nonRead commands */
  MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_6_CYCLE);

  return QSPI_OK;
}

uint8_t BSP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;
  uint32_t end_addr, current_size, current_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_size = FLASH_PAGE_SIZE - (WriteAddr % FLASH_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > Size)
  {
    current_size = Size;
  }

  /* Initialize the address variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr;

  /* Initialize the program command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = FLASH_CMD_PP;
  s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Perform the write page by page */
  do
  {
    s_command.Address = current_addr;
    s_command.NbData  = current_size * 2; // TODO: remove hack

    /* Enable write operations */
    if (QSPI_WriteEnable(&QSPIHandle) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure the command */
    if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* GPIO_InitTypeDef  GPIO_InitStruct; */
    /* GPIO_InitStruct.Pin = GPIO_PIN_11; */
    /* GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; */
    /* GPIO_InitStruct.Pull = GPIO_NOPULL; */
    /* GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; */

    /* HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); */
    /* HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, (GPIO_PinState)0); */

    /* Transmission of the data */
    if (HAL_QSPI_Transmit(&QSPIHandle, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, (GPIO_PinState)1); */


    /* Configure automatic polling mode to wait for end of program */
    if (QSPI_AutoPollingMemReady(&QSPIHandle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    pData += current_size;
    current_size = ((current_addr + FLASH_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : FLASH_PAGE_SIZE;
  } while (current_addr < end_addr);

  return QSPI_OK;
}

uint8_t BSP_QSPI_Erase_Sector(uint32_t BlockAddress)
{
    QSPI_CommandTypeDef s_command;

    /* Initialize the erase command */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
    s_command.Address           = BlockAddress;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (BlockAddress < FLASH_4K_SECTOR_MAX) {
        s_command.Instruction = FLASH_CMD_4P4E;
    } else {
        s_command.Instruction = FLASH_CMD_4SE;
    }

    /* Enable write operations */
    if (QSPI_WriteEnable(&QSPIHandle) != QSPI_OK) {
        return QSPI_ERROR;
    }

    /* Send the command */
    if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    /* Configure automatic polling mode to wait for end of erase */
    if (QSPI_AutoPollingMemReady(&QSPIHandle, SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK) {
        return QSPI_ERROR;
    }

    return QSPI_OK;
}

uint8_t BSP_QSPI_Erase_Chip(void)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the erase command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = FLASH_CMD_BE;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable(&QSPIHandle) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(&QSPIHandle, BULK_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_print_regs(void)
{
    QSPI_CommandTypeDef s_command;

    /* Initialize the read command */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = FLASH_CMD_RDAR;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = DUMMY_CYCLES_READ_QUAD;
    s_command.NbData            = 1;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    s_command.Address = 0x00800000;
    if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_3_CYCLE);

    uint8_t data = 0;
    if (HAL_QSPI_Receive(&QSPIHandle, &data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }
    printf("SR1NV: %x\r\n", data);

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_6_CYCLE);


    s_command.Address = 0x00800001;
    if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_3_CYCLE);

    data = 0xFF;
    if (HAL_QSPI_Receive(&QSPIHandle, &data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }
    printf("SR2NV: %x\r\n", data);

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_6_CYCLE);



    s_command.Address = FLASH_ADDR_CR1NV;
    if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_3_CYCLE);

    data = 0xFF;
    if (HAL_QSPI_Receive(&QSPIHandle, &data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }
    printf("CR1NV: %x\r\n", data);

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_6_CYCLE);


    s_command.Address = FLASH_ADDR_CR2NV;
    if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_3_CYCLE);

    data = 0xFF;
    if (HAL_QSPI_Receive(&QSPIHandle, &data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }
    printf("CR2NV: %x\r\n", data);

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_6_CYCLE);


    s_command.Address = 0x00800004;
    if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_3_CYCLE);

    data = 0xFF;
    if (HAL_QSPI_Receive(&QSPIHandle, &data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }
    printf("CR3NV: %x\r\n", data);

    MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_6_CYCLE);

    return QSPI_OK;
}




static uint8_t QSPI_ResetMemory(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef s_command;

    /* Initialize the reset enable command */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = FLASH_CMD_RSTEN;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Enable memory reset */
    if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK){
        return QSPI_ERROR;
    }

    /* Reset memory */
    s_command.Instruction = FLASH_CMD_RESET;
    if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    if (QSPI_AutoPollingMemReady(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK) {
        return QSPI_ERROR;
    }

    return QSPI_OK;
}

static uint8_t QSPI_EnterFourBytesAddress(QSPI_HandleTypeDef *hqspi)
{
    if (QSPI_WriteEnable(hqspi) != QSPI_OK) {
        return QSPI_ERROR;
    }

    QSPI_CommandTypeDef s_command;

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = FLASH_CMD_4BAM;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    if (QSPI_AutoPollingMemReady(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK) {
        return QSPI_ERROR;
    }

    return QSPI_OK;
}

/* Wait for memory to be ready */
static uint8_t QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi, uint32_t Timeout)
{
    QSPI_CommandTypeDef     s_command;
    QSPI_AutoPollingTypeDef s_config;

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = FLASH_CMD_RDSR1;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    s_config.Match           = 0;
    s_config.Mask            = FLASH_SR1_WIP;
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;
    s_config.StatusBytesSize = 1;
    s_config.Interval        = 0x10;
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, Timeout) != HAL_OK) {
        return QSPI_ERROR;
    }

    return QSPI_OK;
}

static uint8_t QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef     s_command;
    QSPI_AutoPollingTypeDef s_config;

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = FLASH_CMD_WREN;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    s_config.Match           = FLASH_SR1_WEL;
    s_config.Mask            = FLASH_SR1_WEL;
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;
    s_config.StatusBytesSize = 1;
    s_config.Interval        = 0x10;
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    s_command.Instruction    = FLASH_CMD_RDSR1;
    s_command.DataMode       = QSPI_DATA_1_LINE;

    if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    return QSPI_OK;
}


static uint8_t QSPI_Set4KSector()
{
    QSPI_CommandTypeDef s_command;

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = FLASH_CMD_WRAR;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    s_command.Address = 0x00000000;
    s_command.NbData  = 1;

    /* Enable write operations */
    if (QSPI_WriteEnable(&QSPIHandle) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    BSP_print_regs();

    /* Configure the command */
    if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    uint8_t data = 0x0;
    if (HAL_QSPI_Transmit(&QSPIHandle, &data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure automatic polling mode to wait for end of program */
    if (QSPI_AutoPollingMemReady(&QSPIHandle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

  return QSPI_OK;
}



// static uint8_t QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
// {
//   QSPI_CommandTypeDef s_command;
//   uint8_t reg;

//   s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
//   s_command.Instruction       = READ_VOL_CFG_REG_CMD;
//   s_command.AddressMode       = QSPI_ADDRESS_NONE;
//   s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
//   s_command.DataMode          = QSPI_DATA_1_LINE;
//   s_command.DummyCycles       = 0;
//   s_command.NbData            = 1;
//   s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
//   s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
//   s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

//   if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
//   {
//     return QSPI_ERROR;
//   }

//   if (HAL_QSPI_Receive(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
//   {
//     return QSPI_ERROR;
//   }

//   if (QSPI_WriteEnable(hqspi) != QSPI_OK)
//   {
//     return QSPI_ERROR;
//   }

//   s_command.Instruction = WRITE_VOL_CFG_REG_CMD;
//   MODIFY_REG(reg, VCR_NB_DUMMY, (DUMMY_CYCLES_READ_QUAD << POSITION_VAL(VCR_NB_DUMMY)));

//   if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
//   {
//     return QSPI_ERROR;
//   }

//   if (HAL_QSPI_Transmit(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
//   {
//     return QSPI_ERROR;
//   }

//   return QSPI_OK;
// }




void HAL_QSPI_MspInit (QSPI_HandleTypeDef *hqspi)
{
    GPIO_InitTypeDef gpio_init_structure;

    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();

    gpio_init_structure.Pin       = QSPI_CS_PIN;
    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull      = GPIO_PULLUP;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_CS_PORT, &gpio_init_structure);

    gpio_init_structure.Pin       = QSPI_CLK_PIN;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_CLK_PORT, &gpio_init_structure);

    gpio_init_structure.Pin       = QSPI_D0_PIN;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_D0_PORT, &gpio_init_structure);

    gpio_init_structure.Pin       = QSPI_D1_PIN;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_D1_PORT, &gpio_init_structure);

    gpio_init_structure.Pin       = QSPI_D2_PIN;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_D2_PORT, &gpio_init_structure);

    gpio_init_structure.Pin       = QSPI_D3_PIN;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_D3_PORT, &gpio_init_structure);

    HAL_NVIC_SetPriority(QUADSPI_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi)
{
    HAL_NVIC_DisableIRQ(QUADSPI_IRQn);

    HAL_GPIO_DeInit(QSPI_CS_PORT, QSPI_CS_PIN);
    HAL_GPIO_DeInit(QSPI_CLK_PORT, QSPI_CLK_PIN);
    HAL_GPIO_DeInit(QSPI_D0_PORT, QSPI_D0_PIN);
    HAL_GPIO_DeInit(QSPI_D1_PORT, QSPI_D1_PIN);
    HAL_GPIO_DeInit(QSPI_D2_PORT, QSPI_D2_PIN);
    HAL_GPIO_DeInit(QSPI_D3_PORT, QSPI_D3_PIN);

    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();

    __HAL_RCC_QSPI_CLK_DISABLE();
}
