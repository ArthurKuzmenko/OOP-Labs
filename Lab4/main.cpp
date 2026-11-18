#include <windows.h>
#include "editor.h"

//вар11(непарний):глобальний статичний об'єкт класу MyEditor
static MyEditor g_editor;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_editor.Init(hInstance);

    if (!g_editor.Create(L"Графічний редактор(Лаб4)", WS_OVERLAPPEDWINDOW)) {
        return 0;
    }

    ShowWindow(g_editor.Window(), nCmdShow);
    UpdateWindow(g_editor.Window());

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}