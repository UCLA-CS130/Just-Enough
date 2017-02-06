#include "fileLoader.h"
#include "utils.h"
#include <iostream>

using std::string;

const std::map<MIMEType, string> MIMETypeMap = {
    {MIMEType_html, "text/html"},
    {MIMEType_css, "text/css"},
    {MIMEType_js, "text/javascript"},

    {MIMEType_png, "image/png"},
    {MIMEType_gif, "image/gif"},
    {MIMEType_jpeg, "image/jpeg"},

    {MIMEType_text, "text/plain"},
    {MIMEType_unknown, "text/plain"},
};

const std::map<string, MIMEType> fileExtensionTypeMap = {
    {"gif", MIMEType_gif},
    {"png", MIMEType_png},
    {"jpeg", MIMEType_jpeg},
    {"jpg", MIMEType_jpeg},

    {"html", MIMEType_html},
    {"htm", MIMEType_html},

    {"css", MIMEType_css},
    {"js", MIMEType_js},

    {"txt", MIMEType_text},
};


FileLoaderError FileLoader::openFile(string filename, string* data) {
    filename_ = filename;
    std::ifstream file(filename_);
    if ( ! file.good()) {
        return FileErr_NoFile;
    }

    data->assign(
            (std::istreambuf_iterator<char>(file)),
            (std::istreambuf_iterator<char>())
                );

    return FileErr_None;
}

// load a file directly from istream, for testing
FileLoaderError FileLoader::openFile(std::istream* input, string filename, string* data) {
    filename_ = filename;

    *data = BinaryData(std::istreambuf_iterator<char>(*input), {});

    return FileErr_None;
}


MIMEType FileLoader::guessMIMEType() {
    std::vector<string> nameParts = split(filename_, '.');
    if (nameParts.size() <= 1) {
        // No extension, bail
        return MIMEType_unknown;
    }

    string ext = nameParts[nameParts.size() - 1];
    auto typeMapEntry = fileExtensionTypeMap.find(ext);
    if (typeMapEntry == fileExtensionTypeMap.end()) {
        // Can't guess type from extention...
        // TODO: try reading file's header to check for data type
        return MIMEType_unknown;
    }
    return typeMapEntry->second;
}
