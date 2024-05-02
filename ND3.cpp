#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <deque>
#include <algorithm>
#include <mutex>
using namespace std;

class PrimeFinder {
private:
    deque<string> fileDeque;
    HANDLE hSemaphore;
    int minPrime;
    int maxPrime;
    int numProcessedFiles;
    mutex mtx;

public:
    int numConsumerThreads;

    PrimeFinder() : minPrime(INT_MAX), maxPrime(INT_MIN), numProcessedFiles(0), numConsumerThreads(1) {
        hSemaphore = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    }

    ~PrimeFinder() {
        CloseHandle(hSemaphore);
    }

    void pushFile(const string& filename) {
        lock_guard<mutex> lock(mtx);
        fileDeque.push_back(filename);
        ReleaseSemaphore(hSemaphore, 1, NULL);
    }

    string popFile() {
        WaitForSingleObject(hSemaphore, INFINITE);
        lock_guard<mutex> lock(mtx);
        string filename = fileDeque.front();
        fileDeque.pop_front();
        return filename;
    }

    bool isPrimeSieve(int n) {
        if (n <= 1) return false;
        if (n <= 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;
        for (int i = 5; i * i <= n; i += 6)
            if (n % i == 0 || n % (i + 2) == 0)
                return false;
        return true;
    }

    void findPrimesInFile(const string& filename, ofstream& outputFile) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Failed to open input file!: " << filename << endl;
            return;
        }

        int num;
        while (file >> num) {
            if (isPrimeSieve(num)) {
                outputFile << num << endl;
                minPrime = min(minPrime, num);
                maxPrime = max(maxPrime, num);
            }
        }
        file.close();
    }

    void processFiles() {
        while (true) {
            string filename = popFile();
            if (filename.empty()) break;

            ofstream outputFile("all_primes.txt", ios_base::app);
            if (!outputFile.is_open()) {
                cerr << "Failed to create output file: all_primes.txt" << endl;
                return;
            }

            findPrimesInFile(filename, outputFile);
            numProcessedFiles++;

            outputFile.close();
        }
    }

    int getProcessedFileCount() {
        return numProcessedFiles;
    }

    void increaseConsumerThreads() {
        if (numConsumerThreads < 6) {
            numConsumerThreads++;
        } else {
            cout << "Cannot create more than 6 consumer threads." << endl;
        }
    }

    void decreaseConsumerThreads() {
        if (numConsumerThreads > 1) {
            numConsumerThreads--;
        } else {
            cout << "Cannot have less than 1 consumer thread." << endl;
        }
    }

    void printPrimesInfo() {
        if (minPrime != INT_MAX && maxPrime != INT_MIN) {
            cout << "Minimum prime number found: " << minPrime << endl;
            cout << "Maximum prime number found: " << maxPrime << endl;
        } else {
            cout << "No prime numbers found." << endl;
        }
        cout << "Total number of files processed: " << getProcessedFileCount() << endl;
        cout << "Number of consumer threads: " << numConsumerThreads << endl;
    }
};

PrimeFinder primeFinder;

DWORD WINAPI producerThread(LPVOID lpParam) {
    string directory = "rand_files";
    WIN32_FIND_DATAA fileData;
    HANDLE hFind = FindFirstFileA((directory + "\\file*.txt").c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        vector<string> files;
        do {
            files.push_back(directory + "\\" + fileData.cFileName);
        } while (FindNextFileA(hFind, &fileData) != 0);
        FindClose(hFind);

        sort(files.begin(), files.end());

        for (const auto& filename : files) {
            primeFinder.pushFile(filename);
        }
    }
    return 0;
}

DWORD WINAPI consumerThread(LPVOID lpParam) {
    primeFinder.processFiles();
    return 0;
}

int main() {
    HANDLE hProducer = CreateThread(NULL, 0, producerThread, NULL, 0, NULL);
    if (hProducer == NULL) {
        cerr << "Cant create producer thread.." << endl;
        return 1;
    }

    const int NUM_CONSUMER_THREADS = 1;
    vector<HANDLE> hConsumers(NUM_CONSUMER_THREADS);
    for (int i = 0; i < NUM_CONSUMER_THREADS; ++i) {
        hConsumers[i] = CreateThread(NULL, 0, consumerThread, NULL, 0, NULL);
        if (hConsumers[i] == NULL) {
            cerr << "Cant create consumer thread." << endl;
            return 1;
        }
    }

    char choice;
    while (true) {
        cout << "Enter '+' to increase the number of consumer threads, '-' to decrease, 'c' to cancel: ";
        cout << "Current number of consumer threads: " << primeFinder.numConsumerThreads << endl;
        cin >> choice;
        cin.ignore();

        if (choice == '+') {
            primeFinder.increaseConsumerThreads();
            HANDLE hNewConsumer = CreateThread(NULL, 0, consumerThread, NULL, 0, NULL);
            if (hNewConsumer == NULL) {
                cerr << "Cant create consumer thread." << endl;
                return 1;
            }
            hConsumers.push_back(hNewConsumer);
        } else if (choice == '-') {
            primeFinder.decreaseConsumerThreads();
            CloseHandle(hConsumers.back());
            hConsumers.pop_back();
        } else if (choice == 'c') {
            primeFinder.printPrimesInfo();
            break;
        } else {
            cerr << "Invalid choice!" << endl;
        }
    }

    WaitForSingleObject(hProducer, INFINITE);
    WaitForMultipleObjects(NUM_CONSUMER_THREADS, hConsumers.data(), TRUE, INFINITE);

    for (int i = 0; i < NUM_CONSUMER_THREADS; ++i) {
        CloseHandle(hConsumers[i]);
    }

    CloseHandle(hProducer);

    primeFinder.printPrimesInfo();

    return 0;
}
