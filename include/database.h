#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QtSql>

#include "common.h"

bool	init_db(QSqlDatabase& db, const m_settings& settings);

#endif // DATABASE_H
