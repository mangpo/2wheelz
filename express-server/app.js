var fs = require('fs')
var express = require('express')
//var bodyParser = require('body-parser')
//var multer  = require('multer')

var app = express()
app.set('port',3030)

// static file server
app.use(express.static('public'));

var server = app.listen(app.get('port'), function () {

  var host = server.address().address
  var port = server.address().port

  console.log('Example app listening at http://%s:%s', host, port)

})



/*
var form = "<!DOCTYPE HTML><html><body>" +
"<form method='post' action='/upload' enctype='multipart/form-data'>" +
"<input type='file' name='image'/>" +
"<input type='submit' /></form>" +
"</body></html>";

app.get('/', function (req, res){
	res.writeHead(200, {'Content-Type': 'text/html' });
	res.end(form);	
});


app.get('/user', function (req, res) {
  res.send('Got a GET request at /user');
  console.log('GET request at /user');
  console.log(req.ip);
  console.log(req.query.q);
  console.log(req.body);
  //fs.writeFileSync('foo.txt',req.query.q);
  console.log('done');
})


app.post('/upload', function (req, res) {
  console.log('Get a POST request at /upload');
  console.log(req.ip);
  console.log(req.get('Content-Type'));
  console.log(req.get('Content-Length'));
  console.log(req.headers);
  console.log(req.body);
  console.log(req.files);
  res.send('Got a POST request at /upload');

})
*/
