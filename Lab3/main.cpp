#include <windows.h>
#include "editor.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    //створює екземпляр класу-редактора
    EditorWindow editor(hInstance);

    //створює вікно
    if (!editor.Create(L"Графічний редактор", WS_OVERLAPPEDWINDOW)) {
        return 0;
    }

    //показує вікно
    ShowWindow(editor.Window(), nCmdShow);
    UpdateWindow(editor.Window());

    //головний цикл обробки повідомлень
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}