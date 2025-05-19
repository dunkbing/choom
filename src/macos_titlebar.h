//
// Created by Bùi Đặng Bình on 19/5/25.
//

#ifndef MACOS_TITLEBAR_H
#define MACOS_TITLEBAR_H

class MainWindow;

class MacOSTitleBar {
public:
    static void setupToolbar(MainWindow *mainWindow);
    static void hideWindowTitleBar(MainWindow *window);
};

#endif // MACOS_TITLEBAR_H
