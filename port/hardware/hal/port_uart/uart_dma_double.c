/**
 * @file      uart_dma_double.c
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 UART DMA双缓冲实现 (仿HAL)
 * @version   version 1.0.0
 * @date      2025-01-20
 * 
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 * 
 */
#include "uart_dma_double.h"

extern void HAL_UART_DMA_M0_RxCpltCallback(UART_HandleTypeDef* huart);
extern void HAL_UART_DMA_M1_RxCpltCallback(UART_HandleTypeDef* huart);

/**
 * @brief (局部静态函数) 串口-DMA双缓冲 接收完成函数 (仿HAL)
 *
 * @param huart HAL串口句柄
 */
static void UART_EndRxTransfer(UART_HandleTypeDef* huart)
{
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  ATOMIC_CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

  /* In case of reception waiting for IDLE event, disable also the IDLE IE interrupt source */
  if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
  {
    ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_IDLEIE);
  }

  /* At end of Rx process, restore huart->RxState to Ready */
  huart->RxState       = HAL_UART_STATE_READY;
  huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
}

/**
 * @brief (局部静态函数) 串口-DMA双缓冲 发送完成函数 (仿HAL)
 *
 * @param huart HAL串口句柄
 */
static void UART_EndTxTransfer(UART_HandleTypeDef* huart)
{
  /* Disable TXEIE and TCIE interrupts */
  ATOMIC_CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));

  /* At end of Tx process, restore huart->gState to Ready */
  huart->gState = HAL_UART_STATE_READY;
}

/**
 * @brief (局部静态函数) DMA-DMA双缓冲 缓存区0满回调函数 (仿HAL)
 *
 * @param huart HALDMA句柄
 */
static void UART_DMAReceiveCplt_M0(DMA_HandleTypeDef* hdma)
{
  UART_HandleTypeDef* huart = (UART_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;
  /* DMA Normal mode*/
  if (hdma->Init.Mode != DMA_CIRCULAR)
  {
    huart->RxXferCount = 0U;

    /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
    ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);
    ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

    /* Disable the DMA transfer for the receiver request by setting the DMAR bit
       in the UART CR3 register */
    ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* At end of Rx process, restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;

    /* If Reception till IDLE event has been selected, Disable IDLE Interrupt */
    if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
    {
      ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_IDLEIE);
    }
  }

  /* Check current reception Mode :
     If Reception till IDLE event has been selected : use Rx Event callback */
  if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
  {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /*Call registered Rx Event callback*/
    // huart->RxEventCallback(huart, huart->RxXferSize);
#else
    /*Call legacy weak Rx Event callback*/
    // HAL_UARTEx_RxEventCallback(huart, huart->RxXferSize);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
  }
  else
  {
    /* In other cases : use Rx Complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /*Call registered Rx complete callback*/
    // huart->RxCpltCallback(huart);
#else
    /*Call legacy weak Rx complete callback*/
    // HAL_UART_RxCpltCallback(huart);
    HAL_UART_DMA_M0_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
  }
}

/**
 * @brief (局部静态函数) DMA-DMA双缓冲 缓存区1满回调函数 (仿HAL)
 *
 * @param huart HALDMA句柄
 */
static void UART_DMAReceiveCplt_M1(DMA_HandleTypeDef* hdma)
{
  UART_HandleTypeDef* huart = (UART_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;
  /* DMA Normal mode*/
  if (hdma->Init.Mode != DMA_CIRCULAR)
  {
    huart->RxXferCount = 0U;

    /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
    ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);
    ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

    /* Disable the DMA transfer for the receiver request by setting the DMAR bit
       in the UART CR3 register */
    ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* At end of Rx process, restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;

    /* If Reception till IDLE event has been selected, Disable IDLE Interrupt */
    if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
    {
      ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_IDLEIE);
    }
  }

  /* Check current reception Mode :
     If Reception till IDLE event has been selected : use Rx Event callback */
  if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
  {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /*Call registered Rx Event callback*/
    // huart->RxEventCallback(huart, huart->RxXferSize);
#else
    /*Call legacy weak Rx Event callback*/
    // HAL_UARTEx_RxEventCallback(huart, huart->RxXferSize);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
  }
  else
  {
    /* In other cases : use Rx Complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /*Call registered Rx complete callback*/
    // huart->RxCpltCallback(huart);
#else
    /*Call legacy weak Rx complete callback*/
    // HAL_UART_RxCpltCallback(huart);
    HAL_UART_DMA_M1_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
  }
}

/**
 * @brief (局部静态函数) DMA-DMA双缓冲 错误回调函数 (仿HAL)
 *
 * @param huart HALDMA句柄
 */
static void UART_DMAError(DMA_HandleTypeDef* hdma)
{
  uint32_t            dmarequest = 0x00U;
  UART_HandleTypeDef* huart      = (UART_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

  /* Stop UART DMA Tx request if ongoing */
  dmarequest                     = HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT);
  if ((huart->gState == HAL_UART_STATE_BUSY_TX) && dmarequest)
  {
    huart->TxXferCount = 0x00U;
    UART_EndTxTransfer(huart);
  }

  /* Stop UART DMA Rx request if ongoing */
  dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR);
  if ((huart->RxState == HAL_UART_STATE_BUSY_RX) && dmarequest)
  {
    huart->RxXferCount = 0x00U;
    UART_EndRxTransfer(huart);
  }

  huart->ErrorCode |= HAL_UART_ERROR_DMA;
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /*Call registered error callback*/
  huart->ErrorCallback(huart);
#else
  /*Call legacy weak error callback*/
  HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
 * @brief (局部静态函数) DMA-DMA双缓冲 开始DMA双缓冲 (仿HAL)
 *
 * @param huart   HALDMA句柄
 * @param pData0  缓存区0
 * @param pData1  缓存区1
 * @param Size    缓存区大小
 * @return HAL_StatusTypeDef HAL_OK-成功
 */
static HAL_StatusTypeDef UART_Start_Receive_DMA_Double(UART_HandleTypeDef* huart, uint8_t* pData0, uint8_t* pData1, uint16_t Size)
{
  uint32_t* tmp0;
  uint32_t* tmp1;

  huart->pRxBuffPtr                   = pData0;
  huart->RxXferSize                   = Size;

  huart->ErrorCode                    = HAL_UART_ERROR_NONE;
  huart->RxState                      = HAL_UART_STATE_BUSY_RX;

  /* Set the UART DMA transfer complete callback */
  huart->hdmarx->XferCpltCallback     = UART_DMAReceiveCplt_M0;
  huart->hdmarx->XferM1CpltCallback   = UART_DMAReceiveCplt_M1;

  /* Set the UART DMA Half transfer complete callback */
  huart->hdmarx->XferHalfCpltCallback = NULL;

  /* Set the DMA error callback */
  huart->hdmarx->XferErrorCallback    = UART_DMAError;

  /* Set the DMA abort callback */
  huart->hdmarx->XferAbortCallback    = NULL;

  /* Enable the DMA stream */
  tmp0                                = (uint32_t*)&pData0;
  tmp1                                = (uint32_t*)&pData1;
  HAL_DMAEx_MultiBufferStart_IT(huart->hdmarx, (uint32_t)&huart->Instance->DR, *(uint32_t*)tmp0, *(uint32_t*)tmp1, Size);
  /* Clear the Overrun flag just before enabling the DMA Rx request: can be mandatory for the second transfer */
  __HAL_UART_CLEAR_OREFLAG(huart);

  /* Process Unlocked */
  __HAL_UNLOCK(huart);

  if (huart->Init.Parity != UART_PARITY_NONE)
  {
    /* Enable the UART Parity Error Interrupt */
    ATOMIC_SET_BIT(huart->Instance->CR1, USART_CR1_PEIE);
  }

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  ATOMIC_SET_BIT(huart->Instance->CR3, USART_CR3_EIE);

  /* Enable the DMA transfer for the receiver request by setting the DMAR bit
  in the UART CR3 register */
  ATOMIC_SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);

  return HAL_OK;
}

/**
 * @brief (局部静态函数) DMA-DMA双缓冲 DMA双缓冲设置与开始 (仿HAL)
 *
 * @param huart   HALDMA句柄
 * @param pData0  缓存区0
 * @param pData1  缓存区1
 * @param Size    缓存区大小
 * @return HAL_StatusTypeDef HAL_OK-成功
 */
HAL_StatusTypeDef HAL_UART_Receive_DMA_Double(UART_HandleTypeDef* huart, uint8_t* pData0, uint8_t* pData1, uint16_t Size)
{
  /* Check that a Rx process is not already ongoing */
  if (huart->RxState == HAL_UART_STATE_READY)
  {
    if ((pData0 == NULL) || (pData1 == NULL) || (Size == 0U))
    {
      return HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(huart);

    /* Set Reception type to Standard reception */
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

    return (UART_Start_Receive_DMA_Double(huart, pData0, pData1, Size));
  }
  else
  {
    return HAL_BUSY;
  }
}
