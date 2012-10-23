#include "rds_import.h"

void	usage() {
	std::cout << "rds_import" << std::endl;
	std::cout << "	Imports NSRL CSV files to a remote database, according to the configuration file (~/.config/nsrl_toolkit/rds_import)" << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "	zcat nsrlfiletxt.zip | tail -n +2 | rds_import file" << std::endl;
	std::cout << "	tail -n +2 nsrlmfg.txt | rds_import mfg" << std::endl;
	std::cout << "	tail -n +2 nsrlos.txt | rds_import os" << std::endl;
	std::cout << "	tail -n +2 nsrlprod.txt | rds_import prod" << std::endl;
}

bool	init_db(QSqlDatabase& db) {
	db = QSqlDatabase::addDatabase("QMYSQL"); // TODO: set the URI into the settings
	QSettings	settings;

	db.setHostName(settings.value("hostname").toString());
	db.setDatabaseName(settings.value("database").toString());
	db.setUserName(settings.value("username").toString());

	if ( settings.contains("password") == true )
		db.setPassword(settings.value("password").toString());

	if ( db.open() == false ) {
		std::cerr << "Opening db failed: " << db.lastError().text().toStdString() << std::endl;
		return false;
	}

	return true;
}

bool	burst_commit(QSqlDatabase& db, const uint burst) {
	static uint	counter = 0;

	if ( counter < burst) {
		counter++;
		return true;
	} else {
		counter = 0;
		if ( db.commit() == true ) {
			db.transaction();
			return true;
		}
	}

	return false;
}

bool	import_nsrl_file(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query, const uint burst) {
	db.transaction();

	while ( not q_stdin.atEnd() ) {
		QString line = q_stdin.readLine().simplified();
		QString	sql;
		QString buffer;

		if ( line.length() > 1 ) {
			QStringList	fields;

//			std::cout << "line: " << line.toStdString() << std::endl;
			/*
			 * Let's extract the first five field ourselves
			 * We could use regexp instead
			 */
			// sha1
			buffer = line.remove(0,1);
			buffer = line.left(line.indexOf('"'));
			line.remove(0, line.indexOf('"') + 3);
//			std::cout << "sha1: " << buffer.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;
			fields << buffer;

			// md5
			buffer = line.left(line.indexOf('"'));
			line.remove(0, line.indexOf('"') + 3);
//			std::cout << "md5: " << buffer.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;
			fields << buffer;

			// crc32
			buffer = line.left(line.indexOf('"'));
			line.remove(0, line.indexOf('"') + 3);
//			std::cout << "crc32: " << buffer.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;
			fields << buffer;

			// file_name
			buffer = line.left(line.indexOf('"'));
			buffer.replace("'", "\\'");
			line.remove(0, line.indexOf('"') + 2);
//			std::cout << "file_name: " << buffer.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;
			fields << buffer;

			// file_size
			buffer = line.left(line.indexOf(','));
			line.remove(0, line.indexOf(',') + 1);
//			std::cout << "file_size: " << buffer.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;
			fields << buffer;

			// product code (always an int)
			buffer = line.left(line.indexOf(','));
			line.remove(0, line.indexOf(',') + 2);
//			std::cout << "product_code: " << buffer.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;
			fields << buffer;

			// system code (string)
			buffer = line.left(line.indexOf("\",\""));
			line.remove(0, line.indexOf("\",\"") + 3);
//			std::cout << "system_code: " << buffer.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;
			fields << buffer;

			// special code
			buffer = line.remove(0,2);
			buffer = line.left(line.length() - 1);
//			std::cout << "special_code: " << buffer.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;
			fields << buffer;

			/*
			sha1			fields.at(0)
			md5				fields.at(1)
			crc32			fields.at(2)
			file_name		fields.at(3)
			file_size		fields.at(4)
			product_code	fields.at(5)
			op_system_code	fields.at(6)
			special_code	fields.at(7)
			*/

			sql = "INSERT IGNORE INTO hash (sha1, md5, crc32) VALUES ('";
			sql += fields.at(0) % "','";
			sql += fields.at(1) % "','";
			sql += fields.at(2);
			sql += "');";
//			std::cout << sql.toStdString() << std::endl;

			if ( query.exec(sql) == false ) {
				std::cerr << "query: " << sql.toStdString() << std::endl;
				std::cerr << "exec error: " << query.lastError().text().toStdString() << std::endl;
				return false;
			}

			sql = "INSERT IGNORE INTO file (file_name, file_size, product_code, op_system_code, special_code, hash_sha1) VALUES ('";
			sql += fields.at(3) % "','";
			sql += fields.at(4) % "','";
			sql += fields.at(5) % "','";
			sql += fields.at(6) % "','";
			sql += fields.at(7) % "','";
			sql += fields.at(0);
			sql += "');";
//			std::cout << sql.toStdString() << std::endl;

			if ( query.exec(sql) == false ) {
				std::cerr << "query: " << sql.toStdString() << std::endl;
				std::cerr << "exec error: " << query.lastError().text().toStdString() << std::endl;
				return false;
			}

			if ( burst_commit(db, burst) == false ) {
				std::cerr << "transaction error: " << db.lastError().text().toStdString() << std::endl;
				return false;
			}
		}
	}

	if ( db.commit() == false ) {
		std::cerr << "transaction error: " << db.lastError().text().toStdString() << std::endl;
		return false;
	}

	return true;
}

bool	import_nsrl_mfg(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query, const uint burst) {
	db.transaction();
	query.prepare("INSERT IGNORE INTO mfg (code, name) VALUES (:code, :name);");
	uint	line_counter = 0;

	while ( not q_stdin.atEnd() ) {
		QString		buffer;
		QString		line = q_stdin.readLine().simplified();
		QStringList	fields;

		// Skip useless lines
		if ( line.indexOf(",") == -1 )
			continue;

		line_counter++;

//		line.replace("\"", "");
		std::cout << "line: " << line.toStdString() << std::endl;

		// code
		line.remove(0,1);
		buffer = line.left(line.indexOf("\",\""));
		line.remove(0, line.indexOf("\",\"") + 3);
		std::cout << "code: " << buffer.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;
		fields << buffer;

		// name
		line.replace('"', "");
		fields << line;
		std::cout << "name: " << line.toStdString() << std::endl << "line: " << line.toStdString() << std::endl;

		if ( fields.size() != 2 ) {
			std::cerr << "error: cannot extract the 2 required fields from: " << line.toStdString() << std::endl;
			return false;
		}

		/*
		code	fields.at(0)
		name	fields.at(1)
		*/

		query.bindValue(":code", fields.at(0));
		query.bindValue(":name", fields.at(1));

		if ( query.exec() == false ) {
			std::cerr << "exec error: " << query.lastError().text().toStdString() << std::endl;
			return false;
		}

		if ( burst_commit(db, burst) == false ) {
			std::cerr << "transaction error: " << db.lastError().text().toStdString() << std::endl;
			return false;
		}
	}

	if ( db.commit() == false ) {
		std::cerr << "transaction error: " << db.lastError().text().toStdString() << std::endl;
		return false;
	}

	query.exec("SELECT COUNT(*) FROM mfg LIMIT 1;");

	if (query.next()) {
		if ( query.value(0).toUInt() != line_counter ) {
			std::cerr << "error: missing lines: " << line_counter << " processed but " << query.value(0).toUInt() << " in the table" << std::endl;
			return false;
		}
	} else {
		std::cerr << "error: cannot check the number of inserted lines" << std::endl;
		return false;
	}

	return true;
}

bool	import_nsrl_os(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query, const uint burst) {
	db.transaction();
	query.prepare("INSERT INTO os (system_code, system_name, system_version, mfg_code) VALUES (:code, :name, :version, :mfg_code);");
	uint	line_counter = 0;

	while ( not q_stdin.atEnd() ) {
		QString line = q_stdin.readLine().simplified();

		// Skip useless lines
		if ( line.indexOf(",") == -1 )
			continue;

		line_counter++;

		line.replace(QRegExp("\"$"), "");
		line.replace(QRegExp("^\""), "");
		QStringList fields = line.split("\",\"");

		/*
		system_code		fields.at(0)
		system_name		fields.at(1)
		system_version	fields.at(2)
		mfg_code		fields.at(3)
		*/

		query.bindValue(":code", fields.at(0));
		query.bindValue(":name", fields.at(1));
		query.bindValue(":version", fields.at(2));
		query.bindValue(":mfg_code", fields.at(3));

		if ( query.exec() == false ) {
			std::cerr << "exec error: " << query.lastError().text().toStdString() << std::endl;
			std::cerr << "line: " << line.toStdString() << std::endl;
			std::cerr << query.executedQuery().toStdString() << std::endl;
			Q_FOREACH(QString key, query.boundValues().keys()) {
				std::cerr << key.toStdString() << ":" << query.boundValue(key).toString().toStdString()  << ":" << std::endl;
			}
			return false;
		}

		if ( burst_commit(db, burst) == false ) {
			std::cerr << "transaction error: " << db.lastError().text().toStdString() << std::endl;
			return false;
		}
	}

	if ( db.commit() == false ) {
		std::cerr << "transaction error: " << db.lastError().text().toStdString() << std::endl;
		return false;
	}

	query.exec("SELECT COUNT(*) FROM os LIMIT 1;");

	if (query.next()) {
		if ( query.value(0).toUInt() != line_counter ) {
			std::cerr << "error: missing lines: " << line_counter << " processed but " << query.value(0).toUInt() << " in the table" << std::endl;
			return false;
		}
	} else {
		std::cerr << "error: cannot check the number of inserted lines" << std::endl;
		return false;
	}

	return true;
}

bool	import_nsrl_prod(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& product_query, const uint burst) {
	QSqlQuery	link_query(db);
	uint	line_counter = 0;

	db.transaction();

	// We insert duplicates from the file but we  use the second table to link against the os
	product_query.prepare("INSERT IGNORE INTO product (product_code, product_name, product_version, mfg_code, language, application_type) VALUES (:code, :name, :version, :mfg_code, :language, :application_type);");
	link_query.prepare("INSERT IGNORE INTO product_has_os (product_code, system_code) VALUES (:product_code, :system_code);");

	while ( not q_stdin.atEnd() ) {
		QString line = q_stdin.readLine().simplified();

		if ( line.indexOf(",") == -1 )
			continue;

		line_counter++;

		QStringList	fields;

		// Let's extract the first field ourselves
		fields << line.left(line.indexOf(","));
		line.remove(0,line.indexOf(",") + 2);

		// Then we clean the remaining string to make a clean split
//		line.replace(QRegExp("^\""), "");
		line.replace(QRegExp("\"\n$"), "");

		fields << line.split("\",\"");

		/*
		product_code		fields.at(0)
		product_name		fields.at(1)
		product_version		fields.at(2)
		os_code				fields.at(3)
		mfg_code			fields.at(4)
		language			fields.at(5)
		application_type	fields.at(6)
		*/

		product_query.bindValue(":code", fields.value(0).replace("\"", ""));
		product_query.bindValue(":name", fields.value(1).replace("\"", ""));
		product_query.bindValue(":version", fields.value(2).replace("\"", ""));
		product_query.bindValue(":mfg_code", fields.value(4).replace("\"", ""));
		product_query.bindValue(":language", fields.value(5).replace("\"", ""));
		product_query.bindValue(":application_type", fields.value(6).replace("\"", ""));

		if ( product_query.exec() == false ) {
			qCritical() << "product_query exec error: " << product_query.lastError().text();
			qCritical() << "line: " << line;
			qCritical() << product_query.boundValues();
			return false;
		}

		link_query.bindValue(":product_code", fields.value(0).replace("\"", ""));
		link_query.bindValue(":system_code", fields.value(3).replace("\"", ""));

		if ( link_query.exec() == false ) {
			qCritical() << "link_query exec error: " << product_query.lastError().text();
			qCritical() << "line: " << line;
			qCritical() << link_query.boundValues();
			return false;
		}

		if ( burst_commit(db, burst) == false ) {
			std::cerr << "transaction error: " << db.lastError().text().toStdString() << std::endl;
			return false;
		}
	}

	if ( db.commit() == false ) {
		std::cerr << "transaction error: " << db.lastError().text().toStdString() << std::endl;
		return false;
	}

	// Not sure of this query
	product_query.exec("select sum(t.somme) from (select count(*) as somme from product p, product_has_os h where p.product_code = h.product_code group by p.product_code) t limit 1;");

	if (product_query.next()) {
		if ( product_query.value(0).toUInt() != line_counter ) {
			std::cerr << "error: missing lines: " << line_counter << " processed but " << product_query.value(0).toUInt() << " in the table" << std::endl;
			return false;
		}
	} else {
		std::cerr << "error: cannot check the number of inserted lines" << std::endl;
		return false;
	}

	return true;
}

int	main(int argc, char* argv[]) {
	if ( argc != 2 ) {
		std::cerr << "Arg is missing" << std::endl;
		usage();
		return EXIT_FAILURE;
	}

	/*
	 * Dealing with settings
	 */
	QCoreApplication::setOrganizationName("nsrl_toolkit");
	QCoreApplication::setApplicationName("rds_import");

	QSettings	settings;
	check_settings();

	QSqlDatabase	db;
	bool			result = false;

	// Create the db object
	if ( init_db(db) == false )
		return EXIT_FAILURE;

	QSqlQuery	query(db);

	// Open stdin
	QFile q_stdin;

	if ( not q_stdin.open(stdin, QIODevice::ReadOnly) ) {
		std::cerr << "Cannot open stdin" << std::endl;
		return EXIT_FAILURE;
	}

	/*
	 * Let's choose the tables to update
	 */

	if ( QString(argv[1]).compare("file") == 0 )
		result = import_nsrl_file(q_stdin, db, query, settings.value("burst").toUInt());

	if ( QString(argv[1]).compare("mfg") == 0 )
		result = import_nsrl_mfg(q_stdin, db, query, settings.value("burst").toUInt());

	if ( QString(argv[1]).compare("os") == 0 )
		result = import_nsrl_os(q_stdin, db, query, settings.value("burst").toUInt());

	if ( QString(argv[1]).compare("prod") == 0 )
		result = import_nsrl_prod(q_stdin, db, query, settings.value("burst").toUInt());

	/*
	 * Ending
	 */
	q_stdin.close();

	// Close and destroy the db object
	db.close();
	QSqlDatabase::removeDatabase("MYSQL");

	if ( result == true )
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

void	check_settings() {
	QSettings	settings;

	// database connection
	if ( settings.contains("hostname") == false )
		settings.setValue("hostname", "localhost");

	if ( settings.contains("database") == false )
		settings.setValue("database", "nsrl");

	if ( settings.contains("username") == false )
		settings.setValue("username", "nsrl");

//	if ( settings.contains("password") == false )
//		settings.setValue("password", "");

	// burst counter
	if ( settings.contains("burst") == false )
		settings.setValue("burst", 1000);
}
