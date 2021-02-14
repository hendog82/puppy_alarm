#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_event.h"

#include "gpio_if.h"

xQueueHandle button = NULL;
static const char *TAG = "GPIO";


void gpio_init(){
    motor_init();

    button = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(Button_Task, "Button_Task", 2048, NULL, 10, NULL);
    button_init();
}


void motor_init(){
	gpio_config_t io_conf;
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set,e.g.GPIO18/19
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
	//disable pull-down mode
	io_conf.pull_down_en =1;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

void button_init(){
    //initialize button GPIO and ISR
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_FLOATING);//external pull down resistor
    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_POSEDGE);
    gpio_intr_enable(BUTTON_GPIO);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, (void*) BUTTON_GPIO);
}

void button_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(button, &gpio_num, NULL);
    return;
}


void Button_Task(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(button, &io_num, portMAX_DELAY)) {
        	printf("resetting motor\r\n");
        	motor_reverse();
        	//motor_forward();
        }
    }
}


void motor_forward(){
	//turn on GPIO for 3 seconds
	gpio_set_level(MOTOR_FORWARD_GPIO, 1);
	sleep(3);
	gpio_set_level(MOTOR_FORWARD_GPIO, 0);
    ESP_LOGE(TAG, "motor off!");

	return;
}
void motor_reverse(){
	//turn on GPIO for 3 seconds
	gpio_set_level(MOTOR_REVERSE_GPIO, 1);
	sleep(3);
	gpio_set_level(MOTOR_REVERSE_GPIO, 0);
    ESP_LOGE(TAG, "motor off!");

	return;
}
