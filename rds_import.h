#include <cstdlib>
#include <iostream>
#include <stdio.h>

#include <QStringBuilder>
#include <QtSql>

void	usage();

/*
 * Database
 */

bool	init_db(QSqlDatabase& db);
bool	burst_commit(QSqlDatabase& db, uint burst);

/*
 * Files processing
 */

// Last to import
bool	import_nsrl_file(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query, uint burst);

// First to import
bool	import_nsrl_mfg(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query, uint burst);

// Second to import
bool	import_nsrl_os(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query, uint burst);

// Third to import
bool	import_nsrl_prod(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& product_query, uint burst);

/*
 * Main
 */

int		main(int argc, char* argv[]);
void	check_settings();
