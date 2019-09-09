#define DATA_STREAMS 200
#include "GenerateData.h"
#include "WMA.h"

int main()
{
    GenerateData();
    WMA wma;
    wma.Start();
    system("pause");
}
