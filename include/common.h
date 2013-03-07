#ifndef COMMON_H
#define COMMON_H

#include <QMap>
#include <QtGlobal>
#include <QSettings>

#include <iostream>

/*
 * m_settings
 *
 * QMap used to store the database's arguments
 */
typedef	QMap<QString, QString>	m_settings;

/*
 * t_result
 */
typedef struct {
	bool	success;
	uint	processed_lines;
} t_result;

#endif // COMMON_H
