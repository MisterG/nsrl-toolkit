/**
 * Project: nsrl-toolkit
 * File name: nsrl_products.h
 * Description: describes the nsrl_products.cpp functions
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

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include <QStringBuilder>
#include <QMap>
#include <QtSql>

#include "common.h"
#include "database.h"
#include "import.h"

/*
 * main
 *
 * The main function if the program
 *
 * @arg     : the number of arguments
 * @arg     : the arguments
 *
 * @return	: EXIT_FAILURE or EXIT_SUCCESS
 */
int	main(int argc, char* argv[]);


/*
 * process_input
 *
 * Opens std::cin, queries the database and prints the result
 *
 * @arg		: the m_settings to check
 * @return	: true for success
 */
bool	process_input(const m_settings& settings);

/*
 * build_sql
 *
 * Writes the SQL query to _return according to the settings
 *
 * @arg		: the returning value
 * @arg		: the m_settings to check
 */
void	build_sql(QString& _return, const m_settings& settings);

/*
 * check_settings
 *
 * Deals with the settings to use:
 * - hostname:	the remote database server
 * - database:	the name of the schema to use
 * - user:	the username
 *
 * @arg		: the m_settings to check
 * @return	: true for success
 * 
 */
bool	check_settings(const m_settings& settings);

/*
 * usage
 *
 * Prints the usage message to stdout
 */
void	usage();

