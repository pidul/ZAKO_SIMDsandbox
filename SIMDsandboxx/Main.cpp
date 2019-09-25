#define DATA_STREAMS 8
#define DATA_SIZE 1000000
#define DATA_PACKS 16
#define N_PROBES 10000
#include <mpi.h>
#include "GenerateData.h"
#include "WMA.h"

void DispatchData(int toProcess, int process) {
    std::ifstream fin("data" + std::to_string(toProcess) + ".txt");
    std::vector<float> data;
    while (!fin.eof())
    {
        float temp;
        fin >> temp;
        data.push_back(temp);
    }
    MPI_Send(data.data(), DATA_STREAMS * DATA_SIZE, MPI_FLOAT, process, 0, MPI_COMM_WORLD);
}

int main()
{
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
        GenerateData();
        // dispatch others
        int returnCode, started = 0, done = 0, allStreams = DATA_PACKS;
        float toProcess;
        for (int i = 1; i < worldSize; ++i)
        {
            DispatchData(started, i);
            ++started;
        }
        MPI_Status status;
        while (done < allStreams)
        {
            MPI_Recv(&returnCode, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            std::cout << status.MPI_SOURCE << " completed processing" << std::endl;
            ++done;
            if (started < allStreams)
            {
                DispatchData(started, status.MPI_SOURCE);
                ++started;
            }
            else
            {
                toProcess = -1.0f;
                MPI_Send(&toProcess, 1, MPI_FLOAT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
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
        std::vector<float> data(DATA_SIZE * DATA_STREAMS);
        while (true)
        {
            MPI_Recv(data.data(), DATA_SIZE * DATA_STREAMS, MPI_FLOAT,
                    0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (data[0] == -1.0f) break;
            WMA wma;
            wma.Start(data.data());
            MPI_Send(&toProcess, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}
