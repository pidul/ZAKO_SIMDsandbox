#pragma once
#include <iostream>
#include <fstream>
#include <random>

template<typename T>
void Print(std::ofstream &fout, T first)
{
    fout << first << std::endl;
}

template<typename ... Args, typename T>
void Print(std::ofstream &fout, T first, Args... args)
{
    fout << first << " ";
    Print(fout, args...);
}

void GenerateData()
{
    for (int j = 0; j < DATA_PACKS; ++j)
    {
	std::ofstream fout(std::string("data") + std::to_string(j) + ".txt");
	std::random_device rd;
	for (int i = 0; i < DATA_SIZE; ++i)
	{
		Print(fout,
		    (float)(rd()) / rd.max(),
		    (float)(rd()) / rd.max(),
		    (float)(rd()) / rd.max(),
		    (float)(rd()) / rd.max(),
		    (float)(rd()) / rd.max(),
		    (float)(rd()) / rd.max(),
		    (float)(rd()) / rd.max(),
		    (float)(rd()) / rd.max());
	}
    	fout.close();
    }
}
