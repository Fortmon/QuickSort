#pragma once

class QuickSort
{
private:
	double* elements;
	double* procElements;
	int size;
	int procNum;
	int procRank;
	int procSize;


	static int compare(const void* x1, const void* x2);
	double pivotDistribution(double* procElements, int procSize, int dim, int mask, int iter);
	void dataMerge(double* mergeData, int mergeDataSize, double* data, int dataSize, double* recvData, int recvDataSize);
	void distributeElements();

public:
	void loadElements(const std::string fileName);
	void save(const std::string fileName);
	void sort();
	void print(double value);
	QuickSort(int argc, char* argv[]);
	~QuickSort();
};
