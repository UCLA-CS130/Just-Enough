#pragma once
#include <fstream>
#include <map>

// we will store arbitary file data in a std::string as a (data,length) pair
typedef std::string BinaryData;


enum FileLoaderError {
    FileErr_None = 0,
    FileErr_NoFile,
    FileErr_NoPermission,
    FileErr_IOErr,
};

enum MIMEType {
    MIMEType_text = 0,

    MIMEType_html,
    MIMEType_css,
    MIMEType_js,

    MIMEType_png,
    MIMEType_gif,
    MIMEType_jpeg,

    MIMEType_md,

    MIMEType_unknown,
};

extern const std::map<MIMEType, std::string> MIMETypeMap;

class FileLoader {
    public:
        FileLoaderError openFile(std::string filename, std::string* data);
        FileLoaderError openFile(std::istream* input, std::string filename, std::string* data);

        MIMEType guessMIMEType();

    protected:
        std::string filename_;
        BinaryData data_;
};
