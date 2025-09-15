/*
		Project:		dbLIB
		Module:			index.h
		Description:	The definitions for one table index
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

#ifndef DBLIB_INDEX_H
#define DBLIB_INDEX_H

// --------------------------------------------------------------------- //
// ----- switches ------------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- includes ------------------------------------------------------ //
// --------------------------------------------------------------------- //

#include <gak/string.h>
#include <gak/array.h>
#include <gak/xml.h>

#include "db_file_io.h"
#include "record.h"

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

static const char	TABLE_HEADER[] = "0000000000000000";
static const size_t	TABLE_HEADER_SIZE = sizeof(TABLE_HEADER)-1;

// --------------------------------------------------------------------- //
// ----- macros -------------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- type definitions ---------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class definitions --------------------------------------------- //
// --------------------------------------------------------------------- //

class Index
{
	bool			m_dropAfterClose;
	gak::STRING		m_pathName;
	gak::STRING		m_dataFile;


	protected:
	DbFile						*m_dataFileHandle;
	Record						m_currentRecord;
	FieldDefinitions			m_fieldDefinitions;

	const FieldDefinition &getFieldDef( size_t fieldDefIdx ) const
	{
		assert(fieldDefIdx < m_fieldDefinitions.size());
		return m_fieldDefinitions[fieldDefIdx];
	}
	size_t	findField( const char *fieldName );

	public:
	Index( const gak::STRING &pathName )
	{
		m_pathName = pathName;

		m_dataFileHandle = nullptr;
		m_dropAfterClose = false;

		m_dataFile = pathName;
		m_dataFile += ".data";
		m_dataFileHandle = openTableFile( m_dataFile );
	}
	~Index()
	{
		if( m_dataFileHandle )
			closeTableFile( m_dataFileHandle );
		if( m_dropAfterClose )
			strRemove( m_dataFile );
	}
	static const size_t no_index;

	void open( gak::xml::Element*theXmlFieldDefs );
	void writeXmlDefinition( gak::xml::Element *theXmlFieldDefs ) const;

	void truncateFile();
	void create();
	bool bof() const
	{
		return m_currentRecord.bof();
	}
	bool eof() const
	{
		return m_currentRecord.eof();
	}

	size_t	getNumFields() const
	{
		return m_fieldDefinitions.size();
	}

	void addField(
		const gak::STRING &name, fType type,
		bool primary=false,
		bool notNulls = false,
		const gak::STRING &reference = ""
	);

	void insertRecord()
	{
		m_currentRecord.setInsertMode();
	}
	void postRecord()
	{
		m_currentRecord.postRecord( m_dataFileHandle );
	}
	void deleteRecord()
	{
		m_currentRecord.deleteRecord( m_dataFileHandle );
	}
	void root()
	{
		m_currentRecord.root( m_dataFileHandle );
	}


	void firstRecord( const gak::STRING &searchBuffer="" )
	{
		m_currentRecord.firstRecord( m_dataFileHandle, searchBuffer );
	}
	void nextRecord( const gak::STRING &searchBuffer="" )
	{
		m_currentRecord.nextRecord( m_dataFileHandle, searchBuffer );
	}
	void previousRecord( const gak::STRING &searchBuffer="" )
	{
		m_currentRecord.prevRecord( m_dataFileHandle, searchBuffer );
	}
	void lastRecord( const gak::STRING &searchBuffer="" )
	{
		m_currentRecord.lastRecord( m_dataFileHandle, searchBuffer );
	}

	FieldValue *getField( const gak::STRING &name );
	FieldValue *getField( size_t fieldIdx );

	const gak::STRING &getPathName() const
	{
		return m_pathName;
	}

	const gak::STRING getIndexPathName( const gak::STRING &indexName )
	{
		return getPathName() + '.' + indexName;
	}

	int locateValue(
		gak::int64 *posFound,
		const gak::STRING &searchFor, bool primary
	)
	{
		if( (m_dataFileHandle->toEnd() - TABLE_HEADER_SIZE) > 0 )
		{
			RecordHeader headerFound;

			*posFound = TABLE_HEADER_SIZE;
			return Record::locateValue(
				m_dataFileHandle,
				posFound, &headerFound,
				searchFor, primary
			);
		}
		else
		{
			*posFound = 0;
			return 0;
		}
	}

	void readRecord( gak::int64 position )
	{
		m_currentRecord.readRecord( m_dataFileHandle, position );
	}
	const Record &getRecord() const
	{
		return m_currentRecord;
	}

	void dropDataFile()
	{
		m_dropAfterClose = true;
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
