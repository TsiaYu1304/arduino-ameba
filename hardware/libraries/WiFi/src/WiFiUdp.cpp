/*
  WiFiUdp.cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

extern "C" {
extern int rtl_printf(const char *fmt, ...);
}
#include <string.h>
#include "wifi_drv.h"
#include "WiFi.h"
#include "WiFiUdp.h"
#include "WiFiClient.h"
#include "WiFiServer.h"

/* Constructor */
WiFiUDP::WiFiUDP() {
	_sock = -1;
	_pUdpSocket = new UDPSocket();
	if ( _pUdpSocket == NULL ) {
		DiagPrintf("Memory: WiFiUDP : UDPSocket allocation failed\r\n");
	}
	_pEndPoint = new Endpoint();
	if ( _pEndPoint == NULL ) {
		DiagPrintf("Memory: WiFiUDP : Endpoint allocation failed\r\n");
	}
	_localPort = 0;
}

/* Start WiFiUDP socket, listening at local port PORT */
uint8_t WiFiUDP::begin(uint16_t port) {
	int ret;

	if ( _sock < 0 ) {
		_pUdpSocket->init();
		_sock = _pUdpSocket->get_socket_fd();
		_pUdpSocket->set_blocking(false, 5000); // 5 sec. timeout 
	}
	
    if ( _sock >= 0 )
    {
		_localPort = port;
		return 1;
    }
	else{
    	Serial.println("No Socket available");
    	return 0;
    }
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int WiFiUDP::available() {
    int ret;
	char data;
	
	ret = lwip_recv(_sock, &data, 1, 1); // flag = 1 => peek
	return (ret > 0 )? ret : 0;
}

/* Release any resources being used by this WiFiUDP instance */
void WiFiUDP::stop()
{
    if (_sock < 0)
        return;

	_pUdpSocket->close();

	_sock = _pUdpSocket->get_socket_fd();
}

int WiFiUDP::beginPacket(const char *host, uint16_t port)
{
    // Look up the host first
    int ret = 0;
    IPAddress remote_addr;
    if (WiFi.hostByName(host, remote_addr))
    {
        return beginPacket(remote_addr, port);
    }
    return ret;
}

int WiFiUDP::beginPacket(IPAddress ip, uint16_t port)
{
	int ret;
	
	if ( _sock < 0 ) {
		_pUdpSocket->init();
		_sock = _pUdpSocket->get_socket_fd();		
		_pUdpSocket->set_blocking(false, 5000); // 5 sec. timeout 
	}
	
    if ( _sock >= 0 )
    {
    	_pEndPoint->set_address(ip.get_address(), port);
    }	
		
}

int WiFiUDP::endPacket()
{
    return true;
}

size_t WiFiUDP::write(uint8_t byte)
{
  return write(&byte, 1);
}

size_t WiFiUDP::write(const uint8_t *buffer, size_t size)
{
	if ( _sock < 0 ) return 0;

    _pUdpSocket->sendTo(*_pEndPoint, (char*)buffer, size);
    return size;
}


int WiFiUDP::parsePacket()
{
    return available();
}

int WiFiUDP::read()
{
	int n;
    char b;

    n = _pUdpSocket->receiveFrom(*_pEndPoint, &b, 1);
	return ( n != 1 )? (-1) : (int)(b);
}

int WiFiUDP::read(unsigned char* buffer, size_t len)
{
    return _pUdpSocket->receiveFrom(*_pEndPoint, (char*)buffer, len);
}

int WiFiUDP::peek()
{
	int n;
	char b;
	
	n = lwip_recv(_sock, &b, 1, 1); // flag = 1 => peek

    if ( n != 1) return -1;
    return (int)(b);
}

void WiFiUDP::flush()
{
    while (available())
        read();
}

IPAddress  WiFiUDP::remoteIP()
{
	// TODO : may need to implement 
	IPAddress ip((uint8_t*)_pEndPoint->get_address());
	return ip;
}

uint16_t  WiFiUDP::remotePort()
{
	// TODO : may need to implment
	return _pEndPoint->get_port();
}


