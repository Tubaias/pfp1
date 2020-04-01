#include <iostream>
#include <fstream>
#include <bitset>
#include <algorithm>
#include <vector>
#include <chrono>
#include <string>

using namespace std;

int compress(string filename) {
    ifstream in(filename, ios::binary | ios::ate);

    if (in.is_open()) {
        cout << "file " << filename << " was opened successfully.\n";
    } else {
        cout << "file " << filename << " could not be opened.\n";
        return 1;
    }

    const streampos size = in.tellg();
    uint64_t* memblock = new uint64_t[size / 8];
    in.seekg(0, ios::beg);
    in.read(reinterpret_cast<char*> (memblock), size);
    in.close();

    char* compressed = new char[size];
    int bytes = 0;

    for (int i = 0; i < size / 8; i++) {
        uint64_t num = memblock[i];

        while (true) {
            uint8_t byte = num % 128;

            if (num < 128) {
                compressed[bytes] = byte + 128;
                bytes++;
                break;
            }

            compressed[bytes] = byte;
            bytes++;
            num /= 128;
        }
    }

    ofstream out((filename + ".vb"), ios::binary);
    out.write(compressed, bytes);
    out.close();

    delete[] memblock;
    delete[] compressed;
    return 0;
}

int compress2(string filename) {
    ifstream in(filename, ios::binary | ios::ate);

    if (in.is_open()) {
        cout << "file " << filename << " was opened successfully.\n";
    } else {
        cout << "file " << filename << " could not be opened.\n";
        return 1;
    }

    const streampos size = in.tellg();
    uint64_t* memblock = new uint64_t[size / 8];
    in.seekg(0, ios::beg);
    in.read(reinterpret_cast<char*> (memblock), size);
    in.close();

    char* compressed = new char[size];
    int bytes = 0;

    for (int i = 0; i < size / 8; i++) {
        uint64_t num = memblock[i];

        while (true) {
            uint8_t byte = num & 0x7F;

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

    ofstream out((filename + ".vb"), ios::binary);
    out.write(compressed, bytes);
    out.close();

    delete[] memblock;
    delete[] compressed;
    return 0;
}

int sortCompress(string filename) {
    ifstream in(filename, ios::binary | ios::ate);

    if (in.is_open()) {
        cout << "file " << filename << " was opened successfully.\n";
    } else {
        cout << "file " << filename << " could not be opened.\n";
        return 1;
    }

    const streampos size = in.tellg();
    uint64_t* memblock = new uint64_t[size / 8];
    in.seekg(0, ios::beg);
    in.read(reinterpret_cast<char*> (memblock), size);
    in.close();

    sort(memblock, memblock + size / 8);
    uint64_t* diff = new uint64_t[size / 8];
    uint64_t prev = 0;

    for (int i = 0; i < size / 8; i++) {
        uint64_t num = memblock[i];
        diff[i] = num - prev;
        prev = num;
    }

    char* compressed = new char[size];
    int bytes = 0;

    for (int i = 0; i < size / 8; i++) {
        uint64_t num = diff[i];

        while (true) {
            uint8_t byte = num & 0x7F;

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

    ofstream out((filename + ".sorted.vb"), ios::binary);
    out.write(compressed, bytes);
    out.close();

    delete[] memblock;
    delete[] compressed;
    delete[] diff;
    return 0;
}

int decompress(string filename) {
    ifstream in(filename, ios::binary | ios::ate);

    if (!in.is_open()) {
        cout << "file " << filename << " could not be opened.\n";
        return 1;
    }

    streampos size = in.tellg();
    uint8_t* memblock = new uint8_t[size];
    in.seekg(0, ios::beg);
    in.read(reinterpret_cast<char*> (memblock), size);
    in.close();

    vector<uint64_t> decompressed;

    uint64_t num = 0;
    int numbytes = 0;
    for (int i = 0; i < size; i++) {
        uint8_t byte = memblock[i];

        if (byte > 127) {
            num += (byte - 128) << (numbytes * 7);
            decompressed.push_back(num);
            num = 0;
            numbytes = 0;
        } else {
            num += byte << (numbytes * 7);
            numbytes++;
        }
    }

    ofstream out((filename + ".dec"), ios::binary);
    out.write(reinterpret_cast<char*> (&decompressed[0]), decompressed.size() * 8);
    out.close();

    delete[] memblock;
    return 0;
}

int decompress2(string filename) {
    ifstream in(filename, ios::binary | ios::ate);

    if (!in.is_open()) {
        cout << "file " << filename << " could not be opened.\n";
        return 1;
    }

    streampos size = in.tellg();
    uint8_t* memblock = new uint8_t[size];
    in.seekg(0, ios::beg);
    in.read(reinterpret_cast<char*> (memblock), size);
    in.close();

    vector<uint64_t> decompressed;

    uint64_t num = 0;
    for (int i = 0; i < size; i++) {
        uint8_t byte = memblock[i];

        if (byte > 127) {
            num = (num << 7) + byte - 128;
            decompressed.push_back(num);
            num = 0;
        }
        else {
            num = (num << 7) + byte;
        }
    }

    ofstream out((filename + ".dec"), ios::binary);
    out.write(reinterpret_cast<char*> (decompressed[0]), decompressed.size() * 8);
    out.close();

    delete[] memblock;
    return 0;
}

int main(int argc, char** argv) {
    string filename = argv[1];
    
    auto c2begin = chrono::system_clock::now();
    compress2(filename);
    auto c2end = chrono::system_clock::now();
    
    auto scbegin = chrono::system_clock::now();
    sortCompress(filename);
    auto scend = chrono::system_clock::now();

    auto dcbegin = chrono::system_clock::now();
    decompress2(filename + ".vb");
    auto dcend = chrono::system_clock::now();

    chrono::duration<double> c2time = c2end - c2begin;
    chrono::duration<double> dctime = dcend - dcbegin;

    cout << "compression 2 took: " << c2time.count() << "s\n";
    cout << "sort compression took: " << sctime.count() << "s\n";
    cout << "decompression took: " << dctime.count() << "s\n";
}
