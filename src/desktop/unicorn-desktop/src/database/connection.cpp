#include "connection.h"

namespace Unicorn::Database {
    Connection::Connection(const std::string& connStr) : m_ConnectionString(connStr) {}
    Connection::~Connection() {}
    bool Connection::Connect() { return true; }
    void Connection::Disconnect() {}
}
