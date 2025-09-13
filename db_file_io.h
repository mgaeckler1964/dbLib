/*
		Project:		dbLIB
		Module:			db_file_io.h
		Description:	basic file io 
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

#ifndef DB_FILE_IO_H
#define DB_FILE_IO_H

// --------------------------------------------------------------------- //
// ----- switches ------------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- includes ------------------------------------------------------ //
// --------------------------------------------------------------------- //

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <gak/string.h>

#include "db_exception.h"

// operating system (compiler) specific includes

#ifdef __BORLANDC__
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#endif

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#endif

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

/*
	Her are five macros that must be ported to a specific operating system
*/
#ifdef __BORLANDC__
inline long dbFileOpen( const char *file )
{
	return open( file, O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE );
}

inline long dbFileSeek( long handle, gak::int64 position )
{
	return lseek( (int)handle, position, SEEK_SET );
}

inline long dbFileSkip( long handle, gak::int64 offset )
{
	return lseek( (int)handle, offset, SEEK_CUR );
}

inline long dbFileSeekEnd( long handle )
{
	return lseek( (int)handle, 0, SEEK_END );
}

inline long dbMakePath( const char *path )
{
	return mkdir( path );
}

inline long dbFileRead( long handle, void *buffer, size_t size )
{
	return read( (int)handle, buffer, (unsigned)size );
}

inline long dbFileWrite( long handle, const void *buffer, size_t size )
{
	return write( (int)handle, buffer, (unsigned)size );
}

inline long dbFileClose( long handle )
{
	return close( (int)handle );
}

inline bool fileExists( const char *file )
{
	if( !access( file, 06 ) )
		return true;
	else
		return false;
}
#endif

#ifdef _MSC_VER
inline long dbFileOpen( const char *file )
{
	return open( file, O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE );
}

inline gak::int64 dbFileSeek( long handle, gak::int64 position )
{
	/// TODO check Overflow
	return lseek( int(handle), long(position), SEEK_SET );
}

inline gak::int64 dbFileSkip( long handle, gak::int64 offset )
{
	/// TODO check Overflow
	return lseek( int(handle), long(offset), SEEK_CUR );
}

inline gak::int64 dbFileSeekEnd( long handle )
{
	return lseek( int(handle), 0, SEEK_END );
}

inline long dbMakePath( const char *path )
{
	return mkdir( path );
}

inline long dbFileRead( long handle, void *buffer, size_t size )
{
	/// TODO check Overflow
	return read( int(handle), buffer, (unsigned)size );
}

inline long dbFileWrite( long handle, const void *buffer, size_t size )
{
	/// TODO check Overflow
	return write( int(handle), buffer, (unsigned)size );
}

inline long dbFileClose( long handle )
{
	return close( int(handle) );
}

inline bool fileExists( const char *file )
{
	if( !access( file, 06 ) )
		return true;
	else
		return false;
}
#endif

// --------------------------------------------------------------------- //
// ----- type definitions ---------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class definitions --------------------------------------------- //
// --------------------------------------------------------------------- //

class DbFile
{
	private:
	long		usageCounter;
	long		handle;
	gak::STRING	fileName;

	public:
	DbFile()
	{
		usageCounter = 0;
		handle = 0;
	}

	long open( const gak::STRING &fileName )
	{
		if( !usageCounter )
		{
			handle = dbFileOpen( fileName );
			if( handle > 0 )
			{
				this->fileName = fileName;
				usageCounter = 1;
			}
			else
				throw DBopenFaild( fileName );
		}
		else
			usageCounter++;

		return handle;
	}
	gak::int64 seek( gak::int64 position )	const
	{
		return dbFileSeek( handle, position );
	}
	gak::int64 skip( gak::int64 offset )	const
	{
		return dbFileSkip( handle, offset );
	}
	gak::int64 toEnd()	const
	{
		return dbFileSeekEnd( handle );
	}
	gak::int64 toStart()	const
	{
		return seek( 0 );
	}
	long read( void *buffer, size_t len )	const
	{
		return dbFileRead( handle, buffer, len );
	}
	long write( const void *buffer, size_t len )	const
	{
		return dbFileWrite( handle, buffer, len );
	}
	long close()
	{
		usageCounter--;
		if( !usageCounter )
		{
			long	result = dbFileClose( handle );
			handle = 0;

			return result;
		}
		else
			return 0;

	}
	bool isOpen()	const
	{
		return usageCounter > 0 && handle > 0;
	}
	const char *getFileName() const
	{
		return fileName;
	}
	~DbFile()
	{
		if( isOpen() )
		{
			usageCounter = 1;
			close();
		}
	}
};

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

DbFile	*openTableFile( const gak::STRING &path );
void closeTableFile( DbFile *dbFile );

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

// --------------------------------------------------------------------- //
// ----- class privates ------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class protected ----------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class virtuals ------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class publics ------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- entry points -------------------------------------------------- //
// --------------------------------------------------------------------- //

} // namespace dbLib

#ifdef __BORLANDC__
#	pragma option -RT.
#	pragma option -a.
#	pragma option -p.
#endif

#endif
