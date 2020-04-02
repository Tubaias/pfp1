#include <iostream>
#include <fstream>
#include <bitset>
#include <algorithm>
#include <vector>
#include <chrono>
#include <string>

typedef uint64_t u64;
typedef uint8_t u8;

using namespace std;

// compress a binary file into a vbyte file
int compress(string filename) {
    // read the file
    ifstream in(filename, ios::binary | ios::ate);

    if (!in.is_open()) {
        cout << "file " << filename << " could not be opened.\n";
        return 1;
    }

    const streampos size = in.tellg();
    u64* memblock = new u64[size / 8];
    in.seekg(0, ios::beg);
    in.read(reinterpret_cast<char*> (memblock), size);
    in.close();

    // compress the integers with vbyte
    char* compressed = new char[size];
    int bytes = 0;

    for (int i = 0; i < size / 8; i++) {
        u64 num = memblock[i];

        while (true) {
            // same as u8 byte = num % 128;
            u8 byte = num & 0x7F;

            if (num < 128) {
                compressed[bytes] = byte + 128;
                bytes++;
                break;
            }

            compressed[bytes] = byte;
            bytes++;
            // same as num /= 128;
            num = num >> 7;
        }
    }

    // write the compressed vbyte codes into a binary file
    ofstream out((filename + ".vb"), ios::binary);
    out.write(compressed, bytes);
    out.close();

    delete[] memblock;
    delete[] compressed;
    return 0;
}

// sort a binary file and compress the differences between adjacent elements into a vbyte file
int sortCompress(string filename) {
    // read the file
    ifstream in(filename, ios::binary | ios::ate);

    if (!in.is_open()) {
        cout << "file " << filename << " could not be opened.\n";
        return 1;
    }

    const streampos size = in.tellg();
    u64* memblock = new u64[size / 8];
    in.seekg(0, ios::beg);
    in.read(reinterpret_cast<char*> (memblock), size);
    in.close();

    // sort the integers
    sort(memblock, memblock + size / 8);
    u64* diff = new u64[size / 8];
    u64 prev = 0;

    // calculate the differences between adjacent integers
    for (int i = 0; i < size / 8; i++) {
        u64 num = memblock[i];
        diff[i] = num - prev;
        prev = num;
    }

    // compress the differences with vbyte
    char* compressed = new char[size];
    int bytes = 0;

    for (int i = 0; i < size / 8; i++) {
        u64 num = diff[i];

        while (true) {
            u8 byte = num & 0x7F;

            if (num < 128) {
                compressed[bytes] = byte + 128;
                bytes++;
                break;
            }

            compressed[bytes] = byte;
            bytes++;
            num = num >> 7;
        }
    }

    // write the vbyte codes into a binary file
    ofstream out((filename + ".sorted.vb"), ios::binary);
    out.write(compressed, bytes);
    out.close();

    delete[] memblock;
    delete[] compressed;
    delete[] diff;
    return 0;
}

// decompress a vbyte file into the corresponding original binary file
int decompress(string filename) {
    // read the file
    ifstream in(filename, ios::binary | ios::ate);

    if (!in.is_open()) {
        cout << "file " << filename << " could not be opened.\n";
        return 1;
    }

    streampos size = in.tellg();
    u8* memblock = new u8[size];
    in.seekg(0, ios::beg);
    in.read(reinterpret_cast<char*> (memblock), size);
    in.close();

    // decompress vbyte codes into a vector
    vector<u64> decompressed;

    u64 num = 0;
    for (int i = 0; i < size; i++) {
        u8 byte = memblock[i];

        if (byte > 127) {
            num = (num << 7) + byte - 128;
            decompressed.push_back(num);
            num = 0;
        }
        else {
            num = (num << 7) + byte;
        }
    }

    // write the decompressed integers into a binary file
    ofstream out((filename + ".dec"), ios::binary);
    out.write(reinterpret_cast<char*> (decompressed[0]), decompressed.size() * 8);
    out.close();

    delete[] memblock;
    return 0;
}

// Main function. Takes as command line arguments: 
// 1. The name of the file to process.
// 2. The processing mode, either 'c' for compress, 'sc' for sort compress or 'dc' for decompress.
// 3. The number of times the process should be repeated (on the same file), 1 by default.
int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "\ninsufficient arguments.\n";
        return 1;
    }

    string filename = argv[1];
    string mode = argv[2];
    int reps = (argc >= 4) ? atoi(argv[3]) : 1;

    if (mode == "c") {
        auto begin = chrono::system_clock::now();
        for (int i = 0; i < reps; i++) {
            compress(filename);
        }
        chrono::duration<double> time = chrono::system_clock::now() - begin;
        cout << "\ncompression took: " << time.count() << "s\n";
    } else if (mode == "sc") {
        auto begin = chrono::system_clock::now();
        for (int i = 0; i < reps; i++) {
            sortCompress(filename);
        }
        chrono::duration<double> time = chrono::system_clock::now() - begin;
        cout << "\nsort compression took: " << time.count() << "s\n";
    } else if (mode == "dc") {
        auto begin = chrono::system_clock::now();
        for (int i = 0; i < reps; i++) {
            decompress(filename);
        }
        chrono::duration<double> time = chrono::system_clock::now() - begin;
        cout << "\ndecompression took: " << time.count() << "s\n";
    }

    return 0;
}
