#include <iostream>
#include <cctype>
#include <string>

using namespace std;

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        cerr << "Too few arguments — please provide input line" << endl;
        return 1;
    }

    string word = argv[1];
    for (char &c: word) {
        c = toupper(c);
    }
    word += '\n';
    cout << word;

    return 0;
}