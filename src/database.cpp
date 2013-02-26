#include "database.h"

bool	init_db(QSqlDatabase& db) {
    QSettings	settings;
    db = QSqlDatabase::addDatabase(settings.value("driver").toString());

	db.setHostName(settings.value("hostname").toString());
	db.setDatabaseName(settings.value("database").toString());
	db.setUserName(settings.value("username").toString());

	if ( settings.contains("password") == true )
		db.setPassword(settings.value("password").toString());

	if ( db.open() == false ) {
		std::cerr << "Opening db failed: " << db.lastError().text().toAscii().constData() << std::endl;
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

