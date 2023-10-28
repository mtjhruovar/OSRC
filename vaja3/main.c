/* Insert FreeRTOS kernel includes here */
#include <FreeRTOS.h>
#include <pio.h>
#include <task.h>

/* Insert ASF includes here */

#ifdef __cplusplus
extern "C"
{
#endif

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

#define PREDEFINED_NUM_OF_INTERATIONS (uint32_t)(0x1FFF)

    uint8_t ucHeap[configTOTAL_HEAP_SIZE] __attribute__((aligned(configTOTAL_HEAP_SIZE))) __attribute__((section(".heap")));
    uint8_t btns(void);
    void createTaskIfBtnPressed(void);
    void tsk1(void *par);
    void tsk2(void *par);
    void tsk3(void *par);
    void tsk4(void *par);
    void vApplicationIdleHook(void);

    /* Task handles. */
    void (*pTsk[])(void *) = {tsk1, tsk2, tsk3, tsk4};
    TaskHandle_t tskHandle[4];
    /* Previous idle task iteration BTN states. */
    uint8_t prevBtnStates = 0x0F;
    /* Task specific predifined number of interactions */
    uint32_t tskRepeats[] = {PREDEFINED_NUM_OF_INTERATIONS * 1,
                             PREDEFINED_NUM_OF_INTERATIONS * 4,
                             PREDEFINED_NUM_OF_INTERATIONS * 3,
                             PREDEFINED_NUM_OF_INTERATIONS * 2};

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
        /* Configure all LED pins as output */
        pio_configure(PIOC, PIO_OUTPUT_0, LED_ALL_msk, 0);
        pio_configure(PIOB, PIO_OUTPUT_0, PIO_PB27, 0);
        /* Configure all BTN pins as input */
        pio_configure(PIOC, PIO_INPUT, BTN_ALL_msk, PIO_PULLUP | PIO_DEBOUNCE);
    }

    int main(void)
    {
        /* Perform any hardware setup/initialization necessary */
        prvSetupHardware();

        /* Turn LED on PB27 off. */
        pio_clear(PIOB, PIO_PB27);

        /* Start scheduler. */
        vTaskStartScheduler();

        /* RR algoritem:
         *   The RR scheduling algorithm assigns one time slice per task in circular manner.
         */

        /* Some error occurred if vTaskStartScheduler() returned. */
        pio_clear(PIOB, PIO_PB27);

        /* Should never be reached, main() function should never end */
        for (;;)
            ;
        return 0;
    }

    uint8_t btns(void)
    {
        return (pio_get(PIOC, PIO_INPUT, BTN1_msk) << 0 |
                pio_get(PIOC, PIO_INPUT, BTN2_msk) << 1 |
                pio_get(PIOC, PIO_INPUT, BTN3_msk) << 2 |
                pio_get(PIOC, PIO_INPUT, BTN4_msk) << 3);
    }

    void createTaskIfBtnPressed(void)
    {
        /* Enter critical section. */
        taskENTER_CRITICAL();
        /* Get BTN states */
        uint8_t btnStates = btns();
        /* For each of the four keys. */
        for (uint8_t i = 0; i < 4; i++)
        {
            /* If the key is down and was up in the previous (Idle task) iteration. */
            if ((~btnStates & (1 << i)) && (prevBtnStates & (1 << i)))
            {
                /* Create new key corresponding task with priority reflecting its BT. */
                if (xTaskCreate(pTsk[i], "Task", 150, NULL, 0, &tskHandle[i]) != pdPASS)
                {
                    /* Set error LED, we were unable to create new task, no heap space? */
                    pio_clear(PIOB, PIO_PB27);
                }
            }
        }
        /* Save key position for the next Idle task iteration. */
        prevBtnStates = btnStates;
        /* Exit critical section. */
        taskEXIT_CRITICAL();
    }

    void tsk1(void *par)
    {
        /* For predefined number of iterations. */
        for (uint32_t i = 0; i < tskRepeats[0]; i++)
        {
            /* Turn LED belonging to this task on, others off. */
            pio_set(PIOC, LED1_msk);
            pio_clear(PIOC, LED2_msk | LED3_msk | LED4_msk);
            /* Create new key corresponding tasks. */
            createTaskIfBtnPressed();
        }
        /* Delete this task. NULL will delete the task calling. */
        vTaskDelete(NULL);
    }

    void tsk2(void *par)
    {
        /* For predefined number of iterations. */
        for (uint32_t i = 0; i < tskRepeats[1]; i++)
        {
            /* Turn LED belonging to this task on, others off. */
            pio_set(PIOC, LED2_msk);
            pio_clear(PIOC, LED1_msk | LED3_msk | LED4_msk);
            /* Create new key corresponding tasks. */
            createTaskIfBtnPressed();
        }
        /* Delete this task. NULL will delete the task calling. */
        vTaskDelete(NULL);
    }

    void tsk3(void *par)
    {
        /* For predefined number of iterations. */
        for (uint32_t i = 0; i < tskRepeats[2]; i++)
        {
            /* Turn LED belonging to this task on, others off. */
            pio_set(PIOC, LED3_msk);
            pio_clear(PIOC, LED1_msk | LED2_msk | LED4_msk);
            /* Create new key corresponding tasks. */
            createTaskIfBtnPressed();
        }
        /* Delete this task. NULL will delete the task calling. */
        vTaskDelete(NULL);
    }

    void tsk4(void *par)
    {
        /* For predefined number of iterations. */
        for (uint32_t i = 0; i < tskRepeats[3]; i++)
        {
            /* Turn LED belonging to this task on, others off. */
            pio_set(PIOC, LED4_msk);
            pio_clear(PIOC, LED1_msk | LED2_msk | LED3_msk);
            /* Create new key corresponding tasks. */
            createTaskIfBtnPressed();
        }
        /* Delete this task. NULL will delete the task calling. */
        vTaskDelete(NULL);
    }

    void vApplicationIdleHook(void)
    {
        /* Turn all LEDs off. */
        pio_clear(PIOC, LED_ALL_msk);
        /* Create new key corresponding tasks. */
        createTaskIfBtnPressed();
    }

#ifdef __cplusplus
}
#endif
