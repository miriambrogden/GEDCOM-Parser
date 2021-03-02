var fileInfo = []; 
var indiInfo = [];

$(document).ready(function() {
    loadFiles();
    upFileSubmit();
    clearStatus();
    getIndis();  
    createSubmit();
    addSubmit();
    getDesSubmit();
    getAnsSubmit();
    connect();
    storeFiles();
	clearAll();	
	executeQuery();
	$('input[type=radio]').change( function() {
	   if (document.getElementById("dispAllIndi").checked){
			$("#a").addClass('greyBoarder');
			$("#b").removeClass('greyBoarder');
			$("#c").removeClass('greyBoarder');
			$("#d").removeClass('greyBoarder');
			$("#e").removeClass('greyBoarder');
			$("#f").removeClass('greyBoarder');
		} else if (document.getElementById("dispFileIndi").checked){
			$("#a").removeClass('greyBoarder');
			$("#b").addClass('greyBoarder');
			$("#c").removeClass('greyBoarder');
			$("#d").removeClass('greyBoarder');
			$("#e").removeClass('greyBoarder');
			$("#f").removeClass('greyBoarder');
		} else if (document.getElementById("dispSurnameIndi").checked){
			$("#a").removeClass('greyBoarder');
			$("#b").removeClass('greyBoarder');
			$("#c").addClass('greyBoarder');
			$("#d").removeClass('greyBoarder');
			$("#e").removeClass('greyBoarder');
			$("#f").removeClass('greyBoarder');
		} else if (document.getElementById("dispFamSize").checked){
			$("#a").removeClass('greyBoarder');
			$("#b").removeClass('greyBoarder');
			$("#c").removeClass('greyBoarder');
			$("#d").addClass('greyBoarder');
			$("#e").removeClass('greyBoarder');
			$("#f").removeClass('greyBoarder');
		} else if (document.getElementById("dispFileName").checked){
			$("#a").removeClass('greyBoarder');
			$("#b").removeClass('greyBoarder');
			$("#c").removeClass('greyBoarder');
			$("#d").removeClass('greyBoarder');
			$("#e").addClass('greyBoarder');
			$("#f").removeClass('greyBoarder');
		} else if (document.getElementById("customQue").checked){
			$("#a").removeClass('greyBoarder');
			$("#b").removeClass('greyBoarder');
			$("#c").removeClass('greyBoarder');
			$("#d").removeClass('greyBoarder');
			$("#e").removeClass('greyBoarder');
			$("#f").addClass('greyBoarder');
		}
			 
	});
	
});

function executeQuery(){
	$("#subQueButt").on("click", function(e){
		let tableRefA = document.getElementById('resultsTable').getElementsByTagName('thead')[0];
		if (document.getElementById("dispAllIndi").checked) {
			$.ajax({
				type: 'get',
				url: '/optOne',
				success: function(data){
					if (data.length != 0){
						tableRefA.innerHTML="";
						let newRow = tableRefA.insertRow(tableRefA.rows.length);
						newRow.id = "JuliaJ1";
						document.getElementById("JuliaJ1").style.fontWeight = "900";
						let newCell  = newRow.insertCell(0);
						let newCell2  = newRow.insertCell(1);
						let newCell3  = newRow.insertCell(2);
						let newCell4  = newRow.insertCell(3);
						let newCell5  = newRow.insertCell(4);
						let newCell6  = newRow.insertCell(5);
						let Text = document.createTextNode("Individual ID");
						let Text3 = document.createTextNode("Given Name");				
						let Text2 = document.createTextNode("Surname");				
						let Text4 = document.createTextNode("Sex");				
						let Text5 = document.createTextNode("Family Size");				
						let Text6 = document.createTextNode("File ID");				
						newCell.appendChild(Text);
						newCell2.appendChild(Text2);
						newCell3.appendChild(Text3);
						newCell4.appendChild(Text4);
						newCell5.appendChild(Text5);
						newCell6.appendChild(Text6);
						for (let i = 0; i<data.length; i++){
							let indOb = new Object();
							indOb = JSON.parse(data[i]);
							let newRowm = tableRefA.insertRow(tableRefA.rows.length);
							let newCellm = newRowm.insertCell(0);
							let newCell2m = newRowm.insertCell(1);
							let newCell3m = newRowm.insertCell(2);
							let newCell4m = newRowm.insertCell(3);
							let newCell5m = newRowm.insertCell(4);
							let newCell6m = newRowm.insertCell(5);
							let Textm = document.createTextNode(indOb.id);
							let Text3m = document.createTextNode(indOb.givenName);				
							let Text2m = document.createTextNode(indOb.surname);				
							let Text4m = document.createTextNode(indOb.sex);				
							let Text5m = document.createTextNode(indOb.famSize);				
							let Text6m = document.createTextNode(indOb.fileId);				
							newCellm.appendChild(Textm);
							newCell2m.appendChild(Text2m);
							newCell3m.appendChild(Text3m);
							newCell4m.appendChild(Text4m);
							newCell5m.appendChild(Text5m);
							newCell6m.appendChild(Text6m);
						}
					} else {
						tableRefA.innerHTML="";
						let newRow = tableRefA.insertRow(tableRefA.rows.length);
						newRow.id = "JuliaJ1";
						document.getElementById("JuliaJ1").style.fontWeight = "900";
						let newCell  = newRow.insertCell(0);
						let Text = document.createTextNode("No Results");
						newCell.appendChild(Text);
					}
				},
				error: function(err){
					let node = document.createElement("P");
					let textnode = document.createTextNode("There was an error on the server. Please try again.");
					node.appendChild(textnode);
					document.getElementById("status").appendChild(node);
				}
			});
		} else if (document.getElementById("dispFileIndi").checked){
			let y = document.getElementById("queryFileName");
			let theObject = new Object();
			theObject.fileName = y.value;
			$.ajax({
				type: 'get',
				url: '/getFileID',
				data: theObject,
				success: function(miriam){
					let hi = new Object();
					hi.id = miriam.file;
					$.ajax({
						type: 'get',
						url: '/getIndsFromFile',
						data: hi,
						success: function(guelph){
							if (guelph.length == 0){
								tableRefA.innerHTML="";
								let newRow = tableRefA.insertRow(tableRefA.rows.length);
								newRow.id = "JuliaJ1";
								document.getElementById("JuliaJ1").style.fontWeight = "900";
								let newCell  = newRow.insertCell(0);
								let Text = document.createTextNode("No Results");
								newCell.appendChild(Text);
							} else {
								tableRefA.innerHTML="";
								let newRow = tableRefA.insertRow(tableRefA.rows.length);
								newRow.id = "JuliaJ1";
								document.getElementById("JuliaJ1").style.fontWeight = "900";
								let newCell  = newRow.insertCell(0);
								let newCell2  = newRow.insertCell(1);
								let newCell3  = newRow.insertCell(2);
								let newCell4  = newRow.insertCell(3);
								let newCell5  = newRow.insertCell(4);
								let newCell6  = newRow.insertCell(5);
								let Text = document.createTextNode("Individual ID");
								let Text3 = document.createTextNode("Given Name");				
								let Text2 = document.createTextNode("Surname");				
								let Text4 = document.createTextNode("Sex");				
								let Text5 = document.createTextNode("Family Size");				
								let Text6 = document.createTextNode("File ID");				
								newCell.appendChild(Text);
								newCell2.appendChild(Text2);
								newCell3.appendChild(Text3);
								newCell4.appendChild(Text4);
								newCell5.appendChild(Text5);
								newCell6.appendChild(Text6);
								for (let i = 0; i<guelph.length; i++){
									let indOb = new Object();
									indOb = JSON.parse(guelph[i]);
									let newRowm = tableRefA.insertRow(tableRefA.rows.length);
									let newCellm = newRowm.insertCell(0);
									let newCell2m = newRowm.insertCell(1);
									let newCell3m = newRowm.insertCell(2);
									let newCell4m = newRowm.insertCell(3);
									let newCell5m = newRowm.insertCell(4);
									let newCell6m = newRowm.insertCell(5);
									let Textm = document.createTextNode(indOb.id);
									let Text3m = document.createTextNode(indOb.givenName);				
									let Text2m = document.createTextNode(indOb.surname);				
									let Text4m = document.createTextNode(indOb.sex);				
									let Text5m = document.createTextNode(indOb.famSize);				
									let Text6m = document.createTextNode(indOb.fileId);				
									newCellm.appendChild(Textm);
									newCell2m.appendChild(Text2m);
									newCell3m.appendChild(Text3m);
									newCell4m.appendChild(Text4m);
									newCell5m.appendChild(Text5m);
									newCell6m.appendChild(Text6m);
								}
							}
						}, 
						error: function(badGuelph){
							let node = document.createElement("P");
							let textnode = document.createTextNode("There was an error on the server. Please try again.");
							node.appendChild(textnode);
							document.getElementById("status").appendChild(node);
						}
					});
				},
				error: function(anErr){
					tableRefA.innerHTML="";
					let newRow = tableRefA.insertRow(tableRefA.rows.length);
					newRow.id = "JuliaJ1";
					document.getElementById("JuliaJ1").style.fontWeight = "900";
					let newCell  = newRow.insertCell(0);
					let Text = document.createTextNode("No Results");
					newCell.appendChild(Text);
				}
			});
				
		} else if (document.getElementById("dispSurnameIndi").checked){
			let h = document.getElementById("querySurName");
			let anObject = new Object();
			anObject.surname = h.value;
			$.ajax({
				type: 'get', 
				url: '/surNameDisplay',
				data: anObject,
				success: function(thisisfun){
					if (thisisfun.length == 0){
						tableRefA.innerHTML="";
						let newRow = tableRefA.insertRow(tableRefA.rows.length);
						newRow.id = "JuliaJ1";
						document.getElementById("JuliaJ1").style.fontWeight = "900";
						let newCell  = newRow.insertCell(0);
						let Text = document.createTextNode("No Results");
						newCell.appendChild(Text);
					} else {
						tableRefA.innerHTML="";
						let newRow = tableRefA.insertRow(tableRefA.rows.length);
						newRow.id = "JuliaJ1";
						document.getElementById("JuliaJ1").style.fontWeight = "900";
						let newCell  = newRow.insertCell(0);
						let newCell2  = newRow.insertCell(1);
						let newCell3  = newRow.insertCell(2);
						let newCell4  = newRow.insertCell(3);
						let newCell5  = newRow.insertCell(4);
						let newCell6  = newRow.insertCell(5);
						let Text = document.createTextNode("Individual ID");
						let Text3 = document.createTextNode("Given Name");				
						let Text2 = document.createTextNode("Surname");				
						let Text4 = document.createTextNode("Sex");				
						let Text5 = document.createTextNode("Family Size");				
						let Text6 = document.createTextNode("File ID");				
						newCell.appendChild(Text);
						newCell2.appendChild(Text2);
						newCell3.appendChild(Text3);
						newCell4.appendChild(Text4);
						newCell5.appendChild(Text5);
						newCell6.appendChild(Text6);
						for (let i = 0; i<thisisfun.length; i++){
							let indOb = new Object();
							indOb = JSON.parse(thisisfun[i]);
							let newRowm = tableRefA.insertRow(tableRefA.rows.length);
							let newCellm = newRowm.insertCell(0);
							let newCell2m = newRowm.insertCell(1);
							let newCell3m = newRowm.insertCell(2);
							let newCell4m = newRowm.insertCell(3);
							let newCell5m = newRowm.insertCell(4);
							let newCell6m = newRowm.insertCell(5);
							let Textm = document.createTextNode(indOb.id);
							let Text3m = document.createTextNode(indOb.givenName);				
							let Text2m = document.createTextNode(indOb.surname);				
							let Text4m = document.createTextNode(indOb.sex);				
							let Text5m = document.createTextNode(indOb.famSize);				
							let Text6m = document.createTextNode(indOb.fileId);				
							newCellm.appendChild(Textm);
							newCell2m.appendChild(Text2m);
							newCell3m.appendChild(Text3m);
							newCell4m.appendChild(Text4m);
							newCell5m.appendChild(Text5m);
							newCell6m.appendChild(Text6m);
						}
					}					
				}, 
				error: function(thisisnotfun){
					let node = document.createElement("P");
					let textnode = document.createTextNode("There was an error on the server. Please try again.");
					node.appendChild(textnode);
					document.getElementById("status").appendChild(node);
				}
			});			
		} else if (document.getElementById("dispFamSize").checked){
			let f = document.getElementById("querymax");
			let p = document.getElementById("querymin");
			let pineapple = new Object();
			pineapple.max = f.value;
			pineapple.min = p.value;
			$.ajax({
				type: 'get',
				url: '/famSizeDisplay',
				data: pineapple,
				success: function(apple){
					if (apple.length == 0){
						tableRefA.innerHTML="";
						let newRow = tableRefA.insertRow(tableRefA.rows.length);
						newRow.id = "JuliaJ1";
						document.getElementById("JuliaJ1").style.fontWeight = "900";
						let newCell  = newRow.insertCell(0);
						let Text = document.createTextNode("No Results");
						newCell.appendChild(Text);
					} else {
						tableRefA.innerHTML="";
						let newRow = tableRefA.insertRow(tableRefA.rows.length);
						newRow.id = "JuliaJ1";
						document.getElementById("JuliaJ1").style.fontWeight = "900";
						let newCell  = newRow.insertCell(0);
						let newCell2  = newRow.insertCell(1);
						let newCell3  = newRow.insertCell(2);
						let newCell4  = newRow.insertCell(3);
						let newCell5  = newRow.insertCell(4);
						let newCell6  = newRow.insertCell(5);
						let Text = document.createTextNode("Individual ID");
						let Text3 = document.createTextNode("Given Name");				
						let Text2 = document.createTextNode("Surname");				
						let Text4 = document.createTextNode("Sex");				
						let Text5 = document.createTextNode("Family Size");				
						let Text6 = document.createTextNode("File ID");				
						newCell.appendChild(Text);
						newCell2.appendChild(Text2);
						newCell3.appendChild(Text3);
						newCell4.appendChild(Text4);
						newCell5.appendChild(Text5);
						newCell6.appendChild(Text6);
						for (let i = 0; i<apple.length; i++){
							let indOb = new Object();
							indOb = JSON.parse(apple[i]);
							let newRowm = tableRefA.insertRow(tableRefA.rows.length);
							let newCellm = newRowm.insertCell(0);
							let newCell2m = newRowm.insertCell(1);
							let newCell3m = newRowm.insertCell(2);
							let newCell4m = newRowm.insertCell(3);
							let newCell5m = newRowm.insertCell(4);
							let newCell6m = newRowm.insertCell(5);
							let Textm = document.createTextNode(indOb.id);
							let Text3m = document.createTextNode(indOb.givenName);				
							let Text2m = document.createTextNode(indOb.surname);				
							let Text4m = document.createTextNode(indOb.sex);				
							let Text5m = document.createTextNode(indOb.famSize);				
							let Text6m = document.createTextNode(indOb.fileId);				
							newCellm.appendChild(Textm);
							newCell2m.appendChild(Text2m);
							newCell3m.appendChild(Text3m);
							newCell4m.appendChild(Text4m);
							newCell5m.appendChild(Text5m);
							newCell6m.appendChild(Text6m);
						}
					}					
				}, 
				error: function(broccoli){
					let node = document.createElement("P");
					let textnode = document.createTextNode("There was an error on the server. Please try again.");
					node.appendChild(textnode);
					document.getElementById("status").appendChild(node);
				}
			});
		} else if (document.getElementById("dispFileName").checked){
			let r = document.getElementById("queryTheFirstName");
			let v = document.getElementById("queryTheLastName");
			let peppermint = new Object();
			peppermint.givenName = r.value;
			peppermint.surname = v.value;
			$.ajax({
				type: 'get',
				url: '/getsourcefromindi',
				data: peppermint,
				success: function(muffin){
					if (muffin.length != 0){
						tableRefA.innerHTML="";
						let newRow = tableRefA.insertRow(tableRefA.rows.length);
						newRow.id = "JuliaJ1";
						document.getElementById("JuliaJ1").style.fontWeight = "900";
						let newCell  = newRow.insertCell(0);
						let newCell2  = newRow.insertCell(1);
						let newCell3  = newRow.insertCell(2);
						let newCell4  = newRow.insertCell(3);
						let newCell5  = newRow.insertCell(4);
						let newCell6  = newRow.insertCell(5);
						let newCell7  = newRow.insertCell(6);
						let newCell8  = newRow.insertCell(7);
						let newCell9  = newRow.insertCell(8);
						let Text = document.createTextNode("File ID");
						let Text3 = document.createTextNode("File Name");				
						let Text2 = document.createTextNode("Source");				
						let Text4 = document.createTextNode("Version");				
						let Text5 = document.createTextNode("Encoding");				
						let Text6 = document.createTextNode("Submitter Name");				
						let Text7 = document.createTextNode("Submitter Address");				
						let Text8 = document.createTextNode("# Individuals");				
						let Text9 = document.createTextNode("# Families");				
						newCell.appendChild(Text);
						newCell2.appendChild(Text2);
						newCell3.appendChild(Text3);
						newCell4.appendChild(Text4);
						newCell5.appendChild(Text5);
						newCell6.appendChild(Text6);			
						newCell7.appendChild(Text7);			
						newCell8.appendChild(Text8);			
						newCell9.appendChild(Text9);			
						for (let o = 0; o < muffin.length; o++){
							let bob = new Object();
							bob = JSON.parse(muffin[o]);
							$.ajax({
								type: 'get',
								url: '/getFileNAME',
								data: bob,
								success: function(timothy){
									let indOb = new Object();
									indOb = JSON.parse(timothy);
									let newRowm = tableRefA.insertRow(tableRefA.rows.length);
									let newCellm = newRowm.insertCell(0);
									let newCell2m = newRowm.insertCell(1);
									let newCell3m = newRowm.insertCell(2);
									let newCell4m = newRowm.insertCell(3);
									let newCell5m = newRowm.insertCell(4);
									let newCell6m = newRowm.insertCell(5);
									let newCell7m = newRowm.insertCell(6);
									let newCell8m = newRowm.insertCell(7);
									let newCell9m = newRowm.insertCell(8);
									let Textm = document.createTextNode(indOb.id);
									let Text3m = document.createTextNode(indOb.name);				
									let Text2m = document.createTextNode(indOb.source);				
									let Text4m = document.createTextNode(indOb.version);				
									let Text5m = document.createTextNode(indOb.encoding);				
									let Text6m = document.createTextNode(indOb.subName);				
									let Text7m = document.createTextNode(indOb.subAdd);				
									let Text8m = document.createTextNode(indOb.numIndis);				
									let Text9m = document.createTextNode(indOb.numFams);				
									newCellm.appendChild(Textm);
									newCell2m.appendChild(Text2m);
									newCell3m.appendChild(Text3m);
									newCell4m.appendChild(Text4m);
									newCell5m.appendChild(Text5m);
									newCell6m.appendChild(Text6m);
									newCell7m.appendChild(Text7m);
									newCell8m.appendChild(Text8m);
									newCell9m.appendChild(Text9m);
								}, 
								error: function(rag){
									let node = document.createElement("P");
									let textnode = document.createTextNode("There was an error on the server. Please try again.");
									node.appendChild(textnode);
									document.getElementById("status").appendChild(node);
								}
							});			
						}
					} else {
						tableRefA.innerHTML="";
						let newRow = tableRefA.insertRow(tableRefA.rows.length);
						newRow.id = "JuliaJ1";
						document.getElementById("JuliaJ1").style.fontWeight = "900";
						let newCell  = newRow.insertCell(0);
						let Text = document.createTextNode("No Results");
						newCell.appendChild(Text);					
					}
				}, 
				error: function(tea){
					let node = document.createElement("P");
					let textnode = document.createTextNode("There was an error on the server. Please try again.");
					node.appendChild(textnode);
					document.getElementById("status").appendChild(node);
				}
			});
		} else if (document.getElementById("customQue").checked){
			let g = document.getElementById("CustomQueryText");
			let rockwood = new Object();
			rockwood.statement = g.value;
			$.ajax({
				type:'get',
				url:'/customOne',
				data: rockwood,
				success: function(guitar){
					if (guitar.length == 0){
						tableRefA.innerHTML="";
						let newRow = tableRefA.insertRow(tableRefA.rows.length);
						newRow.id = "JuliaJ1";
						document.getElementById("JuliaJ1").style.fontWeight = "900";
						let newCell  = newRow.insertCell(0);
						let Text = document.createTextNode("No Results");
						newCell.appendChild(Text);
					} else {
						tableRefA.innerHTML="";
						for (let i = 0; i<guitar.length; i++){
							let jsonPretty = JSON.stringify(JSON.parse(guitar[i]),null,2);
							jsonPretty = jsonPretty.replace('}', '');
							jsonPretty = jsonPretty.replace('{', '');
							jsonPretty = jsonPretty.replace(/"/g , '');
							jsonPretty = jsonPretty.replace(/,/g , '  |  ');
							let newRow = tableRefA.insertRow(tableRefA.rows.length);
							let newCell  = newRow.insertCell(0);
							let Text = document.createTextNode(jsonPretty);
							newCell.appendChild(Text);
						}					
					}
				}, 
				error: function(ukelele){
					tableRefA.innerHTML="";
					let newRow = tableRefA.insertRow(tableRefA.rows.length);
					newRow.id = "JuliaJ1";
					document.getElementById("JuliaJ1").style.fontWeight = "900";
					let newCell  = newRow.insertCell(0);
					let Text = document.createTextNode("No Results");
					newCell.appendChild(Text);
				}
			});
		}
	});	
}

function displayDB(){
		let numFile;
		var numIndi;
		$.ajax({
			type: 'get',
			url: 'countTables',
			obj: null,
			success: function(data){
				numFile = data.file;
				$.ajax({
					type: 'get',
					url: 'countTablesToo',
					obj: null,
					success: function(theData){
						numIndi = theData.file;
						let node2 = document.createElement("P");
						let x = document.getElementById("username");
						let textnode2 = document.createTextNode(x.value + " has " + numFile + " file(s) and " + numIndi + " Individuals.");
						node2.appendChild(textnode2);
						document.getElementById("status").appendChild(node2);
					},
					error: function(err){
						let node = document.createElement("P");
						let textnode = document.createTextNode("There was an error on the server. Please try again.");
						node.appendChild(textnode);
						document.getElementById("status").appendChild(node);
					}
				});
			}, 
			error: function(error){
				let node = document.createElement("P");
				let textnode = document.createTextNode("There was an error on the server. Please try again.");
				node.appendChild(textnode);
				document.getElementById("status").appendChild(node);
			}
		});		
}

function clearAll(){
	$("#clearAll").on("click", function(e){
		$.ajax({
			type: 'get',
			url: 'clearTables',
			obj: null,
			success: function(data){
				let x = document.getElementById("username");
				let node = document.createElement("P");
				let textnode = document.createTextNode("FILE and INDIVIDUAL tables were cleared from " + x.value);
				node.appendChild(textnode);
				document.getElementById("status").appendChild(node);
				displayDB();
				$.ajax({
						type: 'get',
						url: 'countTables',
						obj: null,
						success: function(data){
							if (data.file != 0){
								document.getElementById("clearAll").disabled = false;
							} else {
								document.getElementById("clearAll").disabled = true;
							}
						},
						error: function(error){
							let node = document.createElement("P");
							let textnode = document.createTextNode("There was an error on the server. Please try again.");
							node.appendChild(textnode);
							document.getElementById("status").appendChild(node);
						}
					});
			},
			error: function(error){
				let node = document.createElement("P");
				let textnode = document.createTextNode("There was an error on the server. Please try again.");
				node.appendChild(textnode);
				document.getElementById("status").appendChild(node);
			}
		});
		
	});	
}


function storeFiles(){
	$("#store").on("click", function(e){
		let flag = 0;
			$.ajax({
				type: 'get',
				url: 'clearTables',
				obj: null,
				success: function(data){
					for (let i = 0; i < fileInfo.length; i++){
						let theObject = new Object();
						theObject = JSON.parse(fileInfo[i]);
						$.ajax({
							type: 'get',
							data: theObject, 
							url: '/store',
							success: function(data){
								let toSent = new Object();
								toSent.fileName = theObject.fileName;
								$.ajax({
									type: 'get',
									url: '/getFileID',
									data: toSent,
									success: function(theID){
										let fileId = theID.file;
										let anObject = new Object();
										anObject.fileName = "uploads/" + toSent.fileName;
										$.ajax({
											type: 'get',
											dataType: 'json',
											data: anObject,
											url: '/indFunction',
											success: function(miriamData) {
												if (miriamData != "[]"){
													if (miriamData.length != 0){
														for (let i=0; i<miriamData.length; i++){
															let indiObj = new Object();
															indiObj.givenName = miriamData[i].givenName;
															indiObj.surname = miriamData[i].surname;
															indiObj.sex = miriamData[i].sex;
															indiObj.familySize = miriamData[i].familySize;
															indiObj.fileId = fileId;
															$.ajax({
																type: 'get',
																data: indiObj,
																url: '/storeIndis',
																success: function(mriiam){
																	if (flag != 1){
																		let x = document.getElementById("username");
																		let node = document.createElement("P");
																		let textnode = document.createTextNode("All files and individuals from " + x.value + " were stored");
																		node.appendChild(textnode);
																		document.getElementById("status").appendChild(node);
																		displayDB();
																		flag = 1;
																	}
																	$.ajax({
																		type: 'get',
																		url: 'countTables',
																		obj: null,
																		success: function(data){
																			if (data.file != 0){
																				document.getElementById("clearAll").disabled = false;
																			} else {
																				document.getElementById("clearAll").disabled = true;
																			}
																		},
																		error: function(error){
																			let node = document.createElement("P");
																			let textnode = document.createTextNode("There was an error on the server. Please try again.");
																			node.appendChild(textnode);
																			document.getElementById("status").appendChild(node);
																		}
																	});
																},
																error: function(mirError){
																	let node = document.createElement("P");
																	let textnode = document.createTextNode("There was an error on the server. Please try again.");
																	node.appendChild(textnode);
																	document.getElementById("status").appendChild(node);
																}
															});
														}
													}
												}
											},
											error: function(error){
												let node = document.createElement("P");
												let textnode = document.createTextNode("There was an error on the server. Please try again.");
												node.appendChild(textnode);
												document.getElementById("status").appendChild(node);
											}
										});								
									}, 
									error: function(theErr){
										let node = document.createElement("P");
										let textnode = document.createTextNode("There was an error on the server. Please try again.");
										node.appendChild(textnode);
										document.getElementById("status").appendChild(node);
									}
								});											
							},
							error: function(error){
								let node = document.createElement("P");
								let textnode = document.createTextNode("There was an error on the server. Please try again.");
								node.appendChild(textnode);
								document.getElementById("status").appendChild(node);
							}
						});
					}
				},
				error: function(error){
					let node = document.createElement("P");
					let textnode = document.createTextNode("There was an error on the server. Please try again.");
					node.appendChild(textnode);
					document.getElementById("status").appendChild(node);
				}
			});
	});
}

function connect(){
	$("#connectButt").on("click", function(e){
		let theName = document.getElementById("dbname");
		let thePass = document.getElementById("password");
		let theUser = document.getElementById("username");
		let obj = new Object();
		obj.dbname = theName.value;
		obj.password = thePass.value;
		obj.username = theUser.value;
		
		$.ajax({
			type: 'get',
			data: obj,
			url: '/connect',
			success: function(data){
				if (data == "ERROR"){
					let node2 = document.createElement("P");
					let textnode2 = document.createTextNode("Could not connect to " + theName.value + ". Please try again");
					node2.appendChild(textnode2);
					document.getElementById("status").appendChild(node2);
					document.getElementById("store").disabled = true;
					document.getElementById("clearAll").disabled = true;
					document.getElementById("displayDB").disabled = true;
					document.getElementById("subQueButt").disabled = true;
				} else if (data == "FINE"){
					$.ajax({
						type: 'get',
						url: 'countTables',
						obj: null,
						success: function(data){
							if (data.file != 0){
								document.getElementById("clearAll").disabled = false;
							} else {
								document.getElementById("clearAll").disabled = true;
							}
						},
						error: function(error){
							let node = document.createElement("P");
							let textnode = document.createTextNode("There was an error on the server. Please try again.");
							node.appendChild(textnode);
							document.getElementById("status").appendChild(node);
						}
					});
					document.getElementById("displayDB").disabled = false;
					document.getElementById("subQueButt").disabled = false;
					let node2 = document.createElement("P");
					let textnode2 = document.createTextNode("Successfully conntected to " + theName.value + "!");
					node2.appendChild(textnode2);
					document.getElementById("status").appendChild(node2);
					displayDB();
					$.ajax({
						type: 'get',         
						url: '/uploadFile',  
						dataType: 'json',
						success: function (data) {
							if (data.length != 0){
								document.getElementById("store").disabled = false;
							}
						},
						error: function(error){
							let node = document.createElement("P");
							let textnode = document.createTextNode("There was an error on the server. Please try again.");
							node.appendChild(textnode);
							document.getElementById("status").appendChild(node);
						}
					});
				}
			}, 
			error: function(err){
				let node = document.createElement("P");
				let textnode = document.createTextNode("There was an error on the server. Please try again.");
				node.appendChild(textnode);
				document.getElementById("status").appendChild(node);
			}
		});
	});
	
}

function getAnsSubmit(){
	$("#getAnsSub").on("click", function(e){
		let x = document.getElementById("fileSelectFour");
		let y = document.getElementById("ansMax");
		let z = document.getElementById("ansFirstName");
		let w = document.getElementById("ansLastName");
		let obj = new Object();
		obj.givenName = z.value;
		obj.surname = w.value;
		let JSONstring = JSON.stringify(obj);
		let obj2 = new Object();
		if (y.value == ""){
			obj2.max = 0;
		} else {
			obj2.max = y.value;
		}
		obj2.theString = JSONstring;
		obj2.fileName = "uploads/"+x.value;
		$.ajax({
			type: 'get',
			data: obj2,
			url: '/getAns',
			success: function(data){
				let obj = JSON.parse(data);
				let aGen;
				let tableRef = document.getElementById('myAnsTable').getElementsByTagName('thead')[0];
				let tableRef2 = document.getElementById('myAnsTable').getElementsByTagName('tbody')[0];
				tableRef.innerHTML = "";
				tableRef2.innerHTML = "";
				
				if (obj.length != 0){
					let newRow = tableRef.insertRow(tableRef.rows.length);
					newRow.id = "miriamSnow2";
					document.getElementById("miriamSnow2").style.fontWeight = "900";
					let newCell  = newRow.insertCell(0);
					let newCell2  = newRow.insertCell(1);
					let Text = document.createTextNode("Generation #");				
					let Text2 = document.createTextNode("Names of Family Members");							
					newCell.appendChild(Text);
					newCell2.appendChild(Text2);
					for (let i = 0; i<obj.length; i++){
						aGen = "";
						for (let j=0;j<obj[i].length; j++){
							if (j==obj[i].length-1){
								aGen = aGen + obj[i][j].givenName + " " + obj[i][j].surname;
							} else {
								aGen = aGen + obj[i][j].givenName + " " + obj[i][j].surname + ", ";
							}
						}
						let newRowm = tableRef2.insertRow(tableRef2.rows.length);
						let newCellm  = newRowm.insertCell(0);
						let newCell2m  = newRowm.insertCell(1);
						let Textm = document.createTextNode(i+1);				
						let Text2m = document.createTextNode(aGen);							
						newCellm.appendChild(Textm);
						newCell2m.appendChild(Text2m);
					}
				} else {
					let newRow = tableRef.insertRow(tableRef.rows.length);
					let newCell  = newRow.insertCell(0);
					let Text = document.createTextNode("No Ancestors");										
					newCell.appendChild(Text);
				}
			},
			error : function(error){
				let node = document.createElement("P");
				let textnode = document.createTextNode("There was an error on the server. Please try again.");
				node.appendChild(textnode);
				document.getElementById("status").appendChild(node);
			}
		});
	});	
	
}

function getDesSubmit(){
	$("#getDesSub").on("click", function(e){
		let x = document.getElementById("fileSelectThree");
		let y = document.getElementById("desMax");
		let z = document.getElementById("desFirstName");
		let w = document.getElementById("desLastName");
		let obj = new Object();
		obj.givenName = z.value;
		obj.surname = w.value;
		let JSONstring = JSON.stringify(obj);
		let obj2 = new Object();
		if (y.value == ""){
			obj2.max = 0;
		} else {
			obj2.max = y.value;
		}
		obj2.theString = JSONstring;
		obj2.fileName = "uploads/"+x.value;
		$.ajax({
			type: 'get',
			data: obj2,
			url: '/getDes',
			success: function(data){
				let obj = JSON.parse(data);
				let aGen;
				let tableRef = document.getElementById('myDesTable').getElementsByTagName('thead')[0];
				let tableRef2 = document.getElementById('myDesTable').getElementsByTagName('tbody')[0];
				tableRef.innerHTML = "";
				tableRef2.innerHTML = "";
				
				if (obj.length != 0){
					let newRow = tableRef.insertRow(tableRef.rows.length);
					newRow.id = "miriamSnow1";
					document.getElementById("miriamSnow1").style.fontWeight = "900";
					let newCell  = newRow.insertCell(0);
					let newCell2  = newRow.insertCell(1);
					let Text = document.createTextNode("Generation #");				
					let Text2 = document.createTextNode("Names of Family Members");				
					newCell.appendChild(Text);
					newCell2.appendChild(Text2);
					for (let i = 0; i<obj.length; i++){
						aGen = "";
						for (let j=0;j<obj[i].length; j++){
							if (j==obj[i].length-1){
								aGen = aGen + obj[i][j].givenName + " " + obj[i][j].surname;
							} else {
								aGen = aGen + obj[i][j].givenName + " " + obj[i][j].surname + ", ";
							}
						}
						let newRowm = tableRef2.insertRow(tableRef2.rows.length);
						let newCellm  = newRowm.insertCell(0);
						let newCell2m  = newRowm.insertCell(1);
						let Textm = document.createTextNode(i+1);				
						let Text2m = document.createTextNode(aGen);							
						newCellm.appendChild(Textm);
						newCell2m.appendChild(Text2m);
					}
				} else {
					let newRow = tableRef.insertRow(tableRef.rows.length);
					let newCell  = newRow.insertCell(0);
					let Text = document.createTextNode("No Descendents");										
					newCell.appendChild(Text);
				}
			},
			error : function(error){
				let node = document.createElement("P");
				let textnode = document.createTextNode("There was an error on the server. Please try again.");
				node.appendChild(textnode);
				document.getElementById("status").appendChild(node);
			}
		});
	});	
}


function addSubmit(){
	$("#addIndButt").on("click", function(e) {
		let x = document.getElementById("addFirstName");
		let y = document.getElementById("addLastName");
		let z = document.getElementById("fileSelectTwo");
		if (z.value == "" || y.value == "" || x.value == ""){
			let node = document.createElement("P");
			let textnode = document.createTextNode("Could not add individual due to missing field");
			node.appendChild(textnode);
			document.getElementById("status").appendChild(node);
		} else {
			let obj = new Object();
			obj.givenName = x.value;
			obj.surname = y.value;
			let string = JSON.stringify(obj);
			let obj2 = new Object();
			obj2.theString = string;
			obj2.fileName = "uploads/"+z.value;
			$.ajax({
				type: 'get',
				data: obj2,
				url: '/addIndi',
				success: function(data){
					if (data == "OK"){
						loadFiles();
						getIndis();
						let node = document.createElement("P");
						let textnode = document.createTextNode("Successfully added "+x.value+ " "+y.value+" to "+z.value);
						node.appendChild(textnode);
						document.getElementById("status").appendChild(node);
					} else {
						let node = document.createElement("P");
						let textnode = document.createTextNode("Error adding "+x.value+ " "+y.value+" to "+z.value);
						node.appendChild(textnode);
						document.getElementById("status").appendChild(node);
					}
				}, 
				error: function(error){
					let node = document.createElement("P");
					let textnode = document.createTextNode("There was an error on the server. Please try again.");
					node.appendChild(textnode);
					document.getElementById("status").appendChild(node);
				}
			});
			
		}	
	});
	
	
}

function clearStatus(){
	$("#clearStatus").on("click", function(e) {
		let x = document.getElementById("status");
		x.innerHTML="<hr>";
	});
}

function createSubmit(){
	$("#createGButt").on("click", function(e){
		let x = document.getElementById("createFileName");
		let y = document.getElementById("createSubName");
		let z = document.getElementById("createSubAdd");
		if (x.value == ""){
			let node = document.createElement("P");
			let textnode = document.createTextNode("Could not create GEDCOM due to missing file name");
			node.appendChild(textnode);
			document.getElementById("status").appendChild(node);	
		} else if (y.value == ""){
			let node = document.createElement("P");
			let textnode = document.createTextNode("Could not create GEDCOM due to missing submitter name");
			node.appendChild(textnode);
			document.getElementById("status").appendChild(node);
		}
		else if (x.value != "" && y.value != ""){
			let doesExists = false;
			$.ajax({
				type: 'get',         
				url: '/uploadFile',  
				dataType: 'json',
				success: function (data) {
					for (let i=0; i< data.length; i++){
						if (x.value == data[i]){
							doesExists = true;
						}
					}
					
					if (doesExists == true){
						let node2 = document.createElement("P");
						let textnode2 = document.createTextNode("Could not create GEDCOM because a file already exists with that name");
						node2.appendChild(textnode2);
						document.getElementById("status").appendChild(node2);
					} else {
						
						let obj = new Object();
						obj.fileName = "uploads/" + x.value;
						obj.subName = y.value;
						obj.subAdd = z.value;
						$.ajax({
							type: 'get',
							data: obj,
							url: '/createSimple',
							success: function(data) {
								if (data == "OK"){
									let node3 = document.createElement("P");
									let textnode3 = document.createTextNode("Successfully created " + x.value);
									node3.appendChild(textnode3);
									document.getElementById("status").appendChild(node3);
									loadFiles();
								} else {
									if (data == "WERROR"){
										let node3 = document.createElement("P");
										let textnode3 = document.createTextNode("There was an error writing the file " + x.value);
										node3.appendChild(textnode3);
										document.getElementById("status").appendChild(node3);
									} else if (data == "OBJERROR"){
										let node3 = document.createElement("P");
										let textnode3 = document.createTextNode("There was an error creating the GEDCOM object");
										node3.appendChild(textnode3);
										document.getElementById("status").appendChild(node3);
									}
										
								}
							},
							error: function(error){
								let node = document.createElement("P");
								let textnode = document.createTextNode("There was an error on the server. Please try again.");
								node.appendChild(textnode);
								document.getElementById("status").appendChild(node);
							}
						});
											
					}
				},
				error: function(error) {
					let node = document.createElement("P");
					let textnode = document.createTextNode("There was an error on the server. Please try again.");
					node.appendChild(textnode);
					document.getElementById("status").appendChild(node); 
				}
			});
			
			
				
		}
	});	
}

function upFileSubmit(){
	$("#upFileSubmit").on("click", function (e) {	
		let file = $("#uploadFile")[0].files[0];
		let formData = new FormData();
		formData.append("uploadFile", file);
		$.ajax({
		   url : '/upload',
		   type : 'POST',
		   data : formData,
		   processData: false,
		   contentType: false, 
		   success : function() {
			let node = document.createElement("P");
			let textnode = document.createTextNode("The file " + file.name + " was successfully uploaded");
			node.appendChild(textnode);
			document.getElementById("status").appendChild(node);
			loadFiles();
		   },
		   error : function() {
			let node = document.createElement("P");
			let textnode = document.createTextNode("The file " + file.name + " was not uploaded");
			node.appendChild(textnode);
			document.getElementById("status").appendChild(node);
		   }
		});
	});	
}


	

function tableUpdate(theFile, num){
		let obj = new Object();
		let file = "uploads/" + theFile;
		obj.fileName = file;
		$.ajax({
			type: 'get',
			dataType: 'json',
			data: obj,
			url: '/tableFunction',
			success: function(data) {
				if (data != "[]"){
					let tableRef = document.getElementById('myTable').getElementsByTagName('tbody')[0];
					if (data.source != undefined){
						let newObject = new Object();
						let themiriam  = tableRef.rows[num].cells[0].innerHTML;
						let themiriamToo = themiriam.split('"')[1];
						let thefile = themiriamToo.split("/")[1];
						let newCell  = tableRef.rows[num].insertCell(1);
						let newCell2  = tableRef.rows[num].insertCell(2);
						let newCell3  = tableRef.rows[num].insertCell(3);
						let newCell4  = tableRef.rows[num].insertCell(4);
						let newCell5  = tableRef.rows[num].insertCell(5);
						let newCell6  = tableRef.rows[num].insertCell(6);
						let newCell7  = tableRef.rows[num].insertCell(7);
						let Text = document.createTextNode(data.source);				
						let Text2 = document.createTextNode(data.version);				
						let Text3 = document.createTextNode(data.encoding);				
						let Text4 = document.createTextNode(data.name);				
						let Text5 = document.createTextNode(data.address);				
						let Text6 = document.createTextNode(data.indis);				
						let Text7 = document.createTextNode(data.fams);				
						newCell.appendChild(Text);
						newCell2.appendChild(Text2);
						newCell3.appendChild(Text3);
						newCell4.appendChild(Text4);
						newCell5.appendChild(Text5);
						newCell6.appendChild(Text6);
						newCell7.appendChild(Text7);
						newObject.fileName = thefile;
						newObject.source = data.source;
						newObject.version = data.version;
						newObject.encoding = data.encoding;
						newObject.name = data.name;
						newObject.address = data.address;
						newObject.indis = data.indis;
						newObject.fams = data.fams;
						let newObString = JSON.stringify(newObject);
						fileInfo.push(newObString);
					} 
				}				
			},
			error: function(error){
				let node = document.createElement("P");
				let textnode = document.createTextNode("There was an error on the server. Please try again.");
				node.appendChild(textnode);
				document.getElementById("status").appendChild(node);
			}
		});
}

function getIndis(){
	let x = document.getElementById("fileSelectOne");
	let Text = document.createTextNode("");
	let tableRef = document.getElementById('myTableTwo').getElementsByTagName('tbody')[0];
	if (x.value == "" || x.value == "none"){
		tableRef.innerHTML = "";
		Text = document.createTextNode("Please select a file");
		Text2 = document.createTextNode("");
		let newRow = tableRef.insertRow(tableRef.rows.length);
		let newCell0 = newRow.insertCell(0);						
		let newCell1 = newRow.insertCell(1);						
		let newCell2 = newRow.insertCell(2);						
		let newCell3 = newRow.insertCell(3);						
		newCell0.appendChild(Text);
		newCell2.appendChild(Text2);
		newCell3.appendChild(Text2);
		newCell1.appendChild(Text2);
	} else {
		tableRef.innerHTML = "";
		let obj = new Object();
		let file = "uploads/" + x.value;
		obj.fileName = file;
		$.ajax({
			type: 'get',
			dataType: 'json',
			data: obj,
			url: '/indFunction',
			success: function(data) {
				if (data != "[]"){
					if (data.length == 0){
						tableRef.innerHTML = "";
						Text = document.createTextNode("No Individuals");
						Text2 = document.createTextNode("");
						let newRow = tableRef.insertRow(tableRef.rows.length);
						let newCell0 = newRow.insertCell(0);						
						let newCell1 = newRow.insertCell(1);						
						let newCell2 = newRow.insertCell(2);						
						let newCell3 = newRow.insertCell(3);						
						newCell0.appendChild(Text);
						newCell2.appendChild(Text2);
						newCell3.appendChild(Text2);
						newCell1.appendChild(Text2);
					} else {
						for (let i=0; i<data.length; i++){
								Text = document.createTextNode(data[i].givenName);
								Text1 = document.createTextNode(data[i].surname);
								Text2 = document.createTextNode(data[i].sex);
								Text3 = document.createTextNode(data[i].familySize);
								let newRow = tableRef.insertRow(tableRef.rows.length);
								let newCell0 = newRow.insertCell(0);						
								let newCell1 = newRow.insertCell(1);						
								let newCell2 = newRow.insertCell(2);						
								let newCell3 = newRow.insertCell(3);						
								newCell0.appendChild(Text);
								newCell1.appendChild(Text1);
								newCell2.appendChild(Text2);
								newCell3.appendChild(Text3);
						}
					}
				}
			},
			error: function(error){
				let node = document.createElement("P");
				let textnode = document.createTextNode("There was an error on the server. Please try again.");
				node.appendChild(textnode);
				document.getElementById("status").appendChild(node);
			}
		});
	}
}


function loadFiles(){
	$.ajax({
        type: 'get',         
        url: '/uploadFile',  
        dataType: 'json',
        success: function (data) {
			let tableRef = document.getElementById('myTable').getElementsByTagName('tbody')[0];
			tableRef.innerHTML = "";
			let x = document.getElementById("fileSelectOne");
			let p = document.getElementById("fileSelectTwo");
			let b = document.getElementById("fileSelectThree");
			let c = document.getElementById("fileSelectFour");
			let option = document.createElement("option");
			option.text = "Choose a File";
			option.value = "none";
			x.innerHTML="";
			x.add(option);
			b.innerHTML="";
			p.innerHTML="";
			c.innerHTML="";
			if (data.length == 0){
				tableRef.innerHTML = "";
				Text = document.createTextNode("No Files");
				Text2 = document.createTextNode("");
				let newRow = tableRef.insertRow(tableRef.rows.length);
				let newCell0 = newRow.insertCell(0);						
				let newCell1 = newRow.insertCell(1);						
				let newCell2 = newRow.insertCell(2);						
				let newCell3 = newRow.insertCell(3);						
				let newCell4 = newRow.insertCell(4);						
				let newCell5 = newRow.insertCell(5);						
				let newCell6 = newRow.insertCell(6);						
				let newCell7 = newRow.insertCell(7);						
				newCell0.appendChild(Text);
				newCell1.appendChild(Text2);
				newCell2.appendChild(Text2);
				newCell3.appendChild(Text2);
				newCell4.appendChild(Text2);
				newCell5.appendChild(Text2);
				newCell6.appendChild(Text2);
				newCell7.appendChild(Text2);
				document.getElementById("store").disabled = true;
			} else {
				let files = data;
				for (let i=0; i<files.length; i++){
					let newRow = tableRef.insertRow(tableRef.rows.length);
					newRow.id = files[i];
					for (let op=0; op<fileInfo.length; op++){
						fileInfo.pop();
					}
						tableUpdate(files[i], i);
						let newCell  = newRow.insertCell(0);
						let a = document.createElement('a');
						let linkText = document.createTextNode(files[i]);
						a.appendChild(linkText);
						a.href = 'uploads/' + files[i];
						a.id = files[i] + "file";				
						newCell.appendChild(a);
						let option1 = document.createElement("option");
						let option2 = document.createElement("option");
						let option4 = document.createElement("option");
						let option3 = document.createElement("option");
						option4.text = files[i];
						option4.id = files[i] + "4";
						option3.text = files[i];
						option3.id = files[i] + "3";
						option1.text = files[i];
						option1.id = files[i] + "1";
						option2.text = files[i];
						option2.id = files[i] + "2";
						x.add(option1);
						p.add(option2);
						b.add(option3);
						c.add(option4);					
					let objectNew = new Object();
					objectNew.fileName = "uploads/" + files[i];
					$.ajax({
					type: 'get',
					url: '/valFile',
					data: objectNew,
					success: function(data){
						if (data != "OK"){
							document.getElementById(files[i]).style.display = "none";
							document.getElementById(files[i]+"1").remove();
							document.getElementById(files[i]+"2").remove();
							document.getElementById(files[i]+"3").remove();
							document.getElementById(files[i]+"4").remove();
						}
					},
					error: function(error){
						let node = document.createElement("P");
						let textnode = document.createTextNode("There was an error on the server. Please try again.");
						node.appendChild(textnode);
						document.getElementById("status").appendChild(node);
					}
				});
					
					
				}
				updateStatus(data);
				let theName2 = document.getElementById("dbname");
				let thePass2 = document.getElementById("password");
				let theUser2 = document.getElementById("username");
				let miriam = new Object();
				miriam.dbname = theName2.value;
				miriam.password = thePass2.value;
				miriam.username = theUser2.value; 
				$.ajax({
					type: 'get',
					data: miriam,
					url: '/connect',
					success: function(data){
						if (data == "ERROR"){
							document.getElementById("store").disabled = true;
							document.getElementById("clearAll").disabled = true;
							document.getElementById("displayDB").disabled = true;
							document.getElementById("subQueButt").disabled = true;
						} else if (data == "FINE"){
							document.getElementById("store").disabled = false;
							$.ajax({
								type: 'get',
								url: 'countTables',
								obj: null,
								success: function(data){
									if (data.file != 0){
										document.getElementById("clearAll").disabled = false;
									} else {
										document.getElementById("clearAll").disabled = true;
									}
								},
								error: function(error){
									let node = document.createElement("P");
									let textnode = document.createTextNode("There was an error on the server. Please try again.");
									node.appendChild(textnode);
									document.getElementById("status").appendChild(node);
								}
							});
							document.getElementById("displayDB").disabled = false;
							document.getElementById("subQueButt").disabled = false;
						}
					}, 
					error: function(err){
						let node = document.createElement("P");
						let textnode = document.createTextNode("There was an error on the server. Please try again.");
						node.appendChild(textnode);
						document.getElementById("status").appendChild(node);
					}
				});
			}     
        },
        error: function(error) {
            let node = document.createElement("P");
			let textnode = document.createTextNode("There was an error on the server. Please try again.");
			node.appendChild(textnode);
			document.getElementById("status").appendChild(node); 
        }
    });
}
	
function updateStatus(data){
	for (let i=0; i<data.length; i++){
		document.getElementById(data[i]+"file").onclick = function() {
			let node = document.createElement("P");
			let textnode = document.createTextNode("The file " + data[i] + " was downloaded");
			node.appendChild(textnode);
			document.getElementById("status").appendChild(node);	
		};
	}
}
	

function myUpload(){
	let x = document.getElementById("upload");
	let y = document.getElementById("create");
	let z = document.getElementById("add");
	let j = document.getElementById("descendents");
	let k = document.getElementById("ancestors");
		if (x.style.display == "none") {
			x.style.display = "block";
			y.style.display = "none";
			z.style.display = "none";
			j.style.display = "none";
			k.style.display = "none";
		}
}

function myCreate(){
	let x = document.getElementById("upload");
	let y = document.getElementById("create");
	let z = document.getElementById("add");
	let j = document.getElementById("descendents");
	let k = document.getElementById("ancestors");
		if (y.style.display == "none") {
			y.style.display = "block";
			x.style.display = "none";
			z.style.display = "none";
			j.style.display = "none";
			k.style.display = "none";
		}
}

function myAdd(){
	let x = document.getElementById("upload");
	let y = document.getElementById("create");
	let z = document.getElementById("add");
	let j = document.getElementById("descendents");
	let k = document.getElementById("ancestors");
		if (z.style.display == "none") {
			z.style.display = "block";
			x.style.display = "none";
			y.style.display = "none";
			j.style.display = "none";
			k.style.display = "none";
		}
}

function myDescendents(){
	let x = document.getElementById("upload");
	let y = document.getElementById("create");
	let z = document.getElementById("add");
	let j = document.getElementById("descendents");
	let k = document.getElementById("ancestors");
		if (j.style.display == "none") {
			j.style.display = "block";
			x.style.display = "none";
			z.style.display = "none";
			y.style.display = "none";
			k.style.display = "none";
		}
}

function myAncestors(){
	let x = document.getElementById("upload");
	let y = document.getElementById("create");
	let z = document.getElementById("add");
	let j = document.getElementById("descendents");
	let k = document.getElementById("ancestors");
		if (k.style.display == "none") {
			k.style.display = "block";
			x.style.display = "none";
			z.style.display = "none";
			j.style.display = "none";
			y.style.display = "none";
		}
}
