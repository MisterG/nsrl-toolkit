#ifndef CSV_EXTRACTOR_H
#define CSV_EXTRACTOR_H

#include <iostream>

#include <QString>
#include <QSqlDriver>
#include <QSqlField>
#include <QStringList>

#include "clean.h"

bool    extract_file(QStringList& _return, QString& line, const QSqlDriver& driver);
bool    extract_mfg(QStringList& _return, QString& line);
bool    extract_os(QStringList& _return, QString& line);
bool    extract_prod(QStringList& _return, QString& line);

#endif // CSV_EXTRACTOR_H
