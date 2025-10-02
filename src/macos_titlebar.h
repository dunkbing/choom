//
// Created by Bùi Đặng Bình on 19/5/25.
//

#ifndef MACOS_TITLEBAR_H
#define MACOS_TITLEBAR_H

#ifdef __OBJC__
@class NSButton;
#else
typedef struct objc_object NSButton;
#endif

class MainWindow;

class MacOSTitleBar {
public:
    static void setupToolbar(MainWindow *mainWindow);
    static void hideWindowTitleBar(MainWindow *window);
    static NSButton* addSidebarToggleButton(MainWindow *mainWindow);
};

#endif // MACOS_TITLEBAR_H
