#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <chrono>
#include "GenerateData.h"
#include "immintrin.h"
#define N_PROBES 100000

union DATA{ __m256 a8; float a[8]; };

class WMA
{
    std::vector<std::vector<float>> data;
    std::vector<std::vector<float>> result;

    inline bool IsInRange(uint32_t index)
    {
        return index > 0;
    }

    void SISDProcess()
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        for (uint32_t i = 0; i < DATA_STREAMS; ++i)
        {
            for (uint32_t j = 0; j < data[i].size(); ++j)
            {
                float valueSum = 0.0f;
                uint32_t weights = 0;
                for (uint32_t k = 0; k < N_PROBES; ++k)
                {
                    if (!IsInRange(j - k))
                    {
                        break;
                    }
                    valueSum += data[i][j - k] * (N_PROBES - k);
                    weights += N_PROBES - k;
                }
                result[i].push_back(valueSum / weights);
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << "SISD processing duration: " << duration << "ms" << std::endl;
        // DumpResults(false);
    }

    void SIMDProcess()
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        for (uint32_t i = 0; i < DATA_STREAMS; i += 8)
        {
            for (uint32_t j = 0; j < data[i].size(); ++j)
            {
                DATA valueSum;
                valueSum.a8 = _mm256_set_ps(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                DATA weights;
                weights.a8 = _mm256_set_ps(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                for (uint32_t k = 0; k < N_PROBES; ++k)
                {
                    if (!IsInRange(j - k))
                    {
                        break;
                    }
                    DATA nProbesMinusK;
                    nProbesMinusK.a8 = _mm256_set_ps(N_PROBES - k, N_PROBES - k, N_PROBES - k, N_PROBES - k, N_PROBES - k, N_PROBES - k, N_PROBES - k, N_PROBES - k);
                    weights.a8 = _mm256_add_ps(weights.a8, nProbesMinusK.a8);
                    valueSum.a8 = _mm256_add_ps(
                        valueSum.a8,
                        _mm256_mul_ps(
                            nProbesMinusK.a8,
                            _mm256_set_ps(data[i][j - k], data[i + 1][j - k], data[i + 2][j - k], data[i + 3][j - k], data[i + 4][j - k], data[i + 5][j - k], data[i + 6][j - k], data[i + 7][j - k])
                        )
                    );
                }
                valueSum.a8 = _mm256_div_ps(valueSum.a8, weights.a8);
                result[i].push_back(valueSum.a[0]);
                result[i + 1].push_back(valueSum.a[1]);
                result[i + 2].push_back(valueSum.a[2]);
                result[i + 3].push_back(valueSum.a[3]);
                result[i + 4].push_back(valueSum.a[4]);
                result[i + 5].push_back(valueSum.a[5]);
                result[i + 6].push_back(valueSum.a[6]);
                result[i + 7].push_back(valueSum.a[7]);
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << "SIMD processing duration: " << duration << "ms" << std::endl;
        // DumpResults(true);
    }

    void DumpResults(bool isSIMD)
    {
        std::string fileName = (isSIMD) ? "result_SIMD.txt" : "result.txt";
        std::ofstream fout(fileName);
        for (uint32_t i = 0; i < data[0].size(); ++i)
        {
            Print(fout,
                data[0][i],
                data[1][i],
                data[2][i],
                data[3][i],
                data[4][i],
                data[5][i],
                data[6][i],
                data[7][i],
                data[8][i],
                data[9][i],
                data[10][i],
                data[11][i]);
        }
    }
public:
    void Start(int toProcess)
    {
        for (int i = 0; i < DATA_STREAMS; ++i)
        {
            std::vector<float> tempData;
            data.push_back(tempData);
            std::vector<float> tempResult;
            result.push_back(tempResult);
        }

        std::ifstream fin("data" + std::to_string(toProcess) + ".txt");
        while (!fin.eof())
        {
            for (uint32_t i = 0; i < DATA_STREAMS; ++i)
            {
                float temp;
                fin >> temp;
                data[i].push_back(temp);
            }
        }
        //SISDProcess();

        /*for (int i = 0; i < DATA_STREAMS; ++i)
        {
            result[i].clear();
        }*/

        SIMDProcess();
    }
};
