#include "stm32f3xx_hal.h"
#include "aioc.h"
#include "led.h"
#include "ptt.h"
#include "usb.h"
#include <assert.h>
#include <stdio.h>

static void SystemClock_Config(void)
{
    HAL_StatusTypeDef status;

    /* Enable external oscillator and configure PLL: 8 MHz (HSE) / 1 * 9 = 72 MHz */
    RCC_OscInitTypeDef OscConfig = {
        .OscillatorType = RCC_OSCILLATORTYPE_HSE,
        .HSEState = RCC_HSE_ON,
        .HSEPredivValue = RCC_HSE_PREDIV_DIV1,
        .PLL = {
            .PLLState = RCC_PLL_ON,
            .PLLSource = RCC_CFGR_PLLSRC_HSE_PREDIV,
            .PLLMUL = RCC_PLL_MUL9
        }
    };

    status = HAL_RCC_OscConfig(&OscConfig);
    assert(status == HAL_OK);

    /* Set correct peripheral clocks. 72 MHz (PLL) / 1.5 = 48 MHz */
    RCC_PeriphCLKInitTypeDef PeriphClk = {
        .PeriphClockSelection = RCC_PERIPHCLK_USB,
        .USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5
    };

    status = HAL_RCCEx_PeriphCLKConfig(&PeriphClk);
    assert(status == HAL_OK);

    /* Set up divider for maximum speeds and switch clock */
    RCC_ClkInitTypeDef ClkConfig = {
        .ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
        .SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
        .AHBCLKDivider = RCC_SYSCLK_DIV1,
        .APB1CLKDivider = RCC_HCLK_DIV2,
        .APB2CLKDivider = RCC_HCLK_DIV1
    };

   status = HAL_RCC_ClockConfig(&ClkConfig, FLASH_LATENCY_2);
   assert(status == HAL_OK);

    NVIC_SetPriority(SysTick_IRQn, AIOC_IRQ_PRIO_SYSTICK);

    /* Enable MCO Pin to PLL/2 output */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GpioInit = {
        .Pin = GPIO_PIN_8,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_HIGH,
        .Alternate = GPIO_AF0_MCO
    };

    HAL_GPIO_Init(GPIOA, &GpioInit);
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK_DIV2, RCC_MCODIV_1);
}

int _write(int file, char *ptr, int len)
{
	for (uint32_t i=0; i<len; i++) {
		ITM_SendChar(*ptr++);
	}

	return len;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    __HAL_RCC_SYSCFG_CLK_ENABLE();

    GPIO_InitTypeDef GpioSWOInit = {
        .Pin = GPIO_PIN_3,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF0_TRACE
    };
    HAL_GPIO_Init(GPIOB, &GpioSWOInit);

    LED_Init();
    LED_MODE(0, LED_MODE_SLOWPULSE2X);
    LED_MODE(1, LED_MODE_SLOWPULSE2X);

    PTT_Init();

    USB_Init();

    uint32_t i = 0;
    while (1) {
        USB_Task();

        if ( (i++ & 0x7FFF) == 0) {
            usb_audio_fbstats_t fb;
            USB_AudioGetSpeakerFeedbackStats(&fb);

            usb_audio_bufstats_t buf;
            USB_AudioGetSpeakerBufferStats(&buf);

#if 0
            printf("buf: (%d/%d/%d) fb: (%06lX/%06lX/%06lX)\n",
                    buf.bufLevelMin, buf.bufLevelMax, buf.bufLevelAvg,
                    fb.feedbackMin, fb.feedbackMax, fb.feedbackAvg);
#endif
        }
    }

  return 0;
}

void NMI_Handler(void) {
}

void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

void MemManage_Handler(void) {
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) {
    }
}

void BusFault_Handler(void) {
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) {
    }
}

void UsageFault_Handler(void) {
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {
    }
}

void SVC_Handler(void) {
}

void DebugMon_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
    HAL_IncTick();
}

