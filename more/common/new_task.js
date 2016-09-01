var amqp = require('amqplib/callback_api');

amqp.connect("amqp://root:123456@192.168.3.21:5672/", function(err, conn){

    conn.createChannel(function(err, ch){
        var q = 'hello';
        var msg = process.argv.slice(2).join(' ') || "Hello World!";

        ch.assertQueue(q, {durable:true});
        ch.sendToQueue(q, new Buffer(msg), {persistent: true});
        console.log(" [x] Sent '%s'", msg);
    });
    setTimeout(function() { conn.close(); process.exit(0) }, 500);

})