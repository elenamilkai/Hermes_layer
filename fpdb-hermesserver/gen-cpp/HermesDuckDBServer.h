/**
 * Autogenerated by Thrift Compiler (0.17.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef HermesDuckDBServer_H
#define HermesDuckDBServer_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include <memory>
#include "HermesDuckDBServer_types.h"

namespace com { namespace thrift { namespace hermesduckdbserver {

#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class HermesDuckDBServerIf {
 public:
  virtual ~HermesDuckDBServerIf() {}
  virtual void getFreshDataHermes(std::vector<int8_t> & _return, const std::string& query, const int64_t queryCounter, const std::string& tableName) = 0;
  virtual void printHelloHermes() = 0;
};

class HermesDuckDBServerIfFactory {
 public:
  typedef HermesDuckDBServerIf Handler;

  virtual ~HermesDuckDBServerIfFactory() {}

  virtual HermesDuckDBServerIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(HermesDuckDBServerIf* /* handler */) = 0;
  };

class HermesDuckDBServerIfSingletonFactory : virtual public HermesDuckDBServerIfFactory {
 public:
  HermesDuckDBServerIfSingletonFactory(const ::std::shared_ptr<HermesDuckDBServerIf>& iface) : iface_(iface) {}
  virtual ~HermesDuckDBServerIfSingletonFactory() {}

  virtual HermesDuckDBServerIf* getHandler(const ::apache::thrift::TConnectionInfo&) override {
    return iface_.get();
  }
  virtual void releaseHandler(HermesDuckDBServerIf* /* handler */) override {}

 protected:
  ::std::shared_ptr<HermesDuckDBServerIf> iface_;
};

class HermesDuckDBServerNull : virtual public HermesDuckDBServerIf {
 public:
  virtual ~HermesDuckDBServerNull() {}
  void getFreshDataHermes(std::vector<int8_t> & /* _return */, const std::string& /* query */, const int64_t /* queryCounter */, const std::string& /* tableName */) override {
    return;
  }
  void printHelloHermes() override {
    return;
  }
};

typedef struct _HermesDuckDBServer_getFreshDataHermes_args__isset {
  _HermesDuckDBServer_getFreshDataHermes_args__isset() : query(false), queryCounter(false), tableName(false) {}
  bool query :1;
  bool queryCounter :1;
  bool tableName :1;
} _HermesDuckDBServer_getFreshDataHermes_args__isset;

class HermesDuckDBServer_getFreshDataHermes_args {
 public:

  HermesDuckDBServer_getFreshDataHermes_args(const HermesDuckDBServer_getFreshDataHermes_args&);
  HermesDuckDBServer_getFreshDataHermes_args& operator=(const HermesDuckDBServer_getFreshDataHermes_args&);
  HermesDuckDBServer_getFreshDataHermes_args() noexcept
                                             : query(),
                                               queryCounter(0),
                                               tableName() {
  }

  virtual ~HermesDuckDBServer_getFreshDataHermes_args() noexcept;
  std::string query;
  int64_t queryCounter;
  std::string tableName;

  _HermesDuckDBServer_getFreshDataHermes_args__isset __isset;

  void __set_query(const std::string& val);

  void __set_queryCounter(const int64_t val);

  void __set_tableName(const std::string& val);

  bool operator == (const HermesDuckDBServer_getFreshDataHermes_args & rhs) const
  {
    if (!(query == rhs.query))
      return false;
    if (!(queryCounter == rhs.queryCounter))
      return false;
    if (!(tableName == rhs.tableName))
      return false;
    return true;
  }
  bool operator != (const HermesDuckDBServer_getFreshDataHermes_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const HermesDuckDBServer_getFreshDataHermes_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class HermesDuckDBServer_getFreshDataHermes_pargs {
 public:


  virtual ~HermesDuckDBServer_getFreshDataHermes_pargs() noexcept;
  const std::string* query;
  const int64_t* queryCounter;
  const std::string* tableName;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _HermesDuckDBServer_getFreshDataHermes_result__isset {
  _HermesDuckDBServer_getFreshDataHermes_result__isset() : success(false) {}
  bool success :1;
} _HermesDuckDBServer_getFreshDataHermes_result__isset;

class HermesDuckDBServer_getFreshDataHermes_result {
 public:

  HermesDuckDBServer_getFreshDataHermes_result(const HermesDuckDBServer_getFreshDataHermes_result&);
  HermesDuckDBServer_getFreshDataHermes_result& operator=(const HermesDuckDBServer_getFreshDataHermes_result&);
  HermesDuckDBServer_getFreshDataHermes_result() noexcept {
  }

  virtual ~HermesDuckDBServer_getFreshDataHermes_result() noexcept;
  std::vector<int8_t>  success;

  _HermesDuckDBServer_getFreshDataHermes_result__isset __isset;

  void __set_success(const std::vector<int8_t> & val);

  bool operator == (const HermesDuckDBServer_getFreshDataHermes_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const HermesDuckDBServer_getFreshDataHermes_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const HermesDuckDBServer_getFreshDataHermes_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _HermesDuckDBServer_getFreshDataHermes_presult__isset {
  _HermesDuckDBServer_getFreshDataHermes_presult__isset() : success(false) {}
  bool success :1;
} _HermesDuckDBServer_getFreshDataHermes_presult__isset;

class HermesDuckDBServer_getFreshDataHermes_presult {
 public:


  virtual ~HermesDuckDBServer_getFreshDataHermes_presult() noexcept;
  std::vector<int8_t> * success;

  _HermesDuckDBServer_getFreshDataHermes_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class HermesDuckDBServer_printHelloHermes_args {
 public:

  HermesDuckDBServer_printHelloHermes_args(const HermesDuckDBServer_printHelloHermes_args&) noexcept;
  HermesDuckDBServer_printHelloHermes_args& operator=(const HermesDuckDBServer_printHelloHermes_args&) noexcept;
  HermesDuckDBServer_printHelloHermes_args() noexcept {
  }

  virtual ~HermesDuckDBServer_printHelloHermes_args() noexcept;

  bool operator == (const HermesDuckDBServer_printHelloHermes_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const HermesDuckDBServer_printHelloHermes_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const HermesDuckDBServer_printHelloHermes_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class HermesDuckDBServer_printHelloHermes_pargs {
 public:


  virtual ~HermesDuckDBServer_printHelloHermes_pargs() noexcept;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class HermesDuckDBServer_printHelloHermes_result {
 public:

  HermesDuckDBServer_printHelloHermes_result(const HermesDuckDBServer_printHelloHermes_result&) noexcept;
  HermesDuckDBServer_printHelloHermes_result& operator=(const HermesDuckDBServer_printHelloHermes_result&) noexcept;
  HermesDuckDBServer_printHelloHermes_result() noexcept {
  }

  virtual ~HermesDuckDBServer_printHelloHermes_result() noexcept;

  bool operator == (const HermesDuckDBServer_printHelloHermes_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const HermesDuckDBServer_printHelloHermes_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const HermesDuckDBServer_printHelloHermes_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class HermesDuckDBServer_printHelloHermes_presult {
 public:


  virtual ~HermesDuckDBServer_printHelloHermes_presult() noexcept;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class HermesDuckDBServerClient : virtual public HermesDuckDBServerIf {
 public:
  HermesDuckDBServerClient(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  HermesDuckDBServerClient(std::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, std::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(std::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, std::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  std::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  std::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void getFreshDataHermes(std::vector<int8_t> & _return, const std::string& query, const int64_t queryCounter, const std::string& tableName) override;
  void send_getFreshDataHermes(const std::string& query, const int64_t queryCounter, const std::string& tableName);
  void recv_getFreshDataHermes(std::vector<int8_t> & _return);
  void printHelloHermes() override;
  void send_printHelloHermes();
  void recv_printHelloHermes();
 protected:
  std::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  std::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class HermesDuckDBServerProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  ::std::shared_ptr<HermesDuckDBServerIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext) override;
 private:
  typedef  void (HermesDuckDBServerProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_getFreshDataHermes(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_printHelloHermes(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  HermesDuckDBServerProcessor(::std::shared_ptr<HermesDuckDBServerIf> iface) :
    iface_(iface) {
    processMap_["getFreshDataHermes"] = &HermesDuckDBServerProcessor::process_getFreshDataHermes;
    processMap_["printHelloHermes"] = &HermesDuckDBServerProcessor::process_printHelloHermes;
  }

  virtual ~HermesDuckDBServerProcessor() {}
};

class HermesDuckDBServerProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  HermesDuckDBServerProcessorFactory(const ::std::shared_ptr< HermesDuckDBServerIfFactory >& handlerFactory) noexcept :
      handlerFactory_(handlerFactory) {}

  ::std::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo) override;

 protected:
  ::std::shared_ptr< HermesDuckDBServerIfFactory > handlerFactory_;
};

class HermesDuckDBServerMultiface : virtual public HermesDuckDBServerIf {
 public:
  HermesDuckDBServerMultiface(std::vector<std::shared_ptr<HermesDuckDBServerIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~HermesDuckDBServerMultiface() {}
 protected:
  std::vector<std::shared_ptr<HermesDuckDBServerIf> > ifaces_;
  HermesDuckDBServerMultiface() {}
  void add(::std::shared_ptr<HermesDuckDBServerIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void getFreshDataHermes(std::vector<int8_t> & _return, const std::string& query, const int64_t queryCounter, const std::string& tableName) override {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->getFreshDataHermes(_return, query, queryCounter, tableName);
    }
    ifaces_[i]->getFreshDataHermes(_return, query, queryCounter, tableName);
    return;
  }

  void printHelloHermes() override {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->printHelloHermes();
    }
    ifaces_[i]->printHelloHermes();
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class HermesDuckDBServerConcurrentClient : virtual public HermesDuckDBServerIf {
 public:
  HermesDuckDBServerConcurrentClient(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot, std::shared_ptr< ::apache::thrift::async::TConcurrentClientSyncInfo> sync) : sync_(sync)
{
    setProtocol(prot);
  }
  HermesDuckDBServerConcurrentClient(std::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, std::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot, std::shared_ptr< ::apache::thrift::async::TConcurrentClientSyncInfo> sync) : sync_(sync)
{
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(std::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, std::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  std::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  std::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void getFreshDataHermes(std::vector<int8_t> & _return, const std::string& query, const int64_t queryCounter, const std::string& tableName) override;
  int32_t send_getFreshDataHermes(const std::string& query, const int64_t queryCounter, const std::string& tableName);
  void recv_getFreshDataHermes(std::vector<int8_t> & _return, const int32_t seqid);
  void printHelloHermes() override;
  int32_t send_printHelloHermes();
  void recv_printHelloHermes(const int32_t seqid);
 protected:
  std::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  std::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
  std::shared_ptr< ::apache::thrift::async::TConcurrentClientSyncInfo> sync_;
};

#ifdef _MSC_VER
  #pragma warning( pop )
#endif

}}} // namespace

#endif
