#include <ctime>
#include <db_cxx.h> // Berkeley DB
#include <exception>
#include <iostream>
#include <string>
#include <sys/time.h> // gettimeofday

enum DbErrorCode {
    DB_SUCCESS,
    DB_ERROR
};

class Database {
  private:
    DbEnv* mEnv; // Berkeley DB environment variable
    Db* mDatabase; // Berkeley DB connection

  public:
    Database(const std::string dbName) : mEnv(nullptr), mDatabase(nullptr) {
        mEnv = new DbEnv(0); // Berkeley DB Environment
        mEnv->set_error_stream(&std::cerr); // Set error stream
        mEnv->set_cachesize(0, 64 * 1024, 0); // Allocate cache memory

        try {
            mEnv->open("./db", DB_CREATE | DB_INIT_MPOOL, 0); // Open environment
        } catch (const DbException& e) {
            std::cerr << "Error: Unable to open db.\n";
            std::cerr << e.what() << std::endl;
            std::exit(DbErrorCode::DB_ERROR);
        } catch (const std::exception& e) {
            std::cerr << "Error: Unable to create environment.\n";
            std::cerr << e.what() << std::endl;
            std::exit(DbErrorCode::DB_ERROR);
        }

        std::cout << "Opening database...\n";
        try {
            mDatabase = new Db(mEnv, 0); // Database
            // Open the database
            mDatabase->open(nullptr, (dbName + ".db").c_str(), nullptr, DB_BTREE, DB_CREATE, 0);
        } catch (const DbException& e) {
            std::cerr << "Error opening database.\n";
            std::cerr << e.what() << std::endl;
            std::exit(DbErrorCode::DB_ERROR);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            std::exit(DbErrorCode::DB_ERROR);
        }
    }

    ~Database() {
        std::cout << "Closing database...\n";
        mDatabase->close(0); // Close the database
        mEnv->close(0); // Close the environment

        delete mDatabase;
        delete mEnv;
    }
};

int main(const int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " database_name" << std::endl;
        std::exit(1);
    }

    timeval start; // Start time
    gettimeofday(&start, nullptr);

    std::string db_name(argv[1]);
    Database* database = new Database(db_name);
    delete database;

    timeval elapsed; // Elapsed time
    gettimeofday(&elapsed, nullptr);

    // Total elapsed time
    std::cout << "Time taken: " << (elapsed.tv_usec - start.tv_usec) / 1000 << "ms.\n";

    return 0;
}
