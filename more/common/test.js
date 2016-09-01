var aQuery = function(selector, context) {
    return  new aQuery.prototype.init();
}
aQuery.prototype = {
    init: function() {
        this.age = 18
        return this;
    },
    name: function() { return "name()"},
    age: 20
}
aQuery.prototype.init.prototype = aQuery.prototype;

aQuery().age  //18

console.log(aQuery().age);
console.log(aQuery().name());