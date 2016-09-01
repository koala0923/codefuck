6//###
//Step 1
//


//Service


#include<stdio.h>
#include<string.h>
#include<errno.h>

#include<unistd.h>
#include<event.h>

int tcp_server_init(int port, int listen_num);
void accept_cb(int fd, short events, void *arg);
void socket_read_cb(int fd, short events, void *arg);


int main(int argc, char **argv)
{
	int listener = tcp_server_init(9999, 10);
	if( listener == -1){
		perror("tcp_server_init error");
		return -1;
	}

	struct event_base* base = event_base_new();

	//添加监听客户端请求连接事件
	strcut event* ev_listen = event_new(base, listener, EV_READ | EV_PERSIST,
							accept_cb, base);
	event_add(ev_listen, NULL);

	event_base_dispatch(base);

	return 0;
}

typedef struct sockaddr SA;

int tcp_server_init(int port, int listen_num)
{
	int errno_save;
	evutil_socket_t listener;

	listener = ::socket(AF_INET, SOCK_STREAM, 0);
	if(listener == -1)
		return-1;

	//允许多次绑定同一个地址，要用在socket和bind之间
	evutil_make_listen_socket_reuseable(listener);

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(port);

	if(::bind(listener,(SA *)&sin, sizeof(sin)) < 0)
		goto error;

	if(::listen(listener, listen_num) < 0)
		goto error;

	//跨平台统一接口， 将套接字设置为非阻塞状态
	evutil_make_socket_nonblocking(listener);


	return listener;


	error:
		errno_save = errno;
		evutil_closesockket(listener);
		errno = errno_save;

	return -1;
}

void accept_cb(int fd, short events, void *arg)
{
	evutil_socket_t sockfd;


	struct sockaddr_in client;
	socklen_t len;

	sockfd = ::accept(fd, (struct sockaddr*)&client, &len);
	evutil_make_socket_nonblocking(sockfd);

	printf("accept a clent %d\n", sockfd);

	struct event_base *base = (event_base *)arg;


	//仅仅是为了动态创建一个event结构
	
	struct event *ev = event_new(NULL, -1, 0, NULL, NULL);

	//将动态创建的结构体作为event的回调函数
	event_assign(ev, base, sockfd, EV_READ | EV_PERSIST,
			socket_read_cb, (void*)ev);

	event_add(ev, NULL);
}


void socket_read_cb(int fd, short events, void *arg)
{
	char msg[4096];
	struct event *ev = (struct event*)arg;
	int len = read(fd, msg, sizeof(msg)-1);


	if(len <= 0){
		printf("some error happen when read\n");
		close(event_get_fd(ev));
		event_free(ev);
		return ;
	}

	msg[len] = '\0';

	printf("recv the client msg: %s\n", msg);

	char reply_msg[4096] = "I have recvieced the msg: ";
	strcat(reply_msg + strlen(reply_msg), msg);

	write(fd, reply_msg, strlen(reply_msg));
}


//###

//Client

#include<sys/types.h>
#incldue<sys/socket.h>
#include<netinet/in.h>
#incldue<arpa/inet.h>
#include<errno.h>
#include<unistd.h>


#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<event.h>
#include<event2/util.h>

int tcp_connect_server(const char* server_ip, int port);
void socket_read_cb(int fd, short events, void *arg);
int main(int argc, char **argv)
{
	if (argc < 3){
		printf("please input 2 parameter\n");
		return  -1;
	}

	//两个参数依次是服务器的IP地址，端口号
	int sockfd = tcp_connect_server(argv[1], atoi[argv[2]]);
	if(sockfd == -1){
		perror("tcp_connect_server error");
		return -1;
	}

	printf("connect to server successful\n");

	struct event_base *base = event_base_new();

	struct event *ev_sockfd = event_new(base, sockfd, EV_READ | EV_PERSIST,
					socket_read_cb, NULL);
	event_add(ev_sockfd, NULL);

	//监听终端输入事件
	struct event *ev_cmd = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST,
					cmd_msg_cb, (void*)&sockfd);
	event_add(ev_cmd, NULL);
	event_base_dispatch(base);

	printf("finnished \n");
	return 0;
}



typedef struct sockaddr SA;

int tcp_connect_server(const char* server_ip, int port)
{
	int sockfd, status, save_errno;
	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	status = inet_aton(server_ip,&server_addr.sin_addr);

	if(status == 0){
		errno = EINVAL;
		return -1;
	}

	sockfd = ::socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		return sockfd;

	status = ::connect(sockfd, (SA*)&server_addr, sizeof(server_addr) );

	if(status == -1){
		save_errno = errno;
		::close(sockfd);
		errno = save_errno;
		return -1;
	}

	evutil_make_socket_nonblocking(sockfd);

	return sockfd;
}


void socket_read_cb(int fd, short events, void *arg)
{
	char msg[1024];

	//为了简单起见，不考虑读一半数据的情况
	int len = read(fd, msg, sizeof(msg) -1);
	if(len <= 0){
		perror("read fail");
		exit(1);
	}

	msg[len] = '\0';
	printf("recv %s from server\n", msg);
}

void cmd_msg_cb(int fd, short events, void *arg)
{
	char msg[1024];

	int ret = read(fd, msg, sizeof(msg));

	if(ret <= 0){
		perror("read fail ");
		exit(1);
	}

	int sockfd = *((int *)arg);

	//把终端消息发送给服务器
	//为了简单起见， 不考虑一半数据的情况
	write(sockfd, msg, ret);
}








//###################
//STEP 2
//



//SERVICE
//


#include<stdio.h>
#include<string.h>
#include<errno.h>

#include<event.h>
#include<event2/bufferevent.h>

int tcp_server_init(int port, int listen_num);
void accept_cb(int fd, short events, void *arg);
void socket_read_cb(bufferevent* bev, void *arg);
void evnet_cb(struct bufferevent *bev, short event,void *arg);

int main(int argc, char **argv)
{
	int listener = tcp_server_init(9999, 10);
	if(listener == -1){
		perror("tcp_server_init error");
		return -1;
	}

	struct event_base *base = event_base_new();

	//添加监听客户端请求连接事件
	
	struct event* ev_listen = event_new(base, listener, EV_READ | EV_PERSIST,
				accept_cb, base);
	event_add(ev_listen, NULL);


	event_base_dispatch(base);
	event_base_free(base);

	return 0;

}



typedef struct sockaddr SA;

int tcp_server_init(int port, int listen_num)
{
	int errno_save;
	evutil_socket_t  listener;


	listener = ::socket(AF_INET, SOCK_STREAM, 0);
	if(listener == -1)
		return -1;

	//允许多次绑定同一个地址， 要用在socket和bind之间
	
	evutil_make_listen_socket_reuseable(listener);

	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(port);

	if(::bind(listener, (SA*)&sin, sizeof(sin))  < 0)
		goto error;

	if(::listen(listener, listen_num) < 0)
		goto error;

	//跨平台统一接口，将套接字设置为非阻塞状态
	evutil_make_socket_nonblocking(listener);

	return listener;

	error:
		errno_save = errno;
		evutil_closesockket(listener);
		errno = errno_save;

		return -1;
}

void accept_cb(int fd, short events, void *arg)
{
	evutil_socket_t sockfd;

	struct sockaddr_in client;
	socklen_t len;

	sockfd = ::accept(fd, (struct sockaddr *)&client, &len);
	evutil_make_socket_nonblocking(sockfd);

	printf("accept a client %d\n", sockfd);

	struct event_base* base = (event_base*)arg;

	bufferevent* bev = bufferevent_socket_new(base, sockfd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, socket_read_cb, NULL, evnet_cb, arg);

	bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

void socket_read_cb(bufferevent* bev, void *arg)
{
	char msg[4096];

	size_t len = bufferevent_read(bev, msg, sizeof(msg));

	msg[len] = '\0';

	printf("recv the client msg: %s\n", msg);

	char reply_msg[4096] = "I have recvieced the msg: ";

	strcat(reply_msg + strlen(reply_msg), msg);

	bufferevent_write(bev, reply_msg, strlen(reply_msg));
}

void event_cb(struct bufferevent *bev, short event, void *arg)
{
	if(evnet & BEV_EVENT_EOF)
		printf("connection closed\n");
	else if(event & BEV_EVENT_ERROR)
		printf("some other error\n");

	bufferevent_free(bev);
}


//###
//Client


#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<unistd.h>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<event.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<event2/util.h>

int tcp_connect_server(const char* server_ip, int port);


void cmd_msg_cb(int fd, short events, void *arg);
void server_msg_cb(struct bufferevent* bev, void *arg);
void event_cb(struct bufferevent* bev, short event, void *arg);

int main(int argc , char **argv)
{
	if( argc < 3 )
  {
    printf("please input 2 parameter\n");
    return -1;
  }


  //两个参数依次是服务器端的IP地址、端口号
  int sockfd = tcp_connect_server(argv[1], atoi(argv[2]));
  if( sockfd == -1)
  {
    perror("tcp_connect error ");
    return -1;
  }

  printf("connect to server successful\n");

  struct event_base* base = event_base_new();

  struct  bufferevent* bev = bufferevent_socket_new(base, sockfd,
  				BEV_OPT_CLOSE_ON_FREE);

  //监听终端输入事件
  struct event* ev_cmd = event_new(base, STDIN_FILENO,
  				EV_READ | EV_PERSIST, cmd_msg_cb, (void*)bev);
  event_add(ev_cmd, NULL);

  //当socket关闭时会用到回调函数
  
  bufferevent_setcb(bev,server_msg_cb, NULL, event_cb, (void*)ev_cmd);
  bufferevent_enable(bev, EV_READ | EV_PERSIST);

  event_base_dispatch(base);

  printf("finished \n");
  return 0;
}


typedef struct sockaddr SA;

int tcp_connect_server(const char *server_ip, int port)
{
	int sockfd, status, save_errno;
  struct sockaddr_in server_addr;

  memset(&server_addr, 0, sizeof(server_addr) );

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  status = inet_aton(server_ip, &server_addr.sin_addr);

  if( status == 0 ) //the server_ip is not valid value
  {
    errno = EINVAL;
    return -1;
  }

  sockfd = ::socket(PF_INET, SOCK_STREAM, 0);
  if( sockfd == -1 )
    return sockfd;


  status = ::connect(sockfd, (SA*)&server_addr, sizeof(server_addr) );

  if( status == -1 )
  {
    save_errno = errno;
    ::close(sockfd);
    errno = save_errno; //the close may be error
    return -1;
  }

  evutil_make_socket_nonblocking(sockfd);

  return sockfd;
}

void cmd_msg_cb(int fd, short events, void *arg)
{
	char msg[1024];

	int ret = read(fd, msg, sizeof(msg));
	if(ret < 0){
		perror("read fail ");
		exit(1);
	}

	struct bufferevent* bev = (struct bufferevent*)arg;


	//把终端的消息发送给服务器
	bufferevent_write(bev, msg, ret);
}

void server_msg_cb(struct bufferevent* bev, void *arg)
{
	char msg[1024];

	size_t len = bufferevent_read(bev, msg, sizeof(msg));
	msg[len] = '\0';

	printf("recv %s from server\n", msg);
}

void event_cb(struct bufferevent *bev, short event, void *arg)
{
	if(event & BEV_EVENT_EOF)
		printf("connection closed\n");
	else if(event & BEV_EVENT_ERROR)
		printf("some other error\n");

	 //这将自动close套接字和free读写缓冲区
    bufferevent_free(bev);

    struct event *ev = (struct event*)arg;
       //因为socket已经没有，所以这个event也没有存在的必要了
    event_free(ev);
}

//####################
//
//STEP 3
//


//Service
//


#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>

#include<stdio.h>
#include<string.h>

#include<event.h>
#include<listener.h>
#include<bufferevent.h>
#include<thread.h>


void listener_cb(evconnlistener *listener, evutil_socket_t fd,
			struct sockaddr *sock, int socklen, void *arg);
int main(int argc , char **argv)
{
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(9999);

	event_base *base = event_base_new();

	evconnlistener *listener 
		= evconnlistener_new_bind(base, listener_cb, base,
						LEV_OPT_REUSEABLE | LEV_CLOSE_ON_FREE,
						10, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));

	event_base_dispatch(base);

	evconnlistener_free(listener);

	event_base_free(base);

	return 0;
}

//一个新客户端连接上服务器了

void listener_cb(evconnlistener *listener, evutil_socket_t fd,
			struct sockaddr *sock, int socklen, void *arg)
{
	printf("accept a cient %d\n", fd);

	event_base *base = (event_base *) arg;

	//为这个客户端分配一个bufferevent
	
	bufferevent *bev = bufferevent_socket_new(base, fd, 
						BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, socket_read_cb, NULL, socket_event_cb, NULL);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

void socket_read_cb(bufferevent *bev, void *arg)
{
	char msg[4096];

	size_t len = bufferevent_read(bev, msg, sizeof(msg) -1);

	msg[len] = '\0';

	printf("server read the data %s\n", msg);

	char reply[] = "I has read your data.";
	bufferevent_write(bev, reply, strlen(reply));
}

void socket_event_cb(bufferevent *bev, short events, void *arg)
{

	if(events & BEV_EVENT_EOF)
		printf("connection closed\n");
	else if(events & BEV_EVENT_ERROR)
		printf("some other error\n");

	 //这将自动close套接字和free读写缓冲区
    bufferevent_free(bev);
}


//###
//Client
//


#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<unistd.h>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<event.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<event2/util.h>




void cmd_msg_cb(int fd, short events, void* arg);
void server_msg_cb(struct bufferevent* bev, void* arg);
void event_cb(struct bufferevent *bev, short event, void *arg);

int main(int argc, char ** argv)
{
	 if( argc < 3 )
  {
    //两个参数依次是服务器端的IP地址、端口号
    printf("please input 2 parameter\n");
    return -1;
  }

  struct event_base *base = event_base_new();

  struct bufferevent* bev = bufferevent_socket_new(base, -1,
  							BEV_OPT_CLOSE_ON_FREE);
  //监听终端输入事件
  
  struct event *ev_cmd = event_new(base, STDIN_FILENO,
  					EV_READ | EV_PERSIST,
  					cmd_msg_cb, (void*)bev);
  event_add(ev_cmd, NULL);

  struct sockaddr_in server_addr;

  memset(&server_addr, 0, sizeof(server_addr) );

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[2]));
  inet_aton(argv[1], &server_addr.sin_addr);

  bufferevent_socket_connect(bev, (struct sockaddr *)&server_addr,
                 sizeof(server_addr));

  bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void*)ev_cmd);
  bufferevent_enable(bev, EV_READ | EV_PERSIST);

  event_base_dispatch(base);


  printf("finished \n");

  return 0;
}

void cmd_msg_cb(int fd, short events, void* arg)
{
  char msg[1024];

  int ret = read(fd, msg, sizeof(msg));
  if( ret < 0 )
  {
    perror("read fail ");
    exit(1);
  }

  struct bufferevent* bev = (struct bufferevent*)arg;

  //把终端的消息发送给服务器端
  bufferevent_write(bev, msg, ret);
}

void server_msg_cb(struct bufferevent* bev, void* arg)
{
  char msg[1024];

  size_t len = bufferevent_read(bev, msg, sizeof(msg));
  msg[len] = '\0';

  printf("recv %s from server\n", msg);
}

void event_cb(struct bufferevent *bev, short event, void *arg)
{

  if (event & BEV_EVENT_EOF)
    printf("connection closed\n");
  else if (event & BEV_EVENT_ERROR)
    printf("some other error\n");
  else if( event & BEV_EVENT_CONNECTED)
  {
    printf("the client has connected to server\n");
    return ;
  }

  //这将自动close套接字和free读写缓冲区
  bufferevent_free(bev);

  struct event *ev = (struct event*)arg;
  event_free(ev);
}


