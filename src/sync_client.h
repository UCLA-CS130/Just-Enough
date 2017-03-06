// The following code is based on the Boost 1.42.0 documentation's sample
// asynchronous HTTP client. The source code can be found here:
//  http://www.boost.org/doc/libs/1_42_0/doc/html/boost_asio/example/http/client/async_client.cpp

// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

/*
 * Synchronous web client.
 */
class SyncClient
{
    public:
        /*
         * Container for Boost guts. Useful for dependency injection.
         */
        struct BoostGuts
        {
            public:
                BoostGuts();

                /*
                 * Dependency injectable constructor for unit testing.
                 */
                BoostGuts(std::unique_ptr<boost::asio::io_service> io_service,
                        std::unique_ptr<tcp::resolver> resolver,
                        std::unique_ptr<tcp::socket> socket);

                std::unique_ptr<tcp::resolver> resolver_;

                std::unique_ptr<tcp::socket> socket_;

            private:
                std::unique_ptr<boost::asio::io_service> io_service_;
        };

        /*
         * Dependency injectable constructor for unit testing.
         */
        SyncClient(std::unique_ptr<BoostGuts> boost_guts)
        {
            boost_guts_ = std::move(boost_guts);
        }

        SyncClient()
        {
            boost_guts_ = std::unique_ptr<BoostGuts>(new BoostGuts());
        }


        /*
         * Open socket with server.
         */
        bool Connect(const std::string& server, const std::string& port);

        bool Write(const std::string& message);

        bool Read(std::string& response);

    private:

        std::unique_ptr<BoostGuts> boost_guts_;

        boost::asio::streambuf request_;
        boost::asio::streambuf response_;
};
