/*
		Project:		dbLIB
		Module:			fieldValue.h
		Description:	The definitions for a table field value
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

#ifndef DB_FIELD_VALUE_H
#define DB_FIELD_VALUE_H

// --------------------------------------------------------------------- //
// ----- switches ------------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- includes ------------------------------------------------------ //
// --------------------------------------------------------------------- //

#include <gak/string.h>
#include <gak/fmtNumber.h>
#include <gak/numericString.h>

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

enum fType
{
	ftBoolean, ftInteger, ftNumber, ftDate, ftString, ftBlob
};

struct FieldDefinition
{
	gak::STRING	name;
	fType		type;
	bool		primary;
	bool		notNull;
	gak::STRING	reference;
};

typedef gak::Array<FieldDefinition> FieldDefinitions;

// --------------------------------------------------------------------- //
// ----- class definitions --------------------------------------------- //
// --------------------------------------------------------------------- //

class FieldValue
{
	const FieldDefinition	*m_definition;
	gak::STRING				m_fieldValue;
	gak::STRING				m_fieldBackup;

	public:
	FieldValue()
	{
		m_definition = NULL;
	}

	void setDefinition( const FieldDefinition *definition )
	{
		m_definition = definition;
	}

	void backupValue()
	{
		m_fieldBackup = m_fieldValue;
	}
	gak::STRING getBackupValue() const
	{
		return m_fieldBackup;
	}
	fType getType() const
	{
		return m_definition ? m_definition->type : ftString;
	}
	bool isPrimary() const
	{
		return m_definition ? m_definition->primary : false;
	}
	bool notNull() const
	{
		return m_definition ? m_definition->notNull : false;
	}
	const gak::STRING &getName() const
	{
		return m_definition->name;
	}

	bool isNull() const
	{
		return !m_fieldValue[0U];
	}
	void setNull()
	{
		backupValue();
		m_fieldValue = (const char*)NULL;
	}

	const gak::STRING &getStringValue() const
	{
		return m_fieldValue;
	}
	void setStringValue( const gak::STRING &value )
	{
		backupValue();
		m_fieldValue = value;
	}
	void setIntegerValue( long value )
	{
		backupValue();
		m_fieldValue = gak::formatBinary(value, 16, 16 );
	}
	long getIntegerValue() const
	{
		return m_fieldValue.getValueN<long>(16);
	}
	void setDoubleValue( double value )
	{
		backupValue();
		m_fieldValue = gak::formatNumber( value );
	}
	double getDoubleValue() const
	{
		double	value;

		value = m_fieldValue.getValueN<double>();

		return value;
	}
	void setBooleanValue( bool value )
	{
		backupValue();
		m_fieldValue = value ? 'Y' : 'N';
	}
	bool getBooleanValue() const
	{
		return m_fieldValue[0U] == 'Y';
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

