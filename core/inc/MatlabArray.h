#pragma once
#include "MatlabAPI_base.h"
#include <vector>
#include <string>

struct mxArray_tag; // Forward declaration of mxArray
typedef struct mxArray_tag mxArray;

namespace MatlabAPI
{
    class MATLAB_API MatlabEngine;

    /**
     * @brief C++ wrapper for MATLAB mxArray with RAII and modern C++ features
     */
    class MATLAB_API MatlabArray 
    {
        friend class MatlabEngine;
    public:
        // ===== Constructors and Destructor =====

        /**
         * @brief Default constructor - creates empty array
         */
        MatlabArray(const std::string &name);

        /**
         * @brief Constructor from existing mxArray (takes ownership)
         */
        explicit MatlabArray(const std::string& name, mxArray* arr, bool take_ownership = true);

        /**
         * @brief Create double scalar
         */
        explicit MatlabArray(const std::string& name, double value);

        /**
         * @brief Create double vector from std::vector
         */
        explicit MatlabArray(const std::string& name, const std::vector<double>& data);

        /**
         * @brief Create double matrix
         */
        MatlabArray(const std::string& name, size_t rows, size_t cols, const std::vector<double>& data);
        

        /**
         * @brief Create string array
         */
        explicit MatlabArray(const std::string& name, const std::string& str);

        /**
         * @brief Create logical array
         */
        explicit MatlabArray(const std::string& name, const std::vector<bool>& data);

        // Copy constructor
        MatlabArray(const MatlabArray& other);

        // Move constructor
        MatlabArray(MatlabArray&& other) noexcept;

        // Copy assignment
        MatlabArray& operator=(const MatlabArray& other);

        // Move assignment
        MatlabArray& operator=(MatlabArray&& other) noexcept;

        // Destructor
        ~MatlabArray();

        // ===== Static Factory Methods =====

        static MatlabArray createDouble(const std::string& name, size_t rows, size_t cols);

        static MatlabArray createComplex(const std::string& name, size_t rows, size_t cols);

        static MatlabArray createLogical(const std::string& name, size_t rows, size_t cols);

        static MatlabArray createCell(const std::string& name, size_t rows, size_t cols);

        // ===== Access to underlying mxArray =====

        mxArray* get() const { return array_; }
        mxArray* release();

		void overwrite(mxArray* arr, bool take_ownership = true, bool deleteOld = true);

		const std::string& getName() const { return m_name; }
		void setName(const std::string& name) { m_name = name; }

        // ===== Type Checking =====

        bool isDouble() const;
        bool isSingle() const;
        bool isInt8() const;
        bool isInt16() const;
        bool isInt32() const;
        bool isInt64() const;
        bool isUint8() const;
        bool isUint16() const;
        bool isUint32() const;
        bool isUint64() const;
        bool isLogical() const;
        bool isChar() const;
        bool isStruct() const;
        bool isCell() const;
        bool isFunction() const;
        bool isComplex() const;
        bool isSparse() const;
        bool isNumeric() const;
        bool isEmpty() const;
        bool isValid() const;

        std::string getClassName() const;

        // ===== Dimension Information =====

        size_t getNumberOfDimensions() const;

        std::vector<size_t> getDimensions() const;

        size_t getM() const;  // rows
        size_t getN() const;  // cols
        size_t getNumberOfElements() const;

        // ===== Data Access =====

        template<typename T>
        T* getData() {
            return static_cast<T*>(getDataRaw());
        }
        
        template<typename T>
        const T* getData() const {
            return static_cast<const T*>(getDataRawC());
        }

        void* getDataRaw();
        void* getDataRawC() const;

        double* getPr() const;

        // ===== Convenience Methods for Common Types =====

        /**
         * @brief Get scalar double value
         */
        double getScalar() const;

        /**
         * @brief Get vector of doubles
         */
        std::vector<double> getDoubleVector() const;

        /**
         * @brief Get string value
         */
        std::string getString() const;

        /**
         * @brief Get logical vector
         */
        std::vector<bool> getLogicalVector() const;

        /**
         * @brief Get matrix data in row-major order (converts from MATLAB's column-major)
         */
        std::vector<std::vector<double>> getDoubleMatrix() const;

        // ===== Cell Array Access =====

        MatlabArray getCell(size_t index) const;

        void setCell(size_t index, const MatlabArray& value);

        // ===== Struct Field Access =====

        std::vector<std::string> getFieldNames() const;

        MatlabArray getField(const std::string& fieldname, size_t index = 0) const;

        void setField(const std::string& fieldname, const MatlabArray& value, size_t index = 0);

        // ===== Utility Methods =====

        /**
         * @brief Print array information to stream
         */
        void print(std::ostream& os = std::cout) const;

        /**
         * @brief Clone the array
         */
        MatlabArray clone() const;

		bool updateFromEngine();
		bool updateToEngine();

        // ===== Operators =====

        explicit operator bool() const { return array_ != nullptr; }

		std::string toString() const;

        // Stream operator
        friend std::ostream& operator<<(std::ostream& os, const MatlabArray& arr);


        private:
            mxArray* array_;
            bool owns_memory_;
			std::string m_name;

			MatlabEngine* m_owner = nullptr;
    };
}