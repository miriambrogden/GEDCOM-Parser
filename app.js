'use strict'

var connection;
var mysql = require('mysql');;

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }
    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);

app.get('/uploadFile', function(req, res){
	let fs = require('fs'); 
	fs.readdir('./uploads', function(err, items) {
		res.send(items);
	});
});

var sharedLib = ffi.Library('./myLibrary', {
	'tableInfo' : [ 'string', [ 'string']],
	'indInfo' : [ 'string', ['string']],
	'createSimple' : [ 'string', ['string', 'string', 'string']],
	'addTheIndi' : ['string', ['string', 'string']],
	'desToJSON' : ['string', ['string', 'string', 'int']],
	'ansToJSON' : ['string', ['string', 'string', 'int']],
	'validateFile' : ['string', ['string']],
});

app.get('/tableFunction', function(req, res){
	let JSON = sharedLib.tableInfo(req.query.fileName);
	res.send(JSON);
});

app.get('/indFunction', function(req, res){
	let JSON = sharedLib.indInfo(req.query.fileName);
	res.send(JSON);
});

app.get('/createSimple', function(req, res){
	let string = sharedLib.createSimple(req.query.fileName, req.query.subName, req.query.subAdd);
	res.send(string);
});

app.get('/addIndi', function(req, res){
	let string = sharedLib.addTheIndi(req.query.fileName, req.query.theString);
	res.send(string);
});

app.get('/getDes', function(req, res){
	let JSON = sharedLib.desToJSON(req.query.fileName, req.query.theString, req.query.max);
	res.send(JSON);
});

app.get('/getAns', function(req, res){
	let JSON = sharedLib.ansToJSON(req.query.fileName, req.query.theString, req.query.max);
	res.send(JSON);
});

app.get('/valFile', function(req, res){
	let string = sharedLib.validateFile(req.query.fileName);
	res.send(string);
});

app.get('/connect', function(req, res){
	connection = mysql.createConnection({
		host     : 'dursley.socs.uoguelph.ca',
		user     : req.query.username,
		password : req.query.password,
		database : req.query.dbname
	});
	connection.connect(function(err){
		let x;
		if(err) {
			x = "ERROR";
		} else {
			x = "FINE";
		}
		let params = "CREATE TABLE FILE (file_id INT AUTO_INCREMENT PRIMARY KEY, file_Name VARCHAR(60) NOT NULL, source VARCHAR(250) NOT NULL, version VARCHAR(10) NOT NULL, encoding VARCHAR(10) NOT NULL, sub_name VARCHAR(62) NOT NULL, sub_addr VARCHAR(256), num_individials INT, num_families INT)";
		connection.query(params, function(err, result) {
		});
		let params2 = "CREATE TABLE INDIVIDUAL (ind_id INT AUTO_INCREMENT PRIMARY KEY, surname VARCHAR(256) NOT NULL, given_name VARCHAR(256) NOT NULL, sex VARCHAR(1), fam_size INT, source_file INT, FOREIGN KEY(source_file) REFERENCES FILE(file_id) ON DELETE CASCADE)";
		connection.query(params2, function(err, result) {
		});
		res.send(x);
	});
});

app.get('/store', function(req, res){
	let temp = "INSERT INTO FILE (file_Name, source, version, encoding, sub_name, sub_addr, num_individials, num_families) VALUES ('"+req.query.fileName + "','" + req.query.source + "','" + req.query.version + "','" + req.query.encoding + "','" + req.query.name + "','" + req.query.address + "','" + req.query.indis + "','" + req.query.fams +"')";
	connection.query(temp, function (err, result) {
	});
	res.send();
});

app.get('/storeIndis', function(req, res){
	let temp = "INSERT INTO INDIVIDUAL (surname, given_name, sex, fam_size, source_file) VALUES ('"+req.query.surname + "','" + req.query.givenName + "','" + req.query.sex + "','" + req.query.familySize + "','" + req.query.fileId + "')";
	connection.query(temp, function (err, result) {
	});
	res.send();
});

app.get('/countTables', function(req,res){
	let temp = "SELECT COUNT (DISTINCT file_Name) FROM FILE";
	let obj = new Object();
	connection.query(temp, function(err, result){
		let me = JSON.stringify(result[0]).split(":");
		let fileVal = me[1].split("}");
		obj.file = fileVal[0];
		res.send(obj);
	});
});

app.get('/countTablesToo', function(req,res){
	let temp = "SELECT COUNT (DISTINCT ind_id) FROM INDIVIDUAL";
	let obj = new Object();
	connection.query(temp, function(err, result){
		let me = JSON.stringify(result[0]).split(":");
		let fileVal = me[1].split("}");
		obj.file = fileVal[0];
		res.send(obj);
	});
});

app.get('/clearTables', function(req, res){
	let temp = "DELETE FROM FILE";
	let temp2 = "DELETE FROM INDIVIDUAL";
	connection.query(temp, function (err, result) {
	});
	connection.query(temp2, function (err, result) {
	});
	res.send();
});
 
app.get('/getFileID', function(req, res){
	let temp = "SELECT file_id FROM FILE WHERE file_Name = '" + req.query.fileName + "'";
	let obj = new Object();
	connection.query(temp, function(err, result){
		if (result.length == 0){
			obj.file = -1;
		} else {
			obj.file = result[0].file_id;
		}
		res.send(obj);
	});
});

app.get('/getFileNAME', function(req, res){
	let temp = "SELECT * FROM FILE WHERE file_id = " + req.query.fileId;
	let obj = new Object();
	connection.query(temp, function(err, result){
		let array = [];
		for (let i=0; i < result.length; i++){
			let obj = new Object();
			obj.id = result[i].file_id;
			obj.name = result[i].file_Name;
			obj.source = result[i].source;
			obj.version = result[i].version;
			obj.encoding = result[i].encoding;
			obj.subName = result[i].sub_name;
			obj.subAdd = result[i].sub_addr;
			obj.numIndis = result[i].num_individials;
			obj.numFams = result[i].num_families;
			let theString = JSON.stringify(obj);
			array.push(theString);
		}
		res.send(array);
	});
});

app.get('/surNameDisplay', function(req, res){
	let temp = "SELECT * FROM INDIVIDUAL WHERE surname = '" + req.query.surname + "' ORDER BY source_file, given_name ASC";
	connection.query(temp, function(err, result){
		let array = [];
		for (let i=0; i < result.length; i++){
			let obj = new Object();
			obj.id = result[i].ind_id;
			obj.givenName = result[i].given_name;
			obj.surname = result[i].surname;
			obj.sex = result[i].sex;
			obj.famSize = result[i].fam_size;
			obj.fileId = result[i].source_file;
			let theString = JSON.stringify(obj);
			array.push(theString);
		}
		res.send(array);
	});
});

app.get('/customOne', function(req, res){
	let temp = req.query.statement;
	connection.query(temp, function(err, result){
		let array = [];
		if (result != undefined){
			for (let i=0; i < result.length; i++){
				let theString = JSON.stringify(result[i]);
				array.push(theString);
			}
		}
		res.send(array);
	});
});



app.get('/famSizeDisplay', function(req, res){
	let temp = "SELECT * FROM INDIVIDUAL WHERE fam_size >= " + req.query.min + " AND fam_size <= " + req.query.max + " ORDER BY surname ASC";
	connection.query(temp, function(err, result){
		let array = [];
		for (let i=0; i < result.length; i++){
			let obj = new Object();
			obj.id = result[i].ind_id;
			obj.givenName = result[i].given_name;
			obj.surname = result[i].surname;
			obj.sex = result[i].sex;
			obj.famSize = result[i].fam_size;
			obj.fileId = result[i].source_file;
			let theString = JSON.stringify(obj);
			array.push(theString);
		}
		res.send(array);
	});
});

app.get('/optOne', function(req, res){
	let temp = "SELECT * FROM INDIVIDUAL ORDER BY surname ASC";
	connection.query(temp, function(err, result){
		let array = [];
		for (let i=0; i < result.length; i++){
			let obj = new Object();
			obj.id = result[i].ind_id;
			obj.givenName = result[i].given_name;
			obj.surname = result[i].surname;
			obj.sex = result[i].sex;
			obj.famSize = result[i].fam_size;
			obj.fileId = result[i].source_file;
			let theString = JSON.stringify(obj);
			array.push(theString);
		}
		res.send(array);
	});
});

app.get('/getIndsFromFile', function(req, res){
	let temp = "SELECT * FROM INDIVIDUAL WHERE source_file = " + req.query.id + " ORDER BY surname ASC";
	connection.query(temp, function(err, result){
		let array = [];
		for (let i = 0; i< result.length; i++){
			let obj = new Object();
			obj.id = result[i].ind_id;
			obj.givenName = result[i].given_name;
			obj.surname = result[i].surname;
			obj.sex = result[i].sex;
			obj.famSize = result[i].fam_size;
			obj.fileId = result[i].source_file;
			let theString = JSON.stringify(obj);
			array.push(theString);
		}
		res.send(array);
	});
});

app.get('/getsourcefromindi', function(req, res){
	let temp = "SELECT source_file FROM INDIVIDUAL WHERE surname = '" + req.query.surname + "' AND given_name = '" + req.query.givenName + "'";
	connection.query(temp, function(err, result){
		let array = [];
		for (let i = 0; i< result.length; i++){
			let obj = new Object();
			obj.id = result[i].ind_id;
			obj.givenName = result[i].given_name;
			obj.surname = result[i].surname;
			obj.sex = result[i].sex;
			obj.famSize = result[i].fam_size;
			obj.fileId = result[i].source_file;
			let theString = JSON.stringify(obj);
			array.push(theString);
		}
		res.send(array);
	});
});
