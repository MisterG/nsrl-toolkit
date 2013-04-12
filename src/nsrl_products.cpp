/**
 * Project: nsrl-toolkit
 * File name: nsrl_products.cpp
 * Description: describes the main source file of nsrl_products
 *
 * @author Mathieu Grzybek on 2013-04-11
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

#include "nsrl_products.h"

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
		while ((c = getopt (argc, argv, "h:u:p:d:s:t:f")) != -1 ) {
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
				case 't':
					settings.insert("checksum_type", optarg);
					break;
				case 'f':
					settings.insert("full_print", "true");
					break;
				case '?':
					return EXIT_FAILURE;
			}
		}
	} else {
		QCoreApplication::setOrganizationName("nsrl_toolkit");
		QCoreApplication::setApplicationName("products");

	}

	/*
	 * Settings checking
	 */

	if ( check_settings(settings) == false )
		return EXIT_FAILURE;

	/*
	 * Processing
	 */

	if ( process_input(settings) == true )
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

bool	process_input(const m_settings& settings) {
	QSqlDatabase	db;
	QString     sql;
	QFile	    q_stdin;

	// Create the db object
	// TODO: prepare the statement
	if ( init_db(db, settings) == false )
		return EXIT_FAILURE;

	QSqlQuery	query(db);

	if ( not q_stdin.open(stdin, QIODevice::ReadOnly) ) {
		std::cerr << "Cannot open stdin" << std::endl;
		return EXIT_FAILURE;
	}

	while ( not q_stdin.atEnd() ) {
		QString line = q_stdin.readLine().simplified();

		build_sql(sql, line.toUpper(), settings);

		if ( sql.isEmpty() == true ) {
			std::cerr << "Cannot build the SQL query" << std::endl;
			return false;
		}

		if ( query.exec(sql) == false ) {
			std::cerr << "Cannot execute the query" << std::endl;
			return false;
		}

		if ( query.next() ) {
			if ( settings.contains("full_print") == true ) {
				std::cout << query.value(0).toString().toLatin1().constData() << "\t";
				std::cout << query.value(1).toString().toLatin1().constData() << "\t";
				std::cout << query.value(2).toString().toLatin1().constData() << std::endl;
			} else {
				std::cout << query.value(0).toString().toLatin1().constData() << std::endl;
			}
		}
	}

	// Let's close / destroy the objects
	q_stdin.close();
	db.close();
	QSqlDatabase::removeDatabase("MYSQL");

	return true;
}

void	build_sql(QString& _return, const QString& hash_value, const m_settings& settings) {
	if ( settings.contains("full_print")  == true )
		if ( settings.value("checksum_type").compare("sha1") == 0 )
			_return = "SELECT DISTINCT h.sha1, f.file_name, p.product_name ";
		else
			_return = "SELECT DISTINCT h.md5, f.file_name, p.product_name ";
	else
		_return = "SELECT DISTINCT p.product_name ";

	if ( settings.value("checksum_type").compare("sha1") == 0 ) {
		_return += " FROM product p, file f WHERE f.product_code = p.product_code AND f.hash_sha1 = '";
	} else {
		_return += " FROM product p, file f, hash h WHERE f.product_code = p.product_code AND h.sha1 = f.hash_sha1 AND h.md5 = '";
	}

	_return += hash_value.toUpper();
	_return += "' ORDER BY p.product_name ASC;";
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

	if ( settings.contains("checksum_type") == false or settings.value("checksum_type").isEmpty() == true ) {
		std::cerr << "checksum_typeis missing" << std::endl;
		return false;
	}

	if ( settings.value("checksum_type").compare("md5") != 0 and settings.value("checksum_type").compare("sha1") != 0 ) {
		std::cerr << "checksum_type must be either \"md5\" or \"sha1\"" << std::endl;
		return false;
	}

	return true;
}

void	usage() {
	std::cout << "nsrl_products" << std::endl;
	std::cout << "	Grabs the NSRL products' list according to the given input using a database, according to the configuration file (according to QSettings' behaviour ~/.config/nsrl_toolkit/rds_import or registry) or the given arguments" << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "	nsrl_products [-f] -t <checksum_type> (-h <hostname> -s <database> -d <driver> -u <username> [-p <password>])" << std::endl;
	std::cout << "Synopsys:" << std::endl;
	std::cout << "	cat md5_list | nsrl_products | sort -u" << std::endl;
	std::cout << "Settings / Arguments:" << std::endl;
	std::cout << "	hostname=	the target host" << std::endl;
	std::cout << "	database=	the database to use" << std::endl;
	std::cout << "	driver= 	the database's type, please refer to http://qt-project.org/doc/qt-5.0/qtsql/sql-driver.html" << std::endl;
	std::cout << "	username=	the login" << std::endl;
	std::cout << "	password=	the target host" << std::endl;
	std::cout << "Arguments:" << std::endl;
	std::cout << "	checksum_type=	the checksum to copare (md5 or sha1)" << std::endl;
	std::cout << "	full_print=	prints the whole fields, not only the product's name" << std::endl;
}

