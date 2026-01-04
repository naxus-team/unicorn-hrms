#pragma once
#include <string>

namespace Unicorn::Database {
    class Connection {
    public:
        Connection(const std::string& connStr);
        ~Connection();
        bool Connect();
        void Disconnect();
    private:
        std::string m_ConnectionString;
    };
}
