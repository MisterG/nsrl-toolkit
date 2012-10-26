#include "regex.h"

int	main() {
	QFile	input;

	if ( not input.open(stdin, QIODevice::ReadOnly) ) {
		qCritical() << "Cannot open stdin";
		return EXIT_FAILURE;
	}

	while ( not input.atEnd() ) {
		QString	line = input.readLine().simplified();

		qDebug() << "Old line is: " << line;
		clean_nsrl_file_line(line);
		qDebug() << "New line is: " << line;
	}

	input.close();

	return EXIT_SUCCESS;
}

