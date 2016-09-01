/**
 * Created by user01 on 2016/8/26.
 */
require process;

var handleInput = function handleInput(data) {
    n = parseInt(data.toString());
    console.log(n);
};
process.stdin.on('data', handleInput);
