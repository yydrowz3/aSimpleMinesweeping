#pragma warning(disable:4996)
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <time.h>
#include <stdio.h>


#define gamebuttonID 3301                       //自己给按钮一个编号
#define ranklistbuttonID 3302
#define exitbuttonID 3303
#define easybuttonID 3304
#define normalbuttonID 3305
#define hardbuttonID 3306
#define deleteonebuttonID 3307
#define deleteallbuttonID 3308
#define returnbuttonID 3309
#define deleteokbuttonID 3310
#define deletecancelbuttonID 3311
#define addokbuttonID 3312
#define timerecordID 3313
#define MAXROW 16                               //最大行
#define MAXCOL 30                               //最大列
#define blocksidelength 30                      //扫雷格子的边长


TCHAR szWindowClass[] = _T("DesktopApp");        //窗口类名字
HINSTANCE hInst;                                 //窗口实例句柄


struct player
{
    int gamemode;
    wchar_t name[1000];
    int score;
    struct player* next;
};


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);                   //窗口处理函数
void starter(void);                                                     //启动界面
void button(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int timeend, HWND haddtext, HWND* hgame, int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int* row, int* col, int* bomb, int *timebegin);   //按钮处理
HWND game(int gamemode, int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int* row, int* col, int* bomb);               //游戏窗口的创建
void gamemodeselect(void);              //难度选择窗口
void boardreset(int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int row, int col);            //游戏重置
void coverdisplay(HWND hgame, char cover[MAXROW][MAXCOL], int row, int col, int bomb, int timebegin, int item);             //游戏的绘制
wchar_t* chartowchar(const char* str);              //宽字符转化
void bombplant(int real[MAXROW][MAXCOL], int row, int col, int bomb);               //埋雷
int locationvalue(int x, int y, int real[MAXROW][MAXCOL], int row, int col);                //计算格子对应的数值
void takestepleft(int x, int y, int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int row, int col, int* judge, int *item);         //左键
void discover(int x, int y, int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int row, int col, int* judge, int *item);             //翻开一个格子
void wincheck(int* judge, char cover[MAXROW][MAXCOL], int row, int col, int bomb);              //判断是否完成游戏
void takestepright(int x, int y, char cover[MAXROW][MAXCOL]);                   //右键
void itemcreate(char cover[MAXROW][MAXCOL], int row, int col);              //道具
HWND ranklist(int gamemode);                            //英雄榜窗口的创建
struct player* loadlist(void);                          //将文件中的数据创建链表
void showlist(int gamemode, struct player* head, HWND hranklist);           //链表信息的绘制
void listsave(struct player *head);             //将链表的数据保存到文件
void freelist(struct player **head);            //释放链表
HWND deleteone(void);                       //删除窗口的创建
void playerdelete(int gamemode, struct player **head, HWND hdeletetext);            //链表的删除
void deleteall(int gamemode, struct player** head);                                 //链表的删除
HWND win(void);                         //游戏完成界面的创建
void listinsert(int gamemode, struct player **head, int score, HWND haddtext);      //链表的插入
void listsort(int gamemode, struct player *head);                                   //链表的排序



int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wcex;            //窗口类

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))                                                //窗口类的注册并判断是否注册成功
    {
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Windows Desktop Guided Tour"), NULL);
        return 1;
    }

    hInst = hInstance;                                          //将窗口实例存入全局变量


    srand((unsigned int)time(NULL));                            //随机数种子

    starter();                                                  //开始界面

    MSG msg;                                                                //消息循环
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}


////////////////////////////////////////////////////////窗口处理函数///////////////////////////////////////////////////////////



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    static int timebegin = 0;
    static int timeend = 0;
    static int judge = 1;                               //用于判断游戏继续、胜利、失败
    static int item = 0;
    int x, y, i, j;
    static HWND haddtext = NULL;
    static HWND hgame = NULL;
    static int real[MAXROW][MAXCOL];
    static char cover[MAXROW][MAXCOL];
    static row, col, bomb;

    switch (message)
    {
    case WM_LBUTTONDOWN:                    //左键点击消息
        if (hWnd == hgame)
        {
            x = LOWORD(lParam);             //获取点击时的坐标
            y = HIWORD(lParam);
            if (x >= 40 && x < 40 + blocksidelength * col && y >= 100 && y < 100 + blocksidelength * row)       //坐标在游戏范围内
            {
                x -= 40;
                y -= 100;
                j = x / blocksidelength;
                i = y / blocksidelength;        //将鼠标坐标转换为数组的坐标
                if (i >= 0 && i < row && j >= 0 && j < col)         //防止越界
                {
                    takestepleft(i, j, real, cover, row, col, &judge, &item);
                    itemcreate(cover, row, col);
                    coverdisplay(hWnd, cover, row, col, bomb, timebegin, item);
                    if (judge == 1)
                    {
                        wincheck(&judge, cover, row, col, bomb);
                    }
                    if (judge == 0)                 //游戏失败
                    {
                        KillTimer(hgame, timerecordID);
                        MessageBox(hWnd, L"你输了！", L"抱歉", MB_OK);
                        DestroyWindow(hWnd);
                        judge = 1;
                        starter();
                    }
                    if (judge == 2)             //游戏完成
                    {
                        timeend = (int)time(NULL);
                        KillTimer(hgame, timerecordID);
                        MessageBox(hWnd, L"你赢了！", L"恭喜", MB_OK);
                        haddtext = win();
                        judge = 1;
                        DestroyWindow(hWnd);
                    }
                }
            }
        }
        break;
    case WM_RBUTTONDOWN:                                            //鼠标右键点击消息
        if (hWnd == hgame)
        {
            x = LOWORD(lParam);                                     //获取坐标
            y = HIWORD(lParam);

            if (x >= 40 && x < 40 + blocksidelength * col && y >= 100 && y <= 100 + blocksidelength * row)
            {
                x -= 40;
                y -= 100;
                j = x / blocksidelength;
                i = y / blocksidelength;
                if (i >= 0 && i < row && j >= 0 && j < col)                 //防止越界
                {
                    takestepright(i, j, cover);
                    coverdisplay(hWnd, cover, row, col, bomb, timebegin, item);
                }
            }
        }
        break;
    case WM_PAINT:                                      //窗口在移动、改变大小、最大最小化后会丢失原来绘制好的内容
        hdc = BeginPaint(hWnd, &ps);                    //窗口在移动、改变大小、最大最小化后会发送WM_PAINT消息，可以利用这条消息来重新绘制窗口
        EndPaint(hWnd, &ps);
        if (hWnd == hgame)
        {
            coverdisplay(hWnd, cover, row, col, bomb, timebegin, item);
        }
        break;
    case WM_COMMAND:                                    //点击按钮后
        button(hWnd, message, wParam, lParam, timeend, haddtext, &hgame, real, cover, &row, &col, &bomb, &timebegin);
        break;
    case WM_TIMER:                                          //计时器到达设定的时间间隔后发送WM_TIMER消息
        coverdisplay(hgame, cover, row, col, bomb, timebegin, item);
        break;
    case WM_DESTROY:
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);            //其他没有处理到的消息交给系统来处理
        break;
    }

    return 0;
}




void button(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int timeend, HWND haddtext, HWND *hgame, int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int *row, int *col, int *bomb, int *timebegin)
{
    int wmID = LOWORD(wParam);                      //每个按钮都会发送消息，通过对每个按钮设定编号来确定具体的按钮，按钮的编号在消息的wParam参数的低位部分
    static int gamerankswitch;
    static int gamemode;
    static struct player* head = NULL;
    static HWND hranklist;
    int score = 0;
    static HWND hdeletetext;

    switch (wmID)
    {
    case gamebuttonID:                  //游戏界面
        gamerankswitch = 1;
        DestroyWindow(hWnd);
        gamemodeselect();
        break;
    case ranklistbuttonID:              //英雄榜
        gamerankswitch = 2;
        DestroyWindow(hWnd);
        gamemodeselect();
        break;
    case exitbuttonID:                  //退出按钮
        exit(0);
        break;
    case easybuttonID:
        DestroyWindow(hWnd);
        gamemode = 1;
        if (gamerankswitch == 1)
        {
            *hgame = game(gamemode, real, cover, row, col, bomb);               //获取游戏窗口句柄
            *timebegin = (int)time(NULL);                                       //记录时间开始点
            coverdisplay(*hgame, cover, *row, *col, *bomb, *timebegin, 0);
            SetTimer(*hgame, timerecordID, 1000, NULL);                         //设置计时器
        }
        else if (gamerankswitch == 2)
        {
            hranklist = ranklist(gamemode);                                     //创建英雄榜窗口并获取句柄
            head = loadlist();                                                  //将文件导入链表并获取头指针
            showlist(gamemode, head, hranklist);
        }
        break;
    case normalbuttonID:
        DestroyWindow(hWnd);
        gamemode = 2;
        if (gamerankswitch == 1)
        {
            *hgame = game(gamemode, real, cover, row, col, bomb);
            *timebegin = (int)time(NULL);
            coverdisplay(*hgame, cover, *row, *col, *bomb, *timebegin, 0);
            SetTimer(*hgame, timerecordID, 1000, NULL);
        }
        else if (gamerankswitch == 2)
        {
            hranklist = ranklist(gamemode);
            head = loadlist();
            showlist(gamemode, head, hranklist);
        }
        break;
    case hardbuttonID:
        DestroyWindow(hWnd);
        gamemode = 3;
        if (gamerankswitch == 1)
        {
            *hgame = game(gamemode, real, cover, row, col, bomb);
            *timebegin = (int)time(NULL);
            coverdisplay(*hgame, cover, *row, *col, *bomb, *timebegin, 0);
            SetTimer(*hgame, timerecordID, 1000, NULL);
        }
        else if (gamerankswitch == 2)
        {
            hranklist = ranklist(gamemode);
            head = loadlist();
            showlist(gamemode, head, hranklist);
        }
        break;
    case deleteonebuttonID:
        hdeletetext = deleteone();                              //创建删除窗口并获取文本框的句柄
        break;
    case deleteallbuttonID:
        deleteall(gamemode, &head);                             //删除全部
        InvalidateRect(hranklist, NULL, TRUE);                  //刷新客户区
        UpdateWindow(hranklist);                                //刷新窗口
        showlist(gamemode, head, hranklist);
        break;
    case returnbuttonID:
        listsave(head);                                         //将链表的数据保存到文件
        freelist(&head);                                        //释放链表
        DestroyWindow(hWnd);
        starter();                                              //关闭窗口重新打开开始窗口
        break;
    case deleteokbuttonID:
        playerdelete(gamemode, &head, hdeletetext);             //删除指定数据
        DestroyWindow(hWnd);
        InvalidateRect(hranklist, NULL, TRUE);                  //刷新窗口
        UpdateWindow(hranklist);
        showlist(gamemode, head, hranklist);
        break;
    case deletecancelbuttonID:
        DestroyWindow(hWnd);
        break;
    case addokbuttonID:
        score = timeend - *timebegin;                           //计算时间
        head = loadlist();                                      //创建链表
        listinsert(gamemode, &head, score, haddtext);           //链表插入
        listsort(gamemode, head);                               //链表排序
        listsave(head);                                         //链表保存
        freelist(&head);                                        //链表释放
        DestroyWindow(hWnd);
        starter();
        break;
    default:
        DefWindowProc(hWnd, message, wParam, lParam);           //其他交给系统处理
    }

}




////////////////////////////////////////////////////////////////////游戏部分/////////////////////////////////////////////////////////////

void starter(void)                      //开始窗口
{
    HWND hstarter;

    hstarter = CreateWindowEx(WS_EX_CLIENTEDGE, szWindowClass, L"开始", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 300, 400,
        NULL, NULL, hInst, NULL);

    if (hstarter == NULL)           //判断创建是否成功
    {
        MessageBox(NULL, L"创建窗口失败", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    CreateWindow(L"Button", L"开始游戏", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 50, 50, 180, 50, hstarter, (HMENU)gamebuttonID, hInst, NULL);
    CreateWindow(L"Button", L"英雄榜", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 50, 150, 180, 50, hstarter, (HMENU)ranklistbuttonID, hInst, NULL);
    CreateWindow(L"Button", L"退出游戏", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 50, 250, 180, 50, hstarter, (HMENU)exitbuttonID, hInst, NULL);

    ShowWindow(hstarter, SW_NORMAL);
    UpdateWindow(hstarter);
}


void gamemodeselect(void)
{
    HWND hgamemodeselect = CreateWindowEx(WS_EX_CLIENTEDGE, szWindowClass, L"选择难度", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 300, 400,
        NULL, NULL, hInst, NULL);

    if (hgamemodeselect == NULL)                //判断是否创建成功
    {
        MessageBox(NULL, L"创建窗口失败", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    CreateWindow(L"Button", L"简单", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 50, 50, 180, 50, hgamemodeselect, (HMENU)easybuttonID, hInst, NULL);
    CreateWindow(L"Button", L"普通", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 50, 150, 180, 50, hgamemodeselect, (HMENU)normalbuttonID, hInst, NULL);
    CreateWindow(L"Button", L"困难", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 50, 250, 180, 50, hgamemodeselect, (HMENU)hardbuttonID, hInst, NULL);


    ShowWindow(hgamemodeselect, SW_NORMAL);
    UpdateWindow(hgamemodeselect);
}


HWND game(int gamemode, int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int *row, int *col, int *bomb)
{
    HWND hgame;

    switch (gamemode)
    {
    case 1:*row = 9; *col = 9; *bomb = 10; break;
    case 2:*row = 16; *col = 16; *bomb = 40; break;
    case 3:*row = 16; *col = 30; *bomb = 99; break;
    }

    boardreset(real, cover, *row, *col);
    bombplant(real, *row, *col, *bomb);

    hgame = CreateWindowEx(WS_EX_CLIENTEDGE, szWindowClass, L"扫雷", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        blocksidelength * (*col) + 100, blocksidelength * (*row) + 200, NULL, NULL, hInst, NULL);
    if (hgame == NULL)                //判断是否创建成功
    {
        MessageBox(NULL, L"创建窗口失败", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    CreateWindow(L"static", L"左键点击翻开", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 10, 100, 20, hgame, NULL, hInst, NULL);
    CreateWindow(L"static", L"右键点击标记", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 30, 100, 20, hgame, NULL, hInst, NULL);
    CreateWindow(L"static", L"左键点击数字翻开一圈", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 50, 150, 20, hgame, NULL, hInst, NULL);

    ShowWindow(hgame, SW_NORMAL);
    UpdateWindow(hgame);

    return hgame;
}


void boardreset(int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int row, int col)         //重置数组
{
    int i, j;
    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            real[i][j] = 0;
            cover[i][j] = '*';
        }
    }
}


void coverdisplay(HWND hgame, char cover[MAXROW][MAXCOL], int row, int col, int bomb, int timebegin, int item)              //游戏界面绘制
{
    int i, j, fcnt = 0;
    HDC hdc = GetDC(hgame);             //获得窗口dc
    char tmpstr[2] = { 0 };
    wchar_t* tmpwstr;
    wchar_t numwstr[101] = { 0 };
    wchar_t timewstr[10000] = { 0 };
    wchar_t itemwstr[100] = { 0 };
    HBRUSH hBrush;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            if (cover[i][j] == '*')
            {
                hBrush = CreateSolidBrush(RGB(192, 192, 192));                  //选择画刷颜色
                SelectObject(hdc, hBrush);                                      //将画刷用到窗口dc
                Rectangle(hdc, 40 + j * blocksidelength, 100 + i * blocksidelength, 40 + (j + 1) * blocksidelength, 100 + (i + 1) * blocksidelength);       //画正方形
                DeleteObject(hBrush);                                       //删除画刷
            }
            if (cover[i][j] == ' ')
            {
                hBrush = CreateSolidBrush(RGB(255, 255, 255));
                SelectObject(hdc, hBrush);
                Rectangle(hdc, 40 + j * blocksidelength, 100 + i * blocksidelength, 40 + (j + 1) * blocksidelength, 100 + (i + 1) * blocksidelength);
                DeleteObject(hBrush);
            }
            if (cover[i][j] == 'F')
            {
                hBrush = CreateSolidBrush(RGB(0, 0, 0));
                SelectObject(hdc, hBrush);
                Rectangle(hdc, 40 + j * blocksidelength, 100 + i * blocksidelength, 40 + (j + 1) * blocksidelength, 100 + (i + 1) * blocksidelength);
                DeleteObject(hBrush);
                if (fcnt < bomb)
                {
                    fcnt++;                     //标记的雷数计数
                }
            }
            if (cover[i][j] >= '0' && cover[i][j] <= '9')
            {
                hBrush = CreateSolidBrush(RGB(255, 255, 255));
                SelectObject(hdc, hBrush);
                Rectangle(hdc, 40 + j * blocksidelength, 100 + i * blocksidelength, 40 + (j + 1) * blocksidelength, 100 + (i + 1) * blocksidelength);
                DeleteObject(hBrush);

                SetBkMode(hdc, TRANSPARENT);                    //设置背景模式
                SetTextColor(hdc, RGB(0, 0, 0));                //设置文本颜色
                tmpstr[0] = cover[i][j];
                tmpwstr = chartowchar(tmpstr);
                TextOut(hdc, 40 + j * blocksidelength + 9, 100 + i * blocksidelength + 8, tmpwstr, 1);
            }
            if (cover[i][j] == '!')
            {
                hBrush = CreateSolidBrush(RGB(255, 0, 0));
                SelectObject(hdc, hBrush);
                Rectangle(hdc, 40 + j * blocksidelength, 100 + i * blocksidelength, 40 + (j + 1) * blocksidelength, 100 + (i + 1) * blocksidelength);
                DeleteObject(hBrush);
                if (fcnt < bomb)
                {
                    fcnt++;
                }
            }
            if (cover[i][j] == 'R')
            {
                hBrush = CreateSolidBrush(RGB(255, 255, 0));
                SelectObject(hdc, hBrush);
                Rectangle(hdc, 40 + j * blocksidelength, 100 + i * blocksidelength, 40 + (j + 1) * blocksidelength, 100 + (i + 1) * blocksidelength);
                DeleteObject(hBrush);
            }
        }
    }

    SetBkMode(hdc, OPAQUE);
    SetTextColor(hdc, RGB(0, 0, 0));
    wsprintf(numwstr, L"%d", bomb - fcnt);
    TextOut(hdc, (col - 2) * blocksidelength, 50, L"剩余雷数：", 5);
    TextOut(hdc, col * blocksidelength + 10, 50, L"    ", 4);
    TextOut(hdc, col * blocksidelength + 10, 50, numwstr, wcslen(numwstr));

    wsprintf(timewstr, L"%d", (int)time(NULL) - timebegin);
    TextOut(hdc, (col - 2) * blocksidelength, 20, L"时间：", 3);
    TextOut(hdc, (col - 1) * blocksidelength + 10, 20, L"    ", 4);
    TextOut(hdc, (col - 1) * blocksidelength + 10, 20, timewstr, wcslen(timewstr));

    if (item != 0)                      //显示道具数
    {
        wsprintf(itemwstr, L"%d", item);
        TextOut(hdc, (col - 4) * blocksidelength, 75, L"道具数：", 4);
        TextOut(hdc, (col - 3) * blocksidelength + 15, 75, L"    ", 4);
        TextOut(hdc, (col - 3) * blocksidelength + 15, 75, itemwstr, wcslen(itemwstr));
    }
    else
    {
        TextOut(hdc, (col - 4) * blocksidelength, 75, L"              ", 14);
    }

    ReleaseDC(hgame, hdc);                                      //与前面的GETDC对应，释放hdc
}


wchar_t* chartowchar(const char* str)                       //宽字符的转换
{
    int length = strlen(str) + 1;
    wchar_t* t = (wchar_t*)malloc(sizeof(wchar_t) * length);
    memset(t, 0, length * sizeof(wchar_t));                 //改变指定内存空间的值
    MultiByteToWideChar(CP_ACP, 0, str, strlen(str), t, length);
    return t;
}


void bombplant(int real[MAXROW][MAXCOL], int row, int col, int bomb)            //随机埋雷
{
    int x = 0, y = 0;
    int cnt = bomb;
    while (cnt)
    {
        x = rand() % row;
        y = rand() % col;
        if (real[x][y] == 0)
        {
            real[x][y] = 1;
            cnt--;
        }
    }
}

int locationvalue(int x, int y, int real[MAXROW][MAXCOL], int row, int col)             //计算格子上对应的值
{
    int cnt = 0;
    int i, j;

    for (i = x - 1; i <= x + 1; i++)
    {
        for (j = y - 1; j <= y + 1; j++)
        {
            if (i >= 0 && i < row && j >= 0 && j < col)      //判断有没有越界
            {
                if (i != x || j != y)                        //去掉自己
                {
                    if (real[i][j] == 1)
                    {
                        cnt++;
                    }
                }
            }
        }
    }

    return cnt;
}

void takestepleft(int x, int y, int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int row, int col, int *judge, int *item)                             //左键点击翻开格子
{
    int i, j, cnt = 0;
    
    if (cover[x][y] == '*')                 //点击的是未翻开的格子
    {
        discover(x, y, real, cover, row, col, judge, item);
    }
    else if (cover[x][y] >= '1' && cover[x][y] <= '9')              //点击的有数字的格子
    {
        cnt = 0;
        for (i = x - 1; i <= x + 1; i++)                //检查周围的标记数
        {
            for (j = y - 1; j <= y + 1; j++)
            {
                if (i >= 0 && i < row && j >= 0 && j < col)      //判断有没有越界
                {
                    if (i != x || j != y)                        //去掉自己
                    {
                        if (cover[i][j] == 'F' || cover[i][j] == '!')
                        {
                            cnt++;
                        }
                    }
                }
            }
        }
        if (cnt == cover[x][y] - '0')               //如果标记数满足条件翻开周围一圈
        {
            for (i = x - 1; i <= x + 1; i++)
            {
                for (j = y - 1; j <= y + 1; j++)
                {
                    if (i >= 0 && i < row && j >= 0 && j < col)      //判断有没有越界
                    {
                        if (i != x || j != y)                        //去掉自己
                        {
                            if (cover[i][j] == '*')
                            {
                                discover(i, j, real, cover, row, col, judge, item);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (cover[x][y] == 'R')                //如果点击的是有道具的格子
    {
        (*item)++;
        cover[x][y] = ' ';
    }


}

void discover(int x, int y, int real[MAXROW][MAXCOL], char cover[MAXROW][MAXCOL], int row, int col, int *judge, int *item)              //翻开一个格子
{
    int i, j;

    if (real[x][y] == 1)            //如果翻开的格子有雷
    {
        cover[x][y] = '!';
        if (*item == 0)
        {
            *judge = 0;
        }
        else
        {
            (*item)--;
        }
    }
    else
    {
        cover[x][y] = locationvalue(x, y, real, row, col) + '0';            //计算格子的数值
        if (cover[x][y] == '0')                     //如果是空的格子翻开这个格子的周围一圈
        {
            for (i = x - 1; i <= x + 1; i++)
            {
                for (j = y - 1; j <= y + 1; j++)
                {
                    if (i >= 0 && i < row && j >= 0 && j < col)
                    {
                        if (i != x || j != y)
                        {
                            if (cover[i][j] == '*')
                            {
                                discover(i, j, real, cover, row, col, judge, item);         //递归调用
                            }
                        }
                    }
                }
            }
            cover[x][y] = ' ';
        }
    }
}


void takestepright(int x, int y, char cover[MAXROW][MAXCOL])                //右键点击标雷
{
    if (cover[x][y] == '*')
    {
        cover[x][y] = 'F';
    }
    else if(cover[x][y] == 'F')
    {
        cover[x][y] = '*';
    }
}


void wincheck(int *judge, char cover[MAXROW][MAXCOL], int row, int col, int bomb)           //检查游戏是否已经胜利
{
    int i, j, count = 0;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            if (cover[i][j] == '*' || cover[i][j] == 'F' || cover[i][j] == '!')
            {
                count++;
            }
        }
    }

    if (count == bomb)
    {
        *judge = 2;
    }
}


void itemcreate(char cover[MAXROW][MAXCOL], int row, int col)               //创建道具
{
    int i, j, cnt = 0, x, y;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            if (cover[i][j] == ' ')
            {
                cnt++;
            }
        }
    }
    if (cnt != 0)
    {
        if (rand() % 50 == 0)                   //1/50的概率生成一个道具
        {
            while (1)
            {
                x = rand() % row;
                y = rand() % col;
                if (cover[x][y] == ' ')
                {
                    cover[x][y] = 'R';
                    break;
                }
            }
        }
    }
}





///////////////////////////////////////////////////////////////////////////////////链表处理部分////////////////////////////////////////////////////////////////////////////////

HWND ranklist(int gamemode)             //创建链表窗口
{
    HWND hranklist;
    
    hranklist = CreateWindowEx(WS_EX_CLIENTEDGE, szWindowClass, L"英雄榜", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInst, NULL);

    if (hranklist == NULL)
    {
        MessageBox(NULL, L"创建窗口失败", L"错误", MB_OK | MB_ICONERROR);
        return NULL;
    }

    CreateWindow(L"Button", L"删除一个", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 60, 480, 80, 25, hranklist, (HMENU)deleteonebuttonID, hInst, NULL);
    CreateWindow(L"Button", L"删除全部", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 350, 480, 80, 25, hranklist, (HMENU)deleteallbuttonID, hInst, NULL);
    CreateWindow(L"Button", L"返  回", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 640, 480, 80, 25, hranklist, (HMENU)returnbuttonID, hInst, NULL);

    ShowWindow(hranklist, SW_NORMAL);
    //UpdateWindow(hranklist);

    return hranklist;
}




struct player* loadlist(void)
{
    struct player* head = NULL, * p1, * p2;
    int n = 0;
    FILE* fp;
    _wsetlocale(0, L"chs");                                             //处理中文字编码
    if ((fp = _wfopen(L"Ranking List.txt", L"r")) == NULL)
    {
        MessageBox(NULL, L"cannot open this file!", L"ERROR", MB_OK | MB_ICONERROR);
        exit(0);
    }

    p2 = p1 = (struct player*)malloc(sizeof(struct player));
    while (fwscanf(fp, L"%d%s%d", &p1->gamemode, p1->name, &p1->score) == 3)
    {
        if (n == 0)
        {
            head = p1;
        }
        else
        {
            p2->next = p1;
        }
        p2 = p1;
        p1 = (struct player*)malloc(sizeof(struct player));
        n++;
    }
    p2->next = NULL;
    free(p1);
    fclose(fp);

    return head;
}


void showlist(int gamemode, struct player *head, HWND hranklist)            //绘制英雄榜窗口
{
    struct player* p;
    int cnt = 0;
    int i = 0;
    int textstart = 130;
    wchar_t tmp[1000];
    HDC hdc = GetDC(hranklist);

    p = head;
    switch (gamemode)
    {
    case 1:TextOut(hdc, 350, 50, L"简单", 2); break;
    case 2:TextOut(hdc, 350, 50, L"普通", 2); break;
    case 3:TextOut(hdc, 350, 50, L"困难", 2); break;
    }
    while (p)           //遍历链表找的相应难度的部分
    {
        if (p->gamemode == gamemode)
        {
            cnt++;
        }
        p = p->next;
    }
    if (cnt == 0)
    {
        TextOut(hdc, 350, 100, L"无", 1);
    }
    else
    {
        TextOut(hdc, 200, 100, L"排名", 2);
        TextOut(hdc, 300, 100, L"姓名", 2);
        TextOut(hdc, 500, 100, L"成绩(秒)", 5);
        p = head;
        i = 0;
        while (p != NULL)
        {
            if (p->gamemode == gamemode)
            {
                i++;
                wsprintf(tmp, L"%d", i);
                TextOut(hdc, 200, textstart, tmp, wcslen(tmp));
                TextOut(hdc, 300, textstart, p->name, wcslen(p->name));
                wsprintf(tmp, L"%d", p->score);
                TextOut(hdc, 500, textstart, tmp, wcslen(tmp));
                textstart += 20;
            }
            p = p->next;
        }
    }


    ReleaseDC(hranklist, hdc);

}


void listsave(struct player *head)              //将链表保存到文件
{
    FILE* fp;
    struct player* p = head;
    _wsetlocale(0, L"chs");
    if ((fp = _wfopen(L"Ranking List.txt", L"w")) == NULL)
    {
        MessageBox(NULL, L"cannot open this file!", L"ERROR", MB_OK | MB_ICONERROR);
        exit(0);
    }
    while (p)
    {
        fwprintf(fp, L"%d %s %d\n", p->gamemode, p->name, p->score);
        p = p->next;
    }
    fclose(fp);
}


void freelist(struct player **head)             //释放链表
{
    struct player* tmp;
    while (*head != NULL)
    {
        tmp = *head;
        *head = (*head)->next;
        free(tmp);
    }
}



HWND deleteone(void)                //创建删除一个的窗口
{
    HWND hwnd, hdeletetext;

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, szWindowClass, L"删除一个", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 200, NULL, NULL, hInst, NULL);
    if (hwnd == NULL)                //判断是否创建成功
    {
        MessageBox(NULL, L"创建窗口失败", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    CreateWindow(L"static", L"输入要删除玩家的排名号", WS_CHILD | WS_VISIBLE | SS_RIGHT, 95, 20, 170, 20, hwnd, NULL, hInst, NULL);
    hdeletetext = CreateWindow(L"Edit", TEXT("(在这里输入)"), WS_CHILD | WS_VISIBLE | ES_LEFT, 140, 60, 100, 50, hwnd, (HMENU)8, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
    CreateWindow(L"Button", L"确定", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 100, 50, 25, hwnd, (HMENU)deleteokbuttonID, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
    CreateWindow(L"Button", L"取消", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 250, 100, 50, 25, hwnd, (HMENU)deletecancelbuttonID, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

    ShowWindow(hwnd, SW_NORMAL);
    UpdateWindow(hwnd);

    return hdeletetext;
}


void playerdelete(int gamemode, struct player **head, HWND hdeletetext)             //删除数据
{
    wchar_t tmpstr[100];
    int num, cnt = 0, i;
    struct player* p = *head, *q, *tmp = NULL;

    GetWindowText(hdeletetext, tmpstr, 99);             //获取指定窗口的数据
    num = _wtoi(tmpstr);

    while (p != NULL && p->gamemode <= gamemode)     //先判断NULL
    {
        if (p->gamemode == gamemode)
        {
            cnt++;
        }
        p = p->next;
    }
    if (num >= 1 && num <= cnt)             //判断输入的数值是否有效
    {
        q = p = *head;
        i = 0;
        while (i < cnt)
        {
            if (p->gamemode == gamemode)
            {
                i++;
                if (i == num)
                {
                    if (p == *head)             //如果删除的是第一个数据，就要改变链表头指针
                    {
                        tmp = p;
                        *head = p->next;
                    }
                    else
                    {
                        tmp = p;
                        q->next = p->next;
                    }
                }
            }
            q = p;
            p = p->next;
        }
        free(tmp);
    }
    else
    {
        MessageBox(NULL, L"输入错误", L"ERROR", MB_OK | MB_ICONINFORMATION);
    }

}

void deleteall(int gamemode, struct player **head)              //删除链表全部数据
{
    struct player * p = *head, * q, * tmp = NULL;

    q = p = *head;
    while (p != NULL && p->gamemode != gamemode)
    {
        q = p;
        p = p->next;
    }
    while (p != NULL && p->gamemode == gamemode)
    {
        if (p == *head)                                     //如果是第一个数据，需要改变链表头指针
        {
            *head = p->next;
            free(q);
            q = p = *head;
        }
        else
        {
            p = p->next;
            free(q->next);
            q->next = p;
        }
    }

}


HWND win(void)                  //创建胜利界面
{
    HWND hwnd, haddtext;

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, szWindowClass, L"你赢了", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 200, NULL, NULL, hInst, NULL);
    if (hwnd == NULL)                //判断是否创建成功
    {
        MessageBox(NULL, L"创建窗口失败", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    CreateWindow(L"static", L"你赢了！", WS_CHILD | WS_VISIBLE | SS_RIGHT, 130, 10, 70, 20, hwnd, NULL, hInst, NULL);
    CreateWindow(L"static", L"你的成绩会被记录，输入你的名字", WS_CHILD | WS_VISIBLE | SS_RIGHT, 60, 40, 230, 20, hwnd, NULL, hInst, NULL);
    haddtext = CreateWindow(L"Edit", TEXT("(在这里输入)"), WS_CHILD | WS_VISIBLE | ES_LEFT, 130, 80, 100, 50, hwnd, (HMENU)8, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
    CreateWindow(L"Button", L"确定", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 300, 100, 50, 25, hwnd, (HMENU)addokbuttonID, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

    ShowWindow(hwnd, SW_NORMAL);
    UpdateWindow(hwnd);

    return haddtext;
}


void listinsert(int gamemode, struct player **head, int score, HWND haddtext)               //链表数据插入
{
    wchar_t tmpstr[100];
    struct player* p, * p1, * p2;

    GetWindowText(haddtext, tmpstr, 99);            //获取指定文本框的数据

    p = (struct player*)malloc(sizeof(struct player));
    wcscpy(p->name, tmpstr);
    p->gamemode = gamemode;
    p->score = score;
    p->next = NULL;


    p2 = p1 = *head;
    while (p1 != NULL && p1->gamemode <= gamemode )          //插入到指定难度数据的最后
    {
        p2 = p1;
        p1 = p1->next;
    }
    if (p1 == *head)                //如果插入的是链表第一个位置，就要改变头指针
    {
        p->next = p1;
        *head = p;
    }
    else
    {
        p2->next = p;
        p->next = p1;
    }
}

void listsort(int gamemode, struct player *head)                //链表的排序（只是复制数据，没有改变链表结构）
{
    struct player* p, * q;
    int tmpscore;
    wchar_t tmpname[1000];

    p = head;
    while (p)
    {
        q = p->next;                                         //指向NULL的指针取不到->score
        while (q != NULL && q->gamemode == p->gamemode)      //必须要先判断q是否为NULL，否则程序会出错!!
        {
            if (q->score <= p->score)
            {
                wcscpy(tmpname, p->name);                       //数据复制
                wcscpy(p->name, q->name);
                wcscpy(q->name, tmpname);
                tmpscore = p->score;
                p->score = q->score;
                q->score = tmpscore;
            }
            q = q->next;
        }

        p = p->next;
    }
}