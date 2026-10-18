/*
To prevent a monolithic codebase, pipeline.h exists
to enable communication between Thermostat.c and pipeline.c

Keeps code modular and maintainable

Included at the top of Thermostat.c to ensure the compiler such
functions exist (prevents errors)
*/

//header guards: keeps compiler from loading this twice, especially considering Thermostat.c and pipeline.c will read this
#ifndef PIPELINE_H
#define PIPELINE_H

//library headers
#include <time.h>
#include <stdbool.h>

//pipeline dimensions
#define PIPELINE_BUFFER_SIZE 10 //gives circular buffer 10 entries
#define MAX_INPUT_LINE 64 //caps string buffer at 64 bytes

typedef struct {
    time_t timestamp; //holds time when whatever is read drops into pipeline
    float filtered_temp; //stores temperature calc in Fahrenheit
    int system_state; //will store the state the thermostat is in (off = 0, heat = 1, cool = 2)
} thermostat_data_t; //name of container

//holds buffer parameters and memory array
typedef struct {
    thermostat_data_t storage[PIPELINE_BUFFER_SIZE]; //makes computer allocate 10 slots in an array. calls above data packet

    //tracking variables
    int write_index; //where next packet goes
    int read_index; //where oldest packet is read from
    int current_count; //tracks filled slots
} circular_buffer_t; //name of container

//function prototypes. basically forward declarations. these are global compared
//to those in Thermostat.c
bool push_pipeline_buffer(circular_buffer_t *buf, thermostat_data_t new_data); //promises a function that will fill 10 slots with data and erase the oldest one when full
float calulate_moving_average(circular_buffer_t buf, float fallback_default); //promises a function that will calculate a rolling average
bool parse_telemetry_stream(const char *raw_string, size_t max_size, thermostat_data_t *out_packet); //promises a function that will break down the raw data so the thermostat can read it

#endif //closes ifndef conditional