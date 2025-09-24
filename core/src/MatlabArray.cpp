#include "MatlabArray.h"
#include "MatlabEngine.h"
#ifdef MATLAB_API_USE_CPP_API
#include "MatlabDataArray.hpp"
#else
#include "matrix.h"
#endif

#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace MatlabAPI
{
#ifdef MATLAB_API_USE_CPP_API
    static matlab::data::ArrayFactory* s_factory = nullptr; // Factory for creating MATLAB data arrays


    MatlabArray::MatlabArray(const std::string& name)
        : array_(nullptr)
        , m_name(name)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
    }

    /**
     * @brief Constructor from existing mxArray (takes ownership)
     */
    MatlabArray::MatlabArray(const std::string& name, const matlab::data::Array& arr)
        : array_(new matlab::data::Array(arr))
        , m_name(name)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
    }

    /**
     * @brief Create double scalar
     */
    MatlabArray::MatlabArray(const std::string& name, double value)
        : array_(nullptr)
        , m_name(name)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
        array_ = new matlab::data::Array(s_factory->createScalar(value));
    }

    /**
     * @brief Create double vector from std::vector
     */
    MatlabArray::MatlabArray(const std::string& name, const std::vector<double>& data)
        : array_(nullptr)
        , m_name(name)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
        array_ = new matlab::data::Array(s_factory->createArray({ data.size(), 1 }, data.begin(), data.end()));
    }

    /**
     * @brief Create double matrix
     */
    MatlabArray::MatlabArray(const std::string& name, size_t rows, size_t cols, const std::vector<double>& data)
        : m_name(name)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
        if (data.size() != rows * cols) {
            throw std::invalid_argument("Data size doesn't match matrix dimensions");
        }
        array_ = new matlab::data::Array(s_factory->createArray({ rows, cols }, data.begin(), data.end()));
    }

    /**
     * @brief Create string array
     */
    MatlabArray::MatlabArray(const std::string& name, const std::string& str)
        : m_name(name)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
        array_ = new matlab::data::Array(s_factory->createCharArray(str));
    }

    /**
     * @brief Create logical array
     */
    MatlabArray::MatlabArray(const std::string& name, const std::vector<bool>& data)
        : m_name(name)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
        array_ = new matlab::data::Array(s_factory->createArray({ data.size(), 1 }, data.begin(), data.end()));
    }

    // Copy constructor
    MatlabArray::MatlabArray(const MatlabArray& other)
        : m_name(other.m_name)
    {
        if (other.array_) {
            array_ = new matlab::data::Array(*(other.array_));
        }
        else {
            array_ = nullptr;
        }
    }

    // Move constructor
    MatlabArray::MatlabArray(MatlabArray&& other) noexcept
        : array_(other.array_)
        , m_name(std::move(other.m_name))
    {
        other.array_ = nullptr;
    }

    // Copy assignment
    MatlabArray& MatlabArray:: operator=(const MatlabArray& other)
    {
        if (this != &other) {
            delete array_;
            if (other.array_) {
                array_ = new matlab::data::Array(*(other.array_));
            }
            else {
                array_ = nullptr;
            }
            m_name = other.m_name;
        }
        return *this;
    }

    // Move assignment
    MatlabArray& MatlabArray::operator=(MatlabArray&& other) noexcept
    {
        if (this != &other) {
            delete array_;
            array_ = other.array_;
            other.array_ = nullptr;
            m_name = std::move(other.m_name);
        }
        return *this;
    }

    // Destructor
    MatlabArray::~MatlabArray()
    {
        delete array_;
    }

    // ===== Static Factory Methods =====

    MatlabArray MatlabArray::createDouble(const std::string& name, size_t rows, size_t cols)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
        auto array = s_factory->createArray<double>({ rows, cols });
        return MatlabArray(name, array);
    }

    MatlabArray MatlabArray::createComplex(const std::string& name, size_t rows, size_t cols)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
        auto array = s_factory->createArray<std::complex<double>>({ rows, cols });
        return MatlabArray(name, array);
    }

    MatlabArray MatlabArray::createLogical(const std::string& name, size_t rows, size_t cols)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
        auto array = s_factory->createArray<bool>({ rows, cols });
        return MatlabArray(name, array);
    }

    MatlabArray MatlabArray::createCell(const std::string& name, size_t rows, size_t cols)
    {
        if (!s_factory)
            s_factory = new matlab::data::ArrayFactory();
        auto array = s_factory->createCellArray({ rows, cols });
        return MatlabArray(name, array);
    }

    // ===== Access to underlying mxArray =====

    matlab::data::Array* MatlabArray::release()
    {
        return array_;
    }

    void MatlabArray::overwrite(const matlab::data::Array& arr)
    {
        delete array_;
        array_ = new matlab::data::Array(arr);
    }

    // ===== Type Checking =====

    bool MatlabArray::isDouble() const { return array_ && array_->getType() == matlab::data::ArrayType::DOUBLE; }
    bool MatlabArray::isSingle() const { return array_ && array_->getType() == matlab::data::ArrayType::SINGLE; }
    bool MatlabArray::isInt8() const { return array_ && array_->getType() == matlab::data::ArrayType::INT8; }
    bool MatlabArray::isInt16() const { return array_ && array_->getType() == matlab::data::ArrayType::INT16; }
    bool MatlabArray::isInt32() const { return array_ && array_->getType() == matlab::data::ArrayType::INT32; }
    bool MatlabArray::isInt64() const { return array_ && array_->getType() == matlab::data::ArrayType::INT64; }
    bool MatlabArray::isUint8() const { return array_ && array_->getType() == matlab::data::ArrayType::UINT8; }
    bool MatlabArray::isUint16() const { return array_ && array_->getType() == matlab::data::ArrayType::UINT16; }
    bool MatlabArray::isUint32() const { return array_ && array_->getType() == matlab::data::ArrayType::UINT32; }
    bool MatlabArray::isUint64() const { return array_ && array_->getType() == matlab::data::ArrayType::UINT64; }
    bool MatlabArray::isLogical() const { return array_ && array_->getType() == matlab::data::ArrayType::LOGICAL; }
    bool MatlabArray::isChar() const { return array_ && array_->getType() == matlab::data::ArrayType::CHAR; }
    bool MatlabArray::isStruct() const { return array_ && array_->getType() == matlab::data::ArrayType::STRUCT; }
    bool MatlabArray::isCell() const { return array_ && array_->getType() == matlab::data::ArrayType::CELL; }
    bool MatlabArray::isFunction() const { return array_ && array_->getType() == matlab::data::ArrayType::HANDLE_OBJECT_REF; }
    bool MatlabArray::isComplex() const {
        if (!array_)
            return false;
        matlab::data::ArrayType type = array_->getType();
        switch (type)
        {
        case matlab::data::ArrayType::COMPLEX_DOUBLE:
        case matlab::data::ArrayType::COMPLEX_SINGLE:
        case matlab::data::ArrayType::COMPLEX_INT8:
        case matlab::data::ArrayType::COMPLEX_UINT8:
        case matlab::data::ArrayType::COMPLEX_INT16:
        case matlab::data::ArrayType::COMPLEX_UINT16:
        case matlab::data::ArrayType::COMPLEX_INT32:
        case matlab::data::ArrayType::COMPLEX_UINT32:
        case matlab::data::ArrayType::COMPLEX_INT64:
        case matlab::data::ArrayType::COMPLEX_UINT64:
            return true;
        }
        return false;
    }
    bool MatlabArray::isSparse() const {
        if (!array_)
            return false;
        matlab::data::ArrayType type = array_->getType();
        switch (type)
        {
        case matlab::data::ArrayType::SPARSE_COMPLEX_DOUBLE:
        case matlab::data::ArrayType::SPARSE_DOUBLE:
        case matlab::data::ArrayType::SPARSE_LOGICAL:
            return true;
        }
        return false;
    }
    bool MatlabArray::isNumeric() const {
        if (!array_)
            return false;
        matlab::data::ArrayType type = array_->getType();
        switch (type)
        {
        case matlab::data::ArrayType::CHAR:
        case matlab::data::ArrayType::MATLAB_STRING:
        case matlab::data::ArrayType::CELL:
        case matlab::data::ArrayType::STRUCT:
        case matlab::data::ArrayType::OBJECT:
        case matlab::data::ArrayType::VALUE_OBJECT:
        case matlab::data::ArrayType::HANDLE_OBJECT_REF:
        case matlab::data::ArrayType::ENUM:
        case matlab::data::ArrayType::UNKNOWN:
            return false;
        }
        return true;
    }
    bool MatlabArray::isEmpty() const { return array_ && array_->isEmpty(); }
    bool MatlabArray::isValid() const { return array_ != nullptr; }

    std::string MatlabArray::getClassName() const
    {
        if (!array_)
            return "none";
        matlab::data::ArrayType type = array_->getType();
        switch (type)
        {
        case matlab::data::ArrayType::LOGICAL: return "logical";
        case matlab::data::ArrayType::CHAR: return "char";
        case matlab::data::ArrayType::MATLAB_STRING: return "string";
        case matlab::data::ArrayType::DOUBLE: return "double";
        case matlab::data::ArrayType::SINGLE: return "single";
        case matlab::data::ArrayType::INT8: return "int8";
        case matlab::data::ArrayType::UINT8: return "uint8";
        case matlab::data::ArrayType::INT16: return "int16";
        case matlab::data::ArrayType::UINT16: return "uint16";
        case matlab::data::ArrayType::INT32: return "int32";
        case matlab::data::ArrayType::UINT32: return "uint32";
        case matlab::data::ArrayType::INT64: return "int64";
        case matlab::data::ArrayType::UINT64: return "uint64";
        case matlab::data::ArrayType::COMPLEX_DOUBLE: return "complex double";
        case matlab::data::ArrayType::COMPLEX_SINGLE: return "complex single";
        case matlab::data::ArrayType::COMPLEX_INT8: return "complex int8";
        case matlab::data::ArrayType::COMPLEX_UINT8: return "complex uint8";
        case matlab::data::ArrayType::COMPLEX_INT16: return "complex int16";
        case matlab::data::ArrayType::COMPLEX_UINT16: return "complex uint16";
        case matlab::data::ArrayType::COMPLEX_INT32: return "complex int32";
        case matlab::data::ArrayType::COMPLEX_UINT32: return "complex uint32";
        case matlab::data::ArrayType::COMPLEX_INT64: return "complex int64";
        case matlab::data::ArrayType::COMPLEX_UINT64: return "complex uint64";
        case matlab::data::ArrayType::CELL: return "cell";
        case matlab::data::ArrayType::STRUCT: return "struct";
        case matlab::data::ArrayType::OBJECT: return "object";
        case matlab::data::ArrayType::VALUE_OBJECT: return "value object";
        case matlab::data::ArrayType::HANDLE_OBJECT_REF: return "handle object";
        case matlab::data::ArrayType::ENUM: return "enum";
        case matlab::data::ArrayType::SPARSE_LOGICAL: return "sparse logical";
        case matlab::data::ArrayType::SPARSE_DOUBLE: return "sparse double";
        case matlab::data::ArrayType::SPARSE_COMPLEX_DOUBLE: return "sparse complex double";
        case matlab::data::ArrayType::UNKNOWN: return "unknown";
        }
        return "unknown";
    }

    // ===== Dimension Information =====

    size_t MatlabArray::getNumberOfDimensions() const
    {
        matlab::data::ArrayDimensions dims = array_ ? array_->getDimensions() : matlab::data::ArrayDimensions();
        return dims.size();
    }

    std::vector<size_t> MatlabArray::getDimensions() const
    {
        matlab::data::ArrayDimensions dims = array_ ? array_->getDimensions() : matlab::data::ArrayDimensions();
        std::vector<size_t> ndims = dims.size() > 0 ? std::vector<size_t>(dims.begin(), dims.end()) : std::vector<size_t>{ 0 };
        return ndims;
    }

    size_t MatlabArray::getM() const { return array_ ? array_->getDimensions()[0] : 0; }  // rows
    size_t MatlabArray::getN() const { return array_ ? array_->getDimensions()[1] : 0; }  // cols
    size_t MatlabArray::getNumberOfElements() const
    {
        return array_ ? array_->getNumberOfElements() : 0;
    }

    // ===== Data Access =====

    //template<typename T>
    //T* getData() {
    //    return array_ ? static_cast<T*>(mxGetData(array_)) : nullptr;
    //}
    //
    //template<typename T>
    //const T* getData() const {
    //    return array_ ? static_cast<const T*>(mxGetData(array_)) : nullptr;
    //}

    template<>
    std::vector<double> MatlabArray::getRowData<double>(size_t row) const
    {
        std::vector<double> result;
        if (!array_ || !isDouble())
            return result;

        size_t rows = getM();
        size_t cols = getN();
        if (row >= rows)
            throw std::out_of_range("Row index out of range");
        result.reserve(cols);
        for (size_t col = 0; col < cols; col++)
            result.push_back(array_[row][col]);
        return result;
    }

    template<>
    std::vector<double> MatlabArray::getColData<double>(size_t col) const
    {
        std::vector<double> result;
        if (!array_ || !isDouble())
            return result;
        size_t rows = getM();
        size_t cols = getN();
        if (col >= cols)
            throw std::out_of_range("Column index out of range");
        result.reserve(rows);
        for (size_t row = 0; row < rows; row++)
            result.push_back(array_[row][col]);
        return result;
    }

    // ===== Convenience Methods for Common Types =====

    /**
     * @brief Get scalar double value
     */
    double MatlabArray::getScalar() const
    {
        if (!isDouble() || getNumberOfElements() != 1) {
            throw std::runtime_error("Array is not a scalar double");
        }
        return (*array_)[0];
    }

    /**
     * @brief Get vector of doubles
     */
    std::vector<double> MatlabArray::getDoubleVector() const
    {
        if (!isDouble()) {
            throw std::runtime_error("Array is not double type");
        }
        size_t size = getNumberOfElements();
        std::vector<double> data(size);
        for (size_t i = 0; i < size; i++)
            data[i] = (*array_)[i];
        return data;
    }

    /**
     * @brief Get string value
     */
    std::string MatlabArray::getString() const
    {
        if (!isChar()) {
            throw std::runtime_error("Array is not char type");
        }
        matlab::data::CharArray charArray = (*array_)[0]; // Assuming single string
        return std::string(charArray.begin(), charArray.end());
    }

    /**
     * @brief Get logical vector
     */
    std::vector<bool> MatlabArray::getLogicalVector() const
    {
        if (!isLogical()) {
            throw std::runtime_error("Array is not logical type");
        }

        size_t size = getNumberOfElements();
        std::vector<bool> data(size);
        for (size_t i = 0; i < size; i++)
            data[i] = static_cast<bool>((*array_)[i]);
        return data;
    }

    /**
     * @brief Get matrix data in row-major order (converts from MATLAB's column-major)
     */
    std::vector<std::vector<double>> MatlabArray::getDoubleMatrix() const
    {
        if (!isDouble()) {
            throw std::runtime_error("Array is not double type");
        }

        size_t rows = getM();
        size_t cols = getN();


        std::vector<std::vector<double>> result(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                result[i][j] = (*array_)[i][j];  // Column-major to row-major
            }
        }
        return result;
    }

    // ===== Cell Array Access =====

    MatlabArray MatlabArray::getCell(size_t index) const
    {
        if (!isCell()) {
            throw std::runtime_error("Array is not a cell array");
        }
        if (index >= getNumberOfElements()) {
            throw std::out_of_range("Cell index out of range");
        }
        std::string name = m_name + "_" + std::to_string(index);
        matlab::data::Array cell = (*array_)[index];
        return MatlabArray(name, cell);
    }

    void MatlabArray::setCell(size_t index, const MatlabArray& value)
    {
        if (!isCell()) {
            throw std::runtime_error("Array is not a cell array");
        }
        if (index >= getNumberOfElements()) {
            throw std::out_of_range("Cell index out of range");
        }
        (*array_)[index] = *(value.array_);
    }

    // ===== Struct Field Access =====

    std::vector<std::string> MatlabArray::getFieldNames() const
    {
        if (!isStruct()) {
            throw std::runtime_error("Array is not a struct");
        }

        size_t nfields = array_->getNumberOfElements();
        std::vector<std::string> names;
        for (size_t i = 0; i < nfields; i++) {
            matlab::data::CharArray charArray = (*array_)[i]; // Assuming single string
            names.push_back(std::string(charArray.begin(), charArray.end()));
        }
        return names;
    }

    /*MatlabArray MatlabArray::getField(const std::string& fieldname, size_t index) const
    {
        if (!isStruct()) {
            throw std::runtime_error("Array is not a struct");
        }
        std::string name = m_name + "_" + fieldname + "_" + std::to_string(index);
        (*array_)[index]

        return MatlabArray(name, , false);
    }

    void MatlabArray::setField(const std::string& fieldname, const MatlabArray& value, size_t index)
    {
        if (!isStruct()) {
            throw std::runtime_error("Array is not a struct");
        }
        mxSetField(array_, index, fieldname.c_str(), mxDuplicateArray(value.get()));
    }*/

    // ===== Utility Methods =====

    /**
     * @brief Print array information to stream
     */
    void MatlabArray::print(std::ostream& os) const
    {
        if (!array_) {
            os << "Invalid/null matlab::data::Array" << std::endl;
            return;
        }
        os << "Variable Name: " << (m_name.empty() ? "<unnamed>" : m_name) << std::endl;
        os << "Class: " << getClassName() << std::endl;
        os << "Size: ";
        auto dims = getDimensions();
        for (size_t i = 0; i < dims.size(); i++) {
            os << dims[i];
            if (i < dims.size() - 1) os << " x ";
        }
        os << " (" << getNumberOfElements() << " elements)" << std::endl;
        os << "Complex: " << (isComplex() ? "Yes" : "No") << std::endl;
        os << "Sparse: " << (isSparse() ? "Yes" : "No") << std::endl;

        // Try casting to known types
        if (auto d = dynamic_cast<const matlab::data::TypedArray<double>*>(array_)) {
            for (size_t i = 0; i < dims[0]; ++i) {
                for (size_t j = 0; j < dims[1]; ++j) {
                    os << (*d)[i][j] << " ";
                }
                os << std::endl;
            }
        }
        /*else if (auto s = dynamic_cast<const matlab::data::TypedArray<std::string>*>(array_)) {
            for (auto& v : *s) {
                os << v << " ";
            }
            os << endl;
        }*/
        else if (auto sa = dynamic_cast<const matlab::data::StructArray*>(array_)) {
            os << "StructArray with " << sa->getNumberOfElements() << " elements" << std::endl;
            // You can iterate elements and fields if you want
        }
        else {
            os << "array type: " << typeid(*array_).name() << std::endl;
        }
    }

    /**
     * @brief Clone the array
     */
    MatlabArray MatlabArray::clone() const {
        if (!array_)
            return MatlabArray("");
        return MatlabArray(m_name, *array_);
    }

    bool MatlabArray::updateFromEngine()
    {
        if (m_owner)
            return m_owner->updateVariableFromEngine(this);
        return false;
    }
    bool MatlabArray::updateToEngine()
    {
        if (m_owner)
            return m_owner->sendVariableToEngine(this);
        return false;
    }

    // ===== Operators =====


    std::string MatlabArray::toString() const
    {
        std::ostringstream oss;
        print(oss);
        return oss.str();
    }

    const matlab::data::Array& MatlabArray::getAPIArray() const
    {
        if (!array_)
            throw std::runtime_error("Array is null");
        return *array_;
    }

    // Stream operator
    std::ostream& operator<<(std::ostream& os, const MatlabArray& arr) {
        arr.print(os);
        return os;
    }


#else
    MatlabArray::MatlabArray(const std::string& name)
        : array_(nullptr)
        , owns_memory_(false)
        , m_name(name)
    {}

    /**
     * @brief Constructor from existing mxArray (takes ownership)
     */
    MatlabArray::MatlabArray(const std::string& name, mxArray* arr, bool take_ownership)
        : array_(arr)
        , owns_memory_(take_ownership)
        , m_name(name)
    {}

    /**
     * @brief Create double scalar
     */
    MatlabArray::MatlabArray(const std::string& name, double value)
        : array_(nullptr)
        , owns_memory_(false)
        , m_name(name)
    {
        array_ = mxCreateDoubleScalar(value);
    }

    /**
     * @brief Create double vector from std::vector
     */
    MatlabArray::MatlabArray(const std::string& name, const std::vector<double>& data)
        : array_(nullptr)
        , owns_memory_(true)
        , m_name(name)
    {
        array_ = mxCreateDoubleMatrix(data.size(), 1, mxREAL);
        double* ptr = mxGetPr(array_);
        std::copy(data.begin(), data.end(), ptr);
    }

    /**
     * @brief Create double matrix
     */
    MatlabArray::MatlabArray(const std::string& name, size_t rows, size_t cols, const std::vector<double>& data)
        : owns_memory_(true)
        , m_name(name)
    {
        if (data.size() != rows * cols) {
            throw std::invalid_argument("Data size doesn't match matrix dimensions");
        }
        array_ = mxCreateDoubleMatrix(rows, cols, mxREAL);
        double* ptr = mxGetPr(array_);
        std::copy(data.begin(), data.end(), ptr);
    }

    /**
     * @brief Create string array
     */
    MatlabArray::MatlabArray(const std::string& name, const std::string& str)
        : owns_memory_(true)
        , m_name(name)
    {
        array_ = mxCreateString(str.c_str());
    }

    /**
     * @brief Create logical array
     */
    MatlabArray::MatlabArray(const std::string& name, const std::vector<bool>& data)
        : owns_memory_(true)
        , m_name(name)
    {
        array_ = mxCreateLogicalMatrix(data.size(), 1);
        bool* ptr = (bool*)mxGetData(array_);
        for (size_t i = 0; i < data.size(); i++) {
            ptr[i] = data[i];
        }
    }

    // Copy constructor
    MatlabArray::MatlabArray(const MatlabArray& other)
        : owns_memory_(true)
        , m_name(other.m_name)
    {
        if (other.array_) {
            array_ = mxDuplicateArray(other.array_);
        }
        else {
            array_ = nullptr;
            owns_memory_ = false;
        }
    }

    // Move constructor
    MatlabArray::MatlabArray(MatlabArray&& other) noexcept
        : array_(other.array_)
        , owns_memory_(other.owns_memory_)
        , m_name(std::move(other.m_name))
    {
        other.array_ = nullptr;
        other.owns_memory_ = false;
    }

    // Copy assignment
    MatlabArray& MatlabArray:: operator=(const MatlabArray& other)
    {
        if (this != &other) {
            if (owns_memory_ && array_) {
                mxDestroyArray(array_);
            }
            if (other.array_) {
                array_ = mxDuplicateArray(other.array_);
                owns_memory_ = true;
            }
            else {
                array_ = nullptr;
                owns_memory_ = false;
            }
            m_name = other.m_name;
        }
        return *this;
    }

    // Move assignment
    MatlabArray& MatlabArray::operator=(MatlabArray&& other) noexcept
    {
        if (this != &other) {
            if (owns_memory_ && array_) {
                mxDestroyArray(array_);
            }
            array_ = other.array_;
            owns_memory_ = other.owns_memory_;
            other.array_ = nullptr;
            other.owns_memory_ = false;
            m_name = std::move(other.m_name);
        }
        return *this;
    }

    // Destructor
    MatlabArray::~MatlabArray()
    {
        if (owns_memory_ && array_) {
            mxDestroyArray(array_);
        }
    }

    // ===== Static Factory Methods =====

    MatlabArray MatlabArray::createDouble(const std::string& name, size_t rows, size_t cols)
    {
        return MatlabArray(name, mxCreateDoubleMatrix(rows, cols, mxREAL), true);
    }

    MatlabArray MatlabArray::createComplex(const std::string& name, size_t rows, size_t cols)
    {
        return MatlabArray(name, mxCreateDoubleMatrix(rows, cols, mxCOMPLEX), true);
    }

    MatlabArray MatlabArray::createLogical(const std::string& name, size_t rows, size_t cols)
    {
        return MatlabArray(name, mxCreateLogicalMatrix(rows, cols), true);
    }

    MatlabArray MatlabArray::createCell(const std::string& name, size_t rows, size_t cols)
    {
        return MatlabArray(name, mxCreateCellMatrix(rows, cols), true);
    }

    // ===== Access to underlying mxArray =====

    mxArray* MatlabArray::release()
    {
        owns_memory_ = false;
        return array_;
    }

    void MatlabArray::overwrite(mxArray* arr, bool take_ownership, bool deleteOld)
    {
        if (arr == array_)
            return;
        if (deleteOld && owns_memory_ && array_) {
            mxDestroyArray(array_);
        }
        array_ = arr;
        owns_memory_ = take_ownership;
    }

    // ===== Type Checking =====

    bool MatlabArray::isDouble() const { return array_ && mxIsDouble(array_); }
    bool MatlabArray::isSingle() const { return array_ && mxIsSingle(array_); }
    bool MatlabArray::isInt8() const { return array_ && mxIsInt8(array_); }
    bool MatlabArray::isInt16() const { return array_ && mxIsInt16(array_); }
    bool MatlabArray::isInt32() const { return array_ && mxIsInt32(array_); }
    bool MatlabArray::isInt64() const { return array_ && mxIsInt64(array_); }
    bool MatlabArray::isUint8() const { return array_ && mxIsUint8(array_); }
    bool MatlabArray::isUint16() const { return array_ && mxIsUint16(array_); }
    bool MatlabArray::isUint32() const { return array_ && mxIsUint32(array_); }
    bool MatlabArray::isUint64() const { return array_ && mxIsUint64(array_); }
    bool MatlabArray::isLogical() const { return array_ && mxIsLogical(array_); }
    bool MatlabArray::isChar() const { return array_ && mxIsChar(array_); }
    bool MatlabArray::isStruct() const { return array_ && mxIsStruct(array_); }
    bool MatlabArray::isCell() const { return array_ && mxIsCell(array_); }
    bool MatlabArray::isFunction() const { return array_ && mxIsFunctionHandle(array_); }
    bool MatlabArray::isComplex() const { return array_ && mxIsComplex(array_); }
    bool MatlabArray::isSparse() const { return array_ && mxIsSparse(array_); }
    bool MatlabArray::isNumeric() const { return array_ && mxIsNumeric(array_); }
    bool MatlabArray::isEmpty() const { return array_ && mxIsEmpty(array_); }
    bool MatlabArray::isValid() const { return array_ != nullptr; }

    std::string MatlabArray::getClassName() const
    {
        return array_ ? mxGetClassName(array_) : "invalid";
    }

    // ===== Dimension Information =====

    size_t MatlabArray::getNumberOfDimensions() const
    {
        return array_ ? mxGetNumberOfDimensions(array_) : 0;
    }

    std::vector<size_t> MatlabArray::getDimensions() const
    {
        if (!array_) return {};
        const size_t* dims = mxGetDimensions(array_);
        size_t ndims = mxGetNumberOfDimensions(array_);
        return std::vector<size_t>(dims, dims + ndims);
    }

    size_t MatlabArray::getM() const { return array_ ? mxGetM(array_) : 0; }  // rows
    size_t MatlabArray::getN() const { return array_ ? mxGetN(array_) : 0; }  // cols
    size_t MatlabArray::getNumberOfElements() const
    {
        return array_ ? mxGetNumberOfElements(array_) : 0;
    }

    // ===== Data Access =====

    //template<typename T>
    //T* getData() {
    //    return array_ ? static_cast<T*>(mxGetData(array_)) : nullptr;
    //}
    //
    //template<typename T>
    //const T* getData() const {
    //    return array_ ? static_cast<const T*>(mxGetData(array_)) : nullptr;
    //}

    void* MatlabArray::getDataRaw()
    {
        return array_ ? mxGetData(array_) : nullptr;
    }
    void* MatlabArray::getDataRawC() const
    {
        return array_ ? mxGetData(array_) : nullptr;
    }

    double* MatlabArray::getPr() const { return array_ ? mxGetPr(array_) : nullptr; }

    // ===== Convenience Methods for Common Types =====

    /**
     * @brief Get scalar double value
     */
    double MatlabArray::getScalar() const
    {
        if (!isDouble() || getNumberOfElements() != 1) {
            throw std::runtime_error("Array is not a scalar double");
        }
        return *getPr();
    }

    /**
     * @brief Get vector of doubles
     */
    std::vector<double> MatlabArray::getDoubleVector() const
    {
        if (!isDouble()) {
            throw std::runtime_error("Array is not double type");
        }
        double* data = getPr();
        size_t size = getNumberOfElements();
        return std::vector<double>(data, data + size);
    }

    /**
     * @brief Get string value
     */
    std::string MatlabArray::getString() const
    {
        if (!isChar()) {
            throw std::runtime_error("Array is not char type");
        }
        char* str = mxArrayToString(array_);
        std::string result(str);
        mxFree(str);
        return result;
    }

    /**
     * @brief Get logical vector
     */
    std::vector<bool> MatlabArray::getLogicalVector() const
    {
        if (!isLogical()) {
            throw std::runtime_error("Array is not logical type");
        }
        const bool* data = getData<bool>();
        size_t size = getNumberOfElements();
        std::vector<bool> result;
        for (size_t i = 0; i < size; i++) {
            result.push_back(data[i]);
        }
        return result;
    }

    /**
     * @brief Get matrix data in row-major order (converts from MATLAB's column-major)
     */
    std::vector<std::vector<double>> MatlabArray::getDoubleMatrix() const
    {
        if (!isDouble()) {
            throw std::runtime_error("Array is not double type");
        }

        size_t rows = getM();
        size_t cols = getN();
        double* data = getPr();

        std::vector<std::vector<double>> result(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                result[i][j] = data[j * rows + i];  // Column-major to row-major
            }
        }
        return result;
    }

    // ===== Cell Array Access =====

    MatlabArray MatlabArray::getCell(size_t index) const
    {
        if (!isCell()) {
            throw std::runtime_error("Array is not a cell array");
        }
        if (index >= getNumberOfElements()) {
            throw std::out_of_range("Cell index out of range");
        }
        return MatlabArray(m_name + "_" + std::to_string(index), mxGetCell(array_, index), false);  // Don't take ownership
    }

    void MatlabArray::setCell(size_t index, const MatlabArray& value)
    {
        if (!isCell()) {
            throw std::runtime_error("Array is not a cell array");
        }
        if (index >= getNumberOfElements()) {
            throw std::out_of_range("Cell index out of range");
        }
        mxSetCell(array_, index, mxDuplicateArray(value.get()));
    }

    // ===== Struct Field Access =====

    std::vector<std::string> MatlabArray::getFieldNames() const
    {
        if (!isStruct()) {
            throw std::runtime_error("Array is not a struct");
        }

        int nfields = mxGetNumberOfFields(array_);
        std::vector<std::string> names;
        for (int i = 0; i < nfields; i++) {
            names.push_back(mxGetFieldNameByNumber(array_, i));
        }
        return names;
    }

    MatlabArray MatlabArray::getField(const std::string& fieldname, size_t index) const
    {
        if (!isStruct()) {
            throw std::runtime_error("Array is not a struct");
        }
        return MatlabArray(m_name + "_" + fieldname + "_" + std::to_string(index), mxGetField(array_, index, fieldname.c_str()), false);
    }

    void MatlabArray::setField(const std::string& fieldname, const MatlabArray& value, size_t index)
    {
        if (!isStruct()) {
            throw std::runtime_error("Array is not a struct");
        }
        mxSetField(array_, index, fieldname.c_str(), mxDuplicateArray(value.get()));
    }

    // ===== Utility Methods =====

    /**
     * @brief Print array information to stream
     */
    void MatlabArray::print(std::ostream& os) const
    {
        if (!array_) {
            os << "Invalid/null mxArray" << std::endl;
            return;
        }
        os << "Variable Name: " << (m_name.empty() ? "<unnamed>" : m_name) << std::endl;
        os << "Class: " << getClassName() << std::endl;
        os << "Size: ";
        auto dims = getDimensions();
        for (size_t i = 0; i < dims.size(); i++) {
            os << dims[i];
            if (i < dims.size() - 1) os << " x ";
        }
        os << " (" << getNumberOfElements() << " elements)" << std::endl;
        os << "Complex: " << (isComplex() ? "Yes" : "No") << std::endl;
        os << "Sparse: " << (isSparse() ? "Yes" : "No") << std::endl;

        // Print some data
        if (isDouble() && !isEmpty()) {
            os << "Data preview: ";
            double* data = getPr();
            size_t preview = std::min((size_t)5, getNumberOfElements());
            for (size_t i = 0; i < preview; i++) {
                os << data[i] << " ";
            }
            if (getNumberOfElements() > 5) os << "...";
            os << std::endl;
        }
        else if (isChar()) {
            os << "String: \"" << getString() << "\"" << std::endl;
        }
    }

    /**
     * @brief Clone the array
     */
    MatlabArray MatlabArray::clone() const {
        if (!array_)
            return MatlabArray("");
        return MatlabArray(m_name, mxDuplicateArray(array_), true);
    }

    bool MatlabArray::updateFromEngine()
    {
        if (m_owner)
            return m_owner->updateVariableFromEngine(this);
        return false;
    }
    bool MatlabArray::updateToEngine()
    {
        if (m_owner)
            return m_owner->sendVariableToEngine(this);
        return false;
    }

    // ===== Operators =====


    std::string MatlabArray::toString() const
    {
        std::ostringstream oss;
        print(oss);
        return oss.str();
    }


    const mxArray* MatlabArray::getAPIArray() const
    {
        return array_;
    }


    // Stream operator
    std::ostream& operator<<(std::ostream& os, const MatlabArray& arr) {
        arr.print(os);
        return os;
    }
#endif
}