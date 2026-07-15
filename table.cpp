/*
		Project:		dbLIB
		Module:			table.cpp
		Description:	The definition for the entire table
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

#include <fstream>

#include <gak/xml.h>
#include <gak/xmlParser.h>

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
using gak::xml::Any;
using gak::xml::Parser;
using gak::xml::Document;
using gak::xml::Element;

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

Table::~Table()
{
	for( size_t i=0; i<m_indices.size(); i++ )
		delete m_indices[i];
}

// --------------------------------------------------------------------- //
// ----- class static functions ---------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class privates ------------------------------------------------ //
// --------------------------------------------------------------------- //

void Table::writeDefinition() const
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::writeDefinition" );
	STRING	indexPath, indexName;
	Index	*theIndex;
	Any		*theXmlIndex;
	STRING	myPath = getPathName();

	std::unique_ptr<Any> theTableDefinition(new Any( TABLE_DEFINITION ));

	Any		*theXmlFieldDefs = static_cast<Any*>(theTableDefinition->addObject(new Any(FIELD_DEFS)));
	Any		*theXmlIndexDefs = static_cast<Any*>(theTableDefinition->addObject(new Any(INDICES)));

	writeXmlDefinition( theXmlFieldDefs );

	for( size_t i=0; i<m_indices.size(); i++ )
	{
		theXmlIndex = static_cast<Any*>(theXmlIndexDefs->addObject(new Any(INDEX)));

		theIndex = m_indices[i];
		indexPath = theIndex->getPathName();
		indexName = indexPath.rightString( strlen( indexPath ) - strlen( myPath ) -1 );
		theXmlIndex->setStringAttribute( NAME, indexName );
		theIndex->writeXmlDefinition( theXmlIndex );
	}
	STRING	xmlCode = theTableDefinition->generateDoc();
	std::ofstream	fStream( m_definitionFile );
	if( !fStream.fail() )
	{
		fStream << xmlCode;
	}
}

Index *Table::findIndexFromPath( const STRING &indexPath ) const
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::findIndexFromPath" );

	size_t	i;

	for( i=0; i<m_indices.size(); i++ )
		if( indexPath == m_indices[i]->getPathName() )
/***/		return m_indices[i];

	return NULL;
}

Index *Table::findIndexFromName( const STRING &indexName ) const
{
	STRING	indexPath = getIndexPathName(indexName);

	return findIndexFromPath( indexPath );
}

void Table::checkKeyViolation(Index *theIndex)
{
	STRING		key;

	for( size_t fieldIdx=0; fieldIdx < theIndex->getNumFields()-1; fieldIdx++ )
	{
		FieldValue	*indexField = theIndex->getField( fieldIdx );
		if( indexField->isPrimary() )
		{
			FieldValue	*myField = getField( indexField->getName() );
			if( fieldIdx )
				key += ';';
			key += myField->getStringValue();
		}
		else
/*v*/		break;
	}

	if( key[0U] )
	{
		gak::int64 posFound;
		bool isDeleted;
		int compareVal = theIndex->locateValue(
			&posFound,
			&isDeleted, key, true
		);
		if( !compareVal && posFound )
		{
			if( m_currentRecord.getCurrentPosition() )
			{
				theIndex->readRecord( posFound );
			}
			if( !m_currentRecord.getCurrentPosition()
			|| theIndex->getField( theIndex->getNumFields()-1 )->getIntegerValue() != m_currentRecord.getCurrentPosition() )
			{
				throw DBkeyViolation( theIndex->getPathName() );
			}
		}
	}
}

void Table::insertKeyRecord(Index *theIndex)
{
	FieldValue	*myField, *indexField;

	theIndex->insertRecord();
	for( size_t fieldIdx=0; fieldIdx < theIndex->getNumFields()-1; fieldIdx++ )
	{
		indexField = theIndex->getField( fieldIdx );
		myField = getField( indexField->getName() );
		indexField->setStringValue( myField->getStringValue() );
	}
	theIndex->getField( theIndex->getNumFields()-1 )->setIntegerValue( m_currentRecord.getCurrentPosition() );
	theIndex->postRecord();
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

void Table::open()
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::open" );

	Parser   theParser( m_definitionFile );

	std::unique_ptr<Document> theDefinitionDoc( theParser.readFile(false) );	/// TODO check boolean

	Element *theTableDefinition = theDefinitionDoc->getElement( TABLE_DEFINITION );
	if( theTableDefinition )
	{
		Element *theXmlFieldDefs = theTableDefinition->getElement( FIELD_DEFS );
		if( theXmlFieldDefs )
			Index::open( theXmlFieldDefs );

		Element *theXmlIndexDefs = theTableDefinition->getElement( INDICES );

		for( size_t i=0; i<theXmlIndexDefs->getNumObjects(); i++ )
		{
			Element		*theXmlIndex = theXmlIndexDefs->getElement( i );
			if( theXmlIndex && theXmlIndex->getTag() == INDEX )
			{
				STRING	indexName = theXmlIndex->getAttribute( NAME );
				STRING	indexPath = getIndexPathName(indexName);

				Index	*newIndex = new Index( m_database, indexName, indexPath );
				newIndex->open( theXmlIndex );
				m_indices.addElement( newIndex );
			}
		}

	}
}

void Table::addRefference(const STRING &fromTable, const STRING &fromField, const STRING &toField)
{
	/// TODO allow empty fieldname and using primary index
	size_t fieldIdx = findField( toField );
	if( no_index == fieldIdx )
	{
		throw DBfieldNotFound( toField );
	}
	FieldDefinition &fieldDef = getFieldDef( fieldIdx );
	FieldSpec &ref = fieldDef.refBy.createElement();
	ref.table = fromTable;
	ref.fieldName = fromField;

	if( !findIndexFromName( toField ) )
	{
		createIndex( toField );
		addFieldToIndex( toField, toField, true, true );
	}

	writeDefinition();
}

void Table::addField(
	const STRING &name, fType type,
	bool primary,
	bool notNulls,
	const STRING &reference
)
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::addField" );

	Index::addField( name, type, primary, notNulls, reference );
	FieldSpec ref( reference );
	if( ref.good() )
	{
		std::unique_ptr<Table> refTable( m_database->openTable(ref.table) );
		refTable->addRefference(m_tableName, name, ref.fieldName);
		createIndex(name);
		addFieldToIndex(name,name,false,true);
	}

	writeDefinition();
}

void Table::checkReferences4Post()
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::checkReferences4Post" );
	size_t idx = 0;
	for(
		FieldDefinitions::const_iterator it = m_fieldDefinitions.cbegin(), endIT = m_fieldDefinitions.cend();
		it != endIT;
		++it, ++idx
	)
	{
		const FieldDefinition &fDef = *it;
		FieldSpec ref = fDef.ref;
		if( ref.good() )
		{
			std::unique_ptr<Table>	tt( m_database->openTable(ref.table) );
			if( !ref.fieldName.isEmpty() )
			{
				tt->setIndex(ref.fieldName);
			}
			FieldValue *fv = getField( idx );
			gak::STRING	fvStr = fv->getStringValue();
			tt->firstRecord( fvStr );
			if( tt->eof() )
			{
				throw DBrefMasterNotFound(m_tableName, fDef.name, fvStr, ref.table, ref.fieldName );
			}
		}
	}
}

void Table::postRecord()
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::postRecord" );

	checkReferences4Post();

	/*
		check for primary keys
	*/

	// my own key:
	//============
	STRING		key;

	m_currentRecord.getRecord( &key, true, NULL );

	if( key[0U] )
	{
		bool isDeleted;
		gak::int64	posFound;
		int compareVal = locateValue(
			&posFound,
			&isDeleted,
			key, true
		);

		if( !compareVal && posFound && m_currentRecord.getCurrentPosition() != posFound )
		{
			throw DBkeyViolation( getPathName() );
		}
	}

	// unique indices
	//===============

	size_t	numIndices = m_indices.size();
	for( size_t i=0; i<numIndices; i++ )
	{
		Index		*theIndex = m_indices[i];
		checkKeyViolation(theIndex);
	}

	if( m_currentRecord.m_theRecMode == rmBrowse )
	{
		deleteRecord( true );
	}

	m_currentRecord.postRecord( m_dataFileHandle );

	for( size_t i=0; i<numIndices; i++ )
	{
		Index		*theIndex = m_indices[i];
		insertKeyRecord(theIndex);
	}

	// if we have survived the post, backup the values
	m_currentRecord.backupValues();
}

void Table::checkReferences4Delete()
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::checkReferences4Delete" );
	size_t idx = 0;
	for(
		FieldDefinitions::const_iterator it = m_fieldDefinitions.cbegin(), endIT = m_fieldDefinitions.cend();
		it != endIT;
		++it, ++idx
	)
	{
		const FieldDefinition	&fd = *it;
		const STRING			&fv = getField( idx )->getStringValue();

		for( 
			FieldSpecs::const_iterator it = fd.refBy.cbegin(), endIT = fd.refBy.cend();
			it != endIT;
			++it
		)
		{
			const FieldSpec fs = *it;
			std::unique_ptr<Table>	tt( m_database->openTable(fs.table) );
			if( !fs.fieldName.isEmpty() )
			{
				tt->setIndex(fs.fieldName);
			}
			tt->firstRecord(fv);
			if( !tt->eof() )
			{
				throw DBrefDetailFound(fs.table, fs.fieldName, fv, m_tableName, fd.name );
			}
		}
	}
}

void Table::deleteRecord( bool noMove )
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::deleteRecord" );

	checkReferences4Delete();

	STRING		searchBuffer;

	for( size_t i=0; i<m_indices.size(); i++ )
	{
		Index		*theIndex = m_indices[i];
		FieldValue	*myField, *indexField;

		searchBuffer = "";
		for( size_t fieldIdx=0; fieldIdx < theIndex->getNumFields()-1; fieldIdx++ )
		{
			indexField = theIndex->getField( fieldIdx );
			myField = getField( indexField->getName() );
			searchBuffer += myField->getBackupValue();
			searchBuffer += ';';
		}

		searchBuffer += gak::formatNumber(m_currentRecord.getCurrentPosition());

		theIndex->firstRecord( searchBuffer );
		if( !theIndex->eof() )
			theIndex->deleteRecord();
	}

	m_currentRecord.deleteRecord( m_dataFileHandle, noMove );
}

void Table::firstRecord( const STRING &searchBuffer )
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::firstRecord" );

	if( m_currentIndex )
	{
		// search the pattern in the index
		m_currentIndex->firstRecord( searchBuffer );
		if( !m_currentIndex->eof() )
		{
			while( m_currentRecord.readRecord(
					m_dataFileHandle,
					m_currentIndex->getField(
						m_currentIndex->getNumFields() -1
					)->getIntegerValue()
				)
			)
			{
				// If there was a problem e.g. deleted record, check next record
				m_currentIndex->nextRecord();
				if( m_currentIndex->eof() )
				{
					// stop if index ist finished, too
					m_currentRecord.m_theRecMode = rmEof;
					break;
				}
			}
		}
		else
			m_currentRecord.m_theRecMode = rmEof;

	}
	else
		Index::firstRecord( searchBuffer );
}

void Table::nextRecord()
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::nextRecord" );

	if( m_currentIndex )
	{
		while( true )
		{
			m_currentIndex->nextRecord();
			if( !m_currentIndex->eof() )
			{
				// we found a record in index
				if( !m_currentRecord.readRecord(
						m_dataFileHandle,
						m_currentIndex->getField(
							m_currentIndex->getNumFields() -1
						)->getIntegerValue()
					)
				)
				{
					// no problems (record was not deleted)
					break;
				}
			}
			else
			{
				// index is at eof
				m_currentRecord.m_theRecMode = rmEof;
				break;
			}
		}

	}
	else
		Index::nextRecord();
}

void Table::previousRecord()
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::previousRecord" );
	if( m_currentIndex )
	{
		while( true )
		{
			m_currentIndex->previousRecord();
			if( !m_currentIndex->bof() )
			{
				// we found a record in index
				if( !m_currentRecord.readRecord(
						m_dataFileHandle,
						m_currentIndex->getField(
							m_currentIndex->getNumFields() -1
						)->getIntegerValue()
					)
				)
				{
					// no problems (record was not deleted)
					break;
				}
			}
			else
			{
				// index is at bof
				m_currentRecord.m_theRecMode = rmBof;
				break;
			}
		}

	}
	else
		Index::previousRecord();
}

void Table::lastRecord( const STRING &searchBuffer )
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::lastRecord" );

	if( m_currentIndex )
	{
		// search the pattern in the index
		m_currentIndex->lastRecord( searchBuffer );
		if( !m_currentIndex->bof() )
		{
			while( 
				m_currentRecord.readRecord(
					m_dataFileHandle,
					m_currentIndex->getField(
						m_currentIndex->getNumFields() -1
					)->getIntegerValue()
				)
			)
			{
				// If there was a problem e.g. deleted record, check previous record
				m_currentIndex->previousRecord();
				if( m_currentIndex->bof() )
				{
					// stop if index ist finished, too
					m_currentRecord.m_theRecMode = rmBof;
					break;
				}
			}
		}
		else
			m_currentRecord.m_theRecMode = rmBof;

	}
	else
		Index::lastRecord( searchBuffer );
}

void Table::createIndex( const STRING &indexName )
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::createIndex" );
	STRING	indexPath = getIndexPathName(indexName);

	if( findIndexFromPath( indexPath ) )
		throw DBindexExist( indexName );

	Index	*newIndex = new Index( m_database, indexName, indexPath );
	newIndex->create();
	m_indices.addElement( newIndex );

	writeDefinition();
}

void Table::addFieldToIndex( const STRING &indexName, const STRING &fieldName, bool primary, bool lastField )
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::addFieldToIndex" );
	Index	*theIndex;
	STRING	indexPath = getIndexPathName(indexName);

	if( (theIndex = findIndexFromPath( indexPath )) == NULL )
		throw DBindexNotFound( indexName );

	if( theIndex == m_currentIndex )
		throw DBindexExist( indexName );		/// TODO better exception

	size_t	fieldIdx = findField( fieldName );
	if( fieldIdx == Index::no_index )
	{
		throw DBfieldNotFound( fieldName );
	}

	const FieldDefinition &fieldDef = getFieldDef( fieldIdx );

	theIndex->addField( fieldName, fieldDef.type, primary );

	if( lastField )
	{
		theIndex->addField( "REC_POS", ftNumber, false );
		try
		{
			refreshIndex(theIndex);
		}
		catch( ... )
		{
			dropIndex(indexName);
			throw;
		}
	}

	writeDefinition();
}

void Table::refreshIndex( Index *theIndex )
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::refreshIndex" );
	assert( theIndex != m_currentIndex );

	theIndex->truncateFile();

	for( firstRecord(); !eof(); nextRecord() )
	{
		checkKeyViolation(theIndex);
		insertKeyRecord(theIndex);
	}
}

void Table::setIndex( const STRING &indexName )
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::setIndex" );
	if( indexName[0U] )
	{
		STRING	indexPath = getIndexPathName(indexName);

		if( (m_currentIndex = findIndexFromPath( indexPath )) == NULL )
			throw DBindexNotFound( indexName );
	}
	else
		m_currentIndex = NULL;
}

void Table::dropIndex( const gak::STRING &indexName )
{
	doEnterFunctionEx( gakLogging::llDetail, "Table::dropIndex" );
	Index	*theIndex;
	STRING	indexPath = getIndexPathName(indexName);

	if( (theIndex = findIndexFromPath( indexPath )) == NULL )
		throw DBindexNotFound( indexName );

	if(  theIndex == m_currentIndex )
		m_currentIndex = nullptr;

	theIndex->dropDataFile();
	m_indices.removeElementVal( theIndex );
	delete theIndex;
	writeDefinition();
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

