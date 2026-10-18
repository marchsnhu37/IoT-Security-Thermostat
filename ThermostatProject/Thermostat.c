/*
Thermostat has three states: heat, off, and cool.
Color of lights lit depends on state. Red aligns with heat, blue with cool, off with off
Whether the light will blink or stay solid depends on if the
current temp is at or below the set temp
Heat: If below set temp, light is solid. Otherwise fades
Cool: If above set temp, light is solid. Otherwise fades

There are also 3 buttons.
One cycles through the states.
Another raises the set temp.
The final button lowers the set temp.

LCD display will display date and time on one line.
Second line will alternate between the current set temp and
the state of the thermostat and the current temp.

Thermostat will send status update to TemperatureServer ~30 seconds
and will include state, current temp in Fahrenheit, and current set temp
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "pipeline.h"
include "database.h"

//make a macro for debugging
#define DEBUG true

//state definitions for the thermostat
typedef enum {
    STATE_OFF, // = 0
    STATE_HEAT, // = 1
    STATE_COOL // = 2
} ThermostatState;

//configurations
//configuring LEDs on the Pi
#define PIN_LED_BLUE 23
#define PIN_LED_RED 18

//global tracking variables
int set_point = 72; //default target temp
float current_smoothed_temperature = 72.0f; //for filtered temp

//current_state can only hold one option (integer from 0-2) at a time.
//if invalid, throws an error
ThermostatState current_state = STATE_OFF; //thermostat is in off state on startup

//prevents race conditions
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

//makes global instance of pipeline buffer data layout
circular_buffer_t telemetry_buffer = {
    .write_index = 0, .read_index = 0, .current_count = 0
};

//volatile is used to inform the compiler these variables can change any time. like when user hits CTRL-C
volatile bool repeat = true; //for main loop. stops when user hits CTRL-C
volatile bool end_display = false; //decides whether the background LCD thread will run

//forward declarations
float get_fahrenheit(void);
void update_lights(void);
void setup_serial_output(char *output_buffer, size_t buffer_size);

//mock updates specific to hardware
void update_screen(const char* line1, const char* line2){
    if (DEBUG){
        printf("\r[LCD] %s - %s  ", line1, line2);
        fflush(stdout); //console forced to redraw the line
    }
}

//when user presses the green button, the function will change the thermostat's state
void cycle_thermostate_state(void){
    pthread_mutex_lock(&state_mutex);
    switch (current_state){
        case STATE_OFF:
            current_state = STATE_HEAT; //moves into heat state
            update_lights(); //calls on_enter_heat to determine if red LED should blink or stay solid
            if (DEBUG) printf("Changing state to Heat\n");
            break;

        case STATE_HEAT:
            current_state = STATE_COOL; //moves into cool state
            update_lights(); //calls on_enter_cool to determine if blue LED should blink or stay solid
            if (DEBUG) printf("Changing state to Cool\n");
            break;

        case STATE_COOL:
            current_state = STATE_OFF; //moves into off state
            update_lights(); //calls on_enter_off
            if (DEBUG) printf("Changing state to Off\n");
            break;
    }
    pthread_mutex_unlock(&state_mutex); //unlocks thread channel
    update_lights();
}

//callbacks. run when you press one of the three buttons
//green button. changes state
void process_temp_state_button(void){
    if (DEBUG) printf("Cycling Temperature State\n");
    cycle_thermostat_state();
}

//red button. increases set temp by 1
void process_temp_inc_button(void){
    if (DEBUG) printf("Increasing Set Point\n");
    pthread_mutex_lock(&state_mutex); //locks variable write operations
    set_point+=1;
    pthread_mutex_unlock(&state_mutex); //unlocks
    update_lights(); //decides if lights should blink or stay solid
}

//blue button. decreases set temp by 1
void process_temp_dec_button(void){
    if (DEBUG) printf("Decreasing Set Point\n");
    pthread_mutex_lock(&state_mutex); //locks variable write operations
    set_point-=1;
    pthread_mutex_unlock(&state_mutex); //unlocks
    update_lights(); //decides if lights should blink or stay solid
}

//calculates temp in Fahrenheit
float get_fahrenheit(void){
    float celsius = 22.2; //currently a simulated sensor value. have yet to load external hardware drivers
    return ((9.0/5.0) * celsius) + 32.0;
}

//reads temp, evlauates state, and updates LEDs
void update_lights(void){
    pthread_mutex_lock(&state_mutex); //opens safety lock before executing
    int temp = (int)floor(current_smoothed_temperature);
    int target = set_point;
    ThermostatState mode = current_state;
    pthread_mutex_unlock(&state_mutex); //close safety lock
    if (DEBUG){
        printf("State: %d, Set Point: %d, Temp: %d\n", current_state, set_point, temp);
    }

    if (current_state == STATE_HEAT){ //heat state
        if (temp < set_point){
            if (DEBUG){
                printf("Red LED pulsing\n");
            }
        }
        else{
            if (DEBUG){
                printf("Red LED solid\n");
            }
        }
    }
    else if (current_state == STATE_COOL){ //cool state
        if (temp > set_point){
            if (DEBUG){
                printf("Blue LED pulsing\n");
            }
        }
        else{
            if (DEBUG){
                printf("Blue LED solid\n");
            }
        }
    }
    else{ //off state
        if (DEBUG){
            printf("All LEDS off\n");
        }
    }
}

//serializes/standardizes data for the thermostat
void setup_serial_output(char *output_buffer, size_t buffer_size){
    //if memory addresses are empty, block execution
    if (output_buffer==NULL || buffer_size==0){
        return;
    }

    pthread_mutex_lock(&state_mutex); //thread-safe lock
    //figure out and temporarily store the thermostat's state
    const char *state_str = (current_state == STATE_HEAT) ? "heat" :
    (current_state == STATE_COOL) ? "cool" : "off";

    //uses pipeline telemetry. prevents erratic behavior
    float temp = current_smoothed_temperature;
    int target = set_point;
    pthread_mutex_unlock(&state_mutex); //releases lock

    //writes into the buffer array. will stop at 64 bytes
    int written = snprintf(output_buffer, buffer_size, "%s,%.1f,%d", state_str, temp, target);

    //catch/report if data string overflows buffer
    if (written < 0 || (size_t)written>=buffer_size){
        if (DEBUG){
            printf("\nOutput truncated to fit memory\n");
        }
    }

}

//background thread
void* manage_my_display(void* arg){
    int counter = 1; //waits 30 seconds to send telemetry data
    int altCounter = 1; //for the LCD display screen to switch what's visible on line 2
    //memory buffers to store text strings
    char lcd_line_1[32]; 
    char lcd_line_2[32];
    char serial_buffer[64];
    char mock_async_stream[MAX_INPUT_LINE]; //allocates buffer for network streaming

    while (!end_display){
        if (DEBUG){ 
            printf("Processing info for the display...\n");
        }

        //pipeline stream ingestion layer. simulation for the moment
        //make an incoming text packet
        //the room temp is shifting
        float shifting_sensor = get_fahrenheit() + ((rand() % 10-5) * 0.1f);
        //gets current status of thermostat variables
        //locks copies values before they can change
        pthread_mutex_lock(&state_mutex);
        const char* current_mode_str = (current_state == STATE_HEAT) ? "heat" : (current_state==STATE_COOL) ? "cool" : "off";
        int current_target = set_point;
        pthread_mutex_unlock(&state_mutex);

        //makes one string of raw data
        snprintf(mock_async_stream, sizeof(mock_async_stream), "%s,%.1f,%d\n", current_mode_str, fluctuating_sensor, current_target);

        //prepares to hold final numbers
        thermostat_data_t fresh_packet;

        //from pipeline.c. parses string
        if (parse_telemetry-stream(mock_async_stream, sizeof(mock_async_stream), &fresh_packet)){

            pthread_mutex_lock(&state_mutex);
            //pushes decoded into buffer array. checks max buffer
            push_pipeline_buffer(&telemetry_buffer, fresh_packet);
            circular_buffer_t buffer_snapshot = telemetry_buffer;
            pthread_mutex_unlock(&state_mutex);
            //calculates moving average
            
            float smooth_result = calulate_moving_average(buffer_snapshot, get_fahrenheit());

            pthread_mutex_lock(&state_mutex);
            current_smoothed_temperature = smooth_result;
            pthread_mutex_unlock(&state_mutex);
        }

        //gets system time and formats line 1 of LCD display
        time_t raw_time;
        struct tm *time_info;
        time(&raw_time); //gets raw time from Pi
        time_info = localtime(&raw_time); //takes raw_time from Pi and structures
        strftime(lcd_line_1, sizeof(lcd_line_1), "%m/%d/%y %I:%M%p", time_info); //formats date and time

        //cycles line 2 display between set temp and current temp + thermostat state
        if (alt_counter < 6){ //needs to cycle every 5 secs
            pthread_mutex_lock(&state_mutex); //atomic variable safety check
            int target = set_point;
            pthread_mutex_unlock(&state_mutex);
            snprintf(lcd_line_2, sizeof(lcd_line_2), "Set temp: %dF", set_point); //one cycle shows set temp
            alt_counter++;
        }
        else {
            //next cycle is state and current temp
            pthread_mutex_lock(&state_mutex);
            const char *state_str = (current_state ==STATE_HEAT) ? : "Heat" :
            (current_state == STATE_COOL) ? "Cool" : "Off";

            float display_temp = current_smoothed_temperature;
            pthread_mutex_unlock(&state_mutex);

            snprintf(lcd_line_2, sizeof(lcd_line_2), "%s Temp:%.1fF", state_str, get_fahrenheit());
            alt_counter++;

            if (alt_counter>=11){
                update_lights();
                alt_counter = 1; //resets to cycle 1
            }

        }

        //pushes to display
        update_screen(lcd_line_1, lcd_line_2);

        //sending to serial port every 30 seconds
        if ((counter % 30) == 0){
            setup_serial_output(serial_buffer, sizeof(serial_buffer)); //gathers info and dumps into buffer
            printf("\n[SERIAL OUTPUT]: %s\n", serial_buffer);

            //records data rows to SQLite ~30 seconds
            pthread_mutex_lock(&state_mutex);
            thermostat_data_t log_packet;
            log_packet.timestamp=time(NULL);
            log_packet.filtered_temperature=current_smoothed_temperature;
            log_packet.system_state=(current_state==STATE_HEAT) ? 1 : (current_state == STATE_COOL) ? 2 : 0;
            pthread_mutex_unlock(&state_mutex);

            //moves packet to database.c
            log_telemetry_packet(&log_packet);

            counter = 1; //resets clock
        }
        else {
            counter++; //if we haven't reached 30 seconds yet, we must continue
        }
        sleep(1);
    }
    return NULL; //clears once you hit CTRL-C and clears RAM
}

//exit safely upon CTRL-C
void handle_signal(int signal){
    printf("\nCleaning up and exiting...\n");
    repeat_loop = false;
    end_display = true;
}

//start
int main(void){
    //seeds random number generator
    srand(time(NULL));

    //initializes SQLite, table layouts upon booting
    if (!init_database()){
        fprintf("Failed to initialize database.\n");
        return 1;
    }

    //signal interrupted upon CTRL-C
    signal(SIGINIT, handle_signal);

    //initialize display thread
    pthread_t display_thread;
    //multi-threading
    if (pthread_create(&display_thread, NULL, manage_my_display, NULL) != 0){
        perror("Failed to create thread");
        return 1;
    }

    //main loop
    while (repeat_loop){
        sleep(1);
    }

    pthread_join(display_thread, NULL);

    //severs drive handles upon hitting CTRL C
    close_database();

    printf("Exiting.\n");
    return 0;
}