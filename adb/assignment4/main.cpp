#include <ctime>
#include <cstring> // memcpy
#include <db_cxx.h> // Berkeley DB
#include <exception>
#include <iostream>
#include <random> // srand, rand
#include <string>
#include <sys/time.h> // gettimeofday
#include <vector>

enum DbErrorCode {
    DB_SUCCESS,
    DB_ERROR,
};

class Database {
  private:
    DbEnv* mEnv; // Berkeley DB environment variable
    Db* mDatabase; // Berkeley DB connection
    std::string dbName;

  public:
    Database(const std::string dbName) : mEnv(nullptr), mDatabase(nullptr), dbName(dbName) {
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

        std::cout << "Opening database " << dbName << "...\n";
        try {
            mDatabase = new Db(mEnv, 0); // Database
            // Open the database
            mDatabase->open(nullptr, (dbName + ".db").c_str(), nullptr, DB_HASH, DB_CREATE, 0);
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
        std::cout << "Closing database " << dbName << "...\n";
        mDatabase->close(0); // Close the database
        mEnv->close(0); // Close the environment

        delete mDatabase;
        delete mEnv;
    }

    const double get(const int row, const int col); // Fetch the value
    void put(const int row, const int col, double v); // Store the value
};

inline const double Database::get(const int row, const int col) {
    // Make key using row and column
    std::string keyString(std::to_string(row) + std::to_string(col));

    // Conver keyString to char string
    char* k = new char[keyString.size()];
    std::memcpy(k, keyString.c_str(), keyString.size());

    // Make database key and value
    Dbt key(static_cast<void*>(k), keyString.size());
    Dbt value;

    this->mDatabase->get(nullptr, &key, &value, 0); // Retrieve the value
    return *static_cast<double*>(value.get_data()); // Return the value
    // return 0;
}

inline void Database::put(const int row, const int col, double v) {
    // Make key using row and column
    std::string keyString(std::to_string(row) + std::to_string(col));

    // Conver keyString to char string
    char* k = new char[keyString.size()];
    std::memcpy(k, keyString.c_str(), keyString.size());

    // Make database key and value
    Dbt key(static_cast<void*>(k), keyString.size());
    Dbt value(static_cast<void*>(&v), sizeof(v));

    this->mDatabase->put(nullptr, &key, &value, 0); // Set/update value
}

class Matrix : public Database {
  private:
    int row;
    int col;
    std::string name; // Database name

  public:
    Matrix(std::string matrixName, int n) : Database(matrixName), row(n), col(n), name(matrixName) {
        // Initialize the matrix to zero values
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j) {
                // Fill the matrix with random values
                this->Database::put(i, j, std::rand() % 100);
            }
        }
    }

    int rowCount() { return this->row; }
    int colCount() { return this->col; }
    int computeInfinityNorm();
    void print();
};

inline int Matrix::computeInfinityNorm() {
    int infNorm = 0;
    for (int i = 0; i < this->row; ++i) {
        int rowSum = 0;
        for (int j = 0; j < this->col; ++j) {
            rowSum += this->get(i, j);
        }

        infNorm = std::max(infNorm, rowSum);
    }

    return infNorm;
}

inline void Matrix::print() {
    for (int i = 0; i < this->row; ++i) {
        for (int j = 0; j < this->col; ++j) {
            std::cout << this->get(i, j) << ' ';
        }
        std::cout << std::endl;
    }
}

void printElapsedTime(const timeval& t1, const timeval& t2) {
    std::cout << std::abs(t2.tv_sec - t1.tv_sec) << "s " << std::abs(t2.tv_usec - t1.tv_usec) / 1000 << "ms";
}


int main(const int argc, const char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " database_name matrix_size" << std::endl;
        std::exit(1);
    }

    int n;
    try {
        n = std::stoi(argv[2]);
    } catch (std::exception& e) {
        std::cout << "Error: Invalid arguments" << std::endl;
        std::exit(1);
    }

    timeval start;
    gettimeofday(&start, nullptr);

    std::srand(std::time(nullptr));
    Matrix* matrix = new Matrix(argv[1], n);
    matrix->print();
    std::cout << "\nInfinity Norm of matrix: " << matrix->computeInfinityNorm() << std::endl;
    delete matrix;

    timeval elapsed;
    gettimeofday(&elapsed, nullptr);

    // Total elapsed time
    std::cout << "\nTotal time taken: ";
    printElapsedTime(start, elapsed); // Print elapsed
    std::cout << ".\n";

    return 0;
}