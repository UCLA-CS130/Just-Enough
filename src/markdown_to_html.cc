#include "markdown_to_html.h"

#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>

#include "markdown.h"

std::string convertMarkdownToHtml(const std::string& markdown) {
    std::cout << "Converting markdown file" << std::endl;

    std::ostringstream oss;

    markdown::Document doc;
    doc.read(markdown);
    doc.write(oss);

    std::string output(oss.str());
    return output;
}

