var fs = require('fs');

try{ // fs.statSync throw ex for non-existing file
  var statbuf1 = fs.statSync('../test');
  print("../test dev : " + statbuf1.dev);
  print("../test mode : " + statbuf1.mode);
  print("../test size : " + statbuf1.size);

  if(statbuf1.isDirectory()){
    print("../test is a directory");
  }
  else {
    print("../test is not a directory");
  }
}
catch(ex){}


try{ // fs.statSync throw ex for non-existing file
  var statbuf2 = fs.statSync('test_stat.js');

  print("test_stat.js dev : " + statbuf2.dev);
  print("test_stat.js mode : " + statbuf2.mode);
  print("test_stat.js size : " + statbuf2.size);

  if(statbuf2.isDirectory()){
    print("test_stat.js is a directory");
  }
  else {
    print("test_stat.js is not a directory");
  }
}
catch(ex){}

try{ // fs.statSync throw ex for non-existing file
  print("non-existing file(non_existing.js) statSync tried.");
  var statbuf3 = fs.statSync('non_existing.js');
}
catch(ex){
  print("but, failed.");
}
