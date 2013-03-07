#include "database.h"

bool	init_db(QSqlDatabase& db, const m_settings& settings) {
    db = QSqlDatabase::addDatabase(settings.value("driver"));

    db.setHostName(settings.value("hostname"));
    db.setDatabaseName(settings.value("database"));
    db.setUserName(settings.value("username"));

    if ( settings.contains("password") == true )
        db.setPassword(settings.value("password"));

    if ( db.open() == false ) {
#ifdef QT_5
        std::cerr << "Opening db failed: " << db.lastError().text().toLatin1().constData() << std::endl;
#else
        std::cerr << "Opening db failed: " << db.lastError().text().toAscii().constData() << std::endl;
#endif
        return false;
    }

    return true;
}

