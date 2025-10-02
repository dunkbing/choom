//
// Created by Bùi Đặng Bình on 19/5/25.
//

#include "macos_titlebar.h"
#include "ui/mainwindow.h"
#include "core/utils.h"

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

// Helper function to get NSWindow from a Qt MainWindow
NSWindow *getNSWindowFromMainWindow(MainWindow *mainWindow) {
    // Get QWindow from QMainWindow
    QWindow *qtWindow = mainWindow->windowHandle();
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
    NSView *nsView = reinterpret_cast<NSView *>(qtWindow->winId());
    if (!nsView) {
        qWarning() << "Failed to get NSView from QWindow";
        return nil;
    }

    // Get NSWindow from NSView
    NSWindow *nsWindow = [nsView window];
    if (!nsWindow) {
        qWarning() << "Failed to get NSWindow from NSView";
        return nil;
    }

    return nsWindow;
}

// Custom button class that implements hover tracking
@interface HoverButton : NSButton
@property(nonatomic, strong) NSColor *normalColor;
@property(nonatomic, strong) NSColor *hoverColor;
@property(nonatomic, assign) BOOL isHovered;
@end

@implementation HoverButton
- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        _normalColor = [NSColor whiteColor];
        _hoverColor = [NSColor colorWithWhite:0.7 alpha:1.0]; // Light gray for hover state
        _isHovered = NO;

        // Enable tracking for mouse enter/exit events
        NSTrackingAreaOptions options =
            NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited;
        NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                                    options:options
                                                                      owner:self
                                                                   userInfo:nil];
        [self addTrackingArea:trackingArea];
    }
    return self;
}

- (void)updateTintColor {
    if (_isHovered) {
        [self setContentTintColor:_hoverColor];
    } else {
        [self setContentTintColor:_normalColor];
    }
}

- (void)mouseEntered:(NSEvent *)event {
    _isHovered = YES;
    [self updateTintColor];

    // Add subtle background highlight
    [self setWantsLayer:YES];
    self.layer.backgroundColor = [NSColor colorWithWhite:0.3 alpha:0.3].CGColor;
    self.layer.cornerRadius = 4.0;
}

- (void)mouseExited:(NSEvent *)event {
    _isHovered = NO;
    [self updateTintColor];

    // Remove background highlight
    self.layer.backgroundColor = [NSColor clearColor].CGColor;
}

@end

// Database client doesn't need browser navigation buttons
// Removed MacOSButtonActions class

// Helper to create a button with icon or text
HoverButton *createToolbarButton(SEL action, id target, NSRect frame, NSString *fallbackText,
                                 NSString *symbolName) {
    HoverButton *button = [[HoverButton alloc] initWithFrame:frame];
    [button setTitle:@""];
    [button setBezelStyle:NSBezelStyleRegularSquare]; // More minimal style
    [button setBordered:NO];                          // Remove border
    [button setAction:action];
    [button setTarget:target];

    // Make button transparent without borders
    [button setBordered:NO];
    [button setWantsLayer:YES];
    button.layer.backgroundColor = [NSColor clearColor].CGColor;

    BOOL usedSymbol = NO;

    // Try to use SF Symbol if available (macOS 11.0+)
    if (@available(macOS 11.0, *)) {
        NSImage *image = [NSImage imageWithSystemSymbolName:symbolName
                                   accessibilityDescription:nil];
        if (image) {
            // Create a basic configuration without using hierarchicalColor
            NSImageSymbolConfiguration *config =
                [NSImageSymbolConfiguration configurationWithPointSize:14
                                                                weight:NSFontWeightRegular
                                                                 scale:NSImageSymbolScaleMedium];
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
        NSMutableAttributedString *attrTitle =
            [[NSMutableAttributedString alloc] initWithString:[button title]];
        [attrTitle addAttribute:NSForegroundColorAttributeName
                          value:[NSColor whiteColor]
                          range:NSMakeRange(0, [attrTitle length])];
        [button setAttributedTitle:attrTitle];
    }

    return button;
}

@interface SidebarToggleTarget : NSObject
@property (nonatomic, assign) MainWindow *mainWindow;
- (void)toggleSidebar:(id)sender;
@end

@implementation SidebarToggleTarget
- (void)toggleSidebar:(id)sender {
    if (_mainWindow) {
        _mainWindow->toggleSidebar();
    }
}
@end

NSButton* MacOSTitleBar::addSidebarToggleButton(MainWindow *mainWindow) {
    NSWindow *nsWindow = getNSWindowFromMainWindow(mainWindow);
    if (!nsWindow) {
        return nil;
    }

    // Create target for button action
    static SidebarToggleTarget *target = [[SidebarToggleTarget alloc] init];
    target.mainWindow = mainWindow;

    // Create sidebar toggle button
    NSRect frame = NSMakeRect(0, 0, 30, 30);
    HoverButton *toggleButton = createToolbarButton(
        @selector(toggleSidebar:),
        target,
        frame,
        @"≡",
        @"sidebar.left"
    );

    // Add button to the leading side of the titlebar
    NSTitlebarAccessoryViewController *accessoryController = [[NSTitlebarAccessoryViewController alloc] init];
    accessoryController.view = toggleButton;
    accessoryController.layoutAttribute = NSLayoutAttributeLeading;
    [nsWindow addTitlebarAccessoryViewController:accessoryController];

    return toggleButton;
}

void MacOSTitleBar::setupToolbar(MainWindow *mainWindow) {
    // Setup delayed initialization to ensure the window is fully created
    QTimer::singleShot(100, [mainWindow]() {
        mainWindow->setUnifiedTitleAndToolBarOnMac(true);
        // Get NSWindow
        NSWindow *nsWindow = getNSWindowFromMainWindow(mainWindow);
        if (!nsWindow) {
            qWarning() << "Could not get NSWindow";
            return;
        }

        // Configure window for custom title bar
        nsWindow.titlebarAppearsTransparent = YES;
        [nsWindow setStyleMask:[nsWindow styleMask] | NSWindowStyleMaskFullSizeContentView];
        nsWindow.titleVisibility = NSWindowTitleHidden;
        nsWindow.backgroundColor = [NSColor clearColor];

        // Make Qt aware of the custom titlebar height for layout calculations
        mainWindow->setContentsMargins(0, 0, 0, 0);

        // Add sidebar toggle button
        MacOSTitleBar::addSidebarToggleButton(mainWindow);
    });
}

void MacOSTitleBar::hideWindowTitleBar(MainWindow *window) {
    QTimer::singleShot(100, [window]() {
        window->setUnifiedTitleAndToolBarOnMac(true);

        NSView *nativeView = reinterpret_cast<NSView *>(window->winId());
        NSWindow *nativeWindow = [nativeView window];

        [nativeWindow setStyleMask:[nativeWindow styleMask] | NSWindowStyleMaskFullSizeContentView |
                                   NSWindowTitleHidden];
        [nativeWindow setTitlebarAppearsTransparent:YES];
        [nativeWindow center];
    });
}
