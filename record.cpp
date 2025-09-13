/*
		Project:		dbLIB
		Module:			record.cpp
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

// --------------------------------------------------------------------- //
// ----- switches ------------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- includes ------------------------------------------------------ //
// --------------------------------------------------------------------- //

#include <sstream>
#include <iomanip>

#include <gak/stdlib.h>
#include <gak/numericString.h>

#include "fieldvalue.h"
#include "record.h"
#include "table.h"

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

static const int INT_LEN = 16;
static const int NUM_INT = 8;
static const int STATUS_LEN = 2;
static const int MAGIC_LEN = 3;

#define HEADER_LENGTH	NUM_INT*(INT_LEN+1)+STATUS_LEN+1+MAGIC_LEN

void Record::readRecordHeader(
	DbFile *dataFileHandle, RecordHeader *theHeader
)
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::readRecordHeader" );

	char	tmpBuffer[HEADER_LENGTH+1];
	long	readLen;

	readLen = dataFileHandle->read( tmpBuffer, HEADER_LENGTH );
	if( readLen == HEADER_LENGTH )
	{
		tmpBuffer[HEADER_LENGTH] = 0;
		std::istringstream	inp( tmpBuffer );
		inp >> theHeader->topPtr;
		inp.get();
		inp >> theHeader->lowerRecordPtr;
		inp.get();
		inp >> theHeader->higherRecordPtr;
		inp.get();
		inp >> theHeader->numRecords;
		inp.get();
		inp >> theHeader->numFields;
		inp.get();
		inp >> theHeader->stringLengths;
		inp.get();
		inp >> theHeader->primaryLen;
		inp.get();
		inp >> theHeader->bufferLen;
		inp.get();
		inp >> theHeader->status;
	}
	else
		throw DBillegalRecordHeader();
}

void Record::writeRecordHeader(
	DbFile *dataFileHandle, const RecordHeader *theHeader
)
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::writeRecordHeader" );

	std::ostringstream	sout;

	sout << std::setfill('0')
		<< std::setw(INT_LEN) << theHeader->topPtr << ';'
		<< std::setw(INT_LEN) << theHeader->lowerRecordPtr << ';'
		<< std::setw(INT_LEN) << theHeader->higherRecordPtr << ';'
		<< std::setw(INT_LEN) << theHeader->numRecords << ';'
		<< std::setw(INT_LEN) << theHeader->numFields << ';'
		<< std::setw(INT_LEN) << theHeader->stringLengths << ';'
		<< std::setw(INT_LEN) << theHeader->primaryLen << ';'
		<< std::setw(INT_LEN) << theHeader->bufferLen << ';'
		<< std::setw(STATUS_LEN) << theHeader->status << ";EOH";
	sout.flush();
	dataFileHandle->write( sout.str().c_str(), HEADER_LENGTH );
}

char *Record::readRecordBuffer(
	DbFile *dataFileHandle, gak::int64 length, bool primary
)
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::readRecordBuffer" );

	/// TODO check overflow
	long	readLen;
	gak::Buffer<char>	recBuffer( std::size_t(length+1) );

	if( recBuffer )
	{
		readLen = dataFileHandle->read( recBuffer, std::size_t(length) );
		if( readLen == length )
		{
			recBuffer[readLen] = 0;
			if( !primary && strcmp( recBuffer+int(readLen)-4, ";EOB" ) )
			{
				throw DBillegalRecordlen();
			}
		}
		else
		{
			throw DBillegalRecordlen();
		}
	}
	else
		throw DBmemoryException();

	return recBuffer.prepareMove();
}

int Record::locateValue(
	DbFile *dataFileHandle,
	gak::int64 *posFound, RecordHeader *headerFound,
	const STRING &searchFor, bool primary
)
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::locateValue" );
	gak::int64	newPosition = *posFound;
	bool		found = false;
	int			compareVal = 0;

	while( !found )
	{
		*posFound = dataFileHandle->seek( newPosition );
		if( newPosition != *posFound )
		{
			*posFound = 0;	// empty/bad file ?
/*v*/		break;
		}

		readRecordHeader( dataFileHandle, headerFound );

		{
			gak::Buffer<char> tmpRecord = readRecordBuffer(
				dataFileHandle,
				primary ? headerFound->primaryLen : headerFound->bufferLen,
				primary
			);
			compareVal = strcmp( tmpRecord, searchFor );
		}

		if( primary && !compareVal && IsDeleted( *headerFound ) )
		{
			gak::int64 lowerRecordPtr = headerFound->lowerRecordPtr;
			gak::int64 higherRecordPtr = headerFound->higherRecordPtr;

			if( higherRecordPtr )
			{
				*posFound = higherRecordPtr;
				compareVal = locateValue( dataFileHandle, posFound, headerFound, searchFor, primary );
				if( !compareVal && *posFound )
					found = true;				// end search, because we have found the matching record
			}
			if( !found && lowerRecordPtr )
			{
				*posFound = lowerRecordPtr;
				compareVal = locateValue( dataFileHandle, posFound, headerFound, searchFor, primary );
				found = true;					// end search in all cases
			}
		}
		else if( compareVal < 0 && headerFound->higherRecordPtr)
			newPosition = headerFound->higherRecordPtr;
		else if( compareVal > 0 && headerFound->lowerRecordPtr )
			newPosition = headerFound->lowerRecordPtr;
		else
			found = true;
	}

	return compareVal;
}

// --------------------------------------------------------------------- //
// ----- class privates ------------------------------------------------ //
// --------------------------------------------------------------------- //

void Record::getRecord( STRING *theValues, bool primary, STRING *theStringLengths )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::getRecord" );
	bool	checkPrimary = true;
	STRING	curValue;

	m_theHeader.primaryLen = 0;

	*theValues = "";

	if( theStringLengths )
		*theStringLengths = "";

	// construct the record
	for( size_t i=0; i<m_theHeader.numFields; i++ )
	{
		if( m_values[i].notNull() && m_values[i].isNull() )
			throw DBnullValueNotAllowed( m_values[i].getName() );

		curValue = m_values[i].getStringValue();
		if( primary && !m_values[i].isPrimary() )
/*v*/		break;

		if( i > 0 )
		{
			if( theStringLengths )
				*theStringLengths += ';';
			*theValues += ';';
		}

		if( theStringLengths )
		{
			*theStringLengths += gak::formatBinary(strlen(curValue), 16);
		}

		*theValues += curValue;

		if( checkPrimary && m_values[i].isPrimary() )
			m_theHeader.primaryLen = strlen( *theValues );
		else
			checkPrimary = false;
	}
}

void Record::createRecord( const FieldDefinitions &definitions )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::createRecord" );

	if( m_values )
		delete [] m_values;

	m_theHeader.numFields = definitions.size();
	m_currentPosition = 0;

	m_values = new FieldValue[m_theHeader.numFields];
	const FieldDefinition *definition = definitions.getDataBuffer();
	for( size_t	i=0; i<definitions.size(); ++i )
		m_values[i].setDefinition( definition++ );

	m_theRecMode = rmInsert;
}

void Record::setInsertMode( void )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::setInsertMode" );

	if( m_theRecMode != rmInsert )
	{
		size_t	i = m_theHeader.numFields;

		memset( &m_theHeader, 0, sizeof( m_theHeader ) );
		m_theHeader.numFields = i;

		for( i=0; i<m_theHeader.numFields; i++ )
			m_values[i].setNull();

		m_currentPosition = 0;
		m_theRecMode = rmInsert;
	}
}

void Record::readRecord( DbFile *dataFileHandle )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::readRecord" );
	char 	*cpLength, *cpData;

	size_t	lenData;

	gak::Buffer<char>recBuffer( readRecordBuffer( dataFileHandle, m_theHeader.bufferLen, true ) );
	gak::Buffer<char>lengthBuffer( readRecordBuffer( dataFileHandle, m_theHeader.stringLengths, true ) );

	cpLength = lengthBuffer;
	cpData = recBuffer;

	for( size_t i=0; i<m_theHeader.numFields; i++ )
	{
		const char *end;
		lenData = gak::getValue<size_t>(cpLength,16,&end);

		cpData[lenData] = 0;

		m_values[i].setStringValue( cpData );
		m_values[i].backupValue();
		cpData += lenData+1;
		cpLength = const_cast<char*>(end)+1;
	}

	m_theRecMode = rmBrowse;
}

gak::int64 Record::rebalance( DbFile *dataFileHandle, gak::int64 curPos, RecordHeader &curHeader, gak::int64 prevPos, RecordHeader &prevHeader, bool cur2Small, bool prev2Small )
{
	RecordHeader	otherHeader;
	// rebalance
	bool higherIsPrev = (curHeader.higherRecordPtr == prevPos);
	gak::int64	otherPos = higherIsPrev ? curHeader.lowerRecordPtr : curHeader.higherRecordPtr;
	if( otherPos ) readRecordHeader( otherPos, dataFileHandle, &otherHeader );

	RecordHeader	tmpHeader;
	gak::int64		tmpPos;

	RecordHeader	rootHeader;
	gak::int64		rootPos = curHeader.topPtr;
	readRecordHeader( rootPos, dataFileHandle, &rootHeader );

	if( !higherIsPrev && cur2Small )
	{
		//                cur(5)
		//      PREV(3) <-      -> other(7)
		// (2)<-     ->tmp(4)   (6)<-      ->(8)

		//             PREV(3)
		//        (2)<-        -> cur(5)
		//                tmp(4)<-      -> other(7)
		tmpPos = prevHeader.higherRecordPtr;
		if(tmpPos) readRecordHeader( tmpPos, dataFileHandle, &tmpHeader );

		prevHeader.topPtr = curHeader.topPtr;
		prevHeader.higherRecordPtr = curPos;

		curHeader.topPtr = prevPos;
		curHeader.lowerRecordPtr = tmpPos;

		prevHeader.numRecords -= tmpHeader.numRecords;
		curHeader.numRecords = tmpHeader.numRecords + otherHeader.numRecords +1;
		prevHeader.numRecords += curHeader.numRecords;

		tmpHeader.topPtr = curPos;

		if( rootHeader.lowerRecordPtr == curPos )
			rootHeader.lowerRecordPtr = prevPos;
		else
			rootHeader.higherRecordPtr = prevPos;
	}
	else if( !higherIsPrev && prev2Small )
	{
		//                 cur(5)
		//      prev(3)<-          ->OTHER(7)
		// (2)<-       ->(4)  tmp(6)<-      ->(8)


		//                        OTHER(7)
		//                cur(5)<-        ->(8)
		//      prev(3) <-      ->tmp(6)
		// (2)<-       ->(4)
		assert( otherPos );
		tmpPos = otherHeader.lowerRecordPtr;
		if(tmpPos) readRecordHeader( tmpPos, dataFileHandle, &tmpHeader );

		otherHeader.topPtr = curHeader.topPtr;
		otherHeader.lowerRecordPtr = curPos;

		curHeader.topPtr = otherPos;
		curHeader.higherRecordPtr = tmpPos;

		otherHeader.numRecords -= tmpHeader.numRecords;
		curHeader.numRecords = prevHeader.numRecords + tmpHeader.numRecords + 1;
		otherHeader.numRecords += curHeader.numRecords;

		tmpHeader.topPtr = curPos;

		if( rootHeader.lowerRecordPtr == curPos )
			rootHeader.lowerRecordPtr = otherPos;
		else
			rootHeader.higherRecordPtr = otherPos;
	}
	else if( higherIsPrev && prev2Small )
	{
		//                  cur(5)
		//      OTHER(3) <-       -> prev(7)
		// (2)<-        ->tmp(4)  (6)<-     ->(8)

		//      OTHER(3)
		// (2)<-        ->cur(5)
		//        tmp(4)<-      ->prev(7)
		//                   (6)<-       ->(8)
		assert( otherPos );
		tmpPos = otherHeader.higherRecordPtr;
		if(tmpPos) readRecordHeader( tmpPos, dataFileHandle, &tmpHeader );

		otherHeader.topPtr = curHeader.topPtr;
		otherHeader.higherRecordPtr = curPos;

		curHeader.topPtr = otherPos;
		curHeader.lowerRecordPtr = tmpPos;

		otherHeader.numRecords -= tmpHeader.numRecords;
		curHeader.numRecords = tmpHeader.numRecords + prevHeader.numRecords +1;
		otherHeader.numRecords += curHeader.numRecords;

		tmpHeader.topPtr = curPos;

		if( rootHeader.lowerRecordPtr == curPos )
			rootHeader.lowerRecordPtr = otherPos;
		else
			rootHeader.higherRecordPtr = otherPos;
	}
	else if( higherIsPrev && cur2Small )
	{
		//                cur(5)
		//      other(3)<-      ->PREV(7)
		// (2)<-        ->(4)  tmp(6)<-  ->(8)

		//                        PREV(7)
		//                cur(5)<-       ->(8)
		//      other(3)<-      ->tmp(6)
		// (2)<-        ->(4)

		tmpPos = prevHeader.lowerRecordPtr;
		if(tmpPos) readRecordHeader( tmpPos, dataFileHandle, &tmpHeader );

		prevHeader.topPtr = curHeader.topPtr;
		prevHeader.lowerRecordPtr = curPos;

		curHeader.topPtr = prevPos;
		curHeader.higherRecordPtr = tmpPos;

		prevHeader.numRecords -= tmpHeader.numRecords;
		curHeader.numRecords = otherHeader.numRecords + tmpHeader.numRecords +1;
		prevHeader.numRecords += curHeader.numRecords;

		tmpHeader.topPtr = curPos;

		if( rootHeader.lowerRecordPtr == curPos )
			rootHeader.lowerRecordPtr = prevPos;
		else
			rootHeader.higherRecordPtr = prevPos;
	}

	writeRecordHeader( curPos, dataFileHandle, &curHeader );
	writeRecordHeader( prevPos, dataFileHandle, &prevHeader );

	if(otherPos) writeRecordHeader( otherPos, dataFileHandle, &otherHeader );
	if(tmpPos) writeRecordHeader( tmpPos, dataFileHandle, &tmpHeader );

	rootHeader.numRecords++;
	writeRecordHeader( rootPos, dataFileHandle, &rootHeader );

	curHeader = rootHeader;
	return rootPos;
}

void Record::postRecord( DbFile *dataFileHandle )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::postRecord" );
	STRING			theValues, theStringLengths;
	int				compareVal = 0;
	gak::int64		fileLength;

	RecordHeader	curHeader;
	gak::int64		curPos = 0;

	// construct the record
	getRecord( &theValues, false, &theStringLengths );

	// find out position of best matching record
	fileLength = dataFileHandle->toEnd() - TABLE_HEADER_SIZE;

	// create the unique node id
	theValues += gak::formatBinary(fileLength, 16, 16, '0');

	if( fileLength )
	{
		curPos = TABLE_HEADER_SIZE;
		compareVal = locateValue( dataFileHandle, &curPos, &curHeader, theValues, false );
	}

	m_theHeader.topPtr = curPos;
	m_theHeader.lowerRecordPtr = m_theHeader.higherRecordPtr = 0; 
	m_theHeader.numRecords = 1;

	// now we can create the new record
	m_currentPosition = dataFileHandle->toEnd();
	theStringLengths += ";EOB";
	theValues += ";EOB";
	m_theHeader.stringLengths = strlen( theStringLengths );
	m_theHeader.bufferLen = strlen( theValues );
	writeRecordHeader( dataFileHandle, &m_theHeader );
	dataFileHandle->write( (void*)((const char *)theValues), m_theHeader.bufferLen );
	dataFileHandle->write( (void*)((const char *)theStringLengths), m_theHeader.stringLengths );

	// now we can insert the new record in our tree
	if( curPos )
	{
		if( compareVal < 0 )
			curHeader.higherRecordPtr = m_currentPosition;
		else if( compareVal > 0 )
			curHeader.lowerRecordPtr = m_currentPosition;
		curHeader.numRecords++;

		writeRecordHeader( curPos, dataFileHandle, &curHeader );
		gak::int64 prevSize = 1;
		gak::int64 prevPos = curPos;
		RecordHeader prevHeader = curHeader;
		while( 1 )
		{
			curPos = curHeader.topPtr;
			if( curPos > 0 )
			{
				readRecordHeader( curPos, dataFileHandle, &curHeader );
				gak::int64 curSize = ++curHeader.numRecords;

				if( curHeader.topPtr > 0 )
				{
#if 1
					// the current record is somewhere within the database
					bool prev2Small = prevSize > 4 && prevSize*4 < curSize;
					bool cur2Small = curSize > 4 && curSize/4 < prevSize;

					if( prev2Small || cur2Small )
					{
						curPos = rebalance(dataFileHandle, curPos, curHeader, prevPos, prevHeader, cur2Small, prev2Small );
						curSize = curHeader.numRecords;
					}
					else
#endif
					{
						writeRecordHeader( curPos, dataFileHandle, &curHeader );
					}
					prevSize = curSize;
					prevPos = curPos;
					prevHeader = curHeader;
				}
				else
				{
					// the current record is the root -> update the recordf then break;
					writeRecordHeader( curPos, dataFileHandle, &curHeader );
					break;
				}
			}
			else
				break;
		}
	}

	m_theRecMode = rmBrowse;
}


void Record::backupValues( void )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::backupValues" );
	for( size_t i=0; i<m_theHeader.numFields; i++ )
	{
		getFieldValue( i )->backupValue();
	}
}


void Record::deleteRecord( DbFile *dataFileHandle, bool noMove )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::deleteRecord" );
	SetDeleted( &m_theHeader );

	writeRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );

	if( !noMove )
	{
		nextRecord( dataFileHandle );
		if( m_theRecMode == rmEof )
			prevRecord( dataFileHandle );
	}
	else
		ClrDeleted( &m_theHeader );
}

void Record::root( DbFile *dataFileHandle )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::firstRecord" );
	gak::int64 fileLength = dataFileHandle->toEnd()-TABLE_HEADER_SIZE;
	if( fileLength<=0 )
		m_theRecMode = rmEof;
	else
	{
		m_currentPosition = TABLE_HEADER_SIZE;
		readRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );
	}
}

void Record::firstRecord( DbFile *dataFileHandle, const STRING &searchBuffer )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::firstRecord" );
	gak::int64 fileLength = dataFileHandle->toEnd()-TABLE_HEADER_SIZE;
	if( fileLength<=0 )
		m_theRecMode = rmEof;
	else
	{
		m_currentPosition = TABLE_HEADER_SIZE;
		while( true )
		{
			readRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );

			if( m_theHeader.lowerRecordPtr )
				m_currentPosition = m_theHeader.lowerRecordPtr;
			else if( !IsDeleted( m_theHeader ) )
			{
				readRecord( dataFileHandle );
				if( searchBuffer[0U] )
				{
					STRING theValues;
					getRecord( &theValues, false, NULL );
					if( strncmp( theValues, searchBuffer, strlen( searchBuffer ) ) )
						nextRecord( dataFileHandle, searchBuffer );
				}
				break;
			}
			else
			{
				nextRecord( dataFileHandle, searchBuffer );
				break;
			}
		}
	}
}

void Record::nextRecord( DbFile *dataFileHandle, const STRING &searchBuffer )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::nextRecord" );
	gak::int64	oldPosition;
	bool		found;
	do
	{
		found = false;

		if( m_theHeader.higherRecordPtr )
		{
			/*
				if there is a higher record, we walk to this record and
				then go to the lowes possible record from here
			*/
			m_currentPosition = m_theHeader.higherRecordPtr;
			readRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );
			while( m_theHeader.lowerRecordPtr )
			{
				m_currentPosition = m_theHeader.lowerRecordPtr;
				readRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );
			}
			found = true;	// whe have found a possible record
		}
		else
		{
			/*
				walk through top pointer until next higherPointer
				does not point to myself
			*/
			while( !found )
			{
				oldPosition = m_currentPosition;
				m_currentPosition = m_theHeader.topPtr;
				if( !m_currentPosition )
/*v*/				break;					// the end no more data

				readRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );

				if( m_theHeader.higherRecordPtr == oldPosition )
/*^*/				continue;				// go to next top node
				else
					found = true;			// may be this is not deleted
			}
		}
	} while( (!found || IsDeleted( m_theHeader )) && m_currentPosition );

	if( !m_currentPosition )
		m_theRecMode = rmEof;
	else
	{
		readRecord( dataFileHandle );
		if( searchBuffer[0U] )
		{
			STRING theValues;
			getRecord( &theValues, false, NULL );
			if( strncmp( theValues, searchBuffer, strlen( searchBuffer ) ) )
				nextRecord( dataFileHandle, searchBuffer );
		}
	}
}

void Record::prevRecord( DbFile *dataFileHandle, const STRING &searchBuffer )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::prevRecord" );
	bool	found;
	do
	{
		found = false;

		if( m_theHeader.lowerRecordPtr )
		{
			/*
				if there is a lower record, we walk to this record and
				then go to the highest possible record from here
			*/
			m_currentPosition = m_theHeader.lowerRecordPtr;
			readRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );
			while( m_theHeader.higherRecordPtr )
			{
				m_currentPosition = m_theHeader.higherRecordPtr;
				readRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );
			}
			found = true;
		}
		else
		{
			/*
				walk through top pointer until next higherPointer
				does not point to myself
			*/
			while( !found )
			{
				gak::int64	oldPosition = m_currentPosition;
				m_currentPosition = m_theHeader.topPtr;

				if( !m_currentPosition )
/*v*/				break;					// the end no more data

				readRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );

				if( m_theHeader.lowerRecordPtr == oldPosition )
/*^*/				continue;				// go to next top node
				else
					found = true;			// may be this is not deleted
			}
		}
	} while( (!found || IsDeleted( m_theHeader )) && m_currentPosition );

	if( !m_currentPosition )
		m_theRecMode = rmBof;
	else
	{
		readRecord( dataFileHandle );
		if( searchBuffer[0U] )
		{
			STRING theValues;
			getRecord( &theValues, false, NULL );
			if( strncmp( theValues, searchBuffer, strlen( searchBuffer ) ) )
				prevRecord( dataFileHandle, searchBuffer );
		}
	}
}

void Record::lastRecord( DbFile *dataFileHandle, const STRING &searchBuffer )
{
	doEnterFunctionEx( gakLogging::llDetail, "Record::lastRecord" );
	gak::int64 fileLength = dataFileHandle->toEnd()-TABLE_HEADER_SIZE;	// table header
	if( !fileLength )
		m_theRecMode = rmBof;
	else
	{
		m_currentPosition = TABLE_HEADER_SIZE;
		while( true )
		{
			readRecordHeader( m_currentPosition, dataFileHandle, &m_theHeader );

			if( m_theHeader.higherRecordPtr )
				m_currentPosition = m_theHeader.higherRecordPtr;
			else if( !IsDeleted( m_theHeader ) )
			{
				readRecord( dataFileHandle );
				if( searchBuffer[0U] )
				{
					STRING theValues;
					getRecord( &theValues, false, NULL );
					if( strncmp( theValues, searchBuffer, strlen( searchBuffer ) ) )
						prevRecord( dataFileHandle, searchBuffer );
				}
				break;
			}
			else
			{
				prevRecord( dataFileHandle, searchBuffer );
				break;
			}
		}
	}
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

// --------------------------------------------------------------------- //
// ----- entry points -------------------------------------------------- //
// --------------------------------------------------------------------- //

} // namespace dbLib

#ifdef __BORLANDC__
#	pragma option -RT.
#	pragma option -a.
#	pragma option -p.
#endif

