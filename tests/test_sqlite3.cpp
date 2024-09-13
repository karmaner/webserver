#include "src/db/sqlite3.h"
#include "src/basic/log.h"
#include "src/basic/util.h"

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void test_batch(webserver::SQLite3::ptr db) {
    auto ts = webserver::GetCurrentMS();
    int n = 1000000;
    webserver::SQLite3Transaction trans(db);
    //db->execute("PRAGMA synchronous = OFF;");
    trans.begin();
    webserver::SQLite3Stmt::ptr stmt = webserver::SQLite3Stmt::Create(db,
                "insert into user(name, age) values(?, ?)");
    for(int i = 0; i < n; ++i) {
        stmt->reset();
        stmt->bind(1, "batch_" + std::to_string(i));
        stmt->bind(2, i);
        stmt->step();
    }
    trans.commit();
    auto ts2 = webserver::GetCurrentMS();

    WEBSERVER_LOG_INFO(g_logger) << "used: " << (ts2 - ts) / 1000.0 << "s batch insert n=" << n;
}

int main(int argc, char** argv) {
    const std::string dbname = "test.db";
    auto db = webserver::SQLite3::Create(dbname, webserver::SQLite3::READWRITE);
    if(!db) {
        WEBSERVER_LOG_INFO(g_logger) << "dbname=" << dbname << " not exists";
        db = webserver::SQLite3::Create(dbname
                , webserver::SQLite3::READWRITE | webserver::SQLite3::CREATE);
        if(!db) {
            WEBSERVER_LOG_INFO(g_logger) << "dbname=" << dbname << " create error";
            return 0;
        }

#define XX(...) #__VA_ARGS__
        int rt = db->execute(
XX(create table user (
        id integer primary key autoincrement,
        name varchar(50) not null default "",
        age int not null default 0
        )));
#undef XX

        if(rt != SQLITE_OK) {
            WEBSERVER_LOG_ERROR(g_logger) << "create table error "
                << db->getErrorCode() << " - " << db->getErrorMsg();
            return 0;
        }
    }

    for(int i = 0; i < 10; ++i) {
        if(db->execute("insert into user(name, age) values(\"name_%d\",%d)", i, i)
                != SQLITE_OK) {
            WEBSERVER_LOG_ERROR(g_logger) << "insert into error " << i << " "
                << db->getErrorCode() << " - " << db->getErrorMsg();
        }
    }

    webserver::SQLite3Stmt::ptr stmt = webserver::SQLite3Stmt::Create(db,
                "insert into user(name, age) values(?, ?)");
    if(!stmt) {
        WEBSERVER_LOG_ERROR(g_logger) << "create statement error "
            << db->getErrorCode() << " - " << db->getErrorMsg();
        return 0;
    }

    for(int i = 0; i < 10; ++i) {
        stmt->bind(1, "stmt_" + std::to_string(i));
        stmt->bind(2, i);
        //stmt->bind(3, "stmt_" + std::to_string(i + 1));
        //stmt->bind(4, i + 1);

        if(stmt->execute() != SQLITE_OK) {
            WEBSERVER_LOG_ERROR(g_logger) << "execute statment error " << i << " "
                << db->getErrorCode() << " - " << db->getErrorMsg();
        }
        stmt->reset();
    }

    webserver::SQLite3Stmt::ptr query = webserver::SQLite3Stmt::Create(db,
            "select * from user");
    if(!query) {
        WEBSERVER_LOG_ERROR(g_logger) << "create statement error "
            << db->getErrorCode() << " - " << db->getErrorMsg();
        return 0;
    }
    auto ds = query->query();
    if(!ds) {
        WEBSERVER_LOG_ERROR(g_logger) << "query error "
            << db->getErrorCode() << " - " << db->getErrorMsg();
        return 0;
    }

    do {
        WEBSERVER_LOG_INFO(g_logger) << "query ";
    } while(ds->next());

    test_batch(db);
    return 0;
}
