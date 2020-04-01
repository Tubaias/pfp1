#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <ctime>

typedef uint64_t u64;
typedef uint8_t u8;

using namespace std;

vector<u64> decompress(string filename) {
    ifstream in(filename, ios::binary | ios::ate);

    if (!in.is_open()) {
        cout << "file " << filename << " could not be opened.\n";
        vector<u64> asd;
        return asd;
    }

    streampos size = in.tellg();
    u8* memblock = new u8[size];
    in.seekg(0, ios::beg);
    in.read(reinterpret_cast<char*> (memblock), size);
    in.close();

    vector<u64> decompressed;

    u64 num = 0;
    int numbytes = 0;
    for (int i = 0; i < size; i++) {
        u8 byte = memblock[i];

        if (byte > 127) {
            num += (byte - 128) << (numbytes * 7);
            decompressed.push_back(num);
            num = 0;
            numbytes = 0;
        }
        else {
            num += byte << (numbytes * 7);
            numbytes++;
        }
    }

    delete[] memblock;
    return decompressed;
}

int proximityIntersection(int lower, int upper, int file1, int file2) {
    vector<u64> numA = decompress("F" + to_string(file1) + ".vb");
    vector<u64> numB = decompress("F" + to_string(file2) + ".vb");
    vector<u64> intersection;

    sort(numA.begin(), numA.end());
    sort(numB.begin(), numB.end());

    int iA = 0;
    int iB = 0;
    u64 b, a;

    while (iA < numA.size() && iB < numB.size()) {
        a = numA[iA];
        b = numB[iB];

        if (b <= a + upper && b >= a - lower) {
            intersection.push_back(b);
            iB++;
        }
        else if (b > a + upper) {
            iA++;
        }
        else if (b < a - lower) {
            iB++;
        }
    }

    return intersection.size();
}

int main(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        cout << argv[i] << " ";
    }

    if (argc < 4) {
        cout << "\ninsufficient parameters\n";
        return 1;
    }

    int lower = atoi(argv[1]);
    int upper = atoi(argv[2]);
    string filename = argv[3];

    ifstream pairs(filename);
    if (!pairs.is_open()) {
        cout << "File " << filename << " could not be opened.";
        return 1;
    } else {
        cout << "File " << filename << " opened. Calculating intersections.\n";
    }

    vector<int> filenums;

    int a, b;
    while (pairs >> a >> b) {
        filenums.push_back(a);
        filenums.push_back(b);
    }

    vector<int> sizes;

    auto begin = chrono::system_clock::now();
    std::clock_t cpubegin = std::clock();

    for (int i = 0; i < filenums.size(); i += 2) {
        sizes.push_back(proximityIntersection(lower, upper, filenums[i], filenums[i + 1]));
    }

    double cputime = (std::clock() - cpubegin) / (double)CLOCKS_PER_SEC;
    chrono::duration<double> walltime = chrono::system_clock::now() - begin;

    cout << "intersection sizes:\n";
    for (int i = 0; i < sizes.size(); i++) {
        cout << sizes[i] << "\n";
    }

    cout << "wall clock time taken: " << walltime.count() << "s\n";
    cout << "cpu time taken: " << cputime << "s\n";
    return 0;
}
