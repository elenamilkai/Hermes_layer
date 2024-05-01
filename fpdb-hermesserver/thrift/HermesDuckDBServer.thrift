namespace cpp com.thrift.hermesduckdbserver

service HermesDuckDBServer {
    list<byte> getFreshDataHermes(1:string query, 2:i64 queryCounter, 3:string tableName),
    void printHelloHermes()
}
