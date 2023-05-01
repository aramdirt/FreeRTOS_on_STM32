#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "usart.h"
#include "delay.h"

#if 1
#include "FreeRTOS.h"
#include "task.h"

#define START_TASK_PRIO        1 
#define TASK1_TASK_PRIO        2
#define TASK2_TASK_PRIO        3

#define START_STK_SIZE         128
#define TASK1_STK_SIZE         128
#define TASK2_STK_SIZE         128

TaskHandle_t StartTask_Handler;
TaskHandle_t Task1Task_Handler;
TaskHandle_t Task2Task_Handler;

/* The task functions prototype*/
void start_task(void *pvParameters);
void task1_task(void *pvParameters);
void task2_task(void *pvParameters);

/* Task parameter to be sent to the task function */
const char *pvTask1  = "Task1 is running.";
const char *pvTask2  = "Task2 is running.";

int main(void)
{
    /* Board initializations */
    delay_init(); 
    USART_User_Init(115200);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* Initialize the serial I/O(console ), making standard output to be send to USART1 */
    Usart_SendString(USART1, "Initialization is done.\r\n");
    delay_ms(500);

    /* Create start task. */
    xTaskCreate((TaskFunction_t )start_task,
            (const char*    )"start_task",
            (uint16_t       )START_STK_SIZE,
            (void*          )NULL,
            (UBaseType_t    )START_TASK_PRIO,
            (TaskHandle_t*  )&StartTask_Handler);

    Usart_SendString(USART1, "Before Start the scheduler.\r\n");
    delay_ms(500);
    /* Start the scheduler so our tasks start executing. */
    vTaskStartScheduler();
    Usart_SendString(USART1, "Start the scheduler.\r\n");
    delay_ms(500);

    while(1)
    {

    }
}

void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();

    /* Create task1. */
    xTaskCreate((TaskFunction_t )task1_task,            /* Pointer to the function that implements the task. */
                (const char*    )"task1",               /* Text name for the task.  This is to facilitate debugging only. */
                (uint16_t       )TASK1_STK_SIZE,        /* Stack depth in words. */
                (void*          )pvTask1,
                (UBaseType_t    )TASK1_TASK_PRIO,       /* This task will run at priority 1. */
                (TaskHandle_t*  )&Task1Task_Handler);

    /* Create task2. */
    xTaskCreate((TaskFunction_t )task2_task,            /* Pointer to the function that implements the task. */
                (const char*    )"task2",               /* Text name for the task.  This is to facilitate debugging only. */
                (uint16_t       )TASK2_STK_SIZE,        /* Stack depth in words. */
                (void*          )pvTask2,
                (UBaseType_t    )TASK2_TASK_PRIO,       /* This task will run at priority 1. */
                (TaskHandle_t*  )&Task2Task_Handler);

    Usart_SendString(USART1, "In start task.\r\n");
    delay_ms(500);
    vTaskDelete(StartTask_Handler);
    Usart_SendString(USART1, "Delete start task.\r\n");
    delay_ms(500);
    taskEXIT_CRITICAL();
}

void task1_task(void *pvParameters)
{
    char *pcTaskName = (char *) pvParameters;
    for(;;)
    {
        /* Print out the name of this task. */
        Usart_SendString(USART1, pcTaskName);
        delay_ms(100);
        Usart_SendString(USART1, "\r\n");
        delay_ms(100);

        vTaskDelay(200);
    }
}

void task2_task(void *pvParameters)
{
    char *pcTaskName = (char *) pvParameters;
    for(;;)
    {
        /* Print out the name of this task. */
        Usart_SendString(USART1, pcTaskName);
        delay_ms(100);
        Usart_SendString(USART1, "\r\n");
        delay_ms(100);

        vTaskDelay(200);
    }
}
#endif

#if 0
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* The task functions prototype*/
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );

/* Task parameter to be sent to the task function */
const char *pvTask1  = "Task1 is running.";
const char *pvTask2  = "Task2 is running.";

/* Extern functions */
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);

/*-----------------------------------------------------------*/
/* Global semaphore variable */
SemaphoreHandle_t xSemaphore = NULL; 
int main( void )
{
	/* Board initializations */
    SystemInit();
    delay_init();
    USART_User_Init(115200);

    /* This function initializes the MCU clock, PLL will be used to generate Main MCU clock */  
	SystemCoreClockUpdate();

    /* Initialize the serial I/O(console ), making standard output to be send to USART1 */
    Usart_SendString(USART1, "Initialization is done.\r\n");
    delay_s(1);

	/* Create one of the two tasks. */
	xTaskCreate(vTask1,             /* Pointer to the function that implements the task. */
        "Task 1",                   /* Text name for the task.  This is to facilitate debugging only. */
        configMINIMAL_STACK_SIZE,   /* Stack depth in words. */
        (void*)pvTask1,		        /* We are not using the task parameter. */
        1,			                /* This task will run at priority 1. */
        NULL);		                /* We are not using the task handle. */

    /* Create the other task in exactly the same way. */
	xTaskCreate( vTask2, "Task 2", configMINIMAL_STACK_SIZE, (void*)pvTask2, 1, NULL );

    /* Create a binary semaphore */
	xSemaphore = xSemaphoreCreateBinary();

	/* make the semaphore token available for the first time */
    xSemaphoreGive( xSemaphore);

	/* Start the scheduler so our tasks start executing. */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/
void vTask1( void *pvParameters )
{
    char *pcTaskName = (char *) pvParameters;

	/* Task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Take semaphore */
		xSemaphoreTake(xSemaphore,(TickType_t) portMAX_DELAY);

		/* Print out the name of this task. */
        Usart_SendString(USART1, pcTaskName);
        delay_ms(500);
        Usart_SendString(USART1, "\r\n");
        delay_ms(500);

		/* Give semaphore */
		xSemaphoreGive(xSemaphore);

		/* Delay for a period. */
		vTaskDelay( 2000 / portTICK_PERIOD_MS );
	}
}
/*-----------------------------------------------------------*/
void vTask2( void *pvParameters )
{
    char *pcTaskName = (char *) pvParameters;

	/* Task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Take semaphore */
		xSemaphoreTake(xSemaphore,(TickType_t) portMAX_DELAY);

		/* Print out the name of this task. */
        Usart_SendString(USART1, pcTaskName);
        delay_ms(500);
        Usart_SendString(USART1, "\r\n");
        delay_ms(500);

		/* Give semaphore */
		xSemaphoreGive(xSemaphore);

		/* Delay for a period. */
		vTaskDelay( 2000 / portTICK_PERIOD_MS );
	}
}
#endif

#if 0
int main (void)
{
	delay_init();	  
	USART_User_Init(115200);
	while (1)
	{
        Usart_SendString(USART1, "AAA\n");
        delay_s(1);
	}
}
#endif
