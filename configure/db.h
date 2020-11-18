#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

sqlite3 *db = NULL;

static int sn = 0;

void create_table(char *filename);
void close_table(void);
void insert_open(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, char *type);
void insert_read(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, int flags, char *result, char *fdname);
void insert_write(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, int flags, char *result, char *fdname);
void insert_close(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *type, char *result);
void insert_kill(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, int gid, int sig, int pid_);
void insert_mkdir(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, int mode);
void insert_fchmodat(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, int mod, int dirfd);
void insert_fchownat(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, int flags, int dirfd, int gid, int user_id);

void create_table(char *filename)
{
    char *sql;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open(filename, &db);
    // rc = sqlite3_open_v2(filename,&db,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc)
    {
        fprintf(stderr,"can't open database%s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    // sql =  "CREATE TABLE save_data(num integer primary key, id int, data text, time text)";
    sql = "CREATE TABLE OPEN("  \
          "ID INTEGER PRIMARY KEY AUTOINCREMENT ,"  \
          "USERNAME       TEXT ,"  \
          "UID            INT  ,"  \
          "COMMANDNAME    TEXT ,"  \
          "PID            INT  ,"  \
          "LOGTIME        TEXT ,"  \
          "FILEPATH       TEXT ,"  \
          "OPENTYPE       TEXT ,"  \
          "RESULT         TEXT );";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sql = "CREATE TABLE READ("  \
          "ID INTEGER PRIMARY KEY AUTOINCREMENT ,"  \
          "USERNAME       TEXT ,"  \
          "UID            INT  ,"  \
          "COMMANDNAME    TEXT ,"  \
          "PID            INT  ,"  \
          "LOGTIME        TEXT ,"  \
          "FILEPATH       TEXT ,"  \
          "ReadBufSize    INT  ,"  \
          "FDNAME         TEXT ,"  \
          "RESULT         TEXT );";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sql = "CREATE TABLE WRITE("  \
          "ID INTEGER PRIMARY KEY AUTOINCREMENT ,"  \
          "USERNAME       TEXT ,"  \
          "UID            INT  ,"  \
          "COMMANDNAME    TEXT ,"  \
          "PID            INT  ,"  \
          "LOGTIME        TEXT ,"  \
          "FILEPATH       TEXT ,"  \
          "WriteBufSize    INT  ,"  \
          "FDNAME         TEXT ,"  \
          "RESULT         TEXT );";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sql = "CREATE TABLE CLOSE("  \
          "ID INTEGER PRIMARY KEY AUTOINCREMENT ,"  \
          "USERNAME       TEXT ,"  \
          "UID            INT  ,"  \
          "COMMANDNAME    TEXT ,"  \
          "PID            INT  ,"  \
          "LOGTIME        TEXT ,"  \
          "FILEPATH       TEXT ,"  \
          "CLOSETYPE      TEXT ,"  \
          "RESULT         TEXT );";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sql = "CREATE TABLE KILL("  \
          "ID INTEGER PRIMARY KEY AUTOINCREMENT ,"  \
          "USERNAME       TEXT ,"  \
          "UID            INT  ,"  \
          "COMMANDNAME    TEXT ,"  \
          "PID            INT  ,"  \
          "GID            INT  ,"  \
          "SIG            INT  ,"  \
          "PID_KILLED     INT  ,"  \
          "LOGTIME        TEXT ,"  \
          "FILEPATH       TEXT ,"  \
          "RESULT         TEXT );";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sql = "CREATE TABLE MKDIR("  \
          "ID INTEGER PRIMARY KEY AUTOINCREMENT ,"  \
          "USERNAME       TEXT ,"  \
          "UID            INT  ,"  \
          "COMMANDNAME    TEXT ,"  \
          "PID            INT  ,"  \
          "LOGTIME        TEXT ,"  \
          "MODE           INT,"    \
          "DIRPATH        TEXT,"   \
          "RESULT         TEXT );";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sql = "CREATE TABLE FCHMODAT("  \
          "ID INTEGER PRIMARY KEY AUTOINCREMENT ,"  \
          "USERNAME       TEXT ,"  \
          "UID            INT  ,"  \
          "COMMANDNAME    TEXT ,"  \
          "PID            INT  ,"  \
          "MOD            INT  ,"  \
          "DIRFD          INT  ,"  \
          "LOGTIME        TEXT ,"  \
          "FILEPATH       TEXT ,"  \
          "RESULT         TEXT );";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sql = "CREATE TABLE FCHOWNAT("  \
          "ID INTEGER PRIMARY KEY AUTOINCREMENT ,"  \
          "USERNAME       TEXT ,"  \
          "UID            INT  ,"  \
          "COMMANDNAME    TEXT ,"  \
          "PID            INT  ,"  \
          "GID            INT  ,"  \
          "FLAG           INT  ,"  \
          "USERID         INT  ,"  \
          "DIRFD          INT  ,"  \
          "LOGTIME        TEXT ,"  \
          "FILEPATH       TEXT ,"  \
          "RESULT         TEXT );";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);
}

void close_table(void)
{
    sqlite3_close(db);
}


void insert_open(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, char *type)
{
    char *sql = NULL;
    char *zErrMsg = NULL; 
    sql = sqlite3_mprintf("INSERT INTO OPEN (ID, USERNAME, UID, COMMANDNAME, PID, LOGTIME, FILEPATH, OPENTYPE, RESULT) " \
        "VALUES (null, '%s', %d, '%s', %d, '%s', '%s', '%s', '%s')",
        username, uid, commandname, pid, logtime, filepath, type, result);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    // printf("%s", sql);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL ERROR: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "OPEN records created successfully\n");
    }
    sqlite3_free(sql);
}

void insert_read(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, int flags, char *result, char *fdname)
{
    char *sql = NULL;
    char *zErrMsg = NULL; 
    sql = sqlite3_mprintf("INSERT INTO READ (ID, USERNAME, UID, COMMANDNAME, PID, LOGTIME, FILEPATH, ReadBufSize, FDNAME,RESULT) " \
        "VALUES (null, '%s', %d, '%s', %d, '%s', '%s', %d, '%s', '%s')",
        username, uid, commandname, pid, logtime, filepath, flags, fdname, result);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    // printf("%s", sql);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL ERROR: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "READ records created successfully\n");
    }
    sqlite3_free(sql);
}

void insert_write(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, int flags, char *result, char *fdname)
{
    char *sql = NULL;
    char *zErrMsg = NULL; 
    sql = sqlite3_mprintf("INSERT INTO WRITE (ID, USERNAME, UID, COMMANDNAME, PID, LOGTIME, FILEPATH, WriteBufSize, FDNAME,RESULT) " \
        "VALUES (null, '%s', %d, '%s', %d, '%s', '%s', %d, '%s', '%s')",
        username, uid, commandname, pid, logtime, filepath, flags, fdname, result);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    // printf("%s", sql);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL ERROR: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "WRITE records created successfully\n");
    }
    sqlite3_free(sql);
}

void insert_close(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *type, char *result)
{
    char *sql = NULL;
    char *zErrMsg = NULL; 
    sql = sqlite3_mprintf("INSERT INTO CLOSE (ID, USERNAME, UID, COMMANDNAME, PID, LOGTIME, FILEPATH, CLOSETYPE, RESULT) " \
        "VALUES (null, '%s', %d, '%s', %d, '%s', '%s', '%s', '%s')",
        username, uid, commandname, pid, logtime, filepath, type, result);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    // printf("%s", sql);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL ERROR: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "CLOSE records created successfully\n");
    }
    sqlite3_free(sql);
}

void insert_kill(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, int gid, int sig, int pid_)
{
    char *sql = NULL;
    char *zErrMsg = NULL; 
    sql = sqlite3_mprintf("INSERT INTO KILL (ID, USERNAME, UID, COMMANDNAME, PID, GID, SIG, PID_KILLED, LOGTIME, FILEPATH, RESULT) " \
        "VALUES (null, '%s', %d, '%s', %d, %d, %d, %d, '%s', '%s', '%s')",
        username, uid, commandname, pid, gid, sig, pid_, logtime, filepath, result);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    // printf("%s", sql);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL ERROR: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "KILL records created successfully\n");
    }
    sqlite3_free(sql);
}

void insert_mkdir(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, int mode)
{
    char *sql = NULL;
    char *zErrMsg = NULL; 
    sql = sqlite3_mprintf("INSERT INTO MKDIR (ID, USERNAME, UID, COMMANDNAME, PID, LOGTIME, MODE, DIRPATH, RESULT) " \
        "VALUES (null, '%s', %d, '%s', %d, '%s', %o, '%s', '%s')",
        username, uid, commandname, pid, logtime, mode, filepath, result);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    // printf("%s", sql);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL ERROR: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "MKDIR records created successfully\n");
    }
    sqlite3_free(sql);
}

void insert_fchmodat(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, int mod, int dirfd)
{
    char *sql = NULL;
    char *zErrMsg = NULL; 
    sql = sqlite3_mprintf("INSERT INTO FCHMODAT (ID, USERNAME, UID, COMMANDNAME, PID, LOGTIME, MODE, FILEPATH, RESULT, DIRFD) " \
        "VALUES (null, '%s', %d, '%s', %d, '%s', %o, '%s', '%s', %d)",
        username, uid, commandname, pid, logtime, mod, filepath, result, dirfd);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    // printf("%s", sql);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL ERROR: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "FCHMODAT records created successfully\n");
    }
    sqlite3_free(sql);
}

void insert_fchownat(char *username, int uid, char *commandname, int pid, char *logtime, char *filepath, char *result, int flags, int dirfd, int gid, int user_id)
{
    char *sql = NULL;
    char *zErrMsg = NULL; 
    sql = sqlite3_mprintf("INSERT INTO FCHOWNAT (ID, USERNAME, UID, COMMANDNAME, PID, LOGTIME, FILEPATH, RESULT, DIRFD, FLAG, GID, USERID) " \
        "VALUES (null, '%s', %d, '%s', %d, '%s', '%s', '%s', %d, %d, %d, %d)",
        username, uid, commandname, pid, logtime, filepath, result, dirfd, flags, gid, user_id);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    // printf("%s", sql);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL ERROR: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "FCHOWNAT records created successfully\n");
    }
    sqlite3_free(sql);
}

int sqlite_callback(void * userData,int numCol,char **colData,char **colName)
{
    int i, offset = 0;
    char *buf, *tmp;

    buf = (char *)malloc(40 * sizeof(char));
    tmp = buf;
    memset(buf,0,40);

    //printf("%d %d\n",sizeof(buf),strlen(buf));
    for (i = 1;i < numCol;i++)
    {
        buf = buf + offset;
        sprintf(buf,"%s ",colData[i]);
        offset = strlen(colData[i]) + 1; //it's need one place for put a blank so the lenght add 1
    //    printf("i %d offset %d\n",i, offset);
    }
    printf("%.4d. %s \n",++sn,tmp);

    free(tmp);
    tmp = NULL;
    buf = NULL;

    return 0;
}


void search_all(char * table)
{
    char *sql;
    char *zErrMsg = 0;

    sn = 0;

    sql = sqlite3_mprintf("select * from %s", table);
    sqlite3_exec(db, sql, &sqlite_callback, 0, &zErrMsg);
    sqlite3_free(sql);

}

void search_by_id(char * table,char * id)
{
    char * sql;
    char * zErrMsg = 0;

    sn = 0;

    sql = sqlite3_mprintf("select * from %s where id=%s",table,id);
    sqlite3_exec(db,sql,&sqlite_callback,0,&zErrMsg);
    sqlite3_free(sql);
}

void delete_by_id(char * table,char * id)
{
    int rc ;
    char * sql;
    char * zErrMsg = 0;
    sql = sqlite3_mprintf("delete from %s where id=%s",table,id);
    rc = sqlite3_exec(db,sql,0,0,&zErrMsg);
    sqlite3_free(sql);
}

void delete_all(char * table)
{
    char * sql;
    char * zErrMsg = 0;

    sql = sqlite3_mprintf("delete from %s",table);
    sqlite3_exec(db,sql,0,0,&zErrMsg);
    sqlite3_free(sql);
}

// int test_table()
// {
//     char *filename = "test.db";
//     int i;

//     create_table(filename);

//     insert_open("OPEN", "username", 123, "commandname", 123, "2020", "filepath", "opentype", "openresult");
//   //  search_all("OPEN");

//     close_table();

//     return 0;
// }
