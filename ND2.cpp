#include <bits/stdc++.h>
#include <windows.h>
using namespace std;

class Formatas {
public:
    char tag[3];
    char pavadinimas[30];
    char atlikejas[30];
    char albumas[30];

    void Read(const char* failo_pavadinimas) {
        HANDLE file = CreateFile(failo_pavadinimas, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        
        if (file == INVALID_HANDLE_VALUE) {
            cerr << "Nepavyko atidaryti failo!";
            return;
        }

        SetFilePointer(file, -128, NULL, FILE_END);
        DWORD bytesRead;
        ReadFile(file, tag, 3, &bytesRead, NULL);       
        ReadFile(file, pavadinimas, 30, &bytesRead, NULL);     
        ReadFile(file, atlikejas, 30, &bytesRead, NULL);    
        ReadFile(file, albumas, 30, &bytesRead, NULL);     
        CloseHandle(file);
    }

    void Write(const char* failo_pavadinimas) {
        HANDLE file = CreateFile(failo_pavadinimas, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        
        SetFilePointer(file, -128, NULL, FILE_END);
        DWORD bytesWritten;
        WriteFile(file, tag, 3, &bytesWritten, NULL);      
        WriteFile(file, pavadinimas, 30, &bytesWritten, NULL); 
        WriteFile(file, atlikejas, 30, &bytesWritten, NULL);   
        WriteFile(file, albumas, 30, &bytesWritten, NULL);   
        CloseHandle(file);
    }
};

int main() {
    const char* failo_pavadinimas = "bensound-far.mp3";
    Formatas* IDv3= static_cast<Formatas*>(VirtualAlloc(NULL, sizeof(Formatas), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    IDv3->Read(failo_pavadinimas);

    cout << "Pavadinimas: " << IDv3->pavadinimas << endl;
    cout << "Atlikejas: " << IDv3->atlikejas << endl;
    cout << "Albumas: " << IDv3->albumas << endl;
   
    cout << "\nIveskite nauja pavadinima: ";
    cin.getline(IDv3->pavadinimas, sizeof(IDv3->pavadinimas));
    cout << "Iveskite nauja atlikeja: ";
    cin.getline(IDv3->atlikejas, sizeof(IDv3->atlikejas));
    cout << "Iveskite nauja albuma: ";
    cin.getline(IDv3->albumas, sizeof(IDv3->albumas));

    IDv3->Write(failo_pavadinimas);
    VirtualFree(IDv3, sizeof(Formatas), MEM_RELEASE);

    return 0;
}
