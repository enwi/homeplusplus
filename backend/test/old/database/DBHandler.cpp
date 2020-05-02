#include "database/DBHandler.h"

#include <gtest/gtest.h>

#include "../mocks/MockDBHandler.h"

TEST(DBValue, Constructor)
{
    {
        DBValue v{};
        EXPECT_TRUE(v.IsInt());
        EXPECT_FALSE(v.IsInt64());
        EXPECT_FALSE(v.IsString());
        EXPECT_EQ(0, v.GetInt());
    }
    {
        int i = 10;
        DBValue v{i};
        EXPECT_TRUE(v.IsInt());
        EXPECT_FALSE(v.IsInt64());
        EXPECT_FALSE(v.IsString());
        EXPECT_EQ(10, v.GetInt());
    }
    {
        int64_t i = 10;
        DBValue v{i};
        EXPECT_FALSE(v.IsInt());
        EXPECT_TRUE(v.IsInt64());
        EXPECT_FALSE(v.IsString());
        EXPECT_EQ(10, v.GetInt64());
    }
    {
        std::string s = "abc";
        DBValue v{s};
        EXPECT_FALSE(v.IsInt());
        EXPECT_FALSE(v.IsInt64());
        EXPECT_TRUE(v.IsString());
        EXPECT_EQ("abc", v.GetStr());
    }
    {
        DBValue v{"xyz"};
        EXPECT_FALSE(v.IsInt());
        EXPECT_FALSE(v.IsInt64());
        EXPECT_TRUE(v.IsString());
        EXPECT_EQ("xyz", v.GetStr());
    }
    {
        // Main use case in initializer list of DBStatement::Execute
        std::initializer_list<DBValue> l{(int64_t)3, 2, std::string("test"), 5};
        auto it = l.begin();
        ASSERT_TRUE(it->IsInt64());
        EXPECT_EQ(3, it->GetInt64());
        ASSERT_TRUE(std::next(it)->IsInt());
        EXPECT_EQ(2, std::next(it)->GetInt());
        ASSERT_TRUE(std::next(it, 2)->IsString());
        EXPECT_EQ("test", std::next(it, 2)->GetStr());
        ASSERT_TRUE(std::next(it, 3)->IsInt());
        EXPECT_EQ(5, std::next(it, 3)->GetInt());
    }
}

TEST(DBValue, Copy)
{
    int i = 21;
    DBValue vi{i};
    int64_t l = 215235897189;
    DBValue vl{l};
    DBValue vs{"abcdasd"};
    // Copy constructor
    {
        DBValue cpy{vi};
        EXPECT_EQ(vi.IsInt(), cpy.IsInt());
        EXPECT_EQ(vi.IsInt64(), cpy.IsInt64());
        EXPECT_EQ(vi.IsString(), cpy.IsString());
        EXPECT_EQ(vi.GetInt(), cpy.GetInt());
    }
    {
        DBValue cpy{vl};
        EXPECT_EQ(vl.IsInt(), cpy.IsInt());
        EXPECT_EQ(vl.IsInt64(), cpy.IsInt64());
        EXPECT_EQ(vl.IsString(), cpy.IsString());
        EXPECT_EQ(vl.GetInt64(), cpy.GetInt64());
    }
    {
        DBValue cpy{vs};
        EXPECT_EQ(vs.IsInt(), cpy.IsInt());
        EXPECT_EQ(vs.IsInt64(), cpy.IsInt64());
        EXPECT_EQ(vs.IsString(), cpy.IsString());
        EXPECT_EQ(vs.GetStr(), cpy.GetStr());
    }
    // Copy assignment
    DBValue cpy;
    {
        cpy = vi;
        EXPECT_EQ(vi.IsInt(), cpy.IsInt());
        EXPECT_EQ(vi.IsInt64(), cpy.IsInt64());
        EXPECT_EQ(vi.IsString(), cpy.IsString());
        EXPECT_EQ(vi.GetInt(), cpy.GetInt());
    }
    {
        cpy = vl;
        EXPECT_EQ(vl.IsInt(), cpy.IsInt());
        EXPECT_EQ(vl.IsInt64(), cpy.IsInt64());
        EXPECT_EQ(vl.IsString(), cpy.IsString());
        EXPECT_EQ(vl.GetInt64(), cpy.GetInt64());
    }
    {
        cpy = vs;
        EXPECT_EQ(vs.IsInt(), cpy.IsInt());
        EXPECT_EQ(vs.IsInt64(), cpy.IsInt64());
        EXPECT_EQ(vs.IsString(), cpy.IsString());
        EXPECT_EQ(vs.GetStr(), cpy.GetStr());
    }
    // Copy assignment (previous string)
    {
        DBValue cpy2{"test"};
        cpy2 = vi;
        EXPECT_EQ(vi.IsInt(), cpy2.IsInt());
        EXPECT_EQ(vi.IsInt64(), cpy2.IsInt64());
        EXPECT_EQ(vi.IsString(), cpy2.IsString());
        EXPECT_EQ(vi.GetInt(), cpy2.GetInt());
    }
    {
        DBValue cpy2{"test"};
        cpy2 = vl;
        EXPECT_EQ(vl.IsInt(), cpy2.IsInt());
        EXPECT_EQ(vl.IsInt64(), cpy2.IsInt64());
        EXPECT_EQ(vl.IsString(), cpy2.IsString());
        EXPECT_EQ(vl.GetInt64(), cpy2.GetInt64());
    }
    {
        DBValue cpy2{"test"};
        cpy2 = vs;
        EXPECT_EQ(vs.IsInt(), cpy2.IsInt());
        EXPECT_EQ(vs.IsInt64(), cpy2.IsInt64());
        EXPECT_EQ(vs.IsString(), cpy2.IsString());
        EXPECT_EQ(vs.GetStr(), cpy2.GetStr());
    }
}

TEST(DBValue, Move)
{
    int i = 21;
    int64_t l = 215235897189;
    std::string s = "absdfkal";
    // Move construct
    {
        DBValue v{i};
        DBValue m{std::move(v)};
        EXPECT_TRUE(m.IsInt());
        EXPECT_FALSE(m.IsInt64());
        EXPECT_FALSE(m.IsString());
        EXPECT_EQ(i, m.GetInt());
    }
    {
        DBValue v{l};
        DBValue m{std::move(v)};
        EXPECT_FALSE(m.IsInt());
        EXPECT_TRUE(m.IsInt64());
        EXPECT_FALSE(m.IsString());
        EXPECT_EQ(l, m.GetInt64());
    }
    {
        DBValue v{s};
        DBValue m{std::move(v)};
        EXPECT_FALSE(m.IsInt());
        EXPECT_FALSE(m.IsInt64());
        EXPECT_TRUE(m.IsString());
        EXPECT_EQ(s, m.GetStr());
    }
    // Move assign
    DBValue mov;
    {
        mov = DBValue(i);
        EXPECT_TRUE(mov.IsInt());
        EXPECT_FALSE(mov.IsInt64());
        EXPECT_FALSE(mov.IsString());
        EXPECT_EQ(i, mov.GetInt());
    }
    {
        mov = DBValue(l);
        EXPECT_FALSE(mov.IsInt());
        EXPECT_TRUE(mov.IsInt64());
        EXPECT_FALSE(mov.IsString());
        EXPECT_EQ(l, mov.GetInt64());
    }
    {
        mov = DBValue(s);
        EXPECT_FALSE(mov.IsInt());
        EXPECT_FALSE(mov.IsInt64());
        EXPECT_TRUE(mov.IsString());
        EXPECT_EQ(s, mov.GetStr());
    }
    // Move assignment (previous string)
    {
        DBValue mov2{"test"};
        mov2 = DBValue(i);
        EXPECT_TRUE(mov2.IsInt());
        EXPECT_FALSE(mov2.IsInt64());
        EXPECT_FALSE(mov2.IsString());
        EXPECT_EQ(i, mov2.GetInt());
    }
    {
        DBValue mov2{"test"};
        mov2 = DBValue(l);
        EXPECT_FALSE(mov2.IsInt());
        EXPECT_TRUE(mov2.IsInt64());
        EXPECT_FALSE(mov2.IsString());
        EXPECT_EQ(l, mov2.GetInt64());
    }
    {
        DBValue mov2{"test"};
        mov2 = DBValue(s);
        EXPECT_FALSE(mov2.IsInt());
        EXPECT_FALSE(mov2.IsInt64());
        EXPECT_TRUE(mov2.IsString());
        EXPECT_EQ(s, mov2.GetStr());
    }
}

TEST(DBValue, Conversion)
{
    DBValue v;
    {
        v = 10;
        EXPECT_TRUE(v.IsInt());
        EXPECT_FALSE(v.IsInt64());
        EXPECT_FALSE(v.IsString());
        EXPECT_EQ(10, v.GetInt());
    }
    {
        v = static_cast<int64_t>(10);
        EXPECT_FALSE(v.IsInt());
        EXPECT_TRUE(v.IsInt64());
        EXPECT_FALSE(v.IsString());
        EXPECT_EQ(10, v.GetInt64());
    }
    {
        v = std::string("asefabjslhlj");
        EXPECT_FALSE(v.IsInt());
        EXPECT_FALSE(v.IsInt64());
        EXPECT_TRUE(v.IsString());
        EXPECT_EQ("asefabjslhlj", v.GetStr());
    }
}

TEST(DBHandler, Constructor)
{
    DBHandler dbHandler("file.db");
    // Constructor must not open database
    EXPECT_THROW(dbHandler.GetROSavepoint("a"), std::logic_error);
    EXPECT_THROW(dbHandler.GetSavepoint("a"), std::logic_error);
    EXPECT_THROW(dbHandler.GetROStatement("a"), std::logic_error);
    EXPECT_THROW(dbHandler.GetStatement("a"), std::logic_error);
}

TEST(DBHandler, Begin)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    DBHandler dbHandler("file.db");
    std::unique_ptr<MockDatabase> db = std::make_unique<MockDatabase>();
    std::unique_ptr<MockDatabase> roDb = std::make_unique<MockDatabase>();
    auto create = [&](const std::string& fname, int timeout, bool ro) {
        EXPECT_EQ("file.db", fname);
        EXPECT_EQ(5000, timeout);
        if (ro)
        {
            EXPECT_NE(nullptr, roDb) << "Create RO DB is called more than once";
            return std::move(roDb);
        }
        else
        {
            EXPECT_NE(nullptr, db) << "Create DB is called more than once";
            return std::move(db);
        }
    };

    EXPECT_TRUE(dbHandler.Begin(create));

    EXPECT_FALSE(DBHandler("test.db").Begin(
        [](const std::string&, int, bool) -> std::unique_ptr<IDatabase> { throw std::exception(); }));
}

TEST(DBHandler, GetSavepointStatement)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    DBHandler dbHandler("file.db");
    std::unique_ptr<MockDatabase> pDb = std::make_unique<MockDatabase>();
    std::unique_ptr<MockDatabase> pRoDb = std::make_unique<MockDatabase>();
    MockDatabase& db = *pDb;
    MockDatabase& roDb = *pRoDb;
    auto create = [&](const std::string& fname, int timeout, bool ro) {
        EXPECT_EQ("file.db", fname);
        EXPECT_EQ(5000, timeout);
        if (ro)
        {
            EXPECT_NE(nullptr, pRoDb) << "Create RO DB is called more than once";
            return std::move(pRoDb);
        }
        else
        {
            EXPECT_NE(nullptr, pDb) << "Create DB is called more than once";
            return std::move(pDb);
        }
    };

    ASSERT_TRUE(dbHandler.Begin(create));

    // Verify that ro database is used
    roDb.UseDefaultStatementImpl();
    EXPECT_CALL(roDb, PrepareStatement(_)).Times(AtLeast(1));
    EXPECT_CALL(roDb, ExecuteStatement(_)).Times(AtLeast(1));
    EXPECT_CALL(db, PrepareStatement(_)).Times(0);
    EXPECT_CALL(db, ExecuteStatement(_)).Times(0);
    dbHandler.GetROSavepoint("name");
    dbHandler.GetROStatement("statement");

    db.UseDefaultStatementImpl();
    EXPECT_CALL(roDb, PrepareStatement(_)).Times(0);
    EXPECT_CALL(roDb, ExecuteStatement(_)).Times(0);
    EXPECT_CALL(db, PrepareStatement(_)).Times(AtLeast(1));
    EXPECT_CALL(db, ExecuteStatement(_)).Times(AtLeast(1));
    dbHandler.GetSavepoint("name");
    dbHandler.GetStatement("statement");
}

TEST(DBHandler, GetLastInsertRowid)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    DBHandler dbHandler("file.db");

    // Not initialized
    EXPECT_THROW(dbHandler.GetLastInsertRowid(), std::logic_error);

    std::unique_ptr<MockDatabase> pDb = std::make_unique<MockDatabase>();
    std::unique_ptr<MockDatabase> pRoDb = std::make_unique<MockDatabase>();
    MockDatabase& db = *pDb;
    MockDatabase& roDb = *pRoDb;
    auto create = [&](const std::string& fname, int timeout, bool ro) {
        EXPECT_EQ("file.db", fname);
        EXPECT_EQ(5000, timeout);
        if (ro)
        {
            EXPECT_NE(nullptr, pRoDb) << "Create RO DB is called more than once";
            return std::move(pRoDb);
        }
        else
        {
            EXPECT_NE(nullptr, pDb) << "Create DB is called more than once";
            return std::move(pDb);
        }
    };

    ASSERT_TRUE(dbHandler.Begin(create));

    // GetLastRowid should only be called on db, because you cannot insert into roDb
    EXPECT_CALL(roDb, GetLastInsertRowid()).Times(0);
    EXPECT_CALL(db, GetLastInsertRowid()).WillOnce(Return(1));
    EXPECT_EQ(1, dbHandler.GetLastInsertRowid());
}

TEST(DBSavepoint, Constructor)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(AnyNumber()).WillRepeatedly(Return(MockDatabase::ok));
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT testsavepoint;")).WillOnce(Return(MockDatabase::ok));
        DBSavepoint savepoint(mockDb, "testsavepoint");
    }
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT asd;")).WillOnce(Return(MockDatabase::ok));
        DBSavepoint savepoint(mockDb, "asd");
    }
    {
        // Return error
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT asd;")).WillOnce(Return(MockDatabase::error));
        EXPECT_CALL(mockDb, GetError()).WillRepeatedly(Return("just testing"));
        EXPECT_THROW(DBSavepoint(mockDb, "asd"), std::runtime_error);
    }
}

TEST(DBSavepoint, MoveConstructor)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "saveA");
        // Move should not open a new savepoint
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(0);
        DBSavepoint b = std::move(a);
        // Destructor rolls back and closes b, but not a
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(2).WillRepeatedly(Return(0));
    }
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "saveA");
        {
            // Move should not open a new savepoint
            EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(0);
            DBSavepoint b = std::move(a);
            // Destructor rolls back and closes b
            EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(2).WillRepeatedly(Return(0));
        }
        // Destructor should not close a
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(0);
    }
}

TEST(DBSavepoint, MoveAssign)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    EXPECT_CALL(mockDb, GetError()).WillRepeatedly(Return("just testing"));
    EXPECT_CALL(mockDb, GetErrorCode()).WillRepeatedly(Return(1));
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveB;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "saveA");
        {
            DBSavepoint b(mockDb, "saveB");
            // Move assign rolls back and closes b
            EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(2).WillRepeatedly(Return(0));
            b = std::move(a);

            // Destructor rolls back and closes saveA, but not saveB
            EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(2).WillRepeatedly(Return(0));
        }
        // Destructor of a does nothing
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(0);
    }
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT xa;")).WillOnce(Return(0));
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT xb;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "xa");
        DBSavepoint b(mockDb, "xb");
        // Rollback fails
        EXPECT_CALL(mockDb, ExecuteStatement(_)).WillOnce(Return(1));
        EXPECT_THROW(b = std::move(a), std::runtime_error);

        // Destructor rolls back and closes xa and xb
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(4).WillRepeatedly(Return(0));
    }
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT xa;")).WillOnce(Return(0));
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT xb;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "xa");
        DBSavepoint b(mockDb, "xb");
        // Rollback succeeds, release fails
        EXPECT_CALL(mockDb, ExecuteStatement(_)).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_THROW(b = std::move(a), std::runtime_error);

        // Destructor rolls back and closes xa and xb (rolls back again)
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(4).WillRepeatedly(Return(0));
    }
}

TEST(DBSavepoint, Release)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    EXPECT_CALL(mockDb, GetError()).WillRepeatedly(Return("just testing"));
    EXPECT_CALL(mockDb, GetErrorCode()).WillRepeatedly(Return(1));
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "saveA");

        EXPECT_CALL(mockDb, ExecuteStatement("RELEASE SAVEPOINT saveA;")).WillOnce(Return(0));
        a.Release();

        // Destructor does not roll back
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(0);
    }
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "saveA");

        EXPECT_CALL(mockDb, ExecuteStatement("RELEASE SAVEPOINT saveA;")).WillOnce(Return(1));
        EXPECT_THROW(a.Release(), std::runtime_error);

        // Destructor does roll back
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(2).WillRepeatedly(Return(0));
    }
}

TEST(DBSavepoint, Rollback)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    EXPECT_CALL(mockDb, GetError()).WillRepeatedly(Return("just testing"));
    EXPECT_CALL(mockDb, GetErrorCode()).WillRepeatedly(Return(1));
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "saveA");

        // First rollback, then release
        {
            InSequence s;
            EXPECT_CALL(mockDb, ExecuteStatement("ROLLBACK TRANSACTION TO SAVEPOINT saveA;")).WillOnce(Return(0));
            EXPECT_CALL(mockDb, ExecuteStatement("RELEASE SAVEPOINT saveA;")).WillOnce(Return(0));
        }
        a.Rollback();

        // Destructor does not roll back
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(0);
    }
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "saveA");

        // Rollback fails, so no release
        {
            InSequence s;
            EXPECT_CALL(mockDb, ExecuteStatement("ROLLBACK TRANSACTION TO SAVEPOINT saveA;")).WillOnce(Return(1));
            EXPECT_CALL(mockDb, ExecuteStatement("RELEASE SAVEPOINT saveA;")).Times(0);
        }
        EXPECT_THROW(a.Rollback(), std::runtime_error);

        // Destructor does roll back
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(2).WillRepeatedly(Return(0));
    }
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "saveA");

        // First rollback, then release fails
        {
            InSequence s;
            EXPECT_CALL(mockDb, ExecuteStatement("ROLLBACK TRANSACTION TO SAVEPOINT saveA;")).WillOnce(Return(0));
            EXPECT_CALL(mockDb, ExecuteStatement("RELEASE SAVEPOINT saveA;")).WillOnce(Return(1));
        }
        EXPECT_THROW(a.Rollback(), std::runtime_error);

        // Destructor does roll back
        EXPECT_CALL(mockDb, ExecuteStatement(_)).Times(2).WillRepeatedly(Return(0));
    }
}

TEST(DBSavepoint, Destructor)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    EXPECT_CALL(mockDb, GetError()).WillRepeatedly(Return("just testing"));
    EXPECT_CALL(mockDb, GetErrorCode()).WillRepeatedly(Return(1));
    {
        EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
        DBSavepoint a(mockDb, "saveA");
        // Destructor rolls back and releases
        {
            InSequence s;
            EXPECT_CALL(mockDb, ExecuteStatement("ROLLBACK TRANSACTION TO SAVEPOINT saveA;")).WillOnce(Return(0));
            EXPECT_CALL(mockDb, ExecuteStatement("RELEASE SAVEPOINT saveA;")).WillOnce(Return(0));
        }
    }
    {
        {
            InSequence s;
            EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
            // Destructor rolls back and releases
            EXPECT_CALL(mockDb, ExecuteStatement("ROLLBACK TRANSACTION TO SAVEPOINT saveA;")).WillOnce(Return(0));
            EXPECT_CALL(mockDb, ExecuteStatement("RELEASE SAVEPOINT saveA;")).WillOnce(Return(1));
        }
        // Destructor should not throw exceptions
        EXPECT_NO_THROW(DBSavepoint(mockDb, "saveA"));
    }
    {
        {
            InSequence s;
            EXPECT_CALL(mockDb, ExecuteStatement("SAVEPOINT saveA;")).WillOnce(Return(0));
            // Destructor rolls back and releases
            EXPECT_CALL(mockDb, ExecuteStatement("ROLLBACK TRANSACTION TO SAVEPOINT saveA;")).WillOnce(Return(1));
            EXPECT_CALL(mockDb, ExecuteStatement("RELEASE SAVEPOINT saveA;")).Times(0);
        }
        // Destructor should not throw exceptions
        EXPECT_NO_THROW(DBSavepoint(mockDb, "saveA"));
    }
}

TEST(DBStatement, Constructor)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    mockDb.UseDefaultStatementImpl();
    {
        const std::string s = "SELECT * FROM test;";
        EXPECT_CALL(mockDb, PrepareStatement(s)).Times(1);
        DBStatement statement(mockDb, s);
    }
    {
        const std::string s = "INSERT INTO test;";
        EXPECT_CALL(mockDb, PrepareStatement(s)).Times(1);
        DBStatement statement(mockDb, s);
    }
    {
        // Return error
        EXPECT_CALL(mockDb, PrepareStatement("asdf;"))
            .WillOnce(Return(ByMove(
                std::pair<MockDatabase::ResultCode, std::unique_ptr<IStatement>>{MockDatabase::error, nullptr})));
        EXPECT_CALL(mockDb, GetError()).WillRepeatedly(Return("just testing"));
        EXPECT_THROW(DBStatement(mockDb, "asdf;"), std::runtime_error);
    }
}

TEST(DBStatement, Execute)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    std::unique_ptr<MockStatement> pStatement = std::make_unique<MockStatement>("test statement;");
    MockStatement& mockSt = *pStatement;

    EXPECT_CALL(mockDb, PrepareStatement("test statement;"))
        .WillOnce(Return(ByMove(std::pair<MockDatabase::ResultCode, std::unique_ptr<IStatement>>{
            MockDatabase::ok, std::move(pStatement)})));
    DBStatement statement(mockDb, "test statement;");

    {
        EXPECT_CALL(mockSt, BindInt(1, 10)).WillOnce(Return(MockDatabase::ok));
        statement.Execute({10});
    }

    {
        int64_t i64 = 0xEEAFEEAFE;
        {
            InSequence s;
            EXPECT_CALL(mockSt, BindInt(1, 1)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(mockSt, BindString(2, "abc")).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(mockSt, BindInt64(3, i64)).WillOnce(Return(MockDatabase::ok));
        }

        statement.Execute({1, std::string("abc"), i64});
    }

    {
        {
            InSequence s;
            EXPECT_CALL(mockSt, BindInt(1, 10)).WillOnce(Return(MockDatabase::error));
            EXPECT_CALL(mockSt, GetError()).WillRepeatedly(Return("just testing"));
        }
        EXPECT_THROW(statement.Execute({10}), std::runtime_error);
    }
}

TEST(DBStatement, ExecuteAll)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    std::unique_ptr<MockStatement> pStatement = std::make_unique<MockStatement>("test statement;");
    MockStatement& mockSt = *pStatement;

    EXPECT_CALL(mockDb, PrepareStatement("test statement;"))
        .WillOnce(Return(ByMove(std::pair<int, std::unique_ptr<IStatement>>{0, std::move(pStatement)})));
    DBStatement statement(mockDb, "test statement;");

    // No binding
    {
        // one result
        EXPECT_CALL(mockSt, Step()).WillOnce(Return(MockStatement::row)).WillOnce(Return(MockStatement::done));
        statement.ExecuteAll();
    }
    {
        // no result
        EXPECT_CALL(mockSt, Step()).WillOnce(Return(MockStatement::done));
        statement.ExecuteAll();
    }

    {
        {
            InSequence s;
            EXPECT_CALL(mockSt, BindInt(1, 10)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(mockSt, Step()).WillOnce(Return(MockStatement::done));
        }

        statement.ExecuteAll({10});
    }

    {
        int64_t i64 = 0xEEAFEEAFE;
        {
            InSequence s;
            EXPECT_CALL(mockSt, BindInt(1, 1)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(mockSt, BindString(2, "abc")).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(mockSt, BindInt64(3, i64)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(mockSt, Step()).WillOnce(Return(MockStatement::row)).WillOnce(Return(MockStatement::done));
        }

        statement.ExecuteAll({1, std::string("abc"), i64});
    }

    {
        {
            InSequence s;
            EXPECT_CALL(mockSt, BindInt(1, 10)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(mockSt, Step()).WillOnce(Return(MockStatement::row)).WillOnce(Return(MockDatabase::error));
            EXPECT_CALL(mockSt, GetError()).WillRepeatedly(Return("just testing"));
        }
        EXPECT_THROW(statement.ExecuteAll({10}), std::runtime_error);
    }
    {
        {
            InSequence s;
            EXPECT_CALL(mockSt, BindInt(1, 10)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(mockSt, Step()).WillOnce(Return(MockDatabase::error));
            EXPECT_CALL(mockSt, GetError()).WillRepeatedly(Return("just testing"));
        }
        EXPECT_THROW(statement.ExecuteAll({10}), std::runtime_error);
    }
    {
        {
            InSequence s;
            EXPECT_CALL(mockSt, BindInt(1, 10)).WillOnce(Return(MockDatabase::error));
            EXPECT_CALL(mockSt, GetError()).WillRepeatedly(Return("just testing"));
        }

        EXPECT_THROW(statement.ExecuteAll({10}), std::runtime_error);
    }
}

TEST(DBStatement, Reset)
{
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    using namespace ::testing;
    MockDatabase mockDb;
    std::unique_ptr<MockStatement> pStatement = std::make_unique<MockStatement>("test statement;");
    MockStatement& mockSt = *pStatement;

    EXPECT_CALL(mockDb, PrepareStatement("test statement;"))
        .WillOnce(Return(ByMove(std::pair<MockDatabase::ResultCode, std::unique_ptr<IStatement>>{
            MockDatabase::ok, std::move(pStatement)})));
    DBStatement statement(mockDb, "test statement;");

    EXPECT_CALL(mockSt, Reset()).WillOnce(Return(MockDatabase::ok));
    statement.Reset();

    EXPECT_CALL(mockSt, GetError()).WillRepeatedly(Return("testing"));
    EXPECT_CALL(mockSt, Reset()).WillOnce(Return(MockDatabase::error));
    EXPECT_THROW(statement.Reset(), std::runtime_error);
}

TEST(DBResult, GetColumnCount)
{
    using namespace ::testing;
    std::shared_ptr<MockStatement> st = std::make_shared<MockStatement>("test;");
    const DBResult r(st);
    EXPECT_CALL(*st, GetColumnCount()).WillOnce(Return(3));
    EXPECT_EQ(3, r.GetColumnCount());

    EXPECT_CALL(*st, GetColumnCount()).WillOnce(Return(1));
    EXPECT_EQ(1, r.GetColumnCount());

    EXPECT_CALL(*st, GetColumnCount()).WillOnce(Throw(std::runtime_error("test")));
    EXPECT_CALL(*st, GetError()).WillRepeatedly(Return("testing"));
    EXPECT_THROW(r.GetColumnCount(), std::runtime_error);
}

TEST(DBResult, GetColumnInt)
{
    using namespace ::testing;
    std::shared_ptr<MockStatement> st = std::make_shared<MockStatement>("test;");
    const DBResult r(st);
    EXPECT_CALL(*st, GetColumnCount()).WillRepeatedly(Return(10));

    EXPECT_CALL(*st, GetInt(0)).WillOnce(Return(3));
    EXPECT_EQ(3, r.GetColumnInt(0));

    EXPECT_CALL(*st, GetInt(3)).WillOnce(Return(1));
    EXPECT_EQ(1, r.GetColumnInt(3));

    EXPECT_CALL(*st, GetInt(1)).WillOnce(Throw(std::runtime_error("test")));
    EXPECT_CALL(*st, GetError()).WillRepeatedly(Return("testing"));
    EXPECT_THROW(r.GetColumnInt(1), std::runtime_error);
}

TEST(DBResult, GetColumnInt64)
{
    using namespace ::testing;
    std::shared_ptr<MockStatement> st = std::make_shared<MockStatement>("test;");
    const DBResult r(st);
    EXPECT_CALL(*st, GetColumnCount()).WillRepeatedly(Return(10));

    EXPECT_CALL(*st, GetInt64(0)).WillOnce(Return(0xFFFFFFFFFFAF));
    EXPECT_EQ(0xFFFFFFFFFFAF, r.GetColumnInt64(0));

    EXPECT_CALL(*st, GetInt64(3)).WillOnce(Return(1));
    EXPECT_EQ(1, r.GetColumnInt64(3));

    EXPECT_CALL(*st, GetInt64(1)).WillOnce(Throw(std::runtime_error("test")));
    EXPECT_CALL(*st, GetError()).WillRepeatedly(Return("testing"));
    EXPECT_THROW(r.GetColumnInt64(1), std::runtime_error);
}

TEST(DBResult, GetColumnString)
{
    using namespace ::testing;
    std::shared_ptr<MockStatement> st = std::make_shared<MockStatement>("test;");
    const DBResult r(st);
    EXPECT_CALL(*st, GetColumnCount()).WillRepeatedly(Return(10));

    EXPECT_CALL(*st, GetString(0)).WillOnce(Return("asdf"));
    EXPECT_EQ("asdf", r.GetColumnString(0));

    EXPECT_CALL(*st, GetString(3)).WillOnce(Return("string"));
    EXPECT_EQ("string", r.GetColumnString(3));

    EXPECT_CALL(*st, GetString(1)).WillOnce(Throw(std::runtime_error("test")));
    EXPECT_CALL(*st, GetError()).WillRepeatedly(Return("testing"));
    EXPECT_THROW(r.GetColumnString(1), std::runtime_error);
}

TEST(DBResult, NextRow)
{
    using namespace ::testing;
    std::shared_ptr<MockStatement> st = std::make_shared<MockStatement>("test;");
    DBResult r(st);
    EXPECT_CALL(*st, Step()).WillOnce(Return(MockStatement::row));
    EXPECT_TRUE(r.NextRow());

    EXPECT_CALL(*st, Step()).WillOnce(Return(MockStatement::done));
    EXPECT_FALSE(r.NextRow());

    EXPECT_CALL(*st, Step()).WillOnce(Return(MockDatabase::error));
    EXPECT_CALL(*st, GetError()).WillRepeatedly(Return("testing"));
    EXPECT_THROW(r.NextRow(), std::runtime_error);
}