/*
		Project:		dbLIB
		Module:			dblib.cpp
		Description:	the unit test
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

#include <memory>

#include <gak/unitTest.h>
#include <gak/directory.h>

#include "db_exception.h"

#include "database.h"
#include "table.h"
#include "record.h"

using gak::STRING;
using gak::F_STRING;

const char test1[] = "test1";
const char simple[] = "simple";
const char MY_ONLY_FIELD[] = "MY_ONLY_FIELD";

class MydbUnitTest : public gak::UnitTest
{
	virtual const char *GetClassName() const
	{
		return "MydbUnitTest";
	}

	void createSimpleTable(dbLib::Database *db);
	void fillSimpleTable(dbLib::Table *tab, int count);

	void createTable(dbLib::Database *db);
	void fillTable(dbLib::Table *tab);

	void assertRecords(dbLib::Table *tab, gak::int64 expected );

	void processTables1(dbLib::Table *tab);
	void processTables2(dbLib::Table *tab);
	void processTables3(dbLib::Table *tab);
	void processTables4(dbLib::Table *tab);

	void simpleTest(dbLib::Database *db);

	virtual void PerformTest();
};

#define printStringField(t,f) \
	std::cout << f << " = " << t->getField( f )->getStringValue() << std::endl

#define printIntegerField(t,f) \
	std::cout << f << " = " << t->getField( f )->getIntegerValue() << std::endl

#define printBoolField(t,f) \
	std::cout << f << " = " << (t->getField( f )->getBooleanValue() ? 'Y' : 'N') << std::endl

void MydbUnitTest::createSimpleTable(dbLib::Database *db)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::createSimpleTable" );

	std::auto_ptr<dbLib::Table> 	 t1( db->createTable( simple ) );

	t1->addField( MY_ONLY_FIELD, dbLib::ftInteger, true, true );	// this is my primary index
}

void MydbUnitTest::createTable(dbLib::Database *db)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::createTable" );

	std::auto_ptr<dbLib::Table> 	 t1( db->createTable( test1 ) );

	t1->addField( "MY_FIRST_FIELD", dbLib::ftString, true, true );	// this is my primary index
	t1->addField( "MY_SECOND_FIELD", dbLib::ftString );
	t1->addField( "MY_THIRD_FIELD", dbLib::ftString );
	t1->addField( "INT_FIELD", dbLib::ftInteger, false, true );
	t1->addField( "BOOL_FIELD", dbLib::ftBoolean );

	t1->createIndex( "INT_FIELD" );
	t1->addFieldToIndex( "INT_FIELD", "INT_FIELD", true, true );
}

void MydbUnitTest::fillSimpleTable(dbLib::Table *tab, int count)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::fillSimpleTable" );

	for( int i=0; i<count; ++i )
	{
		tab->insertRecord();
		tab->getField( MY_ONLY_FIELD )->setIntegerValue( i );
		tab->postRecord();
	}
}

void MydbUnitTest::fillTable(dbLib::Table *tab)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::fillTable" );
	int i=0;

	tab->insertRecord();
	tab->getField( "my_first_field" )->setStringValue( "Blödmann" );
	tab->getField( "MY_SECOND_FIELD" )->setStringValue( "Dummkopf" );
	tab->getField( "MY_THIRD_FIELD" )->setStringValue( "Knalltüte" );
	tab->getField( "INT_FIELD" )->setIntegerValue( ++i );
	tab->getField( "BOOL_FIELD" )->setBooleanValue( false );

	tab->postRecord();

	tab->insertRecord();
	tab->getField( "my_first_field" )->setStringValue( "Martin" );
	tab->getField( "MY_SECOND_FIELD" )->setStringValue( "Richard" );
	tab->getField( "MY_THIRD_FIELD" )->setStringValue( "Gäckler" );
	tab->getField( "INT_FIELD" )->setIntegerValue( ++i );
	tab->getField( "BOOL_FIELD" )->setBooleanValue( true );

	tab->postRecord();

	tab->insertRecord();
	tab->getField( "my_first_field" )->setStringValue( "Armleuchter" );
	tab->getField( "MY_SECOND_FIELD" )->setStringValue( "Idiot" );
	tab->getField( "MY_THIRD_FIELD" )->setStringValue( "Depp" );
	tab->getField( "INT_FIELD" )->setIntegerValue( ++i );
	tab->getField( "BOOL_FIELD" )->setBooleanValue( false );

	tab->postRecord();

	tab->insertRecord();
	tab->getField( "my_first_field" )->setStringValue( "Chaot" );
	tab->getField( "MY_SECOND_FIELD" )->setStringValue( "Fischer" );
	tab->getField( "MY_THIRD_FIELD" )->setStringValue( "Aussi" );
	tab->getField( "INT_FIELD" )->setIntegerValue( ++i );
	tab->getField( "BOOL_FIELD" )->setBooleanValue( true );

	tab->postRecord();
}

void MydbUnitTest::processTables1(dbLib::Table *tab)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::processTables1" );
	std::cout << "Alpha Order:\n";
	tab->firstRecord();

	UT_ASSERT_EQUAL(tab->getField("my_first_field")->getStringValue(), "Armleuchter" );
	STRING lastString;

	while( !tab->eof() )
	{
		lastString = tab->getField("my_first_field")->getStringValue();
		printStringField(tab,"my_first_field");
		printStringField(tab,"MY_SECOND_FIELD");
		printStringField(tab,"MY_THIRD_FIELD");
		printIntegerField(tab,"INT_FIELD");
		printBoolField(tab,"BOOL_FIELD");

		tab->nextRecord();
	}
	UT_ASSERT_EQUAL(lastString, "Martin" );

	std::cout << "Index Order:\n";
	tab->setIndex( "INT_FIELD" );
	tab->firstRecord();

	UT_ASSERT_EQUAL(tab->getField("INT_FIELD")->getIntegerValue(), 1 );
	int lastInt;
	while( !tab->eof() )
	{
		lastInt = tab->getField("INT_FIELD")->getIntegerValue();
		printStringField(tab,"my_first_field");
		printIntegerField(tab,"INT_FIELD");

		tab->nextRecord();
	}
	UT_ASSERT_EQUAL(lastInt, 4 );
}

void MydbUnitTest::processTables2(dbLib::Table *tab)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::processTables2" );
	int i=111;

	tab->firstRecord();
	while( !tab->eof() && !tab->bof() )
	{
		tab->getField( "INT_FIELD" )->setIntegerValue( ++i );
		tab->postRecord();
		tab->nextRecord();
	}


	std::cout << "Updated Values:\n";
	tab->firstRecord();
	UT_ASSERT_EQUAL(tab->getField("INT_FIELD")->getIntegerValue(), 112 );
	int lastInt;
	while( !tab->eof() )
	{
		lastInt = tab->getField("INT_FIELD")->getIntegerValue();
		printStringField(tab,"my_first_field");
		printIntegerField(tab,"INT_FIELD");
		tab->nextRecord();
	}
	UT_ASSERT_EQUAL(lastInt, 115 );
}

void MydbUnitTest::processTables3(dbLib::Table *tab)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::processTables3" );
	// test not nulls (post)
	tab->insertRecord();
	tab->getField( "my_first_field" )->setStringValue( "Chaot2" );
	tab->getField( "MY_SECOND_FIELD" )->setStringValue( "Fischer" );
	tab->getField( "MY_THIRD_FIELD" )->setStringValue( "Aussi" );
	tab->getField( "BOOL_FIELD" )->setBooleanValue( true );
	UT_ASSERT_EXCEPTION(tab->postRecord(), dbLib::DBnullValueNotAllowed);

	// test key violation
	tab->insertRecord();
	tab->getField( "my_first_field" )->setStringValue( "Chaot" );
	tab->getField( "MY_SECOND_FIELD" )->setStringValue( "Fischer" );
	tab->getField( "MY_THIRD_FIELD" )->setStringValue( "Aussi" );
	tab->getField( "INT_FIELD" )->setIntegerValue( 666 );
	tab->getField( "BOOL_FIELD" )->setBooleanValue( true );
	UT_ASSERT_EXCEPTION(tab->postRecord(), dbLib::DBkeyViolation);

}

void MydbUnitTest::assertRecords(dbLib::Table *tab, gak::int64 expected)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::processTables4" );
	tab->root();
	const dbLib::Record &record = tab->getRecord();
	const dbLib::RecordHeader &header = record.getHeader();
	UT_ASSERT_EQUAL( header.numRecords, expected );
}

void MydbUnitTest::processTables4(dbLib::Table *tab)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::processTables6" );
	tab->firstRecord();
	while( !tab->eof() && !tab->bof() )
	{
		tab->deleteRecord();
	}
	UT_ASSERT_EXCEPTION(tab->getField( "test" )->setStringValue( "Blödmann" ), dbLib::DBfieldNotFound);
}

void MydbUnitTest::simpleTest(dbLib::Database *db)
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::simpleTest" );

	createSimpleTable(db);

	std::auto_ptr<dbLib::Table> 	 tt( db->openTable( simple ) );

	fillSimpleTable(tt.get(), 1600);

	int prevValue = -1;
	for( tt->firstRecord(); !tt->eof(); tt->nextRecord() )
	{
		int newValue = tt->getField( MY_ONLY_FIELD )->getIntegerValue();
		UT_ASSERT_LESS( prevValue, newValue );
		prevValue = newValue;
	}
	UT_ASSERT_EQUAL( prevValue, 1599 );
}

void MydbUnitTest::PerformTest()
{
	doEnterFunctionEx( gakLogging::llInfo, "MydbUnitTest::PerformTest" );
	std::auto_ptr<dbLib::Database>	db( dbLib::Database::createDB( "", "c:\\temp\\gak\\", "gak", "" ) );

	simpleTest(db.get());

	createTable(db.get());

	{
		std::auto_ptr<dbLib::Table> 	 t1( db->openTable( test1 ) );
		std::auto_ptr<dbLib::Table> 	 t2( db->openTable( test1 ) );
		std::auto_ptr<dbLib::Table> 	 t3( db->openTable( test1 ) );

		fillTable(t3.get());
		assertRecords(t1.get(),4);
		processTables1(t1.get());
		assertRecords(t1.get(),4);
		processTables2(t2.get());
		assertRecords(t1.get(),8);
		processTables3(t3.get());
		assertRecords(t1.get(),8);
		processTables4(t1.get());
		assertRecords(t1.get(),8);
	}

	db->dropTable(test1);
	db->dropTable(simple);

	UT_ASSERT_EXCEPTION(db->openTable( test1 ), dbLib::DBtableNotFound);
}

static MydbUnitTest mydbUnitTest;

int main( int , const char *argv[] )
{
	//doEnableLogEx(gakLogging::llDetail);
	doDisableLog();
	doEnableProfile(gakLogging::llDetail); 
	F_STRING	sourcePath;
	doEnterFunctionEx( gakLogging::llInfo, "main" );

	gak::fsplit( gak::fullPath( __FILE__ ), &sourcePath );
	if( !sourcePath.isEmpty() )
	{
		std::cout << "Changing directory to " << sourcePath << std::endl;
		if( setcwd( sourcePath ) )
		{
			perror( sourcePath );
		}
	}
	else
	{
		std::cout << "Don't know Source Path, using " << gak::getcwd() << std::endl;
	}

	gak::UnitTest::PerformTests( argv );
	gak::UnitTest::PrintResult();

#ifdef _Windows
	getchar();
#endif
	return EXIT_SUCCESS;
}
