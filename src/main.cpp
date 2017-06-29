/******************************************************************************
*  Project: NextGIS GL Viewer
*  Purpose: GUI viewer for spatial data.
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2016 NextGIS, <info@nextgis.com>
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 2 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include "version.h"

#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

////////////////////////////////////////
class CFStringWrap {
public:
    CFStringWrap(const std::string& str) {
        _cfStr = CFStringCreateWithBytes(NULL, (const UInt8*)str.c_str(), str.length(), kCFStringEncodingUTF8, FALSE);
    }

    CFStringRef cfString() const { return _cfStr; }

    ~CFStringWrap() {
        CFRelease(_cfStr);
        _cfStr = NULL;
    }

private:
    CFStringRef _cfStr;

};

class CFDictionaryWrap {
public:
    CFDictionaryWrap() {
        _cfDict = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    }

    CFMutableDictionaryRef cfDict() const { return _cfDict; }

    void set(CFStringRef key, CFTypeRef val) {
        CFDictionarySetValue(_cfDict, key, val);
    }

    ~CFDictionaryWrap() {
        CFRelease(_cfDict);
        _cfDict = NULL;
    }

private:
    CFMutableDictionaryRef _cfDict;
};

TISInputSourceRef findInputSourceByLang(const std::string& lang) {
    CFDictionaryWrap filter;
    filter.set(kTISPropertyInputSourceIsEnabled, kCFBooleanTrue);
    filter.set(kTISPropertyInputSourceIsSelectCapable, kCFBooleanTrue);

    CFArrayRef sourceList = TISCreateInputSourceList(filter.cfDict(), false);
    if (sourceList == NULL)
        return NULL;

    CFStringWrap requiredLangName(lang);

    TISInputSourceRef is = NULL;

    CFIndex cnt = CFArrayGetCount(sourceList);
    for (CFIndex i = 0; i < cnt; ++i) {
        TISInputSourceRef inputSource = (TISInputSourceRef)CFArrayGetValueAtIndex(sourceList, i);
        CFArrayRef inputSourceLanguages = (CFArrayRef)TISGetInputSourceProperty(inputSource, kTISPropertyInputSourceLanguages);
        CFIndex inputSourceLanguagesCount = CFArrayGetCount(inputSourceLanguages);
        for (CFIndex langIdx = 0; langIdx < inputSourceLanguagesCount; ++langIdx) {
            CFStringRef langName = (CFStringRef)CFArrayGetValueAtIndex(inputSourceLanguages, langIdx);
            if (CFStringCompare(langName, requiredLangName.cfString(), 0) == kCFCompareEqualTo) {
                is = inputSource;
                goto found;
            }
        }
    }
found:

    if (is) CFRetain(is);
    CFRelease(sourceList);

    return is;
}
/////////////////////////////////////////

int main(int argc, char *argv[])
{
    TISInputSourceRef is = findInputSourceByLang("en");
    if (is != NULL) {
        TISSelectInputSource(is);
        CFRelease(is);
    }

    QApplication app(argc, argv);

    app.setOrganizationName("NextGIS");
    app.setApplicationDisplayName ("NextGIS GL Viewer");
    app.setApplicationName("glviewer");
    app.setApplicationVersion(NGGLV_VERSION_STRING);
    app.setOrganizationDomain("nextgis.com");

    // gl stuff
    QSurfaceFormat format;
#ifdef Q_OS_MACOS
    format.setSamples(4);
    format.setRenderableType(QSurfaceFormat::OpenGL);
#else
//    format.setDepthBufferSize(16);
//    format.setStencilBufferSize(8);
//    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(2, 0);
    format.setRenderableType(QSurfaceFormat::OpenGLES);
#endif
    QSurfaceFormat::setDefaultFormat(format);

    // create window
    MainWindow wnd;
    wnd.show();
    
    return app.exec();
}

