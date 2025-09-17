/*
		Project:		dbLIB
		Module:			record.h
		Description:	The definitions for one table record
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

#ifndef DB_RECORD_H
#define DB_RECORD_H

// --------------------------------------------------------------------- //
// ----- switches ------------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- includes ------------------------------------------------------ //
// --------------------------------------------------------------------- //

#include <stdlib.h>

#include <gak/types.h>

#include "fieldvalue.h"
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

#define REC_DELETED		1
#define REC_LOCKED		2

// --------------------------------------------------------------------- //
// ----- macros -------------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- type definitions ---------------------------------------------- //
// --------------------------------------------------------------------- //

enum RecordMode
{
	rmInsert,
	rmUpdate,
	rmBrowse,
	rmEof,
	rmBof
};

struct RecordHeader
{
	gak::int64		address;							// my own address
	gak::int64		topPtr;								// parent record
	gak::int64		lowerRecordPtr, higherRecordPtr;	// lower/higher records
	gak::int64		numRecords;							// number of records in this subtree (incl current)
	std::size_t		numFields;
	gak::uint64		stringLengths, primaryLen, bufferLen;
	gak::int32		status;

	RecordHeader()
	{
		memset( this, 0, sizeof(*this) );
	}
	void clear()
	{
		size_t	i = numFields;

		memset( this, 0, sizeof( *this ) );
		numFields = i;
	}

};

// --------------------------------------------------------------------- //
// ----- class definitions --------------------------------------------- //
// --------------------------------------------------------------------- //

class Record
{
	friend class Index;
	friend class Table;

	private:
	gak::STRING		m_searchBuffer;
	RecordHeader	m_theHeader;
	long			m_nodeId;
	FieldValue		*m_values;
	RecordMode		m_theRecMode;

	Record()
	{
		m_theRecMode = rmInsert;
		m_values = NULL;
	}
	~Record()
	{
		if( m_values )
		{
			delete [] m_values;
		}
	}

	static void readRecordHeader(
		DbFile *dataFileHandle, RecordHeader *theHeader
	);
	static void writeRecordHeader(
		DbFile *dataFileHandle, const RecordHeader &theHeader
	);
	static void loadRecordHeader(
		gak::uint64 pos, DbFile *dataFileHandle, RecordHeader *theHeader
	)
	{
		dataFileHandle->seek( pos );
		readRecordHeader( dataFileHandle, theHeader );
		theHeader->address = pos;
	}
	static void updateRecordHeader(
		DbFile *dataFileHandle, const RecordHeader &theHeader
	)
	{
		dataFileHandle->seek( theHeader.address );
		writeRecordHeader( dataFileHandle, theHeader );
	}
	static char *Record::readRecordBuffer(
		DbFile *dataFileHandle, gak::int64 length, bool primary
	);
	static int locateValue(
		DbFile *dataFileHandle,
		gak::int64 *posFound, RecordHeader *headerFound,
		const gak::STRING &searchFor, bool primarySearch
	);

	void getRecord( gak::STRING *theValues, bool primary, gak::STRING *theStringLengths );

	void createRecord( const FieldDefinitions &definitions );
	void setInsertMode( void );

	FieldValue *getFieldValue( size_t fieldIdx )
	{
		return m_values+fieldIdx;
	}
	void readRecord( DbFile *dataFileHandle );
	void readRecord( DbFile *dataFileHandle, gak::int64 currentPosition )
	{
		loadRecordHeader( currentPosition, dataFileHandle, &m_theHeader );
		readRecord( dataFileHandle );
	}

	gak::int64 rebalance( DbFile *dataFileHandle, gak::int64 curPos, RecordHeader &curHeader, gak::int64 prevPos, RecordHeader &prevHeader, bool cur2Small, bool prev2Small );

	void postRecord( DbFile *dataFileHandle );
	void deleteRecord( DbFile *dataFileHandle, bool noMove=false );
	void root( DbFile *dataFileHandle );

	/*
	 * cursor loop
	 */
	void firstRecord( DbFile *dataFileHandle, const gak::STRING &searchBuffer="" );
	void nextRecord( DbFile *dataFileHandle );
	void prevRecord( DbFile *dataFileHandle );
	void lastRecord( DbFile *dataFileHandle, const gak::STRING &searchBuffer="" );
	bool bof( void ) const
	{
		return m_theRecMode == rmBof;
	}
	bool eof( void ) const
	{
		return m_theRecMode == rmEof;
	}

	void backupValues( void );

	gak::int64 getCurrentPosition() const
	{
		return m_theHeader.address;
	}

	public:
	friend class MydbUnitTest;
	const RecordHeader &getHeader() const
	{
		return m_theHeader;
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

// --------------------------------------------------------------------- //
// ----- module functions ---------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class inlines ------------------------------------------------- //
// --------------------------------------------------------------------- //

inline bool IsDeleted( const RecordHeader &header )
{
	return header.status & REC_DELETED;
}

inline void SetDeleted( RecordHeader *header )
{
	header->status |= REC_DELETED;
}

inline void ClrDeleted( RecordHeader *header )
{
	header->status &= ~REC_DELETED;
}


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
