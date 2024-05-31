#include <AppKit/AppKit.h>
#include "ImFileDialog_osx.hpp"
#include <filesystem>
#import <Foundation/Foundation.h>
#include <sys/stat.h>

namespace ifd {

void Beep() {
  @autoreleasepool {
    NSBeep();
  }
}

struct FileInfoOsx::details {
  NSImage *icon;
  struct stat fileInfo{};
};

FileInfoOsx::FileInfoOsx() {
  m_details = new details();
  m_details->icon = nullptr;
}

FileInfoOsx::FileInfoOsx(std::string apath): FileInfoOsx()  {
  std::error_code ec;

  if (std::filesystem::exists(apath, ec)) {
    if (stat(apath.c_str(), &m_details->fileInfo) == -1) return;
    m_details->icon = [[NSWorkspace sharedWorkspace] iconForFile:[NSString stringWithUTF8String:apath.c_str()]];
  } else {
    if (stat("/bin", &m_details->fileInfo) == -1) return;
    m_details->icon = [[NSWorkspace sharedWorkspace] iconForFile:@"/bin"];
  }
}

FileInfoOsx::~FileInfoOsx() {
  delete m_details;
}

bool FileInfoOsx::HasIcon() {
  return m_details->icon != nullptr;
}

int FileInfoOsx::GetINode() {
  return (int)m_details->fileInfo.st_ino;
}

void * FileInfoOsx::GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture) {
  void * icondata{};
  [m_details->icon lockFocus];
  NSUInteger width = DEFAULT_ICON_SIZE;
  NSUInteger height = DEFAULT_ICON_SIZE;
  [m_details->icon setSize:NSMakeSize(width, height)];
  NSBitmapImageRep *imageRep = [[NSBitmapImageRep alloc] initWithCGImage:[m_details->icon
  CGImageForProposedRect:nullptr context:nullptr hints:nullptr]];
  [imageRep setSize:[m_details->icon size]];
  NSData *data = [imageRep TIFFRepresentation];
  CGImageSourceRef source = CGImageSourceCreateWithData((CFDataRef)data, nullptr);
  CGImageRef imageRef =  CGImageSourceCreateImageAtIndex(source, 0, nullptr);
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  unsigned char *rawData = (unsigned char *)calloc(height * width * 4, sizeof(unsigned char));

  if (rawData) {
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = 8;
    CGContextRef context = CGBitmapContextCreate(rawData, width, height,
    bitsPerComponent, bytesPerRow, colorSpace,
    kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
    CGContextRelease(context);
    unsigned char *invData = (unsigned char *)calloc(height * width * 4, sizeof(unsigned char));
    if (invData) {
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          int index = (y * width + x) * 4;
          invData[index + 2] = rawData[index + 0];
          invData[index + 1] = rawData[index + 1];
          invData[index + 0] = rawData[index + 2];
          invData[index + 3] = rawData[index + 3];
        }
      }
      icondata = createTexture(invData, width, height, 0);
      free(invData);
    }
    free(rawData);
  }
  [imageRep release];
  CFRelease(source);
  CFRelease(imageRef);
  CFRelease(colorSpace);
  [m_details->icon unlockFocus];

  return icondata;
}
}