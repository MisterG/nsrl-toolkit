# Project: nsrl-toolkit
# File name: nsrl_products.pro
# Description: describes the Qt project and how to build it
#
# @author Mathieu Grzybek on 2013-04-11
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

# Version's control
contains(QT_VERSION, ^5\\.[0-9]\\..*) {
	DEFINES	+= QT_5
}

QT	+=	core sql
QT	-=	gui

INCLUDEPATH	+= include

TARGET	=	nsrl_products

SOURCES	+=	src/nsrl_products.cpp \
	src/database.cpp

HEADERS	+=	include/nsrl_products.h \
	include/common.h \
	include/database.h

