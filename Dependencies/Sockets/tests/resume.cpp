#include <EventHandler.h>
#include <TcpSocket.h>
#include <Utility.h>
#include <IEventOwner.h>
#include <ListenSocket.h>


class eHandler : public EventHandler
{
public:
	eHandler() : EventHandler() {}
};


class eSocket : public TcpSocket, public IEventOwner
{
public:
	eSocket(ISocketHandler& h) : TcpSocket(h), IEventOwner(static_cast<eHandler&>(h)), m_listen_socket(NULL), m_server(false) {
		SetLineProtocol();
	}

	bool OnConnectRetry() {
		printf("Retrying connect\n");
		if (GetConnectionRetries() == 5)
		{
			printf("Creating ListenSocket\n");
			m_listen_socket = new ListenSocket<eSocket>(Handler());
			if (m_listen_socket -> Bind(12345))
			{
				printf("Bind port 12345 failed\n");
			}
			m_listen_socket -> SetDeleteByHandler();
			Handler().Add( m_listen_socket );
		}
		return true;
	}

	void OnAccept() {
		m_id_stop_socket = AddEvent(5, 0);
printf("Stop socket id: %d\n", m_id_stop_socket);
		m_server = true;
	}

	void OnConnect() {
		m_id_stop_listen = AddEvent(10, 0);
printf("Stop listen id: %d\n", m_id_stop_listen);
	}

	void OnEvent(int id) {
printf("Event id: %d\n", id);
		if (id == m_id_stop_socket && m_server)
			SetCloseAndDelete();
		if (id == m_id_stop_listen && !m_server)
			m_listen_socket -> SetCloseAndDelete();
	}

	void OnLine(const std::string& line) {
		printf("Incoming data: %s\n", line.c_str());
	}

	void OnDelete() {
		printf("eSocket::OnDelete(), server: %s\n", m_server ? "true" : "false");
	}

	void OnDisconnect() {
		printf("Disconnect, server: %s\n", m_server ? "true" : "false");
	}

private:
	ListenSocket<eSocket> *m_listen_socket;
	int m_id_stop_socket;
	int m_id_stop_listen;
	bool m_server;
};


class Sender : public IEventOwner
{
public:
	Sender(IEventHandler& h, TcpSocket& ref) : IEventOwner(h), m_socket(ref), m_count(1) {
		AddEvent(1, 0);
	}

	void OnEvent(int id) {
		if (static_cast<eHandler&>(EventHandler()).Valid(&m_socket))
			m_socket.Send("Event#" + Utility::l2string(m_count++) + "\n");
		EventHandler().AddEvent(this, 1, 0);
	}

private:
	TcpSocket& m_socket;
	int m_count;
};


int main(int argc, char *argv[])
{
	eHandler h;
	eSocket sock(h);
	sock.SetConnectTimeout(3);
	sock.SetConnectionRetry(-1);
#ifdef ENABLE_RECONNECT
	sock.SetReconnect();
#endif
	sock.Open("localhost", 12345);
	h.Add( &sock );
	Sender send(h, sock);
	h.AddEvent( &send, 1, 0 );
	h.EventLoop();
}

