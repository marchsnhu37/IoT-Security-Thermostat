/*
To prevent a monolithic codebase, pipeline.h exists
to enable communication between Thermostat.c and database.c

Keeps code modular and maintainable

Included at the top of Thermostat.c to ensure the compiler such
functions exist (prevents errors)
*/

//header guards: keeps compiler from loading this twice, especially considering Thermostat.c and database.c will read this
#ifndef DATABASE_H
#define DATABASE_H

//pulls in pipeline.h- logger needs to understand shape of thermostat_data_t
#include "pipeline.h"
//library header- provides bool
#include <stdbool.h>

//global macro configuration: finds DATABASE_NAME and replaces it w thermostat_history.db
//SQLite will use this file to house tables
#define DATABASE_NAME "thermostat_history.db"

//global forward declaration. opens database on boot
bool init_database(void);
//forward declaration for logging. will take info and drop it into rows
bool log_telemetry_packet(const thermostat_data_t *packet);
//shuts down database handlers on CTRL C and reduces risk of corruption
void close_database(void);

//closes ifndef
#endif //DATABASE_H