
//header inclusions
#include "pipeline.h"
#include <stdio.h>
#include <string.h>

/*
functions 1 through 3 were declared at the end
of pipeline.h. there, they were declared. here they are defined*/

//function 1
bool push_pipeline_buffer(circular_buffer_t *buf, thermostat_data_t new_data){
    if (buf->current_count == PIPELINE_BUFFER_SIZE){ //checking if # of items currently in RAM = max
        //if we're at max, we make a slot for the incoming entry
        buf->read_index = (buf->read_index + 1) % PIPELINE_BUFFER_SIZE;
        buf->current_count--;
    }
    buf->storage[buf->write_index] = new_data; //writes new data into the array index location pointed to by write_index
    //prepare for next piece of data. modulo loops in a circle
    buf->write_index=(buf->write_index+1) % PIPELINE_BUFFER_SIZE;
    buf->current_count++;

    return true; //closes function
}

//function 2
float calulate_moving_average(circular_buffer_t buf, float fallback_default){ //const ensures loop can only read and not modify
    //if buffer is empty, return baseline default (prevents dividing by 0 error like when thermostat first boots)
    if (buf.current_count == 0){
        return fallback_default;
    }
    //initialize to accumulate rolling temp fractions and to ensure we start at oldest recorded item
    float temperature_sum = 0.0f;
    int inspection_index = buf.read_index;

    //grabs temperature decimal every loop and adds to total
    for (int i = 0; i < buf.current_count; i++){
        temperature_sum += buf.storage[inspection_index].filtered_temperature;
        inspection_index = (inspection_index + 1) % PIPELINE_BUFFER_SIZE;
    }
    //returns temp decimal
    return temperature_sum / (float)buf.current_count;
}

//function 3
bool parse_telemetry_stream(const char *raw_string, size_t max_size, thermostat_data_t *out_packet){
    //protects from malicious entries. checks if text is broken or > 64 chars
    if (raw_string == NULL || out_packet == NULL || strnlen(raw_string, max_size) >= MAX_INPUT_LINE){
        return false;
    }

    //temps that will catch values separated
    //stores <= 15 chars
    char parsed_state[16] = 0;
    float parsed_temp = 0.0f;
    int parsed_setpoint = 0;

    //splits string into the above 3 variables. will return 3 if all is well. next line will drop if != 3
    int parsed_fields = sscanf(raw_string, "%15[^,],%f,%d", parsed_state, &parsed_temp, &parsed_setpoint);

    //if above != 3, discards line
    if (parsed_fields !=3){
        return false;
    }

    //maps the state of the thermostat
    if (strcmp(parsed_state, "heat") ==0){
        out_packet->system_state=1;
    } else if (strcmp(parsed_state, "cool") ==0){
        out_packet->system_state=2;
    } else {
        out_packet->system_state=0;
    }

    //saves successful parsing and adds timestamp
    out_packet->filtered_temperature = parsed_temp;
    out_packet->timestamp=time(NULL);

    return true;
}