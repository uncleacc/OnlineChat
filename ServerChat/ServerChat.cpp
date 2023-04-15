#include <iostream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <map>


using namespace std;

const int MAX_CONN = 1024;

struct Client {
	int socketfd;
	string name;
};
map<int, Client> mp;


int main()
{
	//����
	int epld = epoll_create1(0);
	if (epld < 0) {
		cout << "epoll create error" << endl;
		return -1;
	}
	//����������socket
	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0) {
		cout << "socket create error" << endl;
		return -1;
	}
	//�󶨱��ص�IP�Ͷ˿�
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9999);

	int ret = bind(socketfd, (struct sockaddr*)&addr, sizeof(addr));
	if (socketfd < 0) {
		cout << "bind error" << endl;
		return -1;
	}
	//�����ͻ���
	ret = listen(socketfd, 1024);
	if (ret < 0) {
		cout << "listen error" << endl;
		return -1;
	}
	//��������socket����epoll
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = socketfd;

	ret = epoll_ctl(epld, EPOLL_CTL_ADD, socketfd, &ev);
	if (ret < 0) {
		cout << "epoll_ctl error" << endl;
		return -1;
	}

	//ѭ������
	while (1) {
		struct epoll_event evs[MAX_CONN];
		int n = epoll_wait(epld, evs, MAX_CONN, -1);
		if (n < 0) {
			cout << "epoll_wait error" << endl;
			break;
		}

		for (int i = 0; i < n; i++) {
			int fd = evs[i].data.fd;
			//���������fd�յ���Ϣ����ʾ�пͻ��˽���������
			if (fd == socketfd) {
				struct sockaddr_in client_addr;
				socklen_t client_addr_len = sizeof client_addr;
				int client_socketfd = accept(socketfd, (struct sockaddr*)&client_addr, &client_addr_len);
				if (client_socketfd < 0) {
					cout << "accept error" << endl;
					continue;
				}
				//���ͻ��˵�socket����epoll
				struct epoll_event ev_client;
				ev_client.events = EPOLLIN;
				ev_client.data.fd = client_socketfd;
				ret = epoll_ctl(epld, EPOLL_CTL_ADD, client_socketfd, &ev_client);
				if (ret < 0) {
					cout << "epoll_ctl error" << endl;
					break;
				}
				cout << client_addr.sin_addr.s_addr << " is connecting ..." << endl;

				//����ͻ�����Ϣ
				Client client;
				client.socketfd = client_socketfd;
				client.name = "";
				 
				mp[client_socketfd] = client;
			}
			else {
				char buffer[1024];
				int n = read(fd, buffer, 1024);
				if (n < 0) {
					cout << "ERROR" << endl;
					break;
				}
				else if (n == 0) {
					//�ͻ��˶Ͽ�����
					close(fd);
					epoll_ctl(epld, EPOLL_CTL_DEL, fd, 0);
					mp.erase(fd);
				}
				else {
					string msg(buffer, n);
					if (mp[fd].name == "") {
						mp[fd].name = msg;
						write(fd, ("Welcome " + msg + "!").c_str(), msg.size() + 9);
					}
					else {
						string name = mp[fd].name;
						for (auto &c : mp) {
							if (c.first != fd) {
								write(c.first, ('[' + name + ']' + ": " + msg).c_str(), msg.size() + name.size() + 4);
							}
						}
					}
				}
			}
			
		}
		
	}
	close(epld);
	close(socketfd);
	return 0;
}