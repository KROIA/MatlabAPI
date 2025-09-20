#pragma once
#include "MatlabAPI_base.h"
#include "Matrix.h"
#include "StateSpaceModel.h"
#include <vector>

namespace MatlabAPI
{
	class MATLAB_API TransferFunction
	{
	public:
		static const TransferFunction ZERO;
		static const TransferFunction ONE;
		static const TransferFunction S;
		static const TransferFunction INTEGRATOR;


		TransferFunction();
		TransferFunction(const std::vector<double>& num, const std::vector<double>& den);
		TransferFunction(const TransferFunction& other);

		void operator=(const TransferFunction& other);


		// Accessors
		void setNumerator(const std::vector<double>& num) { numerator = num; }
		const std::vector<double>& getNumerator() const { return numerator; }

		void setDenominator(const std::vector<double>& den) { denominator = den; }
		const std::vector<double>& getDenominator() const { return denominator; }

		StateSpaceModel toStateSpaceModel() const;
		void putInMatlabWorkspace(const std::string& varName) const;



	private:
		std::vector<double> numerator;
		std::vector<double> denominator;
	};
}