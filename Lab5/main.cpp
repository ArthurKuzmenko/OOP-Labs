#include <windows.h>
#include "editor.h"
#include "my_table.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    //ва11:Singleton Меєрса
    MyEditor& editor = MyEditor::getInstance();
    MyTable table;

    //зв'язка MyEditor та MyTable у головному файлі
    editor.SetTable(&table);
    table.SetListener(&editor);

    editor.Init(hInstance);

    if (!editor.Create(L"Графічний редактор(Лаб5)", WS_OVERLAPPEDWINDOW)) {
        return 0;
    }

    ShowWindow(editor.Window(), nCmdShow);
    UpdateWindow(editor.Window());

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!editor.IsDialogMsg(&msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}