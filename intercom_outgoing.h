#ifndef INTERCOM_PROXY_H
#define INTERCOM_PROXY_H

#include "Particle.h"
#include "plf_utils.h"

class Intercom_Outgoing {
private:
  UDP& _udp;
  IPAddress _remote_ip_address;
  int _remote_port;
  Timer _timer;

  void _onTimeout(void);

public:
  Intercom_Outgoing(IPAddress remote_ip_address, int remote_port, UDP& udp);
};

#endif /*INTERCOM_PROXY_H*/
