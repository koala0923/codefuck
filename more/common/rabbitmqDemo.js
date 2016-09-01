var amqp = require('amqplib/callback_api');

amqp.connect('amqp://root:123456@192.168.3.21:5672/', function(err, conn) {
    conn.createChannel(function(err, ch) {
        var routekey = 'webNode1';
        var ex = 'webToc';
        var msg = routekey +  "|" + (process.argv.slice(2).join(' ') || 'Hello World!');

        /*定义交接机*/
        ch.assertExchange(ex, 'fanout', {durable: false});

        /*发布消息到交换机*/
        ch.publish(ex, '', new Buffer(msg));
        console.log(" [x] Sent message: '%s '", msg);

        /*接收返回消息*/
        conn.createChannel(function(err, ch){

            var ex = 'cToweb';

            /*定义交换机*/
            ch.assertExchange(ex, 'direct', {durable: false});

            /*定义队列*/
            ch.assertQueue('', {exclusive: true}, function(err, q) {
                console.log(' [*] Waiting for Return  Message. To exit press CTRL+C');

                /*绑定交换机， 队列， 路由关键字*/
                ch.bindQueue(q.queue, ex, routekey);

                ch.consume(q.queue, function(msg) {
                    console.log(" [x] routeKey: %s ---  message: '%s'", msg.fields.routingKey, msg.content.toString());
                    setTimeout(function() { conn.close(); process.exit(0) }, 300);
                }, {noAck: true});
            });
        })


    });

   // setTimeout(function() { conn.close(); process.exit(0) }, 500);
});

