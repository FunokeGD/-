// 包含Windows API的头文件，提供了许多Windows系统相关的函数和数据类型
#include <windows.h>
// 包含标准输入输出库的头文件，用于文件操作和格式化输入输出
#include <stdio.h>
// 包含标准库的头文件，提供了一些通用的函数，如内存分配、随机数生成等
#include <stdlib.h>
// 包含时间库的头文件，用于获取系统时间，常用于随机数种子的设置
#include <time.h>
// 包含字符串处理库的头文件，用于字符串的操作，如复制、查找等
#include <string.h>
// 包含本地化设置库的头文件，用于处理不同地区的字符编码和格式
#include <locale.h>

// 定义最大名字数量为100000
#define MAX_NAMES 100000
// 定义每个名字的最大长度为100
#define MAX_NAME_LENGTH 100
// 定义点名按钮的ID为1
#define ID_BUTTON 1
// 定义显示名字的标签的ID为2
#define ID_NAME_LABEL 2
// 定义关闭按钮的ID为3
#define ID_CLOSE_BUTTON 3
// 定义最小化按钮的ID为4
#define ID_MINIMIZE_BUTTON 4
// 定义显示名字的最大宽度为800像素
#define MAX_NAME_DISPLAY_WIDTH 800
// 定义显示名字的最大高度为60像素
#define MAX_NAME_DISPLAY_HEIGHT 60

// 二维数组，用于存储名字，最多可存储MAX_NAMES个名字，每个名字最长为MAX_NAME_LENGTH
wchar_t names[MAX_NAMES][MAX_NAME_LENGTH];
// 记录当前存储的名字数量
int name_count = 0;
// 记录上一次随机选中的名字的索引，初始值为-1
int last_index = -1;

// 将UTF-8编码的字符串转换为宽字符字符串
void utf8_to_wchar(const char* utf8_str, wchar_t* wchar_str, size_t max_length) {
    // 计算UTF-8字符串转换为宽字符字符串所需的长度
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    // 如果转换后的长度大于0且小于最大长度
    if (len > 0 && len < (int)max_length) {
        // 进行实际的转换
        MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, wchar_str, len);
        // 在宽字符字符串末尾添加终止符
        wchar_str[len - 1] = L'\0';
    }
}

// 从文件中读取名字
void read_names_from_file() {
    // 设置当前的本地化环境，使用系统默认设置
    setlocale(LC_ALL, "");
    // 以只读模式打开名为"names.txt"的文件
    FILE* file = fopen("names.txt", "r");
    // 如果文件打开失败
    if (file == NULL) {
        // 直接返回
        return;
    }
    // 用于存储从文件中读取的UTF-8编码的名字
    char utf8_name[MAX_NAME_LENGTH * 3];
    // 循环读取文件中的每一行，直到文件结束或达到最大名字数量
    while (fgets(utf8_name, sizeof(utf8_name), file) != NULL && name_count < MAX_NAMES) {
        // 查找换行符的位置
        char* newline = strchr(utf8_name, '\n');
        // 如果找到换行符
        if (newline) {
            // 将换行符替换为字符串终止符
            *newline = '\0';
        }
        // 将UTF-8编码的名字转换为宽字符字符串并存储到names数组中
        utf8_to_wchar(utf8_name, names[name_count], MAX_NAME_LENGTH);
        // 名字数量加1
        name_count++;
    }
    // 关闭文件
    fclose(file);
}

// 获取一个随机的名字索引
int get_random_index() {
    // 如果名字数量小于等于1
    if (name_count <= 1) {
        // 直接返回0
        return 0;
    }
    int random_index;
    // 循环生成随机索引，直到生成的索引与上一次不同
    do {
        // 生成一个0到name_count-1之间的随机索引
        random_index = rand() % name_count;
    } while (random_index == last_index);
    // 更新上一次的索引
    last_index = random_index;
    // 返回随机索引
    return random_index;
}

// 调整显示名字的字体高度，以适应显示区域
void adjust_font_height(HWND hwnd, const wchar_t* name) {
    // 获取窗口的设备上下文
    HDC hdc = GetDC(hwnd);
    // 初始字体高度为最大显示高度
    int font_height = MAX_NAME_DISPLAY_HEIGHT;
    HFONT hFont;
    SIZE text_size;

    // 循环调整字体高度，直到名字能在指定宽度内显示
    do {
        // 创建一个指定高度的字体
        hFont = CreateFontW(font_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
        // 选择新字体到设备上下文中，并保存原来的字体
        HFONT old_font = (HFONT)SelectObject(hdc, hFont);
        // 计算名字在当前字体下的显示大小
        GetTextExtentPoint32W(hdc, name, wcslen(name), &text_size);
        // 恢复原来的字体
        SelectObject(hdc, old_font);
        // 删除新创建的字体
        DeleteObject(hFont);

        // 如果名字的宽度超过最大显示宽度
        if (text_size.cx > MAX_NAME_DISPLAY_WIDTH) {
            // 字体高度减1
            font_height--;
        } else {
            // 跳出循环
            break;
        }
    } while (font_height > 1);

    // 创建最终合适的字体
    hFont = CreateFontW(font_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
    // 设置显示名字的标签使用新字体
    SendMessageW(GetDlgItem(hwnd, ID_NAME_LABEL), WM_SETFONT, (WPARAM)hFont, TRUE);
    // 释放设备上下文
    ReleaseDC(hwnd, hdc);
}

// 窗口过程函数，处理窗口消息
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // 根据不同的消息类型进行处理
    switch (msg) {
    case WM_CREATE: {
        // 定义一个矩形结构体，用于存储窗口的客户区矩形
        RECT clientRect;
        // 获取窗口的客户区矩形
        GetClientRect(hwnd, &clientRect);
        // 计算标签的宽度
        int labelWidth = clientRect.right - clientRect.left;
        // 定义标签的高度为80像素
        int labelHeight = 80;
        // 计算标签的垂直位置
        int labelY = (clientRect.bottom - clientRect.top - labelHeight) / 2 - 20;
        // 创建一个静态文本控件，用于显示名字
        CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER,
                      0, labelY, labelWidth, labelHeight, hwnd, (HMENU)ID_NAME_LABEL, NULL, NULL);
        // 创建一个字体
        HFONT hFont = CreateFontW(44, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
        // 设置显示名字的标签使用新字体
        SendMessageW(GetDlgItem(hwnd, ID_NAME_LABEL), WM_SETFONT, (WPARAM)hFont, TRUE);

        // 创建一个点名按钮
        CreateWindowW(L"BUTTON", L"点名", WS_CHILD | WS_VISIBLE,
                      400, 480, 200, 60, hwnd, (HMENU)ID_BUTTON, NULL, NULL);

        // 创建一个关闭按钮
        CreateWindowW(L"BUTTON", L"关闭", WS_CHILD | WS_VISIBLE,
                      930, 10, 60, 30, hwnd, (HMENU)ID_CLOSE_BUTTON, NULL, NULL);

        // 创建一个最小化按钮
        CreateWindowW(L"BUTTON", L"最小化", WS_CHILD | WS_VISIBLE,
                      10, 10, 60, 30, hwnd, (HMENU)ID_MINIMIZE_BUTTON, NULL, NULL);
        break;
    }
    case WM_COMMAND: {
        // 如果是点名按钮被点击
        if (LOWORD(wParam) == ID_BUTTON) {
            // 如果有名字可供选择
            if (name_count > 0) {
                // 获取一个随机的名字索引
                int random_index = get_random_index();
                // 在显示名字的标签中显示随机选中的名字
                SetDlgItemTextW(hwnd, ID_NAME_LABEL, names[random_index]);
                // 调整显示名字的字体高度
                adjust_font_height(hwnd, names[random_index]);
            }
        } 
        // 如果是关闭按钮被点击
        else if (LOWORD(wParam) == ID_CLOSE_BUTTON) {
            // 发送关闭窗口的消息
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
        } 
        // 如果是最小化按钮被点击
        else if (LOWORD(wParam) == ID_MINIMIZE_BUTTON) {
            // 最小化窗口
            ShowWindow(hwnd, SW_MINIMIZE);
        }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        // 如果是显示名字的标签的颜色设置消息
        if ((HWND)lParam == GetDlgItem(hwnd, ID_NAME_LABEL)) {
            // 设置标签的背景颜色为白色
            SetBkColor((HDC)wParam, RGB(255, 255, 255));
            // 设置标签的文本颜色为黑色
            SetTextColor((HDC)wParam, RGB(0, 0, 0));
            // 返回白色画刷，用于绘制标签的背景
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }
        break;
    }
    case WM_DESTROY:
        // 发送退出消息，终止消息循环
        PostQuitMessage(0);
        break;
    default:
        // 调用默认的窗口过程函数处理其他消息
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Windows程序的入口函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PSTR szCmdLine, int iCmdShow) {
    // 设置随机数种子，使用当前时间
    srand(time(NULL));
    // 从文件中读取名字
    read_names_from_file();

    // 定义一个窗口类结构体
    WNDCLASSW wc = { 0 };
    // 设置窗口过程函数
    wc.lpfnWndProc = WndProc;
    // 设置窗口类的实例句柄
    wc.hInstance = hInstance;
    // 设置窗口类的名称
    wc.lpszClassName = L"RandomRollCall";
    // 设置窗口的背景画刷
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    // 注册窗口类
    RegisterClassW(&wc);

    // 获取屏幕的宽度
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    // 获取屏幕的高度
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 定义窗口的宽度为1000像素
    int windowWidth = 1000;
    // 定义窗口的高度为595像素
    int windowHeight = 595;

    // 计算窗口的水平位置，使其居中显示
    int windowX = (screenWidth - windowWidth) / 2;
    // 计算窗口的垂直位置，使其居中显示
    int windowY = (screenHeight - windowHeight) / 2;

    // 创建窗口
    HWND hwnd = CreateWindowW(L"RandomRollCall", L"",
                              WS_POPUP, windowX, windowY, windowWidth, windowHeight,
                              NULL, NULL, hInstance, NULL);

    // 显示窗口
    ShowWindow(hwnd, iCmdShow);
    // 更新窗口
    UpdateWindow(hwnd);

    // 定义一个消息结构体
    MSG msg;
    // 循环获取消息，直到接收到退出消息
    while (GetMessageW(&msg, NULL, 0, 0)) {
        // 翻译消息
        TranslateMessage(&msg);
        // 分发消息到窗口过程函数处理
        DispatchMessageW(&msg);
    }
    // 返回消息的退出代码
    return msg.wParam;
}