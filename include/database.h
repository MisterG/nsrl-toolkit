#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QtSql>

#include "common.h"

bool	init_db(QSqlDatabase& db);
bool	burst_commit(QSqlDatabase& db, uint burst);

#endif // DATABASE_H
