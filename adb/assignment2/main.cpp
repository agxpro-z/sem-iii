#include <cmath>
#include <ctime>
#include <db_cxx.h> // Berkeley DB
#include <exception>
#include <iostream>
#include <random>
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

    int get(int k); // Fetch the value
    void put(int k, int v); // Store the value
};

inline int Database::get(int k) {
    Dbt key(static_cast<void*>(&k), sizeof(k)); // Create database key
    Dbt value; // Value
    this->mDatabase->get(nullptr, &key, &value, 0); // Get the value from database
    return *static_cast<int*>(value.get_data()); // Return the value
}

inline void Database::put(int k, int v) {
    Dbt key(static_cast<void*>(&k), sizeof(k)); // Create database key
    Dbt value(static_cast<void*>(&v), sizeof(v)); // Create database value
    this->mDatabase->put(nullptr, &key, &value, 0); // // Store value in database
}

void printElapsedTime(const timeval& t1, const timeval& t2) {
    if (t1.tv_sec == t2.tv_sec) {
        std::cout << "0s " << (t2.tv_usec - t1.tv_usec) / 1000 << "ms";
        return;
    }

    std::cout << std::abs(t2.tv_sec - t1.tv_sec) << "s " << std::abs(t2.tv_usec - t1.tv_usec) / 1000 << "ms";
}

// Store n random number in database
void store(const std::string& db_name, const int n) {
    timeval start; // Start time
    gettimeofday(&start, nullptr);

    Database* database = new Database(db_name); // Open database
    std::srand(std::time(nullptr));

    std::cout << "Storing " << n << " random number..." << std::endl;
    for (int i = 0; i < n; ++i) {
        int r = std::rand() % INT32_MAX;
        database->put(i, r);
    }
    std::cout << "Done..." << std::endl;

    delete database; // Close database

    timeval elapsed; // Elapsed time
    gettimeofday(&elapsed, nullptr);

    // Total elapsed time
    std::cout << "Time taken: ";
    printElapsedTime(start, elapsed); // Print elapsed
    std::cout << ".\n";
}

// Get sum of all the records stored in the database
void printSum(const std::string& db_name, const int n) {
    timeval start; // Start time
    gettimeofday(&start, nullptr);

    Database* database = new Database(db_name); // Open database

    long long int sum = 0;
    std::cout << "Computing sum of values stored in database..." << std::endl;
    for (int i = 0; i < n; ++i) {
        sum += database->get(i);
    }
    std::cout << "Done..." << std::endl;
    std::cout << "Sum = " << sum << std::endl;

    delete database; // Close database

    timeval elapsed; // Elapsed time
    gettimeofday(&elapsed, nullptr);

    // Total elapsed time
    std::cout << "Time taken: ";
    printElapsedTime(start, elapsed); // Print elapsed
    std::cout << ".\n";
}

int main(const int argc, const char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " database_name number" << std::endl;
        std::exit(1);
    }

    int n;
    try { // Try casting to int
        n = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid number." << std::endl;
        exit(1);
    }

    timeval start; // Start time
    gettimeofday(&start, nullptr);

    store(argv[1], n);
    std::cout << std::endl;
    printSum(argv[1], n);

    timeval elapsed; // Elapsed time
    gettimeofday(&elapsed, nullptr);

    // Total elapsed time
    std::cout << "\nTotal time taken: ";
    printElapsedTime(start, elapsed); // Print elapsed
    std::cout << ".\n";

    return 0;
}
