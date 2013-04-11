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
		while ((c = getopt (argc, argv, "h:u:p:d:s:t:")) != -1 ) {
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
				case '?':
					return EXIT_FAILURE;
			}
		}
	} else {
		QCoreApplication::setOrganizationName("nsrl_toolkit");
		QCoreApplication::setApplicationName("products");

	}

	if ( check_settings(settings) == false )
		return EXIT_FAILURE;

	QSqlDatabase	db;
    QString         sql;

//	QElapsedTimer	timer;

	// Create the db object
	if ( init_db(db, settings) == false )
		return EXIT_FAILURE;

	QSqlQuery	query(db);

	// Open stdin
	QFile	q_stdin;

	if ( not q_stdin.open(stdin, QIODevice::ReadOnly) ) {
		std::cerr << "Cannot open stdin" << std::endl;
		return EXIT_FAILURE;
	}

    while ( not q_stdin.atEnd() ) {
        QString line = q_stdin.readLine().simplified();

        if ( settings.value("checksum_type").compare("sha1") == 0 ) {
            sql = "SELECT DISTINCT p.product_name FROM product p, file f WHERE f.product_code = p.product_code AND f.hash_sha1 = '";
        } else {
            sql = "SELECT DISTINCT p.product_name FROM product p, file f, hash h WHERE f.product_code = p.product_code AND h.sha1 = f.hash_sha1 AND h.md5 = '";
        }
        sql += line.toUpper();
        sql += "' ORDER BY p.product_name ASC;";

        query.exec(sql);

        if ( query.next() ) {
            std::cout << query.value(0).toString().toLatin1().constData() << std::endl;
        }
    }

	/*
	 * Ending
	 */
	q_stdin.close();

	// Close and destroy the db object
	db.close();
	QSqlDatabase::removeDatabase("MYSQL");

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
	std::cout << "	nsrl_products -t <checksum_type> (-h <hostname> -s <database> -d <driver> -u <username> [-p <password>])" << std::endl;
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
}

