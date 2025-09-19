#include "MatlabArray.h"
#include "MatlabEngine.h"
#include "matrix.h"

#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace Matlab
{
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
        : owns_memory_(true) 
		, m_name(name)
    {
        array_ = mxCreateDoubleScalar(value);
    }

    /**
     * @brief Create double vector from std::vector
     */
    MatlabArray::MatlabArray(const std::string& name, const std::vector<double>& data)
        : owns_memory_(true) 
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
        return MatlabArray(m_name+"_"+std::to_string(index), mxGetCell(array_, index), false);  // Don't take ownership
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
        return MatlabArray(m_name + "_"+fieldname+"_"+std::to_string(index), mxGetField(array_, index, fieldname.c_str()), false);
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

    // Stream operator
    std::ostream& operator<<(std::ostream& os, const MatlabArray& arr) {
        arr.print(os);
        return os;
    }
}