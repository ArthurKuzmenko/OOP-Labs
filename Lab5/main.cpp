#include <windows.h>
#include "editor.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    //вар11:Singleton Меєрса
    MyEditor& editor = MyEditor::getInstance();
    editor.Init(hInstance);

    if (!editor.Create(L"Графічний редактор (Лаб5)", WS_OVERLAPPEDWINDOW)) {
        return 0;
    }

    ShowWindow(editor.Window(), nCmdShow);
    UpdateWindow(editor.Window());

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}