/* Janez Puhan (23.6.2017) */

/* Insert FreeRTOS kernel includes here */
#include <FreeRTOS.h>
#include <task.h>

/* Insert ASF includes here */

/* LED defines */
#define LED1_msk PIO_PC23
#define LED2_msk PIO_PC22
#define LED3_msk PIO_PC21
#define LED4_msk PIO_PC29
#define LED_ALL_msk (LED1_msk | LED2_msk | LED3_msk | LED4_msk)
/* Button defines */
#define BTN1_msk PIO_PC28
#define BTN2_msk PIO_PC26
#define BTN3_msk PIO_PC25
#define BTN4_msk PIO_PC24
#define BTN_ALL_msk (BTN1_msk | BTN2_msk | BTN3_msk | BTN4_msk)

enum led_e
{
    LED1,
    LED2,
    LED3,
    LED4
};

#ifdef __cplusplus
extern "C"
{
#endif

    uint8_t ucHeap[configTOTAL_HEAP_SIZE]
        __attribute__((aligned(configTOTAL_HEAP_SIZE)))
        __attribute__((section(".heap")));

    /* Hardware setup */
    static void prvSetupHardware(void)
    {
        /* ASF function to setup clocking */
        sysclk_reinit(OSC_MAINCK_XTAL, PMC_MCKR_PRES_CLK_16, PMC_MCKR_CSS_PLLA_CLK, 14, 1);

        /* Ensure all priority bits are assigned as preemption priority bits */
        NVIC_SetPriorityGrouping(0);

        /* Atmel library function to setup for the evaluation kit being used */
        board_init();

        /* Insert hardware initialization code here */
        // buttons
        pio_configure(PIOC, PIO_INPUT, BTN_ALL_msk, PIO_PULLUP | PIO_DEBOUNCE);
        // led
        pio_configure(PIOC, PIO_OUTPUT_0, LED_ALL_msk, 0);
    }

    int led_pin[] = {PIO_PC23, PIO_PC22, PIO_PC21, PIO_PC29};
    uint8_t button_states[4];
    TaskHandle_t xHnd1, xHnd2, xHnd3, xHnd4;
    TaskHandle_t tskHandles[] = {xHnd1, xHnd2, xHnd3, xHnd4};

    void setLedStates(enum led_e ledNum)
    {
        for (int i = 0; i < 4; i++)
        {
            if (i == ledNum)
            {
                pio_set(PIOC, led_pin[i]);
            }
            else
            {
                pio_clear(PIOC, led_pin[i]);
            }
        }
    }

    uint8_t getKeyPositions(void)
    {
        return (uint8_t)(pio_get(PIOC, PIO_INPUT, BTN1_msk) << 0 |
                         pio_get(PIOC, PIO_INPUT, BTN2_msk) << 1 |
                         pio_get(PIOC, PIO_INPUT, BTN3_msk) << 2 |
                         pio_get(PIOC, PIO_INPUT, BTN4_msk) << 3);
    }

    void configureTasks(uint8_t btnStates)
    {
        for (int i = 0; i < 4; i++)
        {
            if (btnStates & (0x01 << i))
            {
                vTaskSuspend(tskHandles[i]);
            }
            else
            {
                vTaskResume(tskHandles[i]);
            }
        }
    }

    void task1(void *)
    {
        while (1)
        {
            setLedStates(LED1);
            uint8_t keyPositions = getKeyPositions();
            configureTasks(keyPositions);
        }
    }

    void task2(void *)
    {
        while (1)
        {
            setLedStates(LED2);
            uint8_t keyPositions = getKeyPositions();
            configureTasks(keyPositions);
        }
    }

    void task3(void *)
    {
        while (1)
        {
            setLedStates(LED3);
            uint8_t keyPositions = getKeyPositions();
            configureTasks(keyPositions);
        }
    }

    void task4(void *)
    {
        while (1)
        {
            setLedStates(LED4);
            uint8_t keyPositions = getKeyPositions();
            configureTasks(keyPositions);
        }
    }

    void vApplicationIdleHook(void)
    {
        /* Turn all LEDs off. */
        pio_clear(PIOC, LED_ALL_msk);
        /* Get key positions */
        uint8_t keyPositions = getKeyPositions();
        /* Suspend tasks belonging to pressed keys, resume others. */
        configureTasks(keyPositions);
    }

    int main(void)
    {
        /* Perform any hardware setup/initialization necessary */
        prvSetupHardware();

        /* Insert program code here */
        xTaskCreate(task1, "Task1", 150, NULL, 1, &xHnd1);
        xTaskCreate(task2, "Task2", 150, NULL, 1, &xHnd2);
        xTaskCreate(task3, "Task3", 150, NULL, 0, &xHnd3);
        xTaskCreate(task4, "Task4", 150, NULL, 0, &xHnd4);

        vTaskStartScheduler();
        /* Should never be reached, main() function should never end */
        for (;;)
            ;
        return 0;
    }

#ifdef __cplusplus
}
#endif
