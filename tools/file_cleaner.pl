# Project: nsrl-toolkit
# File name: file_cleaner.pl
# Description: can be used to clean NSRLFile.txt from delimiter's errors
#
# @author Mathieu Grzybek on 2012-10-24
# @copyright 2012 Mathieu Grzybek. All rights reserved.
# @version $Id: code-gpl-license.txt,v 1.2 2004/05/04 13:19:30 garry Exp $
#
# @see The GNU Public License (GPL) version 3 or higher
#
#
# nsrl-toolkit is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

# To be used with perl -n file_cleaner.pl NSRLFile.txt
# Puts the missing commas (RDS 237 contains "ere" strings instead of '","')

use strict;

my $buffer = $_;

# Between crc32 and file_name
$buffer =~ /^.{88}(.)/;
if ( $1 !~ /,/ ) {
	$buffer =~ s/^(.{86}).{3}/$1\",\"/;
}

# Between md5 and crc32
$buffer =~ /^.{78}(.)/;
if ( $1 =~ /./ ) {
	$buffer =~ s/^(.{76}).{3}/$1\",\"/;
}

# Between sha1 and md5
$buffer =~ /^.{43}(.)/;
if ( $1 =~ /./ ) {
	$buffer =~ s/^(.{71}).{3}/$1\",\"/;
}

# We don't need to add '\n', already included in $buffer
print $buffer;
