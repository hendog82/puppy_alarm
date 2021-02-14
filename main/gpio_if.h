
#define MOTOR_FORWARD_GPIO		27
#define MOTOR_REVERSE_GPIO		33
#define BUTTON_GPIO				14

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<MOTOR_FORWARD_GPIO)|(1ULL<<MOTOR_REVERSE_GPIO))
#define GPIO_INPUT_PIN_SEL   (1ULL<<BUTTON_GPIO)


/*
 * Initializes all motor and button functions:
 *
 * 		(1) calls motor_init(), button_init(),
 * 		(2) sets up button isr handler
 * 		(3) starts button task.
 *
 */
void gpio_init();

/*
 * initialize motor gpios
 */
void motor_init();

/*
 * interrupt handler for button
 */
void button_isr_handler(void* arg);

/*
 * task to receive button interrupts.
 */
void Button_Task(void* arg);

/*
 * initialize button gpio
 */
void button_init();

/*
 * spin motor forwards for 3 seconds
 */
void motor_forward();

/*
 * spin motor backwards for 3 seconds
 */
void motor_reverse();
