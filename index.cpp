/*
		Project:		dbLIB
		Module:			index.cpp
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

// --------------------------------------------------------------------- //
// ----- switches ------------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- includes ------------------------------------------------------ //
// --------------------------------------------------------------------- //

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
using gak::xml::Element;
using gak::xml::Any;

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

const size_t Index::no_index = -1;

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

size_t Index::findField( const char *fieldName )
{
	doEnterFunctionEx( gakLogging::llDetail, "Index::findField" );

	size_t			fieldIdx = no_index;

	for( size_t i=0; i<m_fieldDefinitions.size(); ++i )
	{
		if( !strcmpi( fieldName, m_fieldDefinitions[i].name ) )
		{
			fieldIdx = i;
			break;
		}
	}

	return fieldIdx;
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

void Index::open( Element *theXmlFieldDefs )
{
	doEnterFunctionEx( gakLogging::llDetail, "Index::open" );

	int					tmp;
	STRING				value;
	size_t				defIdx = 0;

	m_fieldDefinitions.setMinSize(theXmlFieldDefs->getNumObjects());

	for( size_t i=0; i<theXmlFieldDefs->getNumObjects(); i++ )
	{
		Any *theField = static_cast<Any*>(theXmlFieldDefs->getElement( i ));

		if( theField && theField->getTag() == "FIELD" )
		{
			FieldDefinition		&fieldDef = m_fieldDefinitions[defIdx++];

			fieldDef.name = theField->getAttribute( "NAME" );
			value = theField->getAttribute( "TYPE" );
			tmp = value.getValueN<int>();
			fieldDef.type = (fType)tmp;

			value = theField->getAttribute( "PRIMARY" );
			fieldDef.primary = (value[0U] == 'Y');

			value = theField->getAttribute( "NOT_NULL" );
			fieldDef.notNull = (value[0U] == 'Y');

			fieldDef.reference = theField->getAttribute( "REFERENCE" );
		}
	}

	m_currentRecord.createRecord( m_fieldDefinitions );

	firstRecord();
}

void Index::writeXmlDefinition( Element *theXmlFieldDefs ) const
{
	doEnterFunctionEx( gakLogging::llDetail, "Index::writeXmlDefinition" );

	for( size_t i=0; i<m_fieldDefinitions.size(); ++i )
	{
		const FieldDefinition &fieldDef = m_fieldDefinitions[i];

		Any	*theField = static_cast<Any*>(theXmlFieldDefs->addObject( new Any( "FIELD" ) ));

		theField->setStringAttribute( "NAME", fieldDef.name );
		theField->setIntegerAttribute( "TYPE", (int)fieldDef.type );
		theField->setStringAttribute( "PRIMARY", fieldDef.primary ? "Y" : "N" );
		theField->setStringAttribute( "NOT_NULL", fieldDef.notNull ? "Y" : "N" );
		theField->setStringAttribute( "REFERENCE", fieldDef.reference );
	}
}

void Index::truncateFile()
{
	closeTableFile( m_dataFileHandle );
	strRemove( m_dataFile );
	m_dataFileHandle = openTableFile( m_dataFile );
	create();
}

void Index::create()
{
	doEnterFunctionEx( gakLogging::llDetail, "Index::create" );

	m_dataFileHandle->toStart();
	m_dataFileHandle->write( TABLE_HEADER, TABLE_HEADER_SIZE );
}

void Index::addField(
	const STRING &name, fType type,
	bool primary,
	bool notNulls,
	const STRING &reference
)
{
	doEnterFunctionEx( gakLogging::llDetail, "Index::addField" );

	size_t	fieldIdx = findField( name );
	if( fieldIdx != no_index )
	{
		throw DBfieldExist( name );
	}

	FieldDefinition	&newDef = m_fieldDefinitions.createElement();
	newDef.name = name;
	newDef.type = type;
	newDef.primary = primary;
	newDef.notNull = notNulls;
	newDef.reference = reference;

	m_currentRecord.createRecord( m_fieldDefinitions );
}

FieldValue *Index::getField( const STRING &name )
{
	doEnterFunctionEx( gakLogging::llDetail, "Index::getField( const STRING &name )" );

	size_t	fieldIdx = findField( name );

	if( fieldIdx == no_index )
	{
		throw DBfieldNotFound( name );
	}

	return m_currentRecord.getFieldValue( fieldIdx );
}

FieldValue *Index::getField( size_t fieldIdx )
{
	doEnterFunctionEx( gakLogging::llDetail, "Index::getField( size_t fieldIdx )" );

	if( fieldIdx >= m_fieldDefinitions.size() )
	{
		STRING	field;

		field += gak::formatNumber(fieldIdx);
		throw DBfieldNotFound( field );
	}

	return m_currentRecord.getFieldValue( fieldIdx );
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

