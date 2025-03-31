#include <ApplicationServices/ApplicationServices.h>
#include <iostream>
#include <sstream>

void CaptureScreenshot(CGDirectDisplayID displayId, int screenNumber) {
    // Capture the screenshot of the specific display
    CGImageRef screenshot = CGDisplayCreateImage(displayId);
    if (!screenshot) {
        std::cerr << "Failed to capture screenshot for display " << screenNumber << "!" << std::endl;
        return;
    }

    // Generate filename dynamically
    std::ostringstream filename;
    filename << "screenshot_" << screenNumber << ".png";

    // Create URL for the file
    CFStringRef path = CFStringCreateWithCString(NULL, filename.str().c_str(), kCFStringEncodingUTF8);
    CFURLRef url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, false);

    // Create image destination
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
    if (!destination) {
        std::cerr << "Failed to create image destination for display " << screenNumber << "!" << std::endl;
        CGImageRelease(screenshot);
        CFRelease(path);
        CFRelease(url);
        return;
    }

    // Write image to file
    CGImageDestinationAddImage(destination, screenshot, NULL);
    if (!CGImageDestinationFinalize(destination)) {
        std::cerr << "Failed to save screenshot for display " << screenNumber << "!" << std::endl;
    } else {
        std::cout << "Screenshot saved as " << filename.str() << std::endl;
    }

    // Cleanup
    CFRelease(destination);
    CGImageRelease(screenshot);
    CFRelease(path);
    CFRelease(url);
}

int main() {
    const int MAX_DISPLAYS = 10;
    CGDirectDisplayID displays[MAX_DISPLAYS];
    uint32_t displayCount;

    // Get list of active displays
    CGGetActiveDisplayList(MAX_DISPLAYS, displays, &displayCount);
    std::cout << "Number of screens detected: " << displayCount << std::endl;

    // Capture each display
    for (uint32_t i = 0; i < displayCount; ++i) {
        CaptureScreenshot(displays[i], i + 1);
    }

    return 0;
}
