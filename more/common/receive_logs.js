var amqp = require('amqplib/callback_api');

amqp.connect('amqp://root:123456@192.168.3.21:5672/', function(err, conn) {
    conn.createChannel(function(err, ch) {
        var ex = 'logs';

        /*定义交换机*/
        ch.assertExchange(ex, 'fanout', {durable: false});

        /*定义队列， 名字随机*/
        ch.assertQueue('', {exclusive: true}, function(err, q) {
            console.log(" [*] Waiting for messages in %s. To exit press CTRL+C", q.queue);
            /*队列绑定到交换机 */
            ch.bindQueue(q.queue, ex, '');

            ch.consume(q.queue, function(msg) {
                console.log(" [x] %s", msg.content.toString());
            }, {noAck: true});
        });
    });
});