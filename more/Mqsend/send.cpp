/*
 * send.cpp
 *
 *  Created on: 2016年3月21日
 *      Author: koala
 */

/*#include "AMQPcpp.h"

int main(int argc , char **argv)
{
	AMQP amqp("amqp://root:123456@192.168.3.21:5672/");

	AMQPExchange *ex = amqp.createExchange();
	ex->Declare();

	AMQPQueue *qu2 = amqp.createQueue("hello");
  qu2->Declare();
  qu2->Bind("", "");

   ex->Publish("FUCK YOU", "");
	return 0;

}*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
#include "utils.h"

int main(int argc, char const *const *argv)
{
	 char const *hostname;
	  int port, status;
	  char const *exchange;
	  char const *exchangetype;
	  char const *messagebody;
	  amqp_socket_t *socket = NULL;
	  amqp_connection_state_t conn;


	  amqp_bytes_t queuename;

//	  if (argc < 6) {
//	    fprintf(stderr, "Usage: amqp_sendstring host port exchange routingkey messagebody\n");
//	    return 1;
//	  }

	//amqp_sendstring 192.168.3.21 5672 amq.direct test "hello world"
	  hostname = "192.168.3.21";
	  port = 5672;
	  exchange = "logs";
	  exchangetype = "fanout";
	  messagebody = "hello world you c++";

	  conn = amqp_new_connection();

	  socket = amqp_tcp_socket_new(conn);
	  if (!socket) {
	    die("creating TCP socket");
	  }

	  status = amqp_socket_open(socket, hostname, port);
	  if (status) {
	    die("opening TCP socket");
	  }

	  die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "root", "123456"),
	                    "Logging in");
	  amqp_channel_open(conn, 1);
	  die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

	  {
	  	  /**/
	  	  amqp_exchange_declare(conn, 1, amqp_cstring_bytes(exchange), amqp_cstring_bytes(exchangetype),
	  	                          0, 0, 0, 0, amqp_empty_table);
	  	    die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring exchange");
	    }


//	  {
//	     amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1,  amqp_cstring_bytes(routingkey), 0, 0, 0, 0,
//	                                  amqp_empty_table);
//	     die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
//	     queuename = amqp_bytes_malloc_dup(r->queue);
//	     if (queuename.bytes == NULL) {
//	       fprintf(stderr, "Out of memory while copying queue name");
//	       return 1;
//	     }
//	     printf("%s\n", queuename.bytes);
//	  }

	  {
	    amqp_basic_properties_t props;
	    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	    props.content_type = amqp_cstring_bytes("text/plain");
	    props.delivery_mode = 2; /* persistent delivery mode */
	    die_on_error(amqp_basic_publish(conn,
	                                    1,
										amqp_cstring_bytes(exchange),
										amqp_empty_bytes,
	                                    0,
	                                    0,
	                                    NULL,//&props,
	                                    amqp_cstring_bytes(messagebody)),
	                 "Publishing");
	  }

	  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
	  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
	  die_on_error(amqp_destroy_connection(conn), "Ending connection");
	  return 0;
}



