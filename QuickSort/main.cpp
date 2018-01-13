#include "stdafx.h"
#include "QuickSort.h"

int main(int argc, char * argv[])
{
	try {
		double dtStart, dtEnd, dtDelta;
		MPI_Init(&argc, &argv);

		QuickSort quickSort = QuickSort(argc, argv);

		dtStart = MPI_Wtime();

		quickSort.sort();

		dtEnd = MPI_Wtime();
		dtDelta = dtEnd - dtStart;
		quickSort.print(dtDelta);

		quickSort.save(argv[2]);		
		MPI_Finalize();
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
	//system("pause");
	return 0;
}
