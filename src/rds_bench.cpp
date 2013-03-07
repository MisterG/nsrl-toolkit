/**
 * Project: nsrl-toolkit
 * File name: rds_bench.cpp
 * Description: describes the main source file of rds_bench
 *
 * @author Mathieu Grzybek on 2013-02-26
 * @copyright 2012 Mathieu Grzybek. All rights reserved.
 * @version $Id: code-gpl-license.txt,v 1.2 2004/05/04 13:19:30 garry Exp $
 *
 * @see The GNU Public License (GPL) version 3 or higher
 *
 *
 * nsrl-toolkit is free software; you can redistribute it and/or
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

#include "rds_bench.h"

int	main(int argc, char** argv) {
	if ( argc < 2 ) {
		std::cerr << "Arg is missing" << std::endl;
		usage();
		return EXIT_FAILURE;
	}

	/*
	 * Dealing with settings
	 */

	m_settings	settings;

	if ( argc > 2 ) {
		int c;
		while ((c = getopt (argc, argv, "h:u:p:d:s:f:")) != -1 ) {
			switch (c) {
				case 'h':
					settings.insert("hostname", optarg);
					break;
				case 'u':
					settings.insert("username", optarg);
					break;
				case 'p':
					settings.insert("password", optarg);
					break;
				case 'd':
					settings.insert("driver", optarg);
					break;
				case 's':
					settings.insert("database", optarg);
					break;
				case 'f':
					settings.insert("import_type", optarg);
					break;
				case '?':
					return EXIT_FAILURE;
			}
		}
	} else {
		QCoreApplication::setOrganizationName("nsrl_toolkit");
		QCoreApplication::setApplicationName("rds_bench");

	}

	if ( check_settings(settings) == false )
		return EXIT_FAILURE;

	QSqlDatabase	db;

	qint64		rows = 0;
	qint64		total_time = 0;

	// Create the db object
	if ( init_db(db, settings) == false )
		return EXIT_FAILURE;

	QSqlQuery	query_1(db);
	QSqlQuery	query_2(db);

	db.transaction();
	if ( db.driverName().compare("PGSQL") == 0 )
		start_pgsql(rows, total_time, db, query_1, query_2);
	if ( db.driverName().compare("MYSQL") == 0 )
		start_mysql(rows, total_time, db, query_1, query_2);
	db.rollback();

	/*
	 * Ending
	 */

	if ( rows > 0 ) {
		std::cout << "Elapsed time: " << total_time << " milliseconds (" << (float)total_time / 1000.0 << " seconds)" << std::endl;
		std::cout << "Processed lines: " << rows << std::endl;
		std::cout << "Speed: " << (float)rows / ((float)total_time / 1000.0 ) << " rows / second" << std::endl;
	} else {
		std::cout << "No line returned" << std::endl;
	}

	// Close and destroy the db object
	db.close();
	QSqlDatabase::removeDatabase(settings.value("driver"));

	return EXIT_SUCCESS;
}

bool	check_settings(const m_settings& settings) {
	if ( settings.contains("hostname") == false ) {
		std::cerr << "hostname is missing" << std::endl;
		return false;
	}

	if ( settings.contains("database") == false ) {
		std::cerr << "database is missing" << std::endl;
		return false;
	}

	if ( settings.contains("driver") == false ) {
		std::cerr << "driver is missing" << std::endl;
		return false;
	}

	if ( settings.contains("username") == false ) {
		std::cerr << "username is missing" << std::endl;
		return false;
	}

	return true;
}

bool	start_mysql(qint64 rows, qint64 total_time, QSqlDatabase& db, QSqlQuery& query_1, QSqlQuery& query_2) {
	if ( query_1.exec("SELECT sha1 FROM hash LIMIT 10000;") == false ) {
		std::cerr << "Cannot select the hashes: " << query_1.lastError().text().toLatin1().constData() << std::endl;
		db.rollback();
		query_1.finish();
		return false;
	}

	// Infinite loop!
	while ( query_1.next() == true ) {
		QElapsedTimer	timer;
		QString		sql_query = "SELECT count(*) FROM hash WHERE sha1='";

		sql_query += query_1.value(0).toString();
		sql_query += "';";

		rows++;

		// This is what we want to benchmark
		timer.start();
		query_2.exec(sql_query);
		total_time += timer.elapsed();

		query_2.finish();
	}

	query_1.finish();
	
	return true;
}

bool	start_pgsql(qint64 rows, qint64 total_time, QSqlDatabase& db, QSqlQuery& query_1, QSqlQuery& query_2) {
	if ( query_1.exec("DECLARE c_benchmark CURSOR FOR SELECT sha1 FROM hash LIMIT 10000;") == false ) {
		std::cerr << "Cannot declare the cursor: " << query_1.lastError().text().toLatin1().constData() << std::endl;
		db.rollback();
		query_1.finish();
		return false;
	}

	// Infinite loop!
	while ( true ) {
		if ( query_1.exec("FETCH NEXT FROM c_benchmark;") == false ) {
			std::cerr << "Cannot fetch from the cursor: " << query_1.lastError().text().toLatin1().constData() << std::endl;
			db.rollback();
			query_1.finish();
			query_2.finish();
			return false;
		}

		if ( query_1.next() == false )
			break;

		if ( query_1.isValid() == false ) {
			std::cerr << "The record is not valid: " << query_1.lastError().text().toLatin1().constData() << std::endl;
			db.rollback();
			query_1.finish();
			query_2.finish();
			return false;
		}

		QElapsedTimer	timer;
		QString		sql_query = "SELECT count(*) FROM hash WHERE sha1='";

		sql_query += query_1.value(0).toString();
		sql_query += "';";

		rows++;

		// This is what we want to benchmark
		timer.start();
		query_2.exec(sql_query);
		total_time += timer.elapsed();

		query_2.finish();
	}

	query_1.exec("CLOSE c_benchmark;");
	query_1.finish();

	return true;
}

void	usage() {
	std::cout << "rds_bench" << std::endl;
	std::cout << "	Tests database's response time using the given NSRL database according to the configuration file (according to QSettings' behaviour ~/.config/nsrl_toolkit/rds_bench or registry) or the given arguments" << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "	rds_bench [-h <hostname> -s <database> -d <driver> -u <username> -p <password>]" << std::endl;
	std::cout << "Settings / Arguments:" << std::endl;
	std::cout << "	hostname=	the target host" << std::endl;
	std::cout << "	database=	the database to use" << std::endl;
	std::cout << "	driver= 	the database's type, please refer to http://qt-project.org/doc/qt-5.0/qtsql/sql-driver.html" << std::endl;
	std::cout << "	username=	the login" << std::endl;
	std::cout << "	password=	the target host" << std::endl;
}

