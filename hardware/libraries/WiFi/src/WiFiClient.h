#ifndef wificlient_h
#define wificlient_h

#include "Arduino.h"	
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"
#include "TCPSocketConnection.h"

class WiFiClient : public Client {

public:
	
  WiFiClient();
  WiFiClient(TCPSocketConnection* s);

  uint8_t status();
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();

  char* get_address(void);

  friend class WiFiServer;

  using Print::write;

private:
	TCPSocketConnection* _pTcpSocket;
	//char _readchar;
	//bool _readchar_set;
	uint8_t _sock;	 

	uint8_t getFirstSocket();

};

#endif

