#include "csv_extractor.h"

bool    extract_file(QStringList& _return, QString& line, const QSqlDriver& driver) {
	QString buffer;

	// Skip useless lines
	if ( line.indexOf(",") == -1 )
		return false;

	if ( line.length() > 1 ) {
		//clean_nsrl_file_line(line);

		/*
		 * Let's extract the first five field ourselves
		 * We could use regexp instead
		 */
		// sha1
		buffer = line.remove(0,1);
		buffer = line.left(line.indexOf('"'));
		line.remove(0, line.indexOf('"') + 3);

		if ( buffer.size() != 40 ) {
			std::cerr << "line: " << line.toLatin1().constData() << std::endl;
			std::cerr << "sha1's length is not 40 characters!" << std::endl;
			return false;
		}
		_return << buffer;

		// md5
		buffer = line.left(line.indexOf('"'));
		line.remove(0, line.indexOf('"') + 3);

		if ( buffer.size() != 32 ) {
			std::cerr << "line: " << line.toLatin1().constData() << std::endl;
			std::cerr << "md5's length is not 32 characters!" << std::endl;
			return false;
		}
		_return << buffer;

		// crc32
		buffer = line.left(line.indexOf('"'));
		line.remove(0, line.indexOf('"') + 3);

		if ( buffer.size() != 8 ) {
			std::cerr << "line: " << line.toLatin1().constData() << std::endl;
			std::cerr << "crc's length is not 8 characters!" << std::endl;
			return false;
		}
		_return << buffer;

		// file_name
		buffer = line.left(line.indexOf('"'));
#ifdef QT_5
		QSqlField   f_buffer("buffer", QVariant::String);
		f_buffer.setValue(buffer);
		buffer = driver.formatValue(f_buffer);
#else
		buffer = driver.formatValue(buffer);
#endif
		line.remove(0, line.indexOf('"') + 2);
		_return << buffer;

		// file_size
		buffer = line.left(line.indexOf(','));
		line.remove(0, line.indexOf(',') + 1);
		_return << buffer;

		// product code (always an int)
		buffer = line.left(line.indexOf(','));
		line.remove(0, line.indexOf(',') + 2);
		_return << buffer;

		// system code (string)
		buffer = line.left(line.indexOf("\",\""));
		line.remove(0, line.indexOf("\",\"") + 3);
		_return << buffer;

		// special code
		buffer = line.remove(0,2);
		buffer = line.left(line.length() - 1);
		_return << buffer;

		return true;
	}
	return false;
}
