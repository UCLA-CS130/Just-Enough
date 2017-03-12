#include "markdown.h"

#include <iostream>
#include <boost/filesystem.hpp>

namespace md {
extern "C" {
#include "mkdio.h" // discount markdown library
}
}

std::string convertMarkdownToHtml(std::string* html) {
    std::cout << "Converting markdown file" << std::endl;
    char* htmlData = &(*html)[0];
    md::MMIOT* doc = md::mkd_string(htmlData, html->size(), 0);

    boost::filesystem::path dir("tmp");
    boost::filesystem::create_directory(dir);

    // note: discount API requires FILE*
    FILE* fout = fopen("tmp/tmpMD.html", "w+");
    if ( ! fout) {
        std::cerr << "Warning: Markdown failed to make temporary file" << std::endl;
        return "";
    }

    int res = md::markdown(doc, fout, 0);
    if (res != 0) {
        std::cerr << "Warning: markdown generation failed: " << res << std::endl;
        return "";
    }

    fseek(fout, 0, SEEK_END);
    size_t filesize = ftell(fout);
    fseek(fout, 0, SEEK_SET);

    std::string output(filesize+1, '\0');
    fread(&output[0], filesize, 1, fout);

    return output;
}

