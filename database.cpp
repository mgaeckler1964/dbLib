/*
		Project:		dbLIB
		Module:			database.cpp
		Description:	The entire database management
		Author:			Martin Gäckler
		Address:		Hofmannsthalweg 14, A-4030 Linz
		Web:			https://www.gaeckler.at/

		Copyright:		(c) 2007-2025 Martin Gäckler

		This program is free software: you can redistribute it and/or modify  
		it under the terms of the GNU General Public License as published by  
		the Free Software Foundation, version 3.

		You should have received a copy of the GNU General Public License 
		along with this program. If not, see <http://www.gnu.org/licenses/>.

		THIS SOFTWARE IS PROVIDED BY Martin Gäckler, Linz, Austria ``AS IS''
		AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
		TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
		PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR
		CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
		SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
		LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
		USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
		ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
		OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
		SUCH DAMAGE.
*/

// --------------------------------------------------------------------- //
// ----- switches ------------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- includes ------------------------------------------------------ //
// --------------------------------------------------------------------- //

#include <gak/directory.h>
#include <gak/strFiles.h>

#include "db_exception.h"
#include "database.h"
#include "table.h"
#include "db_file_io.h"

// --------------------------------------------------------------------- //
// ----- imported datas ------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- module switches ----------------------------------------------- //
// --------------------------------------------------------------------- //

#ifdef __BORLANDC__
#	pragma option -RT-
#	ifdef __WIN32__
#		pragma option -a4
#		pragma option -pc
#	else
#		pragma option -po
#		pragma option -a2
#	endif
#endif

namespace dbLib
{

// --------------------------------------------------------------------- //
// ----- constants ----------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- macros -------------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- type definitions ---------------------------------------------- //
// --------------------------------------------------------------------- //

using gak::STRING;
using gak::F_STRING;

// --------------------------------------------------------------------- //
// ----- class definitions --------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- exported datas ------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- module static data -------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class static data --------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- prototypes ---------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- module functions ---------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class inlines ------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class constructors/destructors -------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class static functions ---------------------------------------- //
// --------------------------------------------------------------------- //

Database *Database::createDB(
	const char *server, const char *db,
	const char *userName, const char *password
)
{
	doEnterFunctionEx( gakLogging::llDetail, "Database::createDB" );

	Database *newDB = new Database( server, db, userName );

	if( fileExists( newDB->m_dbConfigFile ) )
	{
		delete newDB;
		throw DbExist( newDB->m_dbConfigFile );
	}

	return newDB;
}

Database *Database::connectDB(
	const char *server, const char *db,
	const char *userName, const char *password
)
{
	doEnterFunctionEx( gakLogging::llDetail, "Database::connectDB" );

	Database *newDB = new Database( server, db, userName );

	if( !fileExists( newDB->m_dbConfigFile ) )
	{
		STRING	dbConfig = newDB->m_dbConfigFile;
		delete newDB;
		throw DbNotExist( dbConfig );
	}

	newDB->m_configuration.loadConfigFile( newDB->m_dbConfigFile );

	return newDB;
}

// --------------------------------------------------------------------- //
// ----- class privates ------------------------------------------------ //
// --------------------------------------------------------------------- //

STRING Database::findTablePath( const char *tableName )
{
	doEnterFunctionEx( gakLogging::llDetail, "Database::findTablePath" );

	STRING	dbPathConfig, tablePath;
	int		i = 0;
	STRING	tableFile = m_dbPath;
	STRING	dataFile;

	while( tableFile[0U] )
	{
		tableFile += m_dbUser;
		tableFile += DIRECTORY_DELIMITER;

		tableFile += tableName;

		dataFile = tableFile;
		dataFile += ".data";
		if( fileExists( dataFile ) )
		{
			tablePath = tableFile;
			break;
		}

		dbPathConfig="DB_PATH_";
		dbPathConfig += gak::formatNumber(i);
		tableFile = STRING(m_configuration[dbPathConfig]);
		i++;
	}

	return tablePath;
}

// --------------------------------------------------------------------- //
// ----- class protected ----------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class virtuals ------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class publics ------------------------------------------------- //
// --------------------------------------------------------------------- //

void Database::connect()
{
}

void Database::close()
{
}

Table *Database::createTable( const char *tableName )
{
	doEnterFunctionEx( gakLogging::llDetail, "Database::createTable" );

	Table *theNewTable;

	STRING	tableFile = m_dbPath;
	STRING	dataFile;

	STRING	tablePath = findTablePath( tableName );

	if( tablePath[0U] )
	{
		throw DBtableExist( tableName );
	}

	tableFile += m_dbUser;
	tableFile += DIRECTORY_DELIMITER;
	tableFile += tableName;

	theNewTable = new Table( tableFile );
	theNewTable->create();

	return theNewTable;
}

void Database::dropTable(const char *tableName)
{
	doEnterFunctionEx( gakLogging::llDetail, "Database::dropTable" );

	STRING	tablePath = findTablePath( tableName );

	if( tablePath[0U] )
	{
		F_STRING path;
		fsplit(tablePath, &path);

		gak::DirectoryList	dList;
		dList.findFiles(tablePath+"*");
		for(
			gak::DirectoryList::const_iterator it = dList.cbegin(), endIT = dList.cend();
			it != endIT;
			++it
		)
		{
			F_STRING complete = path + it->fileName;
			strRemove( complete );
		}
		return;
	}
}

Table *Database::openTable( const char *tableName )
{
	doEnterFunctionEx( gakLogging::llDetail, "Database::openTable" );

	STRING	tablePath = findTablePath( tableName );

	if( tablePath[0U] )
	{
		Table *theTable = new Table( tablePath );
		theTable->open();

		return theTable;
	}

	throw DBtableNotFound( tableName );
}

// --------------------------------------------------------------------- //
// ----- entry points -------------------------------------------------- //
// --------------------------------------------------------------------- //

} // namespace dbLib

#ifdef __BORLANDC__
#	pragma option -RT.
#	pragma option -a.
#	pragma option -p.
#endif

