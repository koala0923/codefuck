var amqp = require('amqplib/callback_api');

amqp.connect('amqp://root:123456@192.168.3.21:5672/', function(err, conn) {
    conn.createChannel(function(err, ch) {
        var ex = 'logs';
        var msg = process.argv.slice(2).join(' ') || 'Hello World!';

        /*定义交换机*/
        ch.assertExchange(ex, 'fanout', {durable: false});

        /*发布消息到交换机*/
        ch.publish(ex, '', new Buffer(msg));
        console.log(" [x] Sent %s", msg);
    });

    setTimeout(function() { conn.close(); process.exit(0) }, 500);
});