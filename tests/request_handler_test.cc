#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "request_handler.h"
#include <memory>

TEST(RequestHandlerTest, unknownHandler) {
    std::unique_ptr<RequestHandler> handler(RequestHandler::CreateByName("DefinitelyNotARealHandler"));
    ASSERT_EQ(handler, nullptr);
}

