// WinApi_Lab2.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WinApi_Lab2.h"
#include <ctime>

// parameters
#define MAX_LOADSTRING 100
#define BUFSIZE 50
#define BOMB -1
#define TIMER_ID 5
#define TIMER_FREQ 100
#define FIELDSIZE 25
#define MAXWIDTH 30
#define MAXHIGHT 24
#define MINMINES 10

// user defined messages
#define WM_STARTTIMER WM_USER+1
#define WM_INCREASEBOMBS WM_USER+2
#define WM_DECREASEBOMBS WM_USER+3
#define WM_UPDATEBOARDSIZE WM_USER+4
#define WM_DEBUGON WM_USER+5
#define WM_DEBUGOFF WM_USER+6
#define WM_UNCOVERAREA WM_USER+7
#define WM_BOOM WM_USER+8
#define WM_GAMEWON WM_USER+9
#define WM_KILLTIMER WM_USER+10

// field colors
#define COLOR1 RGB(0, 0, 255)
#define COLOR2 RGB(48, 161, 31)
#define COLOR3 RGB(255, 0, 0)
#define COLOR4 RGB(255, 255, 0)
#define COLOR5 RGB(255, 0, 255)
#define COLOR6 RGB(0, 255, 255)
#define COLOR7 RGB(255, 102, 0)
#define COLOR8 RGB(255, 255, 204)

// structure with data to initialize winodws
typedef struct
{
    // shared variables for each game
    int** board;
    int width;
    int hight;
    int mine_count;
    HWND pannelHandle;
    HWND*** fieldsHandles;
    bool debug;
    // unique varaibles for every field window
    int x;
    int y;
    bool covered;
    bool flaged;
} sINITARGS;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WCHAR szPannelClass[MAX_LOADSTRING];            // the pannel window class name
WCHAR szFieldClass[MAX_LOADSTRING];             // the field window class name
int GnCmdShow;

// Forward declarations of functions included in this code module:
BOOL                MyRegisterClasses(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, int**,HWND**);
BOOL                InitGameUI(HWND, int**, HWND**,bool=false,int=10,int=10,int=10);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    PannelProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    FieldProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Custom(HWND, UINT, WPARAM, LPARAM);

int** randomize_board(int, int, int);
void printRedArial(HWND, TCHAR buf[], int, int);
void drawBackground(HWND, HDC,RECT, COLORREF, bool = false);
void drawBomb(HWND, HDC, RECT, COLORREF);
void drawFlag(HWND, HDC);
void printFieldNumber(HWND, HDC, RECT, int);

BOOL CALLBACK DestoryChildCallback(HWND, LPARAM);
BOOL CALLBACK DisableChildCallback(HWND, LPARAM);
BOOL CALLBACK DebugOnCallback(HWND, LPARAM);
BOOL CALLBACK DebugOffCallback(HWND, LPARAM);

// MAIN FUNCTION
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    GnCmdShow = nCmdShow;

    // TODO: Place code here.
    srand(time(NULL));

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPILAB2, szWindowClass, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PANNEL, szPannelClass, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FILED, szFieldClass, MAX_LOADSTRING);
    MyRegisterClasses(hInstance);
    // Perform application initialization:
    int** board = nullptr;
    HWND** fieldsHandles = nullptr;

    if (!InitInstance (hInstance, nCmdShow,board,fieldsHandles))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPILAB2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int) msg.wParam;
}
// register window classes
BOOL MyRegisterClasses(HINSTANCE hInstance)
{
    //main window
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPILAB2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINAPILAB2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassExW(&wcex)) return FALSE;

    // pannel class
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szPannelClass;
    wcex.hIconSm = NULL;
    wcex.lpfnWndProc = PannelProc;
    wcex.hbrBackground = (HBRUSH)(GetStockObject(WHITE_BRUSH));
    if (!RegisterClassExW(&wcex)) return FALSE;
    // field class
    wcex.lpszClassName = szFieldClass;
    wcex.hbrBackground = GetSysColorBrush(COLOR_GRAYTEXT);
    wcex.lpfnWndProc = FieldProc;
    if (!RegisterClassExW(&wcex)) return FALSE;
    return TRUE;

}
// init main window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, int** board, HWND** fieldsHandles)
{
   hInst = hInstance; // Store instance handle in our global variable
   
   sINITARGS sInitArgs;
   sInitArgs.board = board;
   sInitArgs.fieldsHandles = &fieldsHandles;
   sInitArgs.width = 10;
   sInitArgs.hight = 10;
   sInitArgs.mine_count = 10;
   sInitArgs.debug = false;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_SYSMENU,
     1, 1, 1, 1, nullptr, nullptr, hInstance, (LPVOID)&sInitArgs);

   InitGameUI(hWnd,board,fieldsHandles,false);
   return TRUE;
}
// init new game)
BOOL InitGameUI(HWND hWnd, int** board, HWND** fieldsHandles, bool debug, int newWidth, int newHight, int newMines)
{
    int pannel_h = 30;
    int field_h = FIELDSIZE;
    int field_w = FIELDSIZE;
    board = randomize_board(newWidth, newHight, newMines);
    // allocating place for field handlers
    fieldsHandles = new HWND* [newWidth];
    for(int i=0;i< newWidth;i++)
        fieldsHandles[i] = new HWND[newHight];

    RECT clientRc = {
        0,
        0,
        newWidth * field_w + newWidth - 1,
        pannel_h + newHight * field_h + newHight - 1
    };
    AdjustWindowRect(&clientRc, WS_OVERLAPPEDWINDOW, TRUE);
    int window_w = clientRc.right - clientRc.left;
    int window_h = clientRc.bottom - clientRc.top;

    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    int centerX = (rc.left + rc.right + 1) / 2;
    int centerY = (rc.top + rc.bottom + 1) / 2;

    MoveWindow(hWnd, centerX - window_w / 2, centerY - window_h / 2, window_w, window_h, TRUE);
    SendMessage(hWnd, WM_UPDATEBOARDSIZE, MAKEWPARAM(newWidth, newHight),MAKELPARAM(newMines,0));
    ShowWindow(hWnd, GnCmdShow);
    UpdateWindow(hWnd);

    // creating pannel window
    HWND hPannelhWnd = CreateWindowW(szPannelClass, NULL, WS_CHILD | WS_VISIBLE, 0, 0, window_w - 2, pannel_h - 2, hWnd, nullptr, hInst,(LPVOID)&newMines);
    // creating child windows and initializing them with data structure
    sINITARGS sInitArgs;
    sInitArgs.board = board;
    sInitArgs.fieldsHandles = &fieldsHandles;
    sInitArgs.width = newWidth;
    sInitArgs.hight = newHight;
    sInitArgs.pannelHandle = hPannelhWnd;
    sInitArgs.debug = debug;
    sInitArgs.mine_count = newMines;

    for (int i = 0; i < newWidth; i++)
    {
        for (int j = 0; j < newHight; j++)
        {
            sInitArgs.x = i;
            sInitArgs.y = j;
            HWND hChildhWnd = CreateWindowW(szFieldClass, NULL, WS_CHILD | WS_VISIBLE,
                i * (field_w + 1), pannel_h + j * (field_h + 1), field_w, field_h,
                hWnd, NULL, hInst,(LPVOID)&sInitArgs);
            fieldsHandles[i][j] = hChildhWnd;
            if(debug)PostMessage(hChildhWnd, WM_DEBUGON, NULL, NULL);
        }
    }

    if (debug)
    {
        EnumChildWindows(hWnd, DebugOnCallback, NULL);
    }

    return TRUE;
}

// Message handler for Main window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    sINITARGS* myStruct = (sINITARGS*)GetWindowLong(hWnd, GWL_USERDATA);
    switch (message)
    {
    case WM_CREATE:
    {
        SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        
        // getting structure from outside
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(((CREATESTRUCT*)lParam)->lpCreateParams));
        sINITARGS* tmpStruct = (sINITARGS*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        
        // making new local struct and copying contents
        myStruct = new sINITARGS;
        myStruct->board = tmpStruct->board;
        myStruct->fieldsHandles = tmpStruct->fieldsHandles;
        myStruct->width = tmpStruct->width;
        myStruct->hight = tmpStruct->hight;
        myStruct->mine_count = tmpStruct->mine_count;
        myStruct->debug = tmpStruct->debug;
        
        //attaching the new struct to 'hWnd'
        SetWindowLong(hWnd, GWL_USERDATA, (LONG)myStruct);
    }
    break;
    case WM_UPDATEBOARDSIZE:
    {
        myStruct->width = LOWORD(wParam);
        myStruct->hight = HIWORD(wParam);
        myStruct->mine_count = LOWORD(lParam);
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_MENU_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_MENU_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_MENU_NEW:
            {
                EnumChildWindows(hWnd, DestoryChildCallback, NULL);
                InitGameUI(hWnd,myStruct->board,*myStruct->fieldsHandles, myStruct->debug,myStruct->width,myStruct->hight,myStruct->mine_count);
            }
            break;
            case ID_MENU_CUSTOM:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_NEWGAMEBOX), hWnd, Custom);
                break;
            case ID_MENU_DEBUG:
            {   
                if (!myStruct->debug)
                {
                    myStruct->debug = true;
                    CheckMenuItem(GetMenu(hWnd), ID_MENU_DEBUG, MF_CHECKED);
                    EnumChildWindows(hWnd, DebugOnCallback , NULL);
                    MessageBox(hWnd, _T("DEBUG MODE ON"), _T("DEBUG WINDOW"), NULL);
                }
                else
                {
                    myStruct->debug = false;
                    CheckMenuItem(GetMenu(hWnd), ID_MENU_DEBUG, MF_UNCHECKED);
                    EnumChildWindows(hWnd, DebugOffCallback, NULL);
                    MessageBox(hWnd, _T("DEBUG MODE OFF"), _T("DEBUG WINDOW"), NULL);
                }      
            }
            break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_BOOM:
    {
        MessageBox(hWnd, _T("BOOM!"), _T("Mine"), MB_ICONERROR);
        EnumChildWindows(hWnd, DisableChildCallback, NULL);
        MessageBox(hWnd, _T("Game Lost!"), _T("You lost"), NULL);
    }
    break;
    case WM_GAMEWON:
    {
        MessageBox(hWnd, _T("Game Won!"), _T("Win"), NULL);
        EnumChildWindows(hWnd, DisableChildCallback, NULL);
    }
    break;
    case WM_DESTROY:
    {
        if (myStruct->board != nullptr)
        {
            for (int i = 0; i < myStruct->width; i++)
                delete myStruct->board[i];
            delete myStruct->board;
            myStruct->board = nullptr;
        }
        delete myStruct;
        PostQuitMessage(0);
    }
    break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
// Message handler for pannel window.
LRESULT CALLBACK PannelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool timerActive;
    static DWORD startTime;
    static TCHAR buf[BUFSIZE];
    static int bomb_count;
    static int timer_lpos, timer_rpos, bomb_lpos, bomb_rpos;
    switch (message)
    {
    case WM_CREATE:
    {
        timerActive = false;
        // getting data from outside
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(((CREATESTRUCT*)lParam)->lpCreateParams));
        int* tmpBombCount = (int*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        // copying data to local variable
        bomb_count = *tmpBombCount;
        // callculating timer and bomb counter positions
        RECT rc;
        GetClientRect(hWnd, &rc);
        int half = (rc.right - rc.left) / 2;
        timer_lpos = rc.left;
        timer_rpos = rc.left + half;
        bomb_lpos = rc.left + half;
        bomb_rpos = rc.right;
    }
    break;
    case WM_STARTTIMER:
    {
        if (!timerActive)
        {
            timerActive = true;
            startTime = GetTickCount();
            SetTimer(hWnd, TIMER_ID, TIMER_FREQ, NULL);
        }
    }
    break;
    case WM_KILLTIMER:
    {
        KillTimer(hWnd, TIMER_ID);
    }
    break;
    case WM_TIMER:
    {
        // Printing elapsed time
        DWORD timeDiff = GetTickCount() - startTime;
        _stprintf_s(buf, BUFSIZE, _T("%04d.%d"), timeDiff / 1000, (timeDiff / 100) % 10);
        printRedArial(hWnd, buf, timer_lpos, timer_rpos);
    }
    break;
    case WM_INCREASEBOMBS:
    {
        bomb_count++;
        if(bomb_count>=0)
            _stprintf_s(buf, BUFSIZE, _T("%04d"), bomb_count);
        else
            _stprintf_s(buf, BUFSIZE, _T("-%03d"), -bomb_count);
        printRedArial(hWnd, buf, bomb_lpos, bomb_rpos);
    }
    break;
    case WM_DECREASEBOMBS:
    {
        bomb_count--;
        if (bomb_count >= 0)
            _stprintf_s(buf, BUFSIZE, _T("%04d"), bomb_count);
        else
            _stprintf_s(buf, BUFSIZE, _T("-%03d"), -bomb_count);
        printRedArial(hWnd, buf, bomb_lpos, bomb_rpos);
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        _stprintf_s(buf, BUFSIZE, _T("0000.0"));
        printRedArial(hWnd, buf, timer_lpos, timer_rpos);
        _stprintf_s(buf, BUFSIZE, _T("%04d"), bomb_count);
        printRedArial(hWnd, buf, bomb_lpos, bomb_rpos);
        EndPaint(hWnd, &ps);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
// Message handler for field windows.
LRESULT CALLBACK FieldProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int** board;
    static HWND** fieldsHandles;
    static HWND pannelHandle;
    static bool timerActive;
    static int totalBombs;
    static int uncoveredFields;
    static int rightFlags;
    static int missedFlags;
    sINITARGS* myStruct =(sINITARGS*) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message)
    {
    case WM_CREATE:
    {
        // getting structure from outside
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(((CREATESTRUCT*)lParam)->lpCreateParams));
        sINITARGS* tmpStruct = (sINITARGS*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        
        // assigning arrays to new static pointers
        board = tmpStruct->board;
        pannelHandle = tmpStruct->pannelHandle;
        fieldsHandles = *tmpStruct->fieldsHandles;
        timerActive = false;
        
        // making new local struct and copying contents
        myStruct = new sINITARGS;
        myStruct->hight = tmpStruct->hight;
        myStruct->width = tmpStruct->width;
        myStruct->debug = tmpStruct->debug;
        myStruct->covered = true;
        myStruct->flaged = false;
        myStruct->x = tmpStruct->x;
        myStruct->y = tmpStruct->y;
        totalBombs = tmpStruct->mine_count;
        uncoveredFields = 0;
        rightFlags = 0;
        missedFlags = 0;
        
        //attaching the new struct to current window
        SetWindowLong(hWnd, GWL_USERDATA, (LONG)myStruct);
    }
    break;
    case WM_LBUTTONDOWN:
    {
        if (myStruct->covered == false || myStruct->flaged == true)
            return 0;
        if(!timerActive)SendMessage(pannelHandle, WM_STARTTIMER, NULL, NULL);
        timerActive = true;
        
        HDC hdc = GetDC(hWnd);
        RECT rc;
        GetClientRect(hWnd, &rc);
        drawBackground(hWnd, hdc,rc, RGB(200, 200, 200));

        uncoveredFields++;
        switch (board[myStruct->x][myStruct->y])
        {
        case 0:
        {
            ReleaseDC(hWnd, hdc);
            myStruct->covered = false;
            SendMessage(hWnd, WM_UNCOVERAREA, NULL, NULL);
            return 0;
        }
        case BOMB:
        {
            drawBomb(hWnd, hdc, rc, RGB(0, 0, 0));
            ReleaseDC(hWnd, hdc);
            SendMessage(pannelHandle, WM_KILLTIMER, NULL, NULL);
            SendMessage(GetParent(hWnd), WM_BOOM, NULL, NULL);
            return 0;
        }
        default:
            printFieldNumber(hWnd, hdc, rc, board[myStruct->x][myStruct->y]);
            break;
        }
        ReleaseDC(hWnd, hdc);
        myStruct->covered = false;
        if (uncoveredFields == myStruct->width * myStruct->hight - totalBombs)
        {
            SendMessage(pannelHandle, WM_KILLTIMER, NULL, NULL);
            SendMessage(GetParent(hWnd), WM_GAMEWON, NULL, NULL);
        }
    }
    break;
    case WM_UNCOVERAREA:
    {
        if (board[myStruct->x][myStruct->y] == 0)
        {
            if (myStruct->x - 1 >= 0)
            {
                SendMessage(fieldsHandles[myStruct->x - 1][myStruct->y], WM_LBUTTONDOWN, NULL, NULL);
                if (myStruct->y - 1 >= 0)
                    SendMessage(fieldsHandles[myStruct->x - 1][myStruct->y - 1], WM_LBUTTONDOWN, NULL, NULL);
                if (myStruct->y + 1 < myStruct->hight)
                    SendMessage(fieldsHandles[myStruct->x - 1][myStruct->y + 1], WM_LBUTTONDOWN, NULL, NULL);
            }
            if (myStruct->x + 1 < myStruct->width)
            {
                SendMessage(fieldsHandles[myStruct->x + 1][myStruct->y], WM_LBUTTONDOWN, NULL, NULL);
                if (myStruct->y - 1 >= 0)
                    SendMessage(fieldsHandles[myStruct->x + 1][myStruct->y - 1], WM_LBUTTONDOWN, NULL, NULL);
                if (myStruct->y + 1 < myStruct->hight)
                    SendMessage(fieldsHandles[myStruct->x + 1][myStruct->y + 1], WM_LBUTTONDOWN, NULL, NULL);
            }
            if (myStruct->y + 1 < myStruct->hight)
                SendMessage(fieldsHandles[myStruct->x][myStruct->y + 1], WM_LBUTTONDOWN, NULL, NULL);
            if (myStruct->y - 1 >= 0)
                SendMessage(fieldsHandles[myStruct->x][myStruct->y - 1], WM_LBUTTONDOWN, NULL, NULL);
        }
        return 0;
    }
    break;
    case  WM_DESTROY:
    {
        // first child to delete deletes all allocated data from arrays
        if (board!=nullptr)
        {
            for (int i = 0; i < myStruct->width; i++)
                delete board[i];
            delete board;
            board = nullptr;
        }
        if (fieldsHandles != nullptr)
        {
            for (int i = 0; i < myStruct->width; i++)
                delete[] fieldsHandles[i];
            delete[] fieldsHandles;
            fieldsHandles = nullptr;
        }
        // deleting local myStruct
        delete myStruct;
    }
    break;
    case WM_RBUTTONDOWN:
    {
        if (!timerActive)SendMessage(pannelHandle, WM_STARTTIMER, NULL, NULL);
        timerActive = true;

        if (myStruct->covered == false)
            return 0;

        HDC hdc = GetDC(hWnd);
        if (myStruct->flaged == true)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            drawBackground(hWnd, hdc, rc, COLOR_GRAYTEXT, true);
            if (myStruct->debug)
            {
                switch (board[myStruct->x][myStruct->y])
                {
                case 0:
                    break;
                case BOMB:
                    drawBomb(hWnd, hdc, rc, RGB(0, 0, 0));
                    break;
                default:
                    printFieldNumber(hWnd, hdc, rc, board[myStruct->x][myStruct->y]);
                    break;
                }
            }

            myStruct->flaged = false;
            myStruct->covered = true;
            SendMessage(pannelHandle,WM_INCREASEBOMBS, NULL, NULL);
            if (board[myStruct->x][myStruct->y] == BOMB)
                rightFlags--;
            else
                missedFlags--;
        }
        else
        {
            drawFlag(hWnd, hdc);
            myStruct->flaged = true;
            SendMessage(pannelHandle, WM_DECREASEBOMBS, NULL, NULL);
            if (board[myStruct->x][myStruct->y] == BOMB)
                rightFlags++;
            else
                missedFlags++;
        }
        ReleaseDC(hWnd, hdc);

        if (missedFlags == 0 && rightFlags == totalBombs)
        {
            SendMessage(pannelHandle, WM_KILLTIMER, NULL, NULL);
            SendMessage(GetParent(hWnd), WM_GAMEWON, NULL, NULL);
        }

    }
    break;
    case WM_DEBUGON:
    {
        myStruct->debug = true;
        if (myStruct->covered)
        {
            HDC hdc = GetDC(hWnd);
            RECT rc;
            GetClientRect(hWnd, &rc);
            switch (board[myStruct->x][myStruct->y])
            {
            case 0:
                break;
            case BOMB:
                drawBomb(hWnd, hdc, rc, RGB(0, 0, 0));
                break;
            default:
                printFieldNumber(hWnd, hdc, rc, board[myStruct->x][myStruct->y]);
                break;
            }
            ReleaseDC(hWnd, hdc);
            return 0;
        }
    }
    break;
    case WM_DEBUGOFF:
    {
        myStruct->debug = false;
        HDC hdc = GetDC(hWnd);
        if (myStruct->covered && !myStruct->flaged)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            drawBackground(hWnd, hdc, rc, COLOR_GRAYTEXT, true);
        }
        if (myStruct->flaged)
        {
            drawFlag(hWnd, hdc);
            myStruct->flaged = true;
        }
        ReleaseDC(hWnd, hdc);
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (myStruct->debug)
        {
            if (myStruct->covered)
            {
                RECT rc;
                GetClientRect(hWnd, &rc);
                switch (board[myStruct->x][myStruct->y])
                {
                case 0:
                {
                    ReleaseDC(hWnd, hdc);
                    return 0;
                }
                break;
                case BOMB:
                {
                    drawBomb(hWnd, hdc, rc, RGB(0, 0, 0));
                    ReleaseDC(hWnd, hdc);
                    return 0;
                }
                default:
                    printFieldNumber(hWnd, hdc, rc, board[myStruct->x][myStruct->y]);
                    break;
                }
            }
        }
        EndPaint(hWnd, &ps);
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
// Message handler for Custom size box.
INT_PTR CALLBACK Custom(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
        case IDOK:
        {
            int newWidth = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, NULL);
            int newHight = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, NULL);
            int newMines = GetDlgItemInt(hDlg, IDC_EDIT3, NULL, NULL);
            if (newWidth > MAXWIDTH)
                newWidth = MAXWIDTH;
            if (newWidth <= 0)
                newWidth = 1;
            if (newHight > MAXHIGHT)
                newHight = MAXHIGHT;
            if (newHight <= 0)
                newHight = 1;
            if (newMines < MINMINES)
                newMines = MINMINES;
            if (newMines >= newWidth * newHight)
                newMines = newWidth * newHight;
            SendMessage(GetParent(hDlg), WM_UPDATEBOARDSIZE, MAKEWPARAM(newWidth, newHight), MAKELPARAM(newMines, 0));
            SendMessage(GetParent(hDlg), WM_COMMAND, MAKEWPARAM(ID_MENU_NEW,0),NULL);
            EndDialog(hDlg, LOWORD(wParam));
        }
        break;
        }
    }
    break;
        
    }
    return (INT_PTR)FALSE;
}

// CALLBACK functions to specify action on iterated child windows
BOOL CALLBACK DestoryChildCallback(HWND hwnd, LPARAM lParam)
{
    if (hwnd != NULL) 
        DestroyWindow(hwnd);
    return TRUE;
}
BOOL CALLBACK DebugOnCallback(HWND hwnd, LPARAM lParam)
{
    if (hwnd != NULL)
        SendMessage(hwnd, WM_DEBUGON, NULL, NULL);
    return TRUE;
}
BOOL CALLBACK DebugOffCallback(HWND hwnd, LPARAM lParam)
{
    if (hwnd != NULL)
        SendMessage(hwnd, WM_DEBUGOFF, NULL, NULL);
    return TRUE;
}
BOOL CALLBACK DisableChildCallback(HWND hwnd, LPARAM lParam)
{
    if (hwnd != NULL)
        EnableWindow(hwnd,FALSE);
    return TRUE;
}

// utility functions
int** randomize_board(int width, int hight, int bomb_count)
{
    //BOARD board(width, std::vector<int>(hight, 0));
    int** board = new int* [width];
    for (int i = 0; i < width; i++)
        board[i] = new int[hight] {0};

    int counter = 0;
    // main loop with adding bombs
    while (counter < bomb_count)
    {
        // randomize bomb spot (if already chosen before take adjacent)
        int x = rand() % width;
        int y = rand() % hight;
        while (board[x][y] == BOMB)
        {
            x = x + 1;
            if (x >= width)
            {
                y = (y + 1) % hight;
                x = x % width;
            }
        }
        board[x][y] = BOMB;
        counter++;

        if (x + 1 < width)
        {
            if (board[x + 1][y] != BOMB) board[x + 1][y]++;
            if (y + 1 < hight && board[x + 1][y + 1] != BOMB) board[x + 1][y + 1]++;
            if (y - 1 >= 0 && board[x + 1][y - 1] != BOMB) board[x + 1][y - 1]++;
        }
        if (x - 1 >= 0)
        {
            if (board[x - 1][y] != BOMB) board[x - 1][y]++;
            if (y + 1 < hight && board[x - 1][y + 1] != BOMB) board[x - 1][y + 1]++;
            if (y - 1 >= 0 && board[x - 1][y - 1] != BOMB) board[x - 1][y - 1]++;
        }
        if (y - 1 >= 0 && board[x][y - 1] != BOMB) board[x][y - 1]++;
        if (y + 1 < hight && board[x][y + 1] != BOMB) board[x][y + 1]++;
    }
    return board;
}
void printRedArial(HWND hWnd, TCHAR buf[], int left, int right)
{
    HDC hdc = GetDC(hWnd);
    HFONT font = CreateFont(
        25,
        0,
        0,
        0,
        FW_BOLD,
        false,
        FALSE,
        0,
        EASTEUROPE_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        _T("Arial"));
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    RECT rc;
    GetClientRect(hWnd, &rc);
    rc.left = left;
    rc.right = right;
    SetTextColor(hdc, RGB(255, 0, 0));
    DrawText(hdc, buf, (int)_tcslen(buf), &rc,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(font);
    ReleaseDC(hWnd, hdc);
}
void drawBackground(HWND hWnd, HDC hdc, RECT rc, COLORREF color, bool cover)
{
    HBRUSH brush;
    if (cover)
        brush = GetSysColorBrush(COLOR_GRAYTEXT);
    else
        brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    Rectangle(hdc, rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}
void drawBomb(HWND hWnd, HDC hdc, RECT rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    Ellipse(hdc, rc.left + 4, rc.top + 4, rc.right - 4, rc.bottom - 4);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}
void drawFlag(HWND hWnd, HDC hdc)
{
    HBITMAP bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FLAG));
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
    BITMAP qBitmap;
    GetObject(reinterpret_cast<HGDIOBJ>(bitmap), sizeof(BITMAP), reinterpret_cast<LPVOID>(&qBitmap));
    BitBlt(hdc, 0, 0, qBitmap.bmWidth, qBitmap.bmHeight, memDC, 0, 0, SRCCOPY);
    StretchBlt(hdc, 0, 0, FIELDSIZE, FIELDSIZE, memDC, 0, 0, qBitmap.bmWidth, qBitmap.bmHeight, SRCCOPY);
    SelectObject(memDC, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memDC);
}
void printFieldNumber(HWND hWnd, HDC hdc, RECT rc, int num)
{
    TCHAR s[BUFSIZE];
    _stprintf_s(s, BUFSIZE, _T("%d"), num);
    switch (num)
    {
    case 1:
        SetTextColor(hdc, COLOR1);
        break;
    case 2:
        SetTextColor(hdc, COLOR2);
        break;
    case 3:
        SetTextColor(hdc, COLOR3);
        break;
    case 4:
        SetTextColor(hdc, COLOR4);
        break;
    case 5:
        SetTextColor(hdc, COLOR5);
        break;
    case 6:
        SetTextColor(hdc, COLOR6);
        break;
    case 7:
        SetTextColor(hdc, COLOR7);
        break;
    case 8:
        SetTextColor(hdc, COLOR8);
        break;
    default:
        SetTextColor(hdc, RGB(255, 0, 0));
        break;
    }
    SetBkMode(hdc, TRANSPARENT);
    DrawText(hdc, s, (int)_tcslen(s), &rc,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}