#include <Windows.h>
#include <iostream>
#include <string>
using namespace std;

HANDLE hStdOut, hStdIn;
const int MAX_BUFFERS = 100;
string bufferNames[MAX_BUFFERS];
string bufferContents[MAX_BUFFERS];
int numBuffers = 0;
int currentBuffer = 0;
int lastActiveBuffer = 0;
bool inServiceBuffer = false;

void ClearScreen()
{
    COORD coord = { 0, 0 };
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwWritten;

    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    FillConsoleOutputCharacterA(hStdOut, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &dwWritten);
    SetConsoleCursorPosition(hStdOut, coord);
}

void DisplayBuffers()
{
    ClearScreen();

    for (int i = 0; i < numBuffers; ++i)
    {
        if (i == 0 && inServiceBuffer)
        {
            continue;
        }

        if (i == currentBuffer)
        {
            SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        }

        else
        {
            SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
        }

        cout << (i) << ": " << bufferNames[i] << endl;
    }

    SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
    cout << "\nBuffer contents: " << bufferContents[currentBuffer];
}

int main()
{
    hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    cout << "Enter the number of buffers: ";
    cin >> numBuffers;
    cin.ignore();

    if (numBuffers > MAX_BUFFERS - 1)
    {
        cout << "Max number of buffers: " << MAX_BUFFERS - 1 << endl;
        return 1;
    }

    bufferNames[0] = "Service buffer";

    for (int i = 1; i <= numBuffers; ++i)
    {
        cout << "Enter a name for the buffer " << i << ": ";
        getline(cin, bufferNames[i]);
    }

    numBuffers++;
    currentBuffer = 1;
    lastActiveBuffer = 1;
    inServiceBuffer = true;
    DisplayBuffers();

    DWORD cWritten;
    INPUT_RECORD irInputBuffer[128];
    int cNumRead;

    DWORD dwOldMode, dwNewMode;
    GetConsoleMode(hStdIn, &dwOldMode);
    dwNewMode = dwOldMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(hStdIn, dwNewMode);

    while (true)
    {
        ReadConsoleInput(hStdIn, irInputBuffer, 128, (LPDWORD)&cNumRead);

        for (int i = 0; i < cNumRead; i++)
        {
            if (irInputBuffer[i].EventType == KEY_EVENT && irInputBuffer[i].Event.KeyEvent.bKeyDown)
            {
                if (irInputBuffer[i].Event.KeyEvent.wVirtualKeyCode == 0x1B)
                {
                    if (inServiceBuffer)
                    {
                        inServiceBuffer = false;
                        currentBuffer = lastActiveBuffer;
                        ClearScreen();
                        SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);

                        cout << "Press F1 to enter the menu" << endl;
                        cout << "Current buffer: " << bufferNames[currentBuffer] << endl;
                        cout << bufferContents[currentBuffer];
                    }

                    else
                    {
                        return 0;
                    }
                }

                else if (irInputBuffer[i].Event.KeyEvent.wVirtualKeyCode == 0x70)
                {
                    inServiceBuffer = true;
                    DisplayBuffers();
                }

                else if (inServiceBuffer && (irInputBuffer[i].Event.KeyEvent.wVirtualKeyCode == 0x26 || irInputBuffer[i].Event.KeyEvent.wVirtualKeyCode == 0x28))
                {
                    if (irInputBuffer[i].Event.KeyEvent.wVirtualKeyCode == 0x26)
                    {
                        currentBuffer = (currentBuffer - 1 + numBuffers) % numBuffers;

                        if (currentBuffer == 0)
                        {
                            currentBuffer = numBuffers - 1;
                        }
                    }

                    else
                    {
                        currentBuffer = (currentBuffer + 1) % numBuffers;

                        if (currentBuffer == 0)
                        {
                            currentBuffer = 1;
                        }
                    }

                    DisplayBuffers();
                }

                else if (inServiceBuffer && irInputBuffer[i].Event.KeyEvent.wVirtualKeyCode == 0x0D)
                {
                    inServiceBuffer = false;
                    lastActiveBuffer = currentBuffer;
                    ClearScreen();
                    SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
                    cout << "Press F1 to enter the menu" << endl;
                    cout << "Current buffer: " << bufferNames[currentBuffer] << endl;
                    cout << bufferContents[currentBuffer];
                }

                else if (!inServiceBuffer)
                {
                    char c = irInputBuffer[i].Event.KeyEvent.uChar.AsciiChar;

                    if (c != '\0')
                    {
                        bufferContents[currentBuffer] += c;
                        WriteFile(hStdOut, &c, 1, &cWritten, NULL);
                    }
                }
            }
        }
    }

    return 0;
}