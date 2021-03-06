/**
 * Autogenerated by Thrift Compiler (0.9.2)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef NodeManagerService_H
#define NodeManagerService_H

#include <thrift/TDispatchProcessor.h>
#include "service_types.h"



class NodeManagerServiceIf {
 public:
  virtual ~NodeManagerServiceIf() {}
  virtual void launchServiceInstance( ::THostPort& _return, const std::string& serviceType, const double budget) = 0;
};

class NodeManagerServiceIfFactory {
 public:
  typedef NodeManagerServiceIf Handler;

  virtual ~NodeManagerServiceIfFactory() {}

  virtual NodeManagerServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(NodeManagerServiceIf* /* handler */) = 0;
};

class NodeManagerServiceIfSingletonFactory : virtual public NodeManagerServiceIfFactory {
 public:
  NodeManagerServiceIfSingletonFactory(const boost::shared_ptr<NodeManagerServiceIf>& iface) : iface_(iface) {}
  virtual ~NodeManagerServiceIfSingletonFactory() {}

  virtual NodeManagerServiceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(NodeManagerServiceIf* /* handler */) {}

 protected:
  boost::shared_ptr<NodeManagerServiceIf> iface_;
};

class NodeManagerServiceNull : virtual public NodeManagerServiceIf {
 public:
  virtual ~NodeManagerServiceNull() {}
  void launchServiceInstance( ::THostPort& /* _return */, const std::string& /* serviceType */, const double /* budget */) {
    return;
  }
};

typedef struct _NodeManagerService_launchServiceInstance_args__isset {
  _NodeManagerService_launchServiceInstance_args__isset() : serviceType(false), budget(false) {}
  bool serviceType :1;
  bool budget :1;
} _NodeManagerService_launchServiceInstance_args__isset;

class NodeManagerService_launchServiceInstance_args {
 public:

  static const char* ascii_fingerprint; // = "C712EF0DA8599E55DF4D0F13415232EF";
  static const uint8_t binary_fingerprint[16]; // = {0xC7,0x12,0xEF,0x0D,0xA8,0x59,0x9E,0x55,0xDF,0x4D,0x0F,0x13,0x41,0x52,0x32,0xEF};

  NodeManagerService_launchServiceInstance_args(const NodeManagerService_launchServiceInstance_args&);
  NodeManagerService_launchServiceInstance_args& operator=(const NodeManagerService_launchServiceInstance_args&);
  NodeManagerService_launchServiceInstance_args() : serviceType(), budget(0) {
  }

  virtual ~NodeManagerService_launchServiceInstance_args() throw();
  std::string serviceType;
  double budget;

  _NodeManagerService_launchServiceInstance_args__isset __isset;

  void __set_serviceType(const std::string& val);

  void __set_budget(const double val);

  bool operator == (const NodeManagerService_launchServiceInstance_args & rhs) const
  {
    if (!(serviceType == rhs.serviceType))
      return false;
    if (!(budget == rhs.budget))
      return false;
    return true;
  }
  bool operator != (const NodeManagerService_launchServiceInstance_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NodeManagerService_launchServiceInstance_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const NodeManagerService_launchServiceInstance_args& obj);
};


class NodeManagerService_launchServiceInstance_pargs {
 public:

  static const char* ascii_fingerprint; // = "C712EF0DA8599E55DF4D0F13415232EF";
  static const uint8_t binary_fingerprint[16]; // = {0xC7,0x12,0xEF,0x0D,0xA8,0x59,0x9E,0x55,0xDF,0x4D,0x0F,0x13,0x41,0x52,0x32,0xEF};


  virtual ~NodeManagerService_launchServiceInstance_pargs() throw();
  const std::string* serviceType;
  const double* budget;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const NodeManagerService_launchServiceInstance_pargs& obj);
};

typedef struct _NodeManagerService_launchServiceInstance_result__isset {
  _NodeManagerService_launchServiceInstance_result__isset() : success(false) {}
  bool success :1;
} _NodeManagerService_launchServiceInstance_result__isset;

class NodeManagerService_launchServiceInstance_result {
 public:

  static const char* ascii_fingerprint; // = "A7EBA1EF34886CA23D8B187ED3C45C57";
  static const uint8_t binary_fingerprint[16]; // = {0xA7,0xEB,0xA1,0xEF,0x34,0x88,0x6C,0xA2,0x3D,0x8B,0x18,0x7E,0xD3,0xC4,0x5C,0x57};

  NodeManagerService_launchServiceInstance_result(const NodeManagerService_launchServiceInstance_result&);
  NodeManagerService_launchServiceInstance_result& operator=(const NodeManagerService_launchServiceInstance_result&);
  NodeManagerService_launchServiceInstance_result() {
  }

  virtual ~NodeManagerService_launchServiceInstance_result() throw();
   ::THostPort success;

  _NodeManagerService_launchServiceInstance_result__isset __isset;

  void __set_success(const  ::THostPort& val);

  bool operator == (const NodeManagerService_launchServiceInstance_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const NodeManagerService_launchServiceInstance_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NodeManagerService_launchServiceInstance_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const NodeManagerService_launchServiceInstance_result& obj);
};

typedef struct _NodeManagerService_launchServiceInstance_presult__isset {
  _NodeManagerService_launchServiceInstance_presult__isset() : success(false) {}
  bool success :1;
} _NodeManagerService_launchServiceInstance_presult__isset;

class NodeManagerService_launchServiceInstance_presult {
 public:

  static const char* ascii_fingerprint; // = "A7EBA1EF34886CA23D8B187ED3C45C57";
  static const uint8_t binary_fingerprint[16]; // = {0xA7,0xEB,0xA1,0xEF,0x34,0x88,0x6C,0xA2,0x3D,0x8B,0x18,0x7E,0xD3,0xC4,0x5C,0x57};


  virtual ~NodeManagerService_launchServiceInstance_presult() throw();
   ::THostPort* success;

  _NodeManagerService_launchServiceInstance_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

  friend std::ostream& operator<<(std::ostream& out, const NodeManagerService_launchServiceInstance_presult& obj);
};

class NodeManagerServiceClient : virtual public NodeManagerServiceIf {
 public:
  NodeManagerServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  NodeManagerServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
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
  void launchServiceInstance( ::THostPort& _return, const std::string& serviceType, const double budget);
  void send_launchServiceInstance(const std::string& serviceType, const double budget);
  void recv_launchServiceInstance( ::THostPort& _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class NodeManagerServiceProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<NodeManagerServiceIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (NodeManagerServiceProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_launchServiceInstance(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  NodeManagerServiceProcessor(boost::shared_ptr<NodeManagerServiceIf> iface) :
    iface_(iface) {
    processMap_["launchServiceInstance"] = &NodeManagerServiceProcessor::process_launchServiceInstance;
  }

  virtual ~NodeManagerServiceProcessor() {}
};

class NodeManagerServiceProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  NodeManagerServiceProcessorFactory(const ::boost::shared_ptr< NodeManagerServiceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< NodeManagerServiceIfFactory > handlerFactory_;
};

class NodeManagerServiceMultiface : virtual public NodeManagerServiceIf {
 public:
  NodeManagerServiceMultiface(std::vector<boost::shared_ptr<NodeManagerServiceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~NodeManagerServiceMultiface() {}
 protected:
  std::vector<boost::shared_ptr<NodeManagerServiceIf> > ifaces_;
  NodeManagerServiceMultiface() {}
  void add(boost::shared_ptr<NodeManagerServiceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void launchServiceInstance( ::THostPort& _return, const std::string& serviceType, const double budget) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->launchServiceInstance(_return, serviceType, budget);
    }
    ifaces_[i]->launchServiceInstance(_return, serviceType, budget);
    return;
  }

};



#endif
