#define DATA_STREAMS 8 
#include <mpi.h>
#include "GenerateData.h"
#include "WMA.h"

int main()
{
    // GenerateData();
    MPI_Init(NULL, NULL);
    
    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    
    int worldRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    
    char processorName[MPI_MAX_PROCESSOR_NAME];
    int nameLen;
    MPI_Get_processor_name(processorName, &nameLen);

    std::cout << processorName << " number " << worldRank << " out of " << worldSize << std::endl;
    
    if (worldRank == 0)
    {
        // dispatch others
	int toProcess, started = 0, done = 0, allStreams = 16;
	for (int i = 1; i < worldSize; ++i)
	{
	    toProcess = started;
	    MPI_Send(&toProcess, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	    ++started;
	}
	MPI_Status status;
	while (done < allStreams)
	{
	    MPI_Recv(&toProcess, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
	    std::cout << status.MPI_SOURCE << " completed processing: " << toProcess << std::endl;
	    ++done;
	    if (started < allStreams)
	    {
		toProcess = started;
		MPI_Send(&toProcess, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
		++started;
	    }
	    else
	    {
		toProcess = -1;
	        MPI_Send(&toProcess, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
	    }
	    if (done == allStreams)
	    {
		break;
	    }
	}
    }
    else
    {
	int toProcess = 0;
	while (true)
	{
	    MPI_Recv(&toProcess, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    if (toProcess == -1) break;
	    std::cout << processorName << " number " << worldRank << " processing: " << toProcess << std::endl;
	    WMA wma;
	    wma.Start(toProcess);
	    MPI_Send(&toProcess, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
	}
    }

    MPI_Finalize();
}
