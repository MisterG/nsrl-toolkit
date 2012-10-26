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
	QRegExp	col_1_match("^.{43}(.)");
	QRegExp	col_1_replace("^(.{41}).{3}/$1\",\"");

	QRegExp	col_2_match("^.{78}(.)");
	QRegExp	col_2_replace("^(.{76}).{3}/$1\",\"");

	QRegExp	col_3_match("^.{88}(.)");
	QRegExp	col_3_replace("^(.{86}).{3}/$1\",\"");

//	if ( line.contains(col_1_match) == false )
		line.replace(col_1_replace, "\",\"");

//	if ( line.contains(col_2_match) == false )
		line.replace(col_2_replace, "\",\"");

//	if ( line.contains(col_3_match) == false ) {
//		std::cout << "false" << std::endl;
		line.replace(col_3_replace, "\",\"");
//	}
}
