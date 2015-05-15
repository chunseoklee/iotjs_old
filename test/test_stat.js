var fs = require('fs');

var statbuf = fs.statSync('test_stat.js');
print(statbuf.mode);
