(function(i){
	var res = i.split(" ");
	//Check requested scale
	if(res[1] == "ACP"){
		if (I_AC_Power_SF_.state == "-1") {
		var nconv = (res[0] / 10);
		var scale = " W";
		}
		if (I_AC_Power_SF_.state == "-2") {
		var nconv = (res[0] / 100);
		var scale = " W";
		}
	} else if(res[1] == "100VA"){
		var nconv = (res[0] / 100);
		var scale = " VA";
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