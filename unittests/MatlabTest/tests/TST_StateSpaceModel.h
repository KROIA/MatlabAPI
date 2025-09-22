#pragma once

#include "UnitTest.h"
#include "MatlabAPI.h"
#include <fstream>



using namespace MatlabAPI;
class TST_StateSpaceModel : public UnitTest::Test
{
	TEST_CLASS(TST_StateSpaceModel)
public:
	TST_StateSpaceModel()
		: Test("TST_StateSpaceModel")
	{
		ADD_TEST(TST_StateSpaceModel::stepResp);
		ADD_TEST(TST_StateSpaceModel::MIMOstepResp);


	}

private:

	// Tests
	TEST_FUNCTION(stepResp)
	{
		TEST_START;
		std::vector<std::vector<double>> csvData;

		double wn = 30;
		double zeta = 0.2;

		TransferFunction tf({ wn * wn }, { 1, 2 * zeta * wn, wn * wn });
		StateSpaceModel model = tf.toStateSpaceModel(0.01);
		Matrix u({ {1} });

		TEST_MESSAGE("State-Space Model:\n" + model.toString() + "\n");
		
		for (int i = 0; i < 200; i++)
		{
			if (i == 100)
			{
				u(0, 0) = 0.5;
			}
			model.processTimeStep(u);

			// Log data
			{
				TEST_MESSAGE("Step " + std::to_string(i) + ": y = \n" + model.getOutput().toString());
				std::vector<double> row;
				row.push_back(i * model.getTimeStep());

				for (size_t i = 0; i < u.getRows(); ++i)
					row.push_back(u(i, 0));

				const Matrix& y = model.getOutput();
				for (size_t i = 0; i < y.getRows(); ++i)
					row.push_back(y(i, 0));

				csvData.push_back(row);
			}
		}

		// Save to CSV
		std::ofstream file("step_response.csv");
		file << "Time;";
		for (size_t i = 0; i < u.getRows(); ++i)
			file << "u[" << (i)+"];";
		for (size_t i = 0; i < model.getOutput().getRows(); ++i)
			file << "y[" << (i)+"];";
		file << "\n";
		for (const auto& row : csvData)
		{
			for (size_t i = 0; i < row.size(); ++i)
			{
				file << row[i];
				if (i < row.size() - 1)
					file << ";";
			}
			file << "\n";
		}
		file.close();


	}

	TEST_FUNCTION(MIMOstepResp)
	{
		TEST_START;
		std::vector<std::vector<double>> csvData;

		TransferFunction tf_11({ 1 }, { 1, 1 });
		TransferFunction tf_12({ 2 }, { 1, 3 });
		TransferFunction tf_21({ 1, 2 }, { 1, 2, 5 });
		TransferFunction tf_22({ 3 }, { 1, 4 });

		MIMOSystem mimoSys(
			{ 
				{tf_11, tf_12}, 
			    {tf_21, tf_22} 
			});

		StateSpaceModel model = mimoSys.toStateSpaceModel(0.01);
		Matrix u(
			{ {1},
			  {1} });

		std::string modelStr = model.toString();
		TEST_MESSAGE("State-Space Model:\n" + modelStr + "\n");

		for (int i = 0; i < 200; i++)
		{
			if (i == 100)
			{
				u(0, 0) = 0.5;
			}
			model.processTimeStep(u);

			// Log data
			{
				TEST_MESSAGE("Step " + std::to_string(i) + ": y = \n" + model.getOutput().toString());
				std::vector<double> row;
				row.push_back(i * model.getTimeStep());

				for (size_t i = 0; i < u.getRows(); ++i)
					row.push_back(u(i, 0));

				const Matrix& y = model.getOutput();
				for (size_t i = 0; i < y.getRows(); ++i)
					row.push_back(y(i, 0));

				csvData.push_back(row);
			}
		}

		// Save to CSV
		std::ofstream file("MIMO_step_response.csv");
		file << "Time;";
		for (size_t i = 0; i < u.getRows(); ++i)
			file << "u[" << (i)+"];";
		for (size_t i = 0; i < model.getOutput().getRows(); ++i)
			file << "y[" << (i)+"];";
		file << "\n";
		for (const auto& row : csvData)
		{
			for (size_t i = 0; i < row.size(); ++i)
			{
				file << row[i];
				if (i < row.size() - 1)
					file << ";";
			}
			file << "\n";
		}
		file.close();


	}




};

TEST_INSTANTIATE(TST_StateSpaceModel);