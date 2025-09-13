/*
		Project:		dbLIB
		Module:			db_exception.h
		Description:	the database exception
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

#ifndef DBLIB_EXCEPTION_H
#define DBLIB_EXCEPTION_H

// --------------------------------------------------------------------- //
// ----- switches ------------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- includes ------------------------------------------------------ //
// --------------------------------------------------------------------- //

#include <stdexcept>

#include <gak/string.h>

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

class DBexception : public std::exception
{
	protected:
	enum DbErrorCode
	{
		DB_NOERROR,

		// logical errors
		DB_EXISTS, DB_NOT_EXISTS,
		TABLE_EXISTS, TABLE_NOT_EXISTS,
		INDEX_EXISTS, INDEX_NOT_EXISTS,
		FIELD_EXISTS, FIELD_NOT_EXISTS,
		KEY_VIOLATION, NULL_VALUE,

		// FS Errors
		MKDIR_FAILED, OPEN_FAILED,

		// Record Errors
		ILLEGAL_RECORD_HEADER, ILLEGAL_RECORD_LEN,

		// OS Errors
		NO_MEMORY
	};

	gak::STRING		m_objName;
	DbErrorCode		m_errCode;

	protected:

	DBexception( DbErrorCode errCode )
	{
		m_errCode = errCode;
	}
	DBexception( DbErrorCode errCode, const gak::STRING &objName )
	{
		m_errCode = errCode;
		m_objName = objName;
	}
	private:
	virtual const char *getErrText() const
	{
		return "";
	}
	public:
	virtual const char *what() const;
};

class DBmemoryException : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: No Memory";
	}
	public:
	DBmemoryException() : DBexception( NO_MEMORY )
	{
	}
	DBmemoryException(const gak::STRING &objName) : DBexception( NO_MEMORY, objName )
	{
	}
};

class DBillegalRecordlen : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Illegal Record Length";
	}
	public:
	DBillegalRecordlen() : DBexception( ILLEGAL_RECORD_LEN )
	{
	}
	DBillegalRecordlen(const gak::STRING &objName) : DBexception( ILLEGAL_RECORD_LEN, objName )
	{
	}
};

class DBillegalRecordHeader : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Illegal Record Header";
	}
	public:
	DBillegalRecordHeader() : DBexception( ILLEGAL_RECORD_LEN )
	{
	}
	DBillegalRecordHeader(const gak::STRING &objName) : DBexception( ILLEGAL_RECORD_LEN, objName )
	{
	}
};

class DBopenFaild : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Unable to open file %obj%";
	}
	public:
	DBopenFaild() : DBexception( OPEN_FAILED )
	{
	}
	DBopenFaild(const gak::STRING &objName) : DBexception( OPEN_FAILED, objName )
	{
	}
};

class DBmkdirFaild : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Unable to create directory %obj%";
	}
	public:
	DBmkdirFaild() : DBexception( MKDIR_FAILED )
	{
	}
	DBmkdirFaild(const gak::STRING &objName) : DBexception( MKDIR_FAILED, objName )
	{
	}
};

class DBnullValueNotAllowed : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Null value in field %obj%";
	}
	public:
	DBnullValueNotAllowed() : DBexception( NULL_VALUE )
	{
	}
	DBnullValueNotAllowed(const gak::STRING &objName) : DBexception( NULL_VALUE, objName )
	{
	}
};

class DBkeyViolation : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Key violation";
	}
	public:
	DBkeyViolation() : DBexception( KEY_VIOLATION )
	{
	}
	DBkeyViolation(const gak::STRING &objName) : DBexception( KEY_VIOLATION, objName )
	{
	}
};

class DBfieldNotFound : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Field %obj% does not exist";
	}
	public:
	DBfieldNotFound() : DBexception( FIELD_NOT_EXISTS )
	{
	}
	DBfieldNotFound(const gak::STRING &objName) : DBexception( FIELD_NOT_EXISTS, objName )
	{
	}
};

class DBfieldExist : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Field %obj% exists";
	}
	public:
	DBfieldExist() : DBexception( FIELD_EXISTS )
	{
	}
	DBfieldExist(const gak::STRING &objName) : DBexception( FIELD_EXISTS, objName )
	{
	}
};

class DBindexNotFound : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Index %obj% does not exist";
	}
	public:
	DBindexNotFound() : DBexception( INDEX_NOT_EXISTS )
	{
	}
	DBindexNotFound(const gak::STRING &objName) : DBexception( INDEX_NOT_EXISTS, objName )
	{
	}
};

class DBindexExist : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Index %obj% exists";
	}
	public:
	DBindexExist() : DBexception( INDEX_EXISTS )
	{
	}
	DBindexExist(const gak::STRING &objName) : DBexception( INDEX_EXISTS, objName )
	{
	}
};

class DBtableNotFound : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Table %obj% does not exist";
	}
	public:
	DBtableNotFound() : DBexception( TABLE_NOT_EXISTS )
	{
	}
	DBtableNotFound(const gak::STRING &objName) : DBexception( TABLE_NOT_EXISTS, objName )
	{
	}
};

class DBtableExist : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Table %obj% exists";
	}
	public:
	DBtableExist() : DBexception( TABLE_EXISTS )
	{
	}
	DBtableExist(const gak::STRING &objName) : DBexception( TABLE_EXISTS, objName )
	{
	}
};

class DbNotExist : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Database %obj% does not exist";
	}
	public:
	DbNotExist() : DBexception( DB_NOT_EXISTS )
	{
	}
	DbNotExist(const gak::STRING &objName) : DBexception( DB_NOT_EXISTS, objName )
	{
	}
};

class DbExist : public DBexception
{
	virtual const char *getErrText() const
	{
		return "%err%: Database %obj% exists";
	}
	public:
	DbExist() : DBexception( DB_EXISTS )
	{
	}
	DbExist(const gak::STRING &objName) : DBexception( DB_EXISTS, objName )
	{
	}
};



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

