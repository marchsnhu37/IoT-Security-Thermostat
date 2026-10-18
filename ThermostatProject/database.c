
//header inclusions
#include "database.h"
#include <sqlite3.h>
#include <stdio.h>

//macro for debugging. Set to true, it'll print status to terminal
#define DEBUG true

//variable connects to database. static makes sure Thermostat.c can't touch it
static sqlite3 *db_connection = NULL;
//stores SQL template bytecode
static sqlite3_stmt *compiled_log_statement = NULL;

//initializing database
bool init_database(void){
    //calls to open database file and looks in storage for name
    int result = sqlite3_open(DATABASE_NAME, &db_connection);

    //did the connection fail?
    if (result != SQLITE_OK){
        fprintf(stderr, "Can't open connection handle: %s\n", sqlite3_errmsg(db_connection));
        return false;
    }

    //if disk is busy, wait and retry up to 1000 miliseconds before giving up
    sqlite3_busy_timeout(db_connection, 1000);

    //stores table structure inside a standard string
    const char *create_table_sql =
        "CREATE TABLE IF NOT EXISTS history_logs ("
        "id             INTEGER PRIMARY KEY AUTONICREMENT,"
        "timestamp      INTEGER NOT NULL,"
        "room_temp      REAL    NOT NULL,"
        "system_state   INTEGER NOT NULL"
    ");";

    //IF NOT EXISTS = boot-up filter? builds columns. skips on all future runs to keep from overwriting what already exists
    char *error_message = NULL;
    result = sqlite3_exec(db_connection, create_table_sql, NULL, NULL, &error_message);

    //table setup wrap
    if (result != SQLITE_OK){
        fprintf(stderr, "Failed to assert table layout schema: %s\n", error_message);
        sqlite3_free(error_message);
        return false;
    }
    if (DEBUG) printf("\nSuccessfully initialized connection to '%s'\n", DATABASE_NAME);
    return true;
}

//logger function
bool log_telemetry_packet(const thermostat_data_t *packet){
    if (db_connection == NULL || packet == NULL){
        return false;
    }

    //first pass, takes raw SQL string, compiles, and stows. skips on future iterations
    if (compiled_log_statement == NULL){
        const char *insert_sql = "INSERT INTO history_logs (timestamp, room_temp, system_state) VALUES (?, ?, ?);";
        inst result = sqlite3_prepare_v2(db_connection, insert_sql, -1, &compiled_log_statement, NULL);
        if (result != SQLITE_OK){
            fprintf(stderr, "Failed to compile statement template: %s\n", sqlite3_errmsg(db_connection));
            return false;
        }
    }

    //data bindings
    sqlite3_bind_int64(compiled_log_statement, 1, (sqlite3_int64)packet->timestamp);
    sqlite3_bind_double(compiled_log_statement, 2, (double)packet->filtered_temperature);
    sqlite3_bind_int(compiled_log_statement, 3, packet->system_state);

    //writes data parameters onto HD
    int result = sqlite3_step(compiled_log_statement);

    //clears slots, letting the template be reused every 30 seconds
    if(result!=SQLITE_DONE){
        fprintf(stderr, "Failed to append historical log row: %s\n", sqlite3_errmsg(db_connection));
        sqlite3_reset(compiled_log_statement);
        return false;
    }

    sqlite3_reset(compiled_log_statement);
    return true;
}

//deletes template structure and frees memory
void close_database(void){
    if (compiled_log_statement!=NULL){
        sqlite3_finalize(compiled_log_statement);
        compiled_log_statement=NULL;
    }

    //closes connection on disk
    if (db_connection!=NULL){
        sqlite3_close(db_connection);
        db_connection=NULL;
    }
    
    printf("Database engine closed.\n");
}