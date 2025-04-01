#include <ApplicationServices/ApplicationServices.h>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <memory>
#include "json/json.hpp"

const float THRESHOLD = 0.5;

std::string CaptureScreenshot(CGDirectDisplayID displayId, int screenNumber) {
    // Capture the screenshot of the specific display
    CGImageRef screenshot = CGDisplayCreateImage(displayId);
    if (!screenshot) {
        std::cerr << "Failed to capture screenshot for display " << screenNumber << "!" << std::endl;
        return "";
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
        return "";
    }

    // Write image to file
    CGImageDestinationAddImage(destination, screenshot, NULL);
    if (!CGImageDestinationFinalize(destination)) {
        std::cerr << "Failed to save screenshot for display " << screenNumber << "!" << std::endl;
        CFRelease(destination);
        CGImageRelease(screenshot);
        CFRelease(path);
        CFRelease(url);
        return "";
    }/* else {
        std::cout << "Screenshot saved as " << filename.str() << std::endl;
    }*/

    // Cleanup
    CFRelease(destination);
    CGImageRelease(screenshot);
    CFRelease(path);
    CFRelease(url);

    return filename.str();
}

int main() {
    const int MAX_DISPLAYS = 10;
    CGDirectDisplayID displays[MAX_DISPLAYS];
    uint32_t displayCount;

    // Get list of active displays
    CGGetActiveDisplayList(MAX_DISPLAYS, displays, &displayCount);
    //std::cout << "Number of screens detected: " << displayCount << std::endl;

    std::vector<std::string> imagePaths;

    // Capture each display
    for (uint32_t i = 0; i < displayCount; ++i) {
        std::string savedPath = CaptureScreenshot(displays[i], i + 1);

        if (!savedPath.empty()) {
            imagePaths.push_back(savedPath);
        }
    }

    for (uint32 i = 0; i < imagePaths.size(); i++) {
        std::string command = "./bonk/bonk -t 0.5 ./" + imagePaths[i];

        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            std::cerr << "popen() failed!" << std::endl;
            return 1;
        }
        
        std::string result;
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            result += buffer;
        }

        size_t offset = 0;
        json::JSON response = json::parse_object(result, offset);

        bool hasNudity = response.at("has_nudity").ToBool();

        json::JSON categories = response.at("predictions");

        for (const auto& category : categories.ArrayRange()) {

            std::string label = category.at("category").ToString();
            float prob = category.at("probability").ToFloat();

            if (label == "hentai" || label == "sexy" || label == "porn") {

                //std::cout << "Category: " << label << ", Probability: " << prob << std::endl;

                if (prob > THRESHOLD || hasNudity) {
                    std::cout << "Sending request to submit image" << std::endl;
                }
            }
        }
    }

    return 0;
}
