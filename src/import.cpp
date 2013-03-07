#include "import.h"

t_result	import_nsrl_file(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query) {
    db.transaction();
    t_result	result;

    result.success = false;
    result.processed_lines = 0;

    while ( not q_stdin.atEnd() ) {
        QString line = q_stdin.readLine().simplified();
        QString	sql;
        QString buffer;
	QStringList	fields;

	if ( extract_file(fields, line, *db.driver()) == true ) {
            /*
               sha1		fields.at(0)
               md5		fields.at(1)
               crc32		fields.at(2)
               file_name	fields.at(3)
               file_size	fields.at(4)
               product_code	fields.at(5)
               op_system_code	fields.at(6)
               special_code	fields.at(7)
             */

            if ( db.driverName().compare("QPSQL") != 0 ) {
                sql = "REPLACE INTO hash (sha1, md5, crc32) VALUES ('";
            } else {
                sql = "SELECT upsert_hash('";
            }
            sql += fields.at(0) % "','";
            sql += fields.at(1) % "','";
            sql += fields.at(2);
            sql += "');";

            if ( query.exec(sql) == false ) {
                std::cerr << "result.processed_lines:" << result.processed_lines << std::endl;
#ifdef QT_5
                std::cerr << "query: " << sql.toLatin1().constData() << std::endl;
                std::cerr << "exec error: " << query.lastError().text().toLatin1().constData() << std::endl;
#else
                std::cerr << "query: " << sql.toAscii().constData() << std::endl;
                std::cerr << "exec error: " << query.lastError().text().toAscii().constData() << std::endl;
#endif
                return result;
            }

            if ( db.driverName().compare("QPSQL") != 0 ) {
                sql = "REPLACE INTO file (file_name, file_size, product_code, op_system_code, special_code, hash_sha1) VALUES ('";
            } else {
                sql = "SELECT upsert_file('";
            }
            sql += fields.at(3) % "','";
            sql += fields.at(4) % "','";
            sql += fields.at(5) % "','";
            sql += fields.at(6) % "','";
            sql += fields.at(7) % "','";
            sql += fields.at(0);
            sql += "');";

            if ( query.exec(sql) == false ) {
#ifdef QT_5
                std::cerr << "query: " << sql.toLatin1().constData() << std::endl;
                std::cerr << "exec error: " << query.lastError().text().toLatin1().constData() << std::endl;
#else
                std::cerr << "query: " << sql.toAscii().constData() << std::endl;
                std::cerr << "exec error: " << query.lastError().text().toAscii().constData() << std::endl;
#endif
                return result;
            }
		fields.clear();
        }
    }

    if ( db.commit() == false ) {
#ifdef QT_5
        std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
#else
        std::cerr << "transaction error: " << db.lastError().text().toAscii().constData() << std::endl;
#endif
        return result;
    }

    result.success = true;
    return result;
}

t_result	import_nsrl_mfg(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query) {
    db.transaction();

    if ( db.driverName().compare("QPSQL") != 0 )
        query.prepare("INSERT IGNORE INTO mfg (code, name) VALUES (:code, :name);");
    else
        query.prepare("SELECT upsert_mfg(:code, :name)");

    t_result	result;

    result.success = false;
    result.processed_lines = 0;

    while ( not q_stdin.atEnd() ) {
        QString		buffer;
        QString		line = q_stdin.readLine().simplified();
        QStringList	fields;

        // Skip useless lines
        if ( line.indexOf(",") == -1 )
            continue;

        result.processed_lines++;

        // code
        line.remove(0,1);
        buffer = line.left(line.indexOf("\",\""));
        line.remove(0, line.indexOf("\",\"") + 3);
        fields << buffer;

        // name
        line.replace('"', "");
        fields << line;

        if ( fields.size() != 2 ) {
#ifdef QT_5
            std::cerr << "error: cannot extract the 2 required fields from: " << line.toLatin1().constData() << std::endl;
#else
            std::cerr << "error: cannot extract the 2 required fields from: " << line.toAscii().constData() << std::endl;
#endif
            return result;
        }

        /*
           code	fields.at(0)
           name	fields.at(1)
         */

        query.bindValue(":code", fields.at(0));
        query.bindValue(":name", fields.at(1));

        if ( query.exec() == false ) {
#ifdef QT_5
            std::cerr << "query: " << query.executedQuery().toLatin1().constData() << std::endl;
            std::cerr << "exec error: " << query.lastError().text().toLatin1().constData() << std::endl;
#else
            std::cerr << "query: " << query.executedQuery().toAscii().constData() << std::endl;
            std::cerr << "exec error: " << query.lastError().text().toAscii().constData() << std::endl;
#endif
            return result;
        }
    }

    query.finish();

    if ( db.commit() == false ) {
#ifdef QT_5
        std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
#else
        std::cerr << "transaction error: " << db.lastError().text().toAscii().constData() << std::endl;
#endif
        return result;
    }

    query.exec("SELECT COUNT(*) FROM mfg LIMIT 1;");

    if (query.next()) {
        if ( query.value(0).toUInt() != result.processed_lines ) {
            std::cerr << "error: missing lines: " << result.processed_lines << " processed but " << query.value(0).toUInt() << " in the table" << std::endl;
            return result;
        }
    } else {
        std::cerr << "error: cannot check the number of inserted lines" << std::endl;
        return result;
    }

    result.success = true;
    return result;
}

t_result	import_nsrl_os(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query) {
    db.transaction();

    if ( db.driverName().compare("QPSQL") != 0 )
        query.prepare("INSERT INTO os (system_code, system_name, system_version, mfg_code) VALUES (:code, :name, :version, :mfg_code);");
    else
        query.prepare("SELECT upsert_os(:code, :name, :version, :mfg_code);");

    t_result	result;

    result.processed_lines = 0;
    result.success = false;

    while ( not q_stdin.atEnd() ) {
        QString line = q_stdin.readLine().simplified();

        // Skip useless lines
        if ( line.indexOf(",") == -1 )
            continue;

        result.processed_lines++;

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
#ifdef QT_5
            std::cerr << "exec error: " << query.lastError().text().toLatin1().constData() << std::endl;
            std::cerr << "line: " << line.toLatin1().constData() << std::endl;
            std::cerr << query.executedQuery().toLatin1().constData() << std::endl;
#else
            std::cerr << "exec error: " << query.lastError().text().toAscii().constData() << std::endl;
            std::cerr << "line: " << line.toAscii().constData() << std::endl;
            std::cerr << query.executedQuery().toAscii().constData() << std::endl;
#endif
            Q_FOREACH(QString key, query.boundValues().keys()) {
#ifdef QT_5
                std::cerr << key.toLatin1().constData() << ":" << query.boundValue(key).toString().toLatin1().constData()  << ":" << std::endl;
#else
                std::cerr << key.toAscii().constData() << ":" << query.boundValue(key).toString().toAscii().constData()  << ":" << std::endl;
#endif
            }
            return result;
        }
    }

    if ( db.commit() == false ) {
#ifdef QT_5
        std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
#else
        std::cerr << "transaction error: " << db.lastError().text().toAscii().constData() << std::endl;
#endif
        return result;
    }

    query.exec("SELECT COUNT(*) FROM os LIMIT 1;");

    if (query.next()) {
        if ( query.value(0).toUInt() != result.processed_lines ) {
            std::cerr << "error: missing lines: " << result.processed_lines << " processed but " << query.value(0).toUInt() << " in the table" << std::endl;
            return result;
        }
    } else {
        std::cerr << "error: cannot check the number of inserted lines" << std::endl;
        return result;
    }

    result.success = true;
    return result;
}

t_result	import_nsrl_prod(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& product_query) {
    QSqlQuery	link_query(db);
    t_result	result;

    result.processed_lines = 0;
    result.success = false;

    db.transaction();

    // We insert duplicates from the file but we use the second table to link against the os
    if ( db.driverName().compare("QPSQL") != 0 ) {
        product_query.prepare("INSERT IGNORE INTO product (product_code, product_name, product_version, mfg_code, language, application_type) VALUES (:code, :name, :version, :mfg_code, :language, :application_type);");
        link_query.prepare("INSERT IGNORE INTO product_has_os (product_code, system_code) VALUES (:product_code, :system_code);");
    } else {
        if ( product_query.prepare("SELECT upsert_product(:code, :name, :version, :mfg_code, :language, :application_type);") == false ) {
#ifdef QT_5
            std::cerr << "Unable to prepare upsert_product: " << db.lastError().text().toLatin1().constData() << std::endl;
#else
            std::cerr << "Unable to prepare upsert_product: " << db.lastError().text().toAscii().constData() << std::endl;
#endif
            db.rollback();
            return result;
        }
        if ( link_query.prepare("SELECT upsert_product_has_os(:product_code, :system_code);") == false ) {
#ifdef QT_5
            std::cerr << "Unable to prepare upsert_product_has_os: " << db.lastError().text().toLatin1().constData() << std::endl;
#else
            std::cerr << "Unable to prepare upsert_product_has_os: " << db.lastError().text().toAscii().constData() << std::endl;
#endif
            db.rollback();
            return result;
        }
    }

    while ( not q_stdin.atEnd() ) {
        QString line = q_stdin.readLine().simplified();

        if ( line.indexOf(",") == -1 )
            continue;

        result.processed_lines++;

        QStringList	fields;

        // Let's extract the first field ourselves
        fields << line.left(line.indexOf(","));
        line.remove(0, line.indexOf(",") + 2);

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
            db.rollback();
            return result;
        }

        link_query.bindValue(":product_code", fields.value(0).replace("\"", ""));
        link_query.bindValue(":system_code", fields.value(3).replace("\"", ""));

        if ( link_query.exec() == false ) {
            qCritical() << "link_query exec error: " << product_query.lastError().text();
            qCritical() << "line: " << line;
            qCritical() << link_query.boundValues();
            db.rollback();
            return result;
        }
    }

    if ( db.commit() == false ) {
#ifdef QT_5
        std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
#else
        std::cerr << "transaction error: " << db.lastError().text().toAscii().constData() << std::endl;
#endif
        return result;
    }

    // Not sure of this query
    product_query.exec("select sum(t.somme) from (select count(*) as somme from product p, product_has_os h where p.product_code = h.product_code group by p.product_code) t limit 1;");

    if (product_query.next()) {
        if ( product_query.value(0).toUInt() != result.processed_lines ) {
            std::cerr << "error: missing lines: " << result.processed_lines << " processed but " << product_query.value(0).toUInt() << " in the table" << std::endl;
            return result;
        }
    } else {
        std::cerr << "error: cannot check the number of inserted lines" << std::endl;
        return result;
    }

    result.success = true;
    return result;
}

