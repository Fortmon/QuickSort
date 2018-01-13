#include "stdafx.h"
#include "QuickSort.h"

#define root 0
#define tag_size 1
#define epsilon 0.000001

QuickSort::QuickSort(int argc, char * argv[]){
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);

	MPI_Barrier(MPI_COMM_WORLD);

	if (procRank == root) {
		loadElements(argv[1]);
	}

	MPI_Bcast(&size, 1, MPI_INT, root, MPI_COMM_WORLD); 

	if (procRank != root) {
		elements = new double[size];
	}
}

void QuickSort::save(const std::string fileName){
	if (procRank == root){
		fstream outputFile;
		outputFile.open(fileName, fstream::out);

		outputFile << size << endl;
		for (int i = 0; i < size; i++) {
			outputFile << elements[i] << endl;
		}

		outputFile.close();
	}
}

void QuickSort::loadElements(const std::string fileName) {
	try{
		fstream inputFile;
		inputFile.open(fileName);

		if (!inputFile.is_open()) {
			throw exception("Can't open file with elements");
		}

		inputFile >> size;
		this->elements = new double[size];
		for (int i = 0; i < size; i++) {
			inputFile >> elements[i];
		}

		inputFile.close();
	}
	catch (const std::runtime_error re) {
		cout << "Runtime error: " << re.what() << endl;

	}
	catch (exception ex) {
		cout << "Exception: " << ex.what() << endl;
	}

	catch (...) {
		cout << ("Unknown exception");
	}
}

int QuickSort::compare(const void* x1, const void* x2) {
	if (fabs((*(double*)x1 - *(double*)x2)) < epsilon) {
		return 0;
	}
	else if ((*(double*)x1 - *(double*)x2) < 0){
		return -1;
	}
	return 1;
}

void QuickSort::sort() {
	MPI_Status status;

	distributeElements();

	double pivot; // Опорный элемент
	
	int dimension = (int)ceil(log(procNum) / log(2)); // определяем размерность гиперкуба
	int mask = procNum; // битовая маска
	int commProcRank; // Ранг процессора, с которым взаимодействуем

	double *recvData; // Данные, которые получаются с другого процессора
	double *mergeData; // Данные, которые остались+получили

	std::vector<double> data; // Данные, которые остаются после отправки
	std::vector<double> sendData; // Данные, которые отправляются другому процессору


	int sendDataSize, recvDataSize, mergeDataSize;

	//Перемещение данных между процессорами.
	for (int i = dimension; i > 0; i--) {
		pivot = pivotDistribution(procElements, procSize,dimension, mask, i); // определение опорного элемента и рассылка его всем процессорам
		mask = mask >> 1;

		if (((procRank & mask) >> (i - 1)) == 0) { // старший бит = 0
			commProcRank = procRank + mask;
			for (int i = 0; i < procSize; i++) {
				if (procElements[i] < pivot) {
					data.push_back(procElements[i]);
				}
				else {
					sendData.push_back(procElements[i]);
				}
			}
		}
		else { // старший бит = 1
			commProcRank = procRank - mask;
			for (int i = 0; i < procSize; i++) {
				if (procElements[i] < pivot) {
					sendData.push_back(procElements[i]);
				}
				else {
					data.push_back(procElements[i]);
				}
			}
		}

		sendDataSize = sendData.size();

		MPI_Sendrecv(&sendDataSize, 1, MPI_INT, commProcRank, 0, &recvDataSize, 1, MPI_INT, commProcRank, 0, MPI_COMM_WORLD, &status); // Обмениваемся размерами данных.

		recvData = new double[recvDataSize];

		MPI_Sendrecv(sendData.data(), sendDataSize, MPI_DOUBLE, commProcRank, 0,
					 recvData,		  recvDataSize, MPI_DOUBLE, commProcRank, 0, MPI_COMM_WORLD, &status); // Обмениваемся элементами

		mergeDataSize = data.size() + recvDataSize;
		mergeData = new double[mergeDataSize];

		dataMerge(mergeData, mergeDataSize, data.data(), data.size(), recvData, recvDataSize); // Собираем в один массив оставшиеся и полученные данные. 


		delete[] procElements;
		delete[] recvData;
		data.clear();
		sendData.clear();

		procElements = mergeData;
		procSize = mergeDataSize;
	}

	
	qsort(procElements, procSize, sizeof(double), compare); // сортируем оставшиеся данные
	
	int* procSizes = new int[procNum];

	MPI_Gather(&procSize, 1, MPI_INT, procSizes, 1, MPI_INT, root, MPI_COMM_WORLD);

	int* procOffsets = new int[procNum];

	if (procRank == root) {
		procOffsets[0] = 0;
		for (int i = 1; i < procNum; i++) {
			procOffsets[i] = procOffsets[i - 1] + procSizes[i - 1];
		}
	}

	MPI_Gatherv(procElements, procSize, MPI_DOUBLE, elements, procSizes, procOffsets, MPI_DOUBLE, root, MPI_COMM_WORLD);

	delete[] procSizes;
	delete[] procOffsets;
}

void QuickSort::print(double value) {
	if (procRank == root) {
		cout << value;
	}
}

void QuickSort::distributeElements() {
	int* procSizes = new int[procNum];
	int* offsets = new int[procNum];

	if (procRank == root) {
		procSizes[0] = size / procNum;
		offsets[0] = 0;

		for (int i = 1; i < procNum; i++) {
			procSizes[i] = size / procNum;
			offsets[i] = procSizes[i - 1] + offsets[i - 1];
		}
		procSizes[procNum-1] += size % procNum;
	}

	MPI_Scatter(procSizes, 1, MPI_INT, &procSize, 1, MPI_INT, root, MPI_COMM_WORLD); // Распределим данные.

	procElements = new double[procSize];

	MPI_Scatterv(elements, procSizes, offsets, MPI_DOUBLE, procElements, procSize, MPI_DOUBLE, root, MPI_COMM_WORLD);

	delete[] procSizes;
	delete[] offsets;
	
}

double QuickSort::pivotDistribution(double* procElements, int procSize, int dim, int mask, int iter) {
	MPI_Group worldGroup;
	MPI_Group subcubeGroup; // Группа процессов — подгиперкуб
	MPI_Comm  subcubeComm;  // Коммуникатор подгиперкуба
	int j = 0;

	int  groupNum = procNum / (int)pow(2, dim - iter);
	int *procRanks = new int[groupNum];

	// формирование списка рангов процессов для гиперкуба
	int startProc = procRank-groupNum;
	if (startProc < 0) startProc = 0;
	int endProc = procRank + groupNum;
	if (endProc > procNum) endProc = procNum;
	for (int proc = startProc; proc < endProc; proc++) {
		if ((procRank & mask) >> (iter) == (proc & mask) >> (iter)) {
			procRanks[j++] = proc;
		}
	}
	// Объединение процессов подгиперкуба в одну группу  
	MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
	MPI_Group_incl(worldGroup, groupNum, procRanks, &subcubeGroup);
	MPI_Comm_create(MPI_COMM_WORLD, subcubeGroup, &subcubeComm);

	double pivot = 0;
	if (procRank == procRanks[0]) {
		for (int i = 0; i < procSize; i++) {
			pivot += procElements[i];
		}
		pivot /= procSize;
	}

	//Рассылка всем значения опорного элемента
	MPI_Bcast(&pivot, 1, MPI_DOUBLE, root, subcubeComm);

	MPI_Group_free(&subcubeGroup);
	MPI_Comm_free(&subcubeComm);
	delete[] procRanks;

	return pivot;
}

void QuickSort::dataMerge(double* mergeData, int mergeDataSize, double* data, int dataSize, double* recvData, int recvDataSize) {
	int indexData = 0, indexRecvData = 0;

	for (int i = 0; i < dataSize; i++) {
		mergeData[i] = data[i];
	}

	indexData = dataSize;

	for (int i = 0; i < recvDataSize; i++) {
		mergeData[indexData++] = recvData[i];
	}
}
QuickSort::~QuickSort() {
	delete[] elements;
}
