//
// Created by Bùi Đặng Bình on 19/5/25.
//

#include "macos_titlebar.h"
#include "mainwindow.h"
#include "utils.h"

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <QWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QTimer>

// Helper function to get NSWindow from a Qt MainWindow
NSWindow* getNSWindowFromMainWindow(MainWindow* mainWindow) {
    // Get QWindow from QMainWindow
    QWindow* qtWindow = mainWindow->windowHandle();
    if (!qtWindow) {
        // Window might not be created yet, force creation
        mainWindow->winId();
        qtWindow = mainWindow->windowHandle();
        if (!qtWindow) {
            qWarning() << "Failed to get QWindow from MainWindow";
            return nil;
        }
    }

    // Get window ID as NSView
    NSView* nsView = reinterpret_cast<NSView*>(qtWindow->winId());
    if (!nsView) {
        qWarning() << "Failed to get NSView from QWindow";
        return nil;
    }

    // Get NSWindow from NSView
    NSWindow* nsWindow = [nsView window];
    if (!nsWindow) {
        qWarning() << "Failed to get NSWindow from NSView";
        return nil;
    }

    return nsWindow;
}

// Objective-C++ class for button actions
@interface MacOSButtonActions : NSObject
@property (nonatomic, assign) MainWindow* mainWindow;

- (void)backButtonClicked:(id)sender;
- (void)forwardButtonClicked:(id)sender;
- (void)reloadButtonClicked:(id)sender;
@end

@implementation MacOSButtonActions
- (void)backButtonClicked:(id)sender {
    if (self.mainWindow) {
        self.mainWindow->goBack();
    }
}

- (void)forwardButtonClicked:(id)sender {
    if (self.mainWindow) {
        self.mainWindow->goForward();
    }
}

- (void)reloadButtonClicked:(id)sender {
    if (self.mainWindow) {
        self.mainWindow->reload();
    }
}
@end

// Static storage for Objective-C objects
namespace {
    MacOSButtonActions* buttonActions = nil;
}

// Helper to create a button with icon or text
NSButton* createToolbarButton(SEL action, id target, NSRect frame, NSString* fallbackText, NSString* symbolName) {
    NSButton* button = [[NSButton alloc] initWithFrame:frame];
    [button setTitle:@""];
    [button setBezelStyle:NSBezelStyleRegularSquare]; // More minimal style
    [button setBordered:NO]; // Remove border
    [button setAction:action];
    [button setTarget:target];

    // Make button transparent without borders
    [button setBordered:NO];
    [button setWantsLayer:YES];
    button.layer.backgroundColor = [NSColor clearColor].CGColor;

    BOOL usedSymbol = NO;

    // Try to use SF Symbol if available (macOS 11.0+)
    if (@available(macOS 11.0, *)) {
        NSImage* image = [NSImage imageWithSystemSymbolName:symbolName accessibilityDescription:nil];
        if (image) {
            // Create a basic configuration without using hierarchicalColor
            NSImageSymbolConfiguration* config = [NSImageSymbolConfiguration configurationWithPointSize:14 weight:NSFontWeightRegular scale:NSImageSymbolScaleMedium];
            image = [image imageWithSymbolConfiguration:config];

            // Set as template image and apply tint
            [image setTemplate:YES];
            [button setImage:image];
            [button setImagePosition:NSImageOnly];
            [button setContentTintColor:[NSColor whiteColor]];

            usedSymbol = YES;
        }
    }

    // If symbol not used, fall back to text
    if (!usedSymbol) {
        [button setTitle:fallbackText];

        // Create attributed title for color
        NSMutableAttributedString* attrTitle = [[NSMutableAttributedString alloc]
            initWithString:[button title]];
        [attrTitle addAttribute:NSForegroundColorAttributeName
            value:[NSColor whiteColor]
            range:NSMakeRange(0, [attrTitle length])];
        [button setAttributedTitle:attrTitle];
    }

    return button;
}

void MacOSTitleBar::setupToolbar(MainWindow* mainWindow) {
    // Setup delayed initialization to ensure the window is fully created
    QTimer::singleShot(100, [mainWindow]() {
        // Get NSWindow
        NSWindow* nsWindow = getNSWindowFromMainWindow(mainWindow);
        if (!nsWindow) {
            qWarning() << "Could not get NSWindow";
            return;
        }

        // Configure window for custom title bar
        nsWindow.titlebarAppearsTransparent = YES;
        [nsWindow setStyleMask:[nsWindow styleMask] | NSWindowStyleMaskFullSizeContentView];

        // Get close button and title bar view
        NSButton* closeButton = [nsWindow standardWindowButton:NSWindowCloseButton];
        if (!closeButton) {
            qWarning() << "Could not get close button";
            return;
        }

        NSView* titleBarView = closeButton.superview;
        if (!titleBarView) {
            qWarning() << "Could not get title bar view";
            return;
        }

        // Initialize button handler
        if (!buttonActions) {
            buttonActions = [[MacOSButtonActions alloc] init];
        }
        buttonActions.mainWindow = mainWindow;

        // Calculate positions based on titleBarView size
        CGFloat titleBarHeight = titleBarView.frame.size.height;
        CGFloat buttonHeight = titleBarHeight * 0.7;
        CGFloat buttonY = (titleBarHeight - buttonHeight) / 2;
        CGFloat currentX = 70; // Starting X position (adjust as needed)

        // Create back button
        NSButton* backButton = createToolbarButton(@selector(backButtonClicked:),
                                               buttonActions,
                                               NSMakeRect(currentX, buttonY, buttonHeight, buttonHeight),
                                               @"◀",
                                               @"chevron.backward");
        [titleBarView addSubview:backButton];
        currentX += buttonHeight + 5;

        // Create forward button
        NSButton* forwardButton = createToolbarButton(@selector(forwardButtonClicked:),
                                                  buttonActions,
                                                  NSMakeRect(currentX, buttonY, buttonHeight, buttonHeight),
                                                  @"▶",
                                                  @"chevron.forward");
        [titleBarView addSubview:forwardButton];
        currentX += buttonHeight + 5;

        // Create reload button
        NSButton* reloadButton = createToolbarButton(@selector(reloadButtonClicked:),
                                                 buttonActions,
                                                 NSMakeRect(currentX, buttonY, buttonHeight, buttonHeight),
                                                 @"↻",
                                                 @"arrow.clockwise");
        [titleBarView addSubview:reloadButton];

        // Make Qt aware of the custom titlebar height for layout calculations
        mainWindow->setContentsMargins(0, 0, 0, 0);
    });
}
