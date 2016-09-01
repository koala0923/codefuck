var amqp = require('amqplib/callback_api');

amqp.connect("amqp://root:123456@192.168.3.21:5672/", function(err, conn){
    conn.createChannel(function(err, ch){
       var q = 'hello';

        ch.assertQueue(q, {durable:false});
        ch.sendToQueue(q, new Buffer('Hello World  node.js .'));
        console.log("[x] Sent 'Hello World' ");
    });
})