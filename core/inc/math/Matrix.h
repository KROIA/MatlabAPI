#pragma once
#include "MatlabAPI_base.h"
#include "MatlabArray.h"
#include <vector>


namespace MatlabAPI
{
	class MATLAB_API Matrix
	{
	public:
		Matrix();
		explicit Matrix(size_t rows, size_t cols);

		/**
		 * @brief
		 * @param mat row-major 2D vector -> mat[0] is the first row
		 */
		explicit Matrix(const std::vector<std::vector<double>>& mat);
		explicit Matrix(MatlabArray* array);
		
		Matrix(const Matrix& other);
		Matrix(Matrix&& other) noexcept;

		~Matrix();

		static Matrix identity(size_t size);

		bool operator==(const Matrix& other) const;

		double& operator()(size_t row, size_t col) { return m_data[row * m_cols + col]; }
		const double& operator()(size_t row, size_t col) const { return m_data[row * m_cols + col]; }

		Matrix& operator=(const Matrix& other);
		Matrix& operator=(Matrix&& other) noexcept;

		Matrix operator+(const Matrix& other) const;
		Matrix operator-(const Matrix& other) const;
		Matrix operator*(const Matrix& other) const;
		Matrix operator*(double scalar) const;
		Matrix operator/(double scalar) const;
		Matrix& operator+=(const Matrix& other);
		Matrix& operator-=(const Matrix& other);
		Matrix& operator*=(const Matrix& other);
		Matrix& operator*=(double scalar);
		Matrix& operator/=(double scalar);
		Matrix& transpose();
		Matrix getTransposed() const;

		double* data() { return m_data; }
		const double* data() const { return m_data; }


		size_t getRows() const { return m_rows; }
		size_t getCols() const { return m_cols; }

		MatlabArray* toMatlabArray(const std::string& name) const;


		std::string toString() const;

		// Stream operator
		friend std::ostream& operator<<(std::ostream& os, const Matrix& arr);
	protected:
		size_t m_rows = 0;
		size_t m_cols = 0;

		double* m_data = nullptr;
	};
}