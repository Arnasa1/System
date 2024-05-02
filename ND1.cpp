#include <iostream>
#include <cstdio>
#include <Windows.h>
#include <cmath>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <bitset>

using namespace std;

void formatNumbers() {
    double num;
    for (int count = 0; count < 3;count++) {
        cin >> num;
        if (num >= 1000 || num <= -1000) {
            printf("%.2e\n", num);
        } else {
            int decDigits = 0;
            double decimal = num - floor(num);
            while (decimal != 0 && decDigits < 10) {
                num *= 10;
                decimal = num - floor(num);
                decDigits++;
            }
            if (decimal == 0) {
                printf("%.*f\n", max(0, decDigits - 1), num / pow(10, decDigits));
            } else {
                printf("%.2f\n", num / pow(10, decDigits));
            }
        }
    }
}


void displayErrorMessage() {
    DWORD errorCode;
    for(int i=1;i<=3;i++){
        cin >> errorCode;
        int help = errorCode;

        if (help > 0) 
        {
            LPSTR errorMessageBuffer = nullptr;

            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&errorMessageBuffer),
                0,
                NULL
            );

            if (errorMessageBuffer[0] != '\0') {
                cout << "Message"<<" "<<help <<": " << errorMessageBuffer << endl;
            } else {
                cout << "No such error exists." << endl;
            }

            LocalFree(errorMessageBuffer);
        }

        else 
        {
            cout << "No such error exists" << endl;
            break;
        }

    }
}

void displaySystemInformation() {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    cout << "Number of Logical Processors: " << systemInfo.dwNumberOfProcessors << endl;
    cout << "Page size: " << systemInfo.dwPageSize << " Bytes" << endl;
    cout << "Processor Mask: 0x" << hex << setw(16) << setfill('0') << systemInfo.dwActiveProcessorMask << dec << endl;
    cout << "Minimum process address: 0x" << hex << setw(16) << setfill('0') << reinterpret_cast<uintptr_t>(systemInfo.lpMinimumApplicationAddress) << dec << endl;
    cout << "Maximum process address: 0x" << hex << setw(16) << setfill('0') << reinterpret_cast<uintptr_t>(systemInfo.lpMaximumApplicationAddress) << dec << endl;
}

void computeBase64() {
    string text = "Secure Programming";
    string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string base64Result;

    string binaryText;
    for (char c : text) {
        binaryText += bitset<8>(c).to_string();
    }

    size_t padding = 0;
    if (binaryText.length() % 3 != 0) {
        padding = 3 - (binaryText.length() % 3);
        binaryText += string(padding, '0');
    }

    for (size_t i = 0; i < binaryText.length(); i += 6) {
        size_t index = bitset<6>(binaryText.substr(i, 6)).to_ulong();
        base64Result += base64Chars[index];
    }

    base64Result += string(padding, '=');

    cout << base64Result << endl;
}

void findPrimes() {
    int number;
    while (cin >> number) {
        if (number == 0)
            break;

        bool isPrime = true;
        if (number <= 1 || (number > 2 && number % 2 == 0)) {
            isPrime = false;
        } else {
            for (int i = 3; i * i <= number; i += 2) {
                if (number % i == 0) {
                    isPrime = false;
                    break;
                }
            }
        }
        cout << (isPrime ? "TRUE" : "FALSE") << endl;
    }
}

int main() {
    int choice;
    do {
        cout << "1. Formatting the entered number with printf.\n";
        cout << "2. Displaying error messages using Windows API.\n";
        cout << "3. Displaying system information.\n";
        cout << "4. BASE64 computation for \"Secure Programming\" text.\n";
        cout << "5. Finding prime numbers.\n";
        cin >> choice;

        switch (choice) {
        case 1:
            formatNumbers();
            choice = 6;
            break;
        case 2:
            displayErrorMessage();
            choice = 6;
            break;
        case 3:
            displaySystemInformation();
            choice = 6;
            break;
        case 4:
            computeBase64();
            choice = 6;
            break;
        case 5:
            findPrimes();
            choice = 6;
            break;
        default:
            break;
        }
    } while (choice != 6);

    return 0;
}
