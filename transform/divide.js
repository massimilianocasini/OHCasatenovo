(function(i){
	var res = i.split(" ");
	//Check requested scale
	if(res[1] == "10A"){
		var nconv = (res[0] / 10);
		var scale = " A";
	} else if(res[1] == "100VA"){
		var nconv = (res[0] / 100);
		var scale = " VA";
	} else if (res[1] == "10W") {
		var nconv = (res[0] / 10);		
		var scale = " W";
	} else if (res[1] == "100W") {
		var nconv = (res[0] / 100);		
		var scale = " W";
	} else if (res[1] == "1000W") {
		var nconv = (res[0] / 1000);		
		var scale = " W";
	} else if (res[1] == "100H") {
		var nconv = (res[0] / 100);		
		var scale = " Hz";
	} else if (res[1] == "1000H") {
		var nconv = (res[0] / 1000);		
		var scale = " Hz";
	} else if (res[1] == "100P") {
		var nconv = (res[0] / 100);		
		var scale = " %";
	} else if (res[1] == "10V") {
		var nconv = (res[0] / 10);		
		var scale = " V";
	} else if (res[1] == "100C") {
		var nconv = (res[0] / 100);		
		var scale = " C°";
	} else {
		//No scale passed
		var nconv = 1000;
		var scale = " ERROR";
	}

	//var nconv = (i / 10);
	//var scale = " A"
	//return (i / 10);
	//Return result with one decimal accuracy
	return +nconv.toFixed(1) + scale;
})(input)