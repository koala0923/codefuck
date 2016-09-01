/*
 * receive.cpp
 *
 *  Created on: 2016年3月21日
 *      Author: koala
 */




#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include <assert.h>

#include "utils.h"


/*amqp_listen.c  默认的队列名*/
int main(int argc, char const *const *argv)
{
  char const *hostname;
  int port, status;
  char const *exchange;
  char const *exchangetype;
  amqp_socket_t *socket = NULL;
  amqp_connection_state_t conn;

  amqp_bytes_t queuename;

  hostname = "192.168.3.21";
  port = 5672;
  exchange = "logs";
  exchangetype ="fanout";

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

  {
    amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, , 0,
                                 amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
    queuename = amqp_bytes_malloc_dup(r->queue);
    if (queuename.bytes == NULL) {
      fprintf(stderr, "Out of memory while copying queue name");
      return 1;
    }
  }

  {

  amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(exchange), amqp_empty_bytes,
                  amqp_empty_table);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");

  }
  amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

  {
    while (1) {
      amqp_rpc_reply_t res;
      amqp_envelope_t envelope;

      amqp_maybe_release_buffers(conn);

      res = amqp_consume_message(conn, &envelope, NULL, 0);

      if (AMQP_RESPONSE_NORMAL != res.reply_type) {
        break;
      }

      printf("Delivery %u, exchange %.*s routingkey %.*s\n",
             (unsigned) envelope.delivery_tag,
             (int) envelope.exchange.len, (char *) envelope.exchange.bytes,
             (int) envelope.routing_key.len, (char *) envelope.routing_key.bytes);

      if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
        printf("Content-type: %.*s\n",
               (int) envelope.message.properties.content_type.len,
               (char *) envelope.message.properties.content_type.bytes);
      }
      printf("----\n");

      amqp_dump(envelope.message.body.bytes, envelope.message.body.len);

      amqp_destroy_envelope(&envelope);
    }
  }

  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
  die_on_error(amqp_destroy_connection(conn), "Ending connection");

  return 0;
}




///*amqp_listenq.c 自定义队列名*/
//int main(int argc, char const *const *argv)
//{
//  char const *hostname;
//  int port, status;
//  char const *queuename;
//  amqp_socket_t *socket = NULL;
//  amqp_connection_state_t conn;
//
////  if (argc < 4) {
////    fprintf(stderr, "Usage: amqp_listenq host port queuename\n");
////    return 1;
////  }
// /*192.168.3.21 5672 amq.direct test*/
//  hostname = "192.168.3.21";
//  port = 5672;
//  queuename = "hello";
//
//  conn = amqp_new_connection();
//
//  socket = amqp_tcp_socket_new(conn);
//  if (!socket) {
//    die("creating TCP socket");
//  }
//
//  status = amqp_socket_open(socket, hostname, port);
//  if (status) {
//    die("opening TCP socket");
//  }
//
//  die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "root", "123456"),
//                    "Logging in");
//  amqp_channel_open(conn, 1);
//  die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");
//
//  	  {
//	       amqp_bytes_t Getqueuename;
//  	     amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1,  amqp_cstring_bytes(queuename), 0, 1, 0, 0,
//  	                                  amqp_empty_table);
//  	     die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
//  	   Getqueuename = amqp_bytes_malloc_dup(r->queue);
//  	     if (Getqueuename.bytes == NULL) {
//  	       fprintf(stderr, "Out of memory while copying queue name");
//  	       return 1;
//  	     }
//  	     printf("%s\n", Getqueuename.bytes);
//  	  }
//
//
//  amqp_basic_consume(conn, 1, amqp_cstring_bytes(queuename), amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
//  die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");
//
//  {
//    while (1) {
//      amqp_rpc_reply_t res;
//      amqp_envelope_t envelope;
//
//      amqp_maybe_release_buffers(conn);
//
//      res = amqp_consume_message(conn, &envelope, NULL, 0);
//
//      if (AMQP_RESPONSE_NORMAL != res.reply_type) {
//        break;
//      }
//
//      printf("Delivery %u, exchange %.*s routingkey %.*s\n",
//             (unsigned) envelope.delivery_tag,
//             (int) envelope.exchange.len, (char *) envelope.exchange.bytes,
//             (int) envelope.routing_key.len, (char *) envelope.routing_key.bytes);
//
//      if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
//        printf("Content-type: %.*s\n",
//               (int) envelope.message.properties.content_type.len,
//               (char *) envelope.message.properties.content_type.bytes);
//      }
//      printf("----\n");
//
//      amqp_dump(envelope.message.body.bytes, envelope.message.body.len);
//
//      amqp_destroy_envelope(&envelope);
//    }
//  }
//
//  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
//  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
//  die_on_error(amqp_destroy_connection(conn), "Ending connection");
//
//  return 0;
//}

