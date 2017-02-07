#ifndef INTERCOM_INCOMING_H
#define INTERCOM_INCOMING_H

class Intercom_Incoming {
private:
  UDP _udp;

public:
  Intercom_Incoming(int local_port);

  void receive(void);
  void drain(void);

  inline UDP& getSocket(void) {
    return _udp;
  }
};

#endif /*INTERCOM_INCOMING_H*/
