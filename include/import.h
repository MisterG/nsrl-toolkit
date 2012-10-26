#ifndef IMPORT_H
#define IMPORT_H

#include <QtSql>

#include "clean.h"
#include "common.h"
#include "database.h"

// Last to import
t_result	import_nsrl_file(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query, uint burst);

// First to import
t_result	import_nsrl_mfg(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query, uint burst);

// Second to import
t_result	import_nsrl_os(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query, uint burst);

// Third to import
t_result	import_nsrl_prod(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& product_query, uint burst);

#endif // IMPORT_H
