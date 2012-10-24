/**
 * Project: nsrl-toolkit
 * File name: rds_import.h
 * Description: describes the main source file of rds_import
 *
 * @author Mathieu Grzybek on 2012-10-24
 * @copyright 2012 Mathieu Grzybek. All rights reserved.
 * @version $Id: code-gpl-license.txt,v 1.2 2004/05/04 13:19:30 garry Exp $
 *
 * @see The GNU Public License (GPL) version 3 or higher
 *
 *
 * forensics-data-extractor is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <cstdlib>
#include <iostream>
#include <stdio.h>

#include <QStringBuilder>
#include <QtSql>

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

/*
 * clean_nsrl_file_line
 *
 * Puts the missing commas (RDS 237 contains "ere" strings instead of '","')
 *
 * @arg	line	:	 the line to clean
 *
 */
void	clean_nsrl_file_line(QString& line);

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

/*
 * check_settings
 *
 * Deals with the saved settings according to the platform (using QSettings):
 * - hostname:	the remote database server
 * - database:	the name of the schema to use
 * - user:		the username
 * - burst:		the number of queries to commit at each commit
 */
void	check_settings();

/*
 * usage
 *
 * Prints the usage message to stdout
 */
void	usage();

