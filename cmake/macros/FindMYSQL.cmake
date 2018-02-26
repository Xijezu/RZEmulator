#--------------------------------------------------------
# Copyright (C) 1995-2007 MySQL AB
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# There are special exceptions to the terms and conditions of the GPL
# as it is applied to this software. View the full text of the exception
# in file LICENSE.exceptions in the top-level directory of this software
# distribution.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#
# The MySQL Connector/ODBC is licensed under the terms of the
# GPL, like most MySQL Connectors. There are special exceptions
# to the terms and conditions of the GPL as it is applied to
# this software, see the FLOSS License Exception available on
# mysql.com.

##########################################################################

SET(PROGRAM_FILES_ARCH_PATH)
if(PLATFORM EQUAL 32)
    SET(PROGRAM_FILES_ARCH_PATH $ENV{ProgramFiles})
elseif(PLATFORM EQUAL 64)
    SET(PROGRAM_FILES_ARCH_PATH $ENV{ProgramW6432})
endif()

if (${PROGRAM_FILES_ARCH_PATH})
    STRING(REPLACE "\\\\" "/" PROGRAM_FILES_ARCH_PATH ${PROGRAM_FILES_ARCH_PATH})
endif(${PROGRAM_FILES_ARCH_PATH})


#-------------- FIND MYSQL_INCLUDE_DIR ------------------
FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
        /usr/include/mysql
        /usr/local/include/mysql
        /opt/mysql/mysql/include
        /opt/mysql/mysql/include/mysql
        /opt/mysql/include
        /opt/local/include/mysql5
        /usr/local/mysql/include
        /usr/local/mysql/include/mysql
        ${PROGRAM_FILES_ARCH_PATH}/MySQL/*/include
        $ENV{ProgramFiles}/MySQL/*/include
        $ENV{SystemDrive}/MySQL/*/include)

#----------------- FIND MYSQL_LIB_DIR -------------------
IF (WIN32)
    FIND_LIBRARY(MYSQL_LIB NAMES libmysql
            PATHS
            $ENV{MYSQL_DIR}/lib
            $ENV{MYSQL_DIR}/libmysql
            $ENV{MYSQL_DIR}/libmysql
            $ENV{MYSQL_DIR}/client
            $ENV{MYSQL_DIR}/libmysql
            ${PROGRAM_FILES_ARCH_PATH}/MySQL/*/lib
            $ENV{ProgramFiles}/MySQL/*/lib
            $ENV{SystemDrive}/MySQL/*/lib)
ELSE (WIN32)
    FIND_LIBRARY(MYSQL_LIB NAMES mysqlclient mysqlclient_r mysql libmysql
            PATHS
            /usr/lib/mysql
            /usr/local/lib/mysql
            /usr/local/mysql/lib
            /usr/local/mysql/lib/mysql
            /opt/local/mysql5/lib
            /opt/local/lib/mysql5/mysql
            /opt/mysql/mysql/lib/mysql
            /opt/mysql/lib/mysql)
ENDIF (WIN32)

IF(MYSQL_LIB)
    GET_FILENAME_COMPONENT(MYSQL_LIB_DIR ${MYSQL_LIB} PATH)
ENDIF(MYSQL_LIB)

IF (MYSQL_INCLUDE_DIR AND MYSQL_LIB_DIR)
    SET(MYSQL_FOUND TRUE)
    INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR})
    LINK_DIRECTORIES(${MYSQL_LIB_DIR})
ELSEIF (MySQL_FIND_REQUIRED)
    set(MYSQL_FOUND FALSE)
    MESSAGE(FATAL_ERROR "Cannot find MySQL. Include dir: ${MYSQL_INCLUDE_DIR}  library dir: ${MYSQL_LIB_DIR}")
ENDIF (MYSQL_INCLUDE_DIR AND MYSQL_LIB_DIR)
