/**
 * Autogenerated by Thrift Compiler (0.9.2)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef SchedulerService_H
#define SchedulerService_H

#include <thrift/TDispatchProcessor.h>
#include "service_types.h"



class SchedulerServiceIf {
 public:
  virtual ~SchedulerServiceIf() {}
  virtual void registerBackend(const  ::RegMessage& message) = 0;
  virtual void enqueueFinishedQuery(const  ::QuerySpec& query) = 0;
  virtual void consultAddress( ::THostPort& _return, const std::string& serviceType) = 0;
};

class SchedulerServiceIfFactory {
 public:
  typedef SchedulerServiceIf Handler;

  virtual ~SchedulerServiceIfFactory() {}

  virtual SchedulerServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(SchedulerServiceIf* /* handler */) = 0;
};

class SchedulerServiceIfSingletonFactory : virtual public SchedulerServiceIfFactory {
 public:
  SchedulerServiceIfSingletonFactory(const boost::shared_ptr<SchedulerServiceIf>& iface) : iface_(iface) {}
  virtual ~SchedulerServiceIfSingletonFactory() {}

  virtual SchedulerServiceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(SchedulerServiceIf* /* handler */) {}

 protected:
  boost::shared_ptr<SchedulerServiceIf> iface_;
};

class SchedulerServiceNull : virtual public SchedulerServiceIf {
 public:
  virtual ~SchedulerServiceNull() {}
  void registerBackend(const  ::RegMessage& /* message */) {
    return;
  }
  void enqueueFinishedQuery(const  ::QuerySpec& /* query */) {
    return;
  }
  void consultAddress( ::THostPort& /* _return */, const std::string& /* serviceType */) {
    return;
  }
};

typedef struct _SchedulerService_registerBackend_args__isset {
  _SchedulerService_registerBackend_args__isset() : message(false) {}
  bool message :1;
} _SchedulerService_registerBackend_args__isset;

class SchedulerService_registerBackend_args {
 public:

  static const char* ascii_fingerprint; // = "2D4A87D1CE7C5D56A7DB05C360DBBE23";
  static const uint8_t binary_fingerprint[16]; // = {0x2D,0x4A,0x87,0xD1,0xCE,0x7C,0x5D,0x56,0xA7,0xDB,0x05,0xC3,0x60,0xDB,0xBE,0x23};

  SchedulerService_registerBackend_args(const SchedulerService_registerBackend_args&);
  SchedulerService_registerBackend_args& operator=(const SchedulerService_registerBackend_args&);
  SchedulerService_registerBackend_args() {
  }

  virtual ~SchedulerService_registerBackend_args() throw();
   ::RegMessage message;

  _SchedulerService_registerBackend_args__isset __isset;

  void __set_message(const  ::RegMessage& val);

  bool operator == (const SchedulerService_registerBackend_args & rhs) const
  {
    if (!(message == rhs.message))
      return false;
    return true;
  }
  bool operator != (const SchedulerService_registerBackend_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SchedulerService_registerBackend_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_registerBackend_args& obj);
};


class SchedulerService_registerBackend_pargs {
 public:

  static const char* ascii_fingerprint; // = "2D4A87D1CE7C5D56A7DB05C360DBBE23";
  static const uint8_t binary_fingerprint[16]; // = {0x2D,0x4A,0x87,0xD1,0xCE,0x7C,0x5D,0x56,0xA7,0xDB,0x05,0xC3,0x60,0xDB,0xBE,0x23};


  virtual ~SchedulerService_registerBackend_pargs() throw();
  const  ::RegMessage* message;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_registerBackend_pargs& obj);
};


class SchedulerService_registerBackend_result {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};

  SchedulerService_registerBackend_result(const SchedulerService_registerBackend_result&);
  SchedulerService_registerBackend_result& operator=(const SchedulerService_registerBackend_result&);
  SchedulerService_registerBackend_result() {
  }

  virtual ~SchedulerService_registerBackend_result() throw();

  bool operator == (const SchedulerService_registerBackend_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SchedulerService_registerBackend_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SchedulerService_registerBackend_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_registerBackend_result& obj);
};


class SchedulerService_registerBackend_presult {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};


  virtual ~SchedulerService_registerBackend_presult() throw();

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_registerBackend_presult& obj);
};

typedef struct _SchedulerService_enqueueFinishedQuery_args__isset {
  _SchedulerService_enqueueFinishedQuery_args__isset() : query(false) {}
  bool query :1;
} _SchedulerService_enqueueFinishedQuery_args__isset;

class SchedulerService_enqueueFinishedQuery_args {
 public:

  static const char* ascii_fingerprint; // = "82246FE820A3526C6D99BE1DF7A0AD14";
  static const uint8_t binary_fingerprint[16]; // = {0x82,0x24,0x6F,0xE8,0x20,0xA3,0x52,0x6C,0x6D,0x99,0xBE,0x1D,0xF7,0xA0,0xAD,0x14};

  SchedulerService_enqueueFinishedQuery_args(const SchedulerService_enqueueFinishedQuery_args&);
  SchedulerService_enqueueFinishedQuery_args& operator=(const SchedulerService_enqueueFinishedQuery_args&);
  SchedulerService_enqueueFinishedQuery_args() {
  }

  virtual ~SchedulerService_enqueueFinishedQuery_args() throw();
   ::QuerySpec query;

  _SchedulerService_enqueueFinishedQuery_args__isset __isset;

  void __set_query(const  ::QuerySpec& val);

  bool operator == (const SchedulerService_enqueueFinishedQuery_args & rhs) const
  {
    if (!(query == rhs.query))
      return false;
    return true;
  }
  bool operator != (const SchedulerService_enqueueFinishedQuery_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SchedulerService_enqueueFinishedQuery_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_enqueueFinishedQuery_args& obj);
};


class SchedulerService_enqueueFinishedQuery_pargs {
 public:

  static const char* ascii_fingerprint; // = "82246FE820A3526C6D99BE1DF7A0AD14";
  static const uint8_t binary_fingerprint[16]; // = {0x82,0x24,0x6F,0xE8,0x20,0xA3,0x52,0x6C,0x6D,0x99,0xBE,0x1D,0xF7,0xA0,0xAD,0x14};


  virtual ~SchedulerService_enqueueFinishedQuery_pargs() throw();
  const  ::QuerySpec* query;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_enqueueFinishedQuery_pargs& obj);
};


class SchedulerService_enqueueFinishedQuery_result {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};

  SchedulerService_enqueueFinishedQuery_result(const SchedulerService_enqueueFinishedQuery_result&);
  SchedulerService_enqueueFinishedQuery_result& operator=(const SchedulerService_enqueueFinishedQuery_result&);
  SchedulerService_enqueueFinishedQuery_result() {
  }

  virtual ~SchedulerService_enqueueFinishedQuery_result() throw();

  bool operator == (const SchedulerService_enqueueFinishedQuery_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SchedulerService_enqueueFinishedQuery_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SchedulerService_enqueueFinishedQuery_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_enqueueFinishedQuery_result& obj);
};


class SchedulerService_enqueueFinishedQuery_presult {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};


  virtual ~SchedulerService_enqueueFinishedQuery_presult() throw();

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_enqueueFinishedQuery_presult& obj);
};

typedef struct _SchedulerService_consultAddress_args__isset {
  _SchedulerService_consultAddress_args__isset() : serviceType(false) {}
  bool serviceType :1;
} _SchedulerService_consultAddress_args__isset;

class SchedulerService_consultAddress_args {
 public:

  static const char* ascii_fingerprint; // = "EFB929595D312AC8F305D5A794CFEDA1";
  static const uint8_t binary_fingerprint[16]; // = {0xEF,0xB9,0x29,0x59,0x5D,0x31,0x2A,0xC8,0xF3,0x05,0xD5,0xA7,0x94,0xCF,0xED,0xA1};

  SchedulerService_consultAddress_args(const SchedulerService_consultAddress_args&);
  SchedulerService_consultAddress_args& operator=(const SchedulerService_consultAddress_args&);
  SchedulerService_consultAddress_args() : serviceType() {
  }

  virtual ~SchedulerService_consultAddress_args() throw();
  std::string serviceType;

  _SchedulerService_consultAddress_args__isset __isset;

  void __set_serviceType(const std::string& val);

  bool operator == (const SchedulerService_consultAddress_args & rhs) const
  {
    if (!(serviceType == rhs.serviceType))
      return false;
    return true;
  }
  bool operator != (const SchedulerService_consultAddress_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SchedulerService_consultAddress_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_consultAddress_args& obj);
};


class SchedulerService_consultAddress_pargs {
 public:

  static const char* ascii_fingerprint; // = "EFB929595D312AC8F305D5A794CFEDA1";
  static const uint8_t binary_fingerprint[16]; // = {0xEF,0xB9,0x29,0x59,0x5D,0x31,0x2A,0xC8,0xF3,0x05,0xD5,0xA7,0x94,0xCF,0xED,0xA1};


  virtual ~SchedulerService_consultAddress_pargs() throw();
  const std::string* serviceType;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_consultAddress_pargs& obj);
};

typedef struct _SchedulerService_consultAddress_result__isset {
  _SchedulerService_consultAddress_result__isset() : success(false) {}
  bool success :1;
} _SchedulerService_consultAddress_result__isset;

class SchedulerService_consultAddress_result {
 public:

  static const char* ascii_fingerprint; // = "A7EBA1EF34886CA23D8B187ED3C45C57";
  static const uint8_t binary_fingerprint[16]; // = {0xA7,0xEB,0xA1,0xEF,0x34,0x88,0x6C,0xA2,0x3D,0x8B,0x18,0x7E,0xD3,0xC4,0x5C,0x57};

  SchedulerService_consultAddress_result(const SchedulerService_consultAddress_result&);
  SchedulerService_consultAddress_result& operator=(const SchedulerService_consultAddress_result&);
  SchedulerService_consultAddress_result() {
  }

  virtual ~SchedulerService_consultAddress_result() throw();
   ::THostPort success;

  _SchedulerService_consultAddress_result__isset __isset;

  void __set_success(const  ::THostPort& val);

  bool operator == (const SchedulerService_consultAddress_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const SchedulerService_consultAddress_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SchedulerService_consultAddress_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_consultAddress_result& obj);
};

typedef struct _SchedulerService_consultAddress_presult__isset {
  _SchedulerService_consultAddress_presult__isset() : success(false) {}
  bool success :1;
} _SchedulerService_consultAddress_presult__isset;

class SchedulerService_consultAddress_presult {
 public:

  static const char* ascii_fingerprint; // = "A7EBA1EF34886CA23D8B187ED3C45C57";
  static const uint8_t binary_fingerprint[16]; // = {0xA7,0xEB,0xA1,0xEF,0x34,0x88,0x6C,0xA2,0x3D,0x8B,0x18,0x7E,0xD3,0xC4,0x5C,0x57};


  virtual ~SchedulerService_consultAddress_presult() throw();
   ::THostPort* success;

  _SchedulerService_consultAddress_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

  friend std::ostream& operator<<(std::ostream& out, const SchedulerService_consultAddress_presult& obj);
};

class SchedulerServiceClient : virtual public SchedulerServiceIf {
 public:
  SchedulerServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  SchedulerServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void registerBackend(const  ::RegMessage& message);
  void send_registerBackend(const  ::RegMessage& message);
  void recv_registerBackend();
  void enqueueFinishedQuery(const  ::QuerySpec& query);
  void send_enqueueFinishedQuery(const  ::QuerySpec& query);
  void recv_enqueueFinishedQuery();
  void consultAddress( ::THostPort& _return, const std::string& serviceType);
  void send_consultAddress(const std::string& serviceType);
  void recv_consultAddress( ::THostPort& _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class SchedulerServiceProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<SchedulerServiceIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (SchedulerServiceProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_registerBackend(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_enqueueFinishedQuery(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_consultAddress(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  SchedulerServiceProcessor(boost::shared_ptr<SchedulerServiceIf> iface) :
    iface_(iface) {
    processMap_["registerBackend"] = &SchedulerServiceProcessor::process_registerBackend;
    processMap_["enqueueFinishedQuery"] = &SchedulerServiceProcessor::process_enqueueFinishedQuery;
    processMap_["consultAddress"] = &SchedulerServiceProcessor::process_consultAddress;
  }

  virtual ~SchedulerServiceProcessor() {}
};

class SchedulerServiceProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  SchedulerServiceProcessorFactory(const ::boost::shared_ptr< SchedulerServiceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< SchedulerServiceIfFactory > handlerFactory_;
};

class SchedulerServiceMultiface : virtual public SchedulerServiceIf {
 public:
  SchedulerServiceMultiface(std::vector<boost::shared_ptr<SchedulerServiceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~SchedulerServiceMultiface() {}
 protected:
  std::vector<boost::shared_ptr<SchedulerServiceIf> > ifaces_;
  SchedulerServiceMultiface() {}
  void add(boost::shared_ptr<SchedulerServiceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void registerBackend(const  ::RegMessage& message) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->registerBackend(message);
    }
    ifaces_[i]->registerBackend(message);
  }

  void enqueueFinishedQuery(const  ::QuerySpec& query) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->enqueueFinishedQuery(query);
    }
    ifaces_[i]->enqueueFinishedQuery(query);
  }

  void consultAddress( ::THostPort& _return, const std::string& serviceType) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->consultAddress(_return, serviceType);
    }
    ifaces_[i]->consultAddress(_return, serviceType);
    return;
  }

};



#endif
