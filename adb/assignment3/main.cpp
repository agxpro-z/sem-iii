#include <ctime>
#include <cstring> // memcpy
#include <db_cxx.h> // Berkeley DB
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
    Matrix(std::string matrixName, int n, int m) : Database(matrixName), row(n), col(m), name(matrixName) {
        // Initialize the matrix to zero values
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j) {
                this->Database::put(i, j, 0);
            }
        }
    }

    Matrix(std::string matrixName, std::vector<std::vector<double>>& matrix)
            : Database(matrixName), row(matrix.size()), col(matrix[0].size()), name(matrixName) {

        // Initialize the matrix to provided matrix
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j) {
                this->Database::put(i, j, matrix[i][j]);
            }
        }
    }

    Matrix(std::string matrixName, Matrix& matrixA, Matrix& matrixB)
        : Database(matrixName), row(matrixA.rowCount()), col(matrixB.colCount()) {

        // Initialize the matrix with product of matrix A and B
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j) {
                double res = 0;
                for (int k = 0; k < matrixB.rowCount(); ++k) {
                    res += matrixA.get(i, k) * matrixB.get(k, j);
                }
                this->set(i, j, res);
            }
        }
    }

    void set(int row, int col, double value) {
        this->Database::put(row, col, value);
    }

    double get(int row, int col) {
        return this->Database::get(row, col);
    }

    int rowCount() { return this->row; }
    int colCount() { return this->col; }
    void print();
};

inline void Matrix::print() {
    for (int i = 0; i < this->row; ++i) {
        for (int j = 0; j < this->col; ++j) {
            std::cout << this->get(i, j) << ' ';
        }
        std::cout << std::endl;
    }
}

// Fill matrix with random numbers
void fillMatrix(const std::vector<std::vector<double>>& matrix) {
    for (const std::vector<double>& mat : matrix) {
        for (const double& m : mat) {
            const_cast<double&>(m) = std::rand() % 100;
        }
    }
}

void printElapsedTime(const timeval& t1, const timeval& t2) {
    if (t1.tv_sec == t2.tv_sec) {
        std::cout << "0s " << (t2.tv_usec - t1.tv_usec) / 1000 << "ms";
        return;
    }

    std::cout << std::abs(t2.tv_sec - t1.tv_sec) << "s " << std::abs(t2.tv_usec - t1.tv_usec) / 1000 << "ms";
}

int main(const int argc, const char* argv[]) {
    if (argc < 6) {
        std::cerr << "Usage: " << argv[0] << " database_name <Matrix A row col> <Matrix B row col>" << std::endl;
        std::exit(1);
    }

    std::string database_name = argv[1];

    if (std::stoi(argv[3]) != std::stoi(argv[4])) {
        std::cerr << "Cannot multiply matrix A and B" << std::endl;
        exit(1);
    }

    int n = std::stoi(argv[2]);
    int k = std::stoi(argv[3]);
    int m = std::stoi(argv[5]);

    // Matrix of vectors
    std::vector<std::vector<double>> A(n, std::vector<double>(k, 0.0));
    std::vector<std::vector<double>> B(k, std::vector<double>(m, 0.0));

    // Fill matrix with random values
    std::srand(std::time(nullptr));
    fillMatrix(A);
    fillMatrix(B);

    timeval start; // Start timer
    gettimeofday(&start, nullptr);

    // Store matrices in database
    Matrix* ma = new Matrix(database_name + "_a", A);
    Matrix* mb = new Matrix(database_name + "_b", B);

    // Multiply matrices and store in database
    Matrix* mc = new Matrix(database_name + "_c", *ma, *mb);

    // Print matrices
    std::cout << "\nMatrix A:" << std::endl;
    ma->print();
    std::cout << "\nMatrix B:" << std::endl;
    mb->print();
    std::cout << "\nMatrix A x B:" << std::endl;
    mc->print();
    std::cout << std::endl;

    delete ma;
    delete mb;
    delete mc;

    timeval elapsed;
    gettimeofday(&elapsed, nullptr);

    // Total elapsed time
    std::cout << "\nTotal time taken: ";
    printElapsedTime(start, elapsed); // Print elapsed
    std::cout << ".\n";

    return 0;
}
