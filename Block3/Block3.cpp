#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

//Network
#define SERVER_IP "147.175.115.34"
#define SERVER_PORT "777"

#define DEFAULT_BUFLEN 4096

//Chat markup
#define ME_X_START        0
#define ME_WIDTH         35
#define MORPHEUS_X_START 50
#define MORPHEUS_WIDTH   40

typedef struct {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO cbsi;
    COORD coord;
}Screen;

Screen SetCursorX(Screen* screen, int x)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO cbsi;
    GetConsoleScreenBufferInfo(hConsole, &cbsi);
    COORD coord = cbsi.dwCursorPosition;

    coord.X = x;
    coord.Y = coord.Y + 1;
    SetConsoleCursorPosition(hConsole, coord);

    screen->hConsole = hConsole;
    screen->cbsi = cbsi;
    screen->coord = coord;

    return *screen;
}

int Send(SOCKET ConnectSocket,const char* sendBuf, int sendLen)
{
    int res  = send(ConnectSocket, sendBuf, sendLen, 0);
    if (res == SOCKET_ERROR) {
        printf("Send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    return 0;
}

void DialogAnswer(Screen* s, int dialog, char* text, FILE* file)
{
    //Depends on dialog sets parameters
    int x_start = 0, width = 0;
    if (dialog) {
        x_start = MORPHEUS_X_START;
        width = MORPHEUS_WIDTH;
        *s = SetCursorX(s, x_start);
        printf("Morpheus: ");
    }
    else {
        x_start = ME_X_START;
        width = ME_WIDTH;
        *s = SetCursorX(s, x_start);
        printf("Me: ");
    }

    //replace \n with \0
    char* p = strchr(text, '\n');
    if (p != 0) {
        *p = '\0';
    }

    //Wrap lines
    int cnt = 0;
    char* ch = text;
    while (*ch != '\0') {
        if (cnt > width && *ch == ' ') {
            s->coord.X = x_start;
            s->coord.Y = s->coord.Y + 1;
            SetConsoleCursorPosition(s->hConsole, s->coord);
            cnt = 0;
        }

        //Skip space on the beginning on line
        GetConsoleScreenBufferInfo(s->hConsole, &s->cbsi);
        COORD coord = s->cbsi.dwCursorPosition;
        if (coord.X == x_start && *ch == ' ') {
            ch++;
            continue;
        }

        char test = *ch;
        putchar(test);
        cnt++;
        ch++;
    }

    //Save to file
    fprintf(file, "%s\n", text);
    fflush(file);
}

int Receive(SOCKET ConnectSocket, char* recvbuf, int recLen, FILE* file, Screen* s)
{
    int res = recv(ConnectSocket, recvbuf, recLen, 0);
    if (res > 0)
    {        
        DialogAnswer(s, 1, recvbuf, file);        
    }
    else if (res == 0)
    {
        printf("Connection closed\n");
    }        
    else
        printf("Receive failed with error: %d\n", WSAGetLastError());
    return 0;
}

int Connect(SOCKET *ConnectSocket)
{
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0)
    {
        printf("WSAStartup failed: %d\n", res);
        return 1;
    }

    struct addrinfo* result = NULL, * ptr = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    res = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &result);
    if (res != 0)
    {
        printf("getaddrinfo failed : %d\n", res);
        WSACleanup();
        return 1;
    }
    //else
    //    printf("getaddrinfo did not fail.\n");

    ptr = result;

    *ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (*ConnectSocket == INVALID_SOCKET)
    {
        printf("Error at socket() : %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    //else
    //    printf("Error at socket did not occur.\n");

    res = connect(*ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (res == SOCKET_ERROR)
        printf("Not connected to server…\n");
    else
    {
        Send(*ConnectSocket, "Hello", 6);
        //   printf("Connected to server!\n");
    }

    if (res == SOCKET_ERROR)
    {
        closesocket(*ConnectSocket);
        *ConnectSocket = INVALID_SOCKET;
        WSACleanup();
        return 1;
    }

    Sleep(250);

    return 0;
}

void SetCmdColor(WORD color) 
{
    HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStd, color);
}

char* DecoderXOR(const char* str, int key)
{
    int cnt = 0, dc = 0;
    // Count length of input char array
    while ((char)str[cnt] != '\n') {
        cnt++;
    }
    // Allocate memory for new string
    char* res = (char*)malloc(sizeof(char) * cnt);
    // loop whole char array
    for (int i = 0; i < 132; i++){
        dc = (int)str[i] ^ key; // char XOR key and cast to int
        res[i] = (char)dc;      // cast int to char
    } 

    return res;
}

int IsPrime(int num)
{// returns 1 if prime over wise 0
    int res = 1;
    // 0 and 1 are not prime numbers
    if (num == 0 || num == 1) {
        res = 0;
    }
    else {
        for (int i = 2; i <= num / 2; ++i) {
            if (num % i == 0) {
                return 0;
            }
        }
    }
    return res;
}

char* DecoderPrimeNumber(char* str)
{
    int i = 0, j = 0;
    int len = (int)strlen(str);

    // Allocate memory for new string
    char* res = (char*)malloc(sizeof(char) * len);

    // Take characters index equal to prime number
    for (i = 0; i < len; i++) 
    {
        if (IsPrime(i)) 
        {
            res[j++] = str[i-1]; // i-1 because count from 0
        }
    }
    res[j] = '\0';
    return res;
}

int main()
{
    // Connect to the server
    SOCKET ConnectSocket = INVALID_SOCKET;
    if (Connect(&ConnectSocket) == 1)
        return 1;

    // Task1
    SetConsoleOutputCP(CP_UTF8);
    // Task2
    SetCmdColor(10);
    // Task 5-6
    double x = 48.858596, y = 2.294471;

    Screen screen;
    char myID[7] = {0};
    int isConsists = 0; 
    char sendBuf[DEFAULT_BUFLEN];
    char recvBuf[DEFAULT_BUFLEN];    
    
    // File to save dialog
    FILE* file;
    fopen_s(&file, "block3.txt", "a");

    // Chat with server 
    while (1)
    {
        //Receive        
        Receive(ConnectSocket, recvBuf, DEFAULT_BUFLEN, file, &screen);

        // Task0
        isConsists = (int)strstr(recvBuf, "Send me your ID");
        if (isConsists > 0) {
            isConsists = 0;
            memcpy(myID, sendBuf, sizeof(myID)); // Copy entered ID to local variable
        }        

        // Task1
        isConsists = (int)strstr(recvBuf, "SetConsoleOutputCP");
        if (isConsists > 0) {
            memset(sendBuf, 0, sizeof sendBuf);
            DialogAnswer(&screen, 0, sendBuf, file);
            printf("Dobrý deň. \n");
        }
        
        // Task2 Font color
        // Task3
        isConsists = (int)strstr(recvBuf, "send me the remainder");
        if (isConsists > 0) {
            // Sum of first 5 digits
            int sum5 = 0, remainder = 0;
            for (int i = 0; i < 5; i++) {
                sum5 += (int)(myID[i] - '0');
            }
            // Remainder
            remainder = sum5 % (int)(myID[4] - '0');

            sprintf_s(sendBuf, "%d", remainder);
            DialogAnswer(&screen, 0, sendBuf , file);
            Send(ConnectSocket, sendBuf, (int)strlen(sendBuf));
            isConsists = 0;
            continue;
        }

        // Task4 DecoderXOR
        isConsists = (int)strstr(recvBuf, "send me the code 123");
        if (isConsists > 0) {
            //Send 
            memset(sendBuf, 0, sizeof sendBuf);
            DialogAnswer(&screen, 0, sendBuf, file);
            fgets(sendBuf, DEFAULT_BUFLEN, stdin);
            Send(ConnectSocket, sendBuf, (int)strlen(sendBuf));
            //Receive        
            Receive(ConnectSocket, recvBuf, DEFAULT_BUFLEN, file, &screen);
            // Decode            
            char* decoded = DecoderXOR(recvBuf, 55);
            DialogAnswer(&screen, 0, decoded, file);
            Send(ConnectSocket, recvBuf, (int)strlen(recvBuf));          
            continue;
        }

        // Task5
        isConsists = (int)strstr(recvBuf, "Send me the integral part of the first coordinate");
        if (isConsists > 0) {
            sprintf_s(sendBuf, "%d", (int)x); //cast to int and we have only the integral part
            DialogAnswer(&screen, 0, sendBuf, file);
            Send(ConnectSocket, sendBuf, (int)strlen(sendBuf));
            continue;
        }

        // Task6
        isConsists = (int)strstr(recvBuf, "Send me the integral part of the second coordinate");
        if (isConsists > 0) {
            sprintf_s(sendBuf, "%d", (int)y); //cast to int and we have only the integral part
            DialogAnswer(&screen, 0, sendBuf, file);
            Send(ConnectSocket, sendBuf, (int)strlen(sendBuf));
            continue;
        }

        // Task7
        // Task8 DecoderPrimeNumber
        isConsists = (int)strstr(recvBuf, "Trinity was in the Eiffel Tower");
        if (isConsists > 0) {
            //Send 
            memset(sendBuf, 0, sizeof sendBuf);
            DialogAnswer(&screen, 0, sendBuf, file);
            fgets(sendBuf, DEFAULT_BUFLEN, stdin);
            Send(ConnectSocket, sendBuf, (int)strlen(sendBuf));
            //Receive        
            Receive(ConnectSocket, recvBuf, DEFAULT_BUFLEN, file, &screen);
            //Decode
            char* result = DecoderPrimeNumber(recvBuf);
            DialogAnswer(&screen, 0, result, file);
            Send(ConnectSocket, result, (int)strlen(result));
            continue;
        }

        // Exit
        isConsists = (int)strstr(recvBuf, "Good answer, great job!");
        if (isConsists > 0) {
            break;
        }
        
        //Send
        memset(sendBuf, 0, sizeof sendBuf);
        DialogAnswer(&screen, 0, sendBuf, file);
        fgets(sendBuf, DEFAULT_BUFLEN, stdin);
        Send(ConnectSocket, sendBuf, (int)strlen(sendBuf));
    }   

    // Cleanup socket
    closesocket(ConnectSocket);
    WSACleanup();
    fclose(file);

    return 0;
}