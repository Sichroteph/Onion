#ifndef PLAY_ACTIVITY_H
#define PLAY_ACTIVITY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3/sqlite3.h>
#include <time.h>

#include "utils/log.h"
#include "utils/str.h"
#include "utils/file.h"

#define INIT_TIMER_PATH "/tmp/initTimer"
#define PLAY_ACTIVITY_SQLITE_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/play_activity.sqlite"
#define PLAY_ACTIVITY_DB_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db"

typedef struct PlayActivity PlayActivity;

struct PlayActivity{
    char *name;
    char *file_path;
    int play_count;
    int play_time;
};

sqlite3 *db;

void create_db(void) {
    printf_debug("%s\n", "create_table()");
    int rc = sqlite3_open(PLAY_ACTIVITY_SQLITE_PATH, &db);
    if (rc != SQLITE_OK) {
        printf_debug("Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    char *sql = "DROP TABLE IF EXISTS play_activities;CREATE TABLE play_activities(name TEXT PRIMARY KEY, relative_path TEXT, play_count INTEGER, play_time INTEGER);CREATE UNIQUE INDEX name_index ON play_activities(name);";
    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        printf_debug("Error: could not create table: %s\n", sqlite3_errmsg(db));
    }
    printf_debug("%s\n", "create_table() return");
}

void close_db(void) {
    printf_debug("%s\n", "close_db()");
    sqlite3_close(db);
    printf_debug("%s\n", "close_db() return");
}

void open_db(void) {
    printf_debug("%s\n", "open_db()");
    if (!exists(PLAY_ACTIVITY_SQLITE_PATH)) {
        create_db();
        if (exists(PLAY_ACTIVITY_DB_PATH)) {
            upgrade_rom_db();
        }
    }
    int rc = sqlite3_open(PLAY_ACTIVITY_SQLITE_PATH, &db);
    if (rc != SQLITE_OK) {
        printf_debug("Cannot open database: %s\n", sqlite3_errmsg(db));
        close_db();
    }
    printf_debug("%s\n", "open_db() return");
}

void insert_data(const char *name, const char *relative_path, int play_count, int play_time) {
    printf_debug("insert_data(%s, %s, %d, %d)\n", name, relative_path, play_count, play_time);
    char *sql = sqlite3_mprintf("INSERT OR REPLACE INTO play_activities (name, relative_path, play_count, play_time) VALUES ('%q', '%q', COALESCE((SELECT play_count FROM play_activities WHERE name='%q'), 0) + %d, COALESCE((SELECT play_time FROM play_activities WHERE name='%q'), 0) + %d);", name, relative_path, name, play_count, name, play_time);
    int rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        printf_debug("Error: could not insert play_activity: %s\n", sqlite3_errmsg(db));
        sqlite3_free(sql);
    }
    sqlite3_free(sql);
    printf_debug("%s\n", "insert_data() return");
}

void upgrade_rom_db(void) {
    printf_debug("%s\n", "upgrade_rom_db()");
    struct PlayActivityStruct{
        char name[STR_MAX];
        int play_time;
    };
    printf_debug("%s\n", "PlayActivityStruct defined");
    FILE *file = fopen(PLAY_ACTIVITY_DB_PATH, "rb");
    printf_debug("%s\n", "file opened");
    if (file != NULL) {
        printf_debug("%s\n", "file not null");
        struct PlayActivityStruct play_activity;
        printf_debug("%s\n", "define play_activity");
        while (fread(&play_activity, sizeof(struct PlayActivityStruct), 1, file) == 1) {
            if (strlen(play_activity.name) > 0) {
                printf_debug("%s\n", "read rom");
                insert_data(play_activity.name, NULL, 1, play_activity.play_time);
            }
        }
        fclose(file);
    }
    printf_debug("%s\n", "upgrade_rom_db() return");
}


int get_play_time(const char* name) {
    printf_debug("get_play_time(%s)\n", name);
    int play_time = 0;
    char* sql = sqlite3_mprintf("SELECT play_time FROM play_activities WHERE name = '%q'", name);
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing SQL statement: %s\n", sqlite3_errmsg(db));
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            play_time = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_free(sql);
    printf_debug("get_play_time(%s) return %s\n", name);
    return play_time;
}


PlayActivity ** find_play_activities(const char *name) {
    printf_debug("find(%s)\n", name);
    PlayActivity **play_activities = NULL;
    char* sql = sqlite3_mprintf("SELECT * FROM play_activities WHERE name LIKE CONCAT('%|','%q','|%';", name);
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf_debug("Error preparing SQL statement: %s\n", sqlite3_errmsg(db));
    } else {
        int num_rows = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            num_rows++;
        }
        sqlite3_reset(stmt);
        play_activities = (PlayActivity **)malloc(sizeof(PlayActivity *) * num_rows);
        int i = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            play_activities[i] = (PlayActivity *)malloc(sizeof(PlayActivity));
            strcpy(play_activities[i]->name, (const char*) sqlite3_column_text(stmt, 0));
            strcpy(play_activities[i]->file_path, (const char*) sqlite3_column_text(stmt, 1));
            play_activities[i]->play_count = sqlite3_column_int(stmt, 2);
            play_activities[i]->play_time = sqlite3_column_int(stmt, 3);
            i++;
        }
    }
    sqlite3_free(sql);
    sqlite3_finalize(stmt);
    printf_debug("find(%s) return\n", name);
    return play_activities;
}
#endif