#include "import.h"

t_result	import_nsrl_file(QFile& q_stdin, QSqlDatabase& db, QSqlQuery& query) {
    db.transaction();
    t_result	result;

    result.success = false;
    result.processed_lines = 0;

    while ( ! q_stdin.atEnd() ) {
        QString line = q_stdin.readLine().simplified();
        QString	sql;
        QString buffer;

        // Skip useless lines
        if ( line.indexOf(",") == -1 )
            continue;

        if ( line.length() > 1 ) {
            QStringList	fields;
            clean_nsrl_file_line(line);

            result.processed_lines++;

            //std::cout << "line: " << line.toLatin1().constData() << std::endl;

            /*
             * Let's extract the first five field ourselves
             * We could use regexp instead
             */
            // sha1
            buffer = line.remove(0,1);
            buffer = line.left(line.indexOf('"'));
            line.remove(0, line.indexOf('"') + 3);
            //			std::cout << "sha1: " << buffer.toLatin1().constData() << std::endl << "line: " << line.toLatin1().constData() << std::endl;
            if ( buffer.size() != 40 ) {
                std::cerr << "line: " << result.processed_lines << std::endl;
                std::cerr << "sha1's length is not 40 characters!" << std::endl;
                return result;
            }
            fields << buffer;

            // md5
            buffer = line.left(line.indexOf('"'));
            line.remove(0, line.indexOf('"') + 3);
            //			std::cout << "md5: " << buffer.toLatin1().constData() << std::endl << "line: " << line.toLatin1().constData() << std::endl;
            if ( buffer.size() != 32 ) {
                std::cerr << "line: " << result.processed_lines << std::endl;
                std::cerr << "md5's length is not 32 characters!" << std::endl;
                return result;
            }
            fields << buffer;

            // crc32
            buffer = line.left(line.indexOf('"'));
            line.remove(0, line.indexOf('"') + 3);
            //			std::cout << "crc32: " << buffer.toLatin1().constData() << std::endl << "line: " << line.toLatin1().constData() << std::endl;
            if ( buffer.size() != 8 ) {
                std::cerr << "line: " << result.processed_lines << std::endl;
                std::cerr << "crc's length is not 8 characters!" << std::endl;
                return result;
            }
            fields << buffer;

            // file_name
            buffer = line.left(line.indexOf('"'));
            //buffer.replace("'", "\\'");
#ifdef QT_5
            QSqlField   f_buffer("buffer", QVariant::String);
            f_buffer.setValue(buffer);
            buffer = db.driver()->formatValue(f_buffer);
#else
            buffer = db.driver()->formatValue(buffer);
#endif
            line.remove(0, line.indexOf('"') + 2);
            fields << buffer;

            // file_size
            buffer = line.left(line.indexOf(','));
            line.remove(0, line.indexOf(',') + 1);
            fields << buffer;

            // product code (always an int)
            buffer = line.left(line.indexOf(','));
            line.remove(0, line.indexOf(',') + 2);
            fields << buffer;

            // system code (string)
            buffer = line.left(line.indexOf("\",\""));
            line.remove(0, line.indexOf("\",\"") + 3);
            fields << buffer;

            // special code
            buffer = line.remove(0,2);
            buffer = line.left(line.length() - 1);
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

            if ( db.driverName().compare("QPSQL") != 0 ) {
                sql = "REPLACE INTO hash (sha1, md5, crc32) VALUES ('";
                sql += fields.at(0) % "','";
                sql += fields.at(1) % "','";
                sql += fields.at(2);
                sql += "');";
            } else {
                sql = "SELECT upsert_hash('";
                sql += fields.at(0) % "','";
                sql += fields.at(1) % "','";
                sql += fields.at(2);
                sql += "');";
            }
            //			std::cout << sql.toLatin1().constData() << std::endl;

            if ( query.exec(sql) == false ) {
                std::cerr << "result.processed_lines:" << result.processed_lines << std::endl;
                std::cerr << "query: " << sql.toLatin1().constData() << std::endl;
                std::cerr << "exec error: " << query.lastError().text().toLatin1().constData() << std::endl;
                return result;
            }

            if ( db.driverName().compare("QPSQL") != 0 ) {
                sql = "REPLACE INTO file (file_name, file_size, product_code, op_system_code, special_code, hash_sha1) VALUES ('";
                sql += fields.at(3) % "','";
                sql += fields.at(4) % "','";
                sql += fields.at(5) % "','";
                sql += fields.at(6) % "','";
                sql += fields.at(7) % "','";
                sql += fields.at(0);
                sql += "');";
            } else {
                sql = "SELECT upsert_file('";
                sql += fields.at(3) % "','";
                sql += fields.at(4) % "','";
                sql += fields.at(5) % "','";
                sql += fields.at(6) % "','";
                sql += fields.at(7) % "','";
                sql += fields.at(0);
                sql += "');";
            }

            if ( query.exec(sql) == false ) {
                std::cerr << "fields size: " << fields.size() << std::endl;
                std::cerr << "query: " << sql.toLatin1().constData() << std::endl;
                std::cerr << "exec error: " << query.lastError().text().toLatin1().constData() << std::endl;
                return result;
            }
            /*
               if ( burst_commit(db, burst) == false ) {
               std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
               return result;
               }
             */
        }
    }

    if ( db.commit() == false ) {
        std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
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

    while ( ! q_stdin.atEnd() ) {
        QString		buffer;
        QString		line = q_stdin.readLine().simplified();
        QStringList	fields;

        // Skip useless lines
        if ( line.indexOf(",") == -1 )
            continue;

        result.processed_lines++;

        //		line.replace("\"", "");
        //		std::cout << "line: " << line.toLatin1().constData() << std::endl;

        // code
        line.remove(0,1);
        buffer = line.left(line.indexOf("\",\""));
        line.remove(0, line.indexOf("\",\"") + 3);
        //		std::cout << "code: " << buffer.toLatin1().constData() << std::endl << "line: " << line.toLatin1().constData() << std::endl;
        fields << buffer;

        // name
        line.replace('"', "");
        fields << line;

        if ( fields.size() != 2 ) {
            std::cerr << "error: cannot extract the 2 required fields from: " << line.toLatin1().constData() << std::endl;
            return result;
        }

        /*
           code	fields.at(0)
           name	fields.at(1)
         */

        query.bindValue(":code", fields.at(0));
        query.bindValue(":name", fields.at(1));

        if ( query.exec() == false ) {
            std::cerr << "exec error: " << query.lastError().text().toLatin1().constData() << std::endl;
            return result;
        }
        /*
           if ( burst_commit(db, burst) == false ) {
           std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
           return result;
           }
         */
    }

    query.finish();

    if ( db.commit() == false ) {
        std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
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

    while ( ! q_stdin.atEnd() ) {
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
            std::cerr << "exec error: " << query.lastError().text().toLatin1().constData() << std::endl;
            std::cerr << "line: " << line.toLatin1().constData() << std::endl;
            std::cerr << query.executedQuery().toLatin1().constData() << std::endl;
            Q_FOREACH(QString key, query.boundValues().keys()) {
                std::cerr << key.toLatin1().constData() << ":" << query.boundValue(key).toString().toLatin1().constData()  << ":" << std::endl;
            }
            return result;
        }
        /*
           if ( burst_commit(db, burst) == false ) {
           std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
           return result;
           }
         */
    }

    if ( db.commit() == false ) {
        std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
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
            std::cerr << "Unable to prepare upsert_product: " << db.lastError().text().toLatin1().constData() << std::endl;
            db.rollback();
            return result;
        }
        if ( link_query.prepare("SELECT upsert_product_has_os(:product_code, :system_code);") == false ) {
            std::cerr << "Unable to prepare upsert_product_has_os: " << db.lastError().text().toLatin1().constData() << std::endl;
            db.rollback();
            return result;
        }
    }

    while ( ! q_stdin.atEnd() ) {
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
        /*
           if ( burst_commit(db, burst) == false ) {
           std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
           return result;
           }
         */
    }

    if ( db.commit() == false ) {
        std::cerr << "transaction error: " << db.lastError().text().toLatin1().constData() << std::endl;
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

