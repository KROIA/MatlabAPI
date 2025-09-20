#include "math/Matrix.h"
#include <memory>

namespace MatlabAPI
{
	Matrix::Matrix()
		: m_rows(0)
		, m_cols(0)
		, m_data(nullptr)
	{

	}
	Matrix::Matrix(size_t rows, size_t cols)
		: m_rows(rows)
		, m_cols(cols)
	{
		m_data = new double[rows * cols]();
		memset(m_data, 0, sizeof(double) * rows * cols);
	}
	Matrix::Matrix(MatlabArray* array)
	{
		if (array == nullptr || !array->isDouble())
		{
			throw std::invalid_argument("Matrix can only be constructed from a valid double MatlabArray.");
		}
		m_rows = array->getM();
		m_cols = array->getN();
		m_data = new double[m_rows * m_cols];
		// Matlab uses column-major order, so we need to transpose while copying
		const double* arrayData = array->getPr();
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				m_data[r * m_cols + c] = arrayData[c * m_rows + r];
			}
		}
	}
	Matrix::Matrix(const std::vector<std::vector<double>>& mat)
	{
		m_rows = mat.size();
		for (const auto& row : mat)
		{
			m_cols = std::max(m_cols, row.size());
		}
		m_data = new double[m_rows * m_cols]();
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				if (c < mat[r].size())
					m_data[r * m_cols + c] = mat[r][c];
				else
					m_data[r * m_cols + c] = 0.0;
			}
		}
	}
	Matrix::Matrix(const Matrix& other)
		: m_rows(other.m_rows)
		, m_cols(other.m_cols)
		, m_data(new double[other.m_rows * other.m_cols])
	{
		memcpy(m_data, other.m_data, sizeof(double) * other.m_rows * other.m_cols);
	}
	Matrix::Matrix(Matrix&& other) noexcept
		: m_rows(other.m_rows)
		, m_cols(other.m_cols)
		, m_data(other.m_data)
	{
		other.m_rows = 0;
		other.m_cols = 0;
		other.m_data = nullptr;
	}
	Matrix::~Matrix()
	{
		delete[] m_data;
	}

	Matrix Matrix::identity(size_t size)
	{
		Matrix id(size, size);
		for (size_t i = 0; i < size; i++)
		{
			id(i, i) = 1.0;
		}
		return id;
	}

	bool Matrix::operator==(const Matrix& other) const
	{
		if (m_rows != other.m_rows || m_cols != other.m_cols)
			return false;
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				if ((*this)(r, c) != other(r, c))
					return false;
			}
		}
		return true;
	}


	Matrix& Matrix::operator=(const Matrix& other)
	{
		if (this != &other)
		{
			delete[] m_data;

			m_rows = other.m_rows;
			m_cols = other.m_cols;
			m_data = new double[other.m_rows * other.m_cols];
			memcpy(m_data, other.m_data, sizeof(double) * other.m_rows * other.m_cols);
		}
		return *this;
	}
	Matrix& Matrix::operator=(Matrix&& other) noexcept
	{
		if (this != &other)
		{
			delete[] m_data;

			m_rows = other.m_rows;
			m_cols = other.m_cols;
			m_data = other.m_data;

			other.m_rows = 0;
			other.m_cols = 0;
			other.m_data = nullptr;
		}
		return *this;
	}

	Matrix Matrix::operator+(const Matrix& other) const
	{
		if (m_rows != other.m_rows || m_cols != other.m_cols)
		{
			throw std::invalid_argument("Matrix dimensions must agree for addition.");
		}
		Matrix result(m_rows, m_cols);
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				result(r, c) = (*this)(r, c) + other(r, c);
			}
		}
		return result;
	}
	Matrix Matrix::operator-(const Matrix& other) const
	{
		if (m_rows != other.m_rows || m_cols != other.m_cols)
		{
			throw std::invalid_argument("Matrix dimensions must agree for subtraction.");
		}
		Matrix result(m_rows, m_cols);
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				result(r, c) = (*this)(r, c) - other(r, c);
			}
		}
		return result;
	}
	Matrix Matrix::operator*(const Matrix& other) const
	{
		if (m_cols != other.m_rows)
		{
			throw std::invalid_argument("Matrix dimensions must agree for multiplication.");
		}
		Matrix result(m_rows, other.m_cols);
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < other.m_cols; c++)
			{
				result(r, c) = 0.0;
				for (size_t k = 0; k < m_cols; k++)
				{
					result(r, c) += (*this)(r, k) * other(k, c);
				}
			}
		}
		return result;
	}
	Matrix Matrix::operator*(double scalar) const
	{
		Matrix result(m_rows, m_cols);
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				result(r, c) = (*this)(r, c) * scalar;
			}
		}
		return result;
	}
	Matrix Matrix::operator/(double scalar) const
	{
		Matrix result(m_rows, m_cols);
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				result(r, c) = (*this)(r, c) / scalar;
			}
		}
		return result;
	}
	Matrix& Matrix::operator+=(const Matrix& other)
	{
		if (m_rows != other.m_rows || m_cols != other.m_cols)
		{
			throw std::invalid_argument("Matrix dimensions must agree for addition.");
		}
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				(*this)(r, c) += other(r, c);
			}
		}
		return *this;
	}
	Matrix& Matrix::operator-=(const Matrix& other)
	{
		if (m_rows != other.m_rows || m_cols != other.m_cols)
		{
			throw std::invalid_argument("Matrix dimensions must agree for subtraction.");
		}
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				(*this)(r, c) -= other(r, c);
			}
		}
		return *this;
	}
	Matrix& Matrix::operator*=(const Matrix& other)
	{
		if (m_cols != other.m_rows)
		{
			throw std::invalid_argument("Matrix dimensions must agree for multiplication.");
		}
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < other.m_cols; c++)
			{
				double sum = 0.0;
				for (size_t k = 0; k < m_cols; k++)
				{
					sum += (*this)(r, k) * other(k, c);
				}
				(*this)(r, c) = sum;
			}
		}
		return *this;
	}
	Matrix& Matrix::operator*=(double scalar)
	{
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				(*this)(r, c) *= scalar;
			}
		}
		return *this;
	}
	Matrix& Matrix::operator/=(double scalar)
	{
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				(*this)(r, c) /= scalar;
			}
		}
		return *this;
	}
	Matrix& Matrix::transpose()
	{
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = r + 1; c < m_cols; c++)
			{
				std::swap((*this)(r, c), (*this)(c, r));
			}
		}
		std::swap(m_rows, m_cols);
		return *this;
	}
	Matrix Matrix::getTransposed() const
	{
		Matrix result(m_cols, m_rows);
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				result(c, r) = (*this)(r, c);
			}
		}
		return result;
	}

	MatlabArray* Matrix::toMatlabArray(const std::string& name) const
	{
		// Matlab uses column-major order, so we need to transpose while copying
		std::vector<double> colMajorData(m_rows * m_cols);
		for (size_t r = 0; r < m_rows; r++)
		{
			for (size_t c = 0; c < m_cols; c++)
			{
				colMajorData[c * m_rows + r] = m_data[r * m_cols + c];
			}
		}
		MatlabArray* array = new MatlabArray(name, m_rows, m_cols, colMajorData);
		return array;
	}


	std::string Matrix::toString() const
	{
		std::stringstream ss;
		ss << *this;
		return ss.str();
	}

	// Stream operator
	std::ostream& operator<<(std::ostream& os, const Matrix& arr)
	{
		os << "[";
		for (size_t r = 0; r < arr.m_rows; r++)
		{
			if (r > 0)
				os << " ";
			for (size_t c = 0; c < arr.m_cols; c++)
			{
				os << arr(r, c);
				if (c < arr.m_cols - 1)
					os << ", ";
			}
			if (r < arr.m_rows - 1)
				os << ";\n";
		}
		os << "]";

		return os;
	}

}