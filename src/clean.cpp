/**
 * Project: nsrl-toolkit
 * File name: clean.cpp
 * Description: implements the cleaning functions
 *
 * @author Mathieu Grzybek on 2012-10-24
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

#include "clean.h"

/*
 * clean_nsrl_file_line
 *
 * Puts the missing commas (RDS 237 contains "ere" strings instead of '","')
 *
 * @arg	line	:	 the line to clean
 *
 */
void	clean_nsrl_file_line(QString& line) {
	QRegExp	col_1_match("^.{43}(,)");
	QRegExp	col_1_replace("^(.{41}).{3}");

	QRegExp	col_2_match("^.{78}(,)");
	QRegExp	col_2_replace("^(.{76}).{3}");

	QRegExp	col_3_match("^.{89}(,)");
	QRegExp	col_3_replace("^(.{87}).{3}");

	if ( line.contains(col_1_match) == false )
		line.replace(col_1_replace, "\\1\",\"");

	if ( line.contains(col_2_match) == false )
		line.replace(col_2_replace, "\\1\",\"");

	if ( line.contains(col_3_match) == false )
		line.replace(col_3_replace, "\\1\",\"");
}
